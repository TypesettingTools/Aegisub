// Copyright (c) 2007, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file audio_player_pulse.cpp
/// @brief PulseAudio-based audio output
/// @ingroup audio_output
///

#ifdef WITH_LIBPULSE
#include "include/aegisub/audio_player.h"

#include "audio_controller.h"
#include "utils.h"

#include <libaegisub/audio/provider.h>
#include <libaegisub/log.h>

#include <cstdio>
#include <pulse/pulseaudio.h>
#include <wx/thread.h>

namespace {

struct PAThreadedMainloopDeleter {
	void operator()(pa_threaded_mainloop *m) { pa_threaded_mainloop_free(m);}
};

using PAThreadedMainloop = std::unique_ptr<pa_threaded_mainloop, PAThreadedMainloopDeleter>;

// We cannot use std::unique_ptr for these since the mainloop needs to be locked
// during the destructor.
class PAContext {
	pa_context *context = nullptr;
	pa_threaded_mainloop *mainloop = nullptr;
	bool connected = false;

public:
	PAContext() = default;

	PAContext(const PAContext&) = delete;
	PAContext& operator=(const PAContext&) = delete;
	PAContext(PAContext&&) = delete;
	PAContext& operator=(PAContext&&) = delete;

	void reset(pa_context *context, pa_threaded_mainloop *mainloop) {
		this->context = context;
		this->mainloop = mainloop;
	}

	template<typename ...Args>
	[[nodiscard]] int connect(Args&&... args) {
		assert(!connected);
		pa_threaded_mainloop_lock(mainloop);
		int error = pa_context_connect(context, std::forward<Args>(args)...);
		pa_threaded_mainloop_unlock(mainloop);
		if (error >= 0)
			connected = true;

		return error;
	}

	[[nodiscard]] int get_errno() {
		pa_threaded_mainloop_lock(mainloop);
		int result = pa_context_errno(context);
		pa_threaded_mainloop_unlock(mainloop);
		return result;
	}

	pa_context *get() { return context; }

	~PAContext() {
		if (context) {
			pa_threaded_mainloop_lock(mainloop);
			if (connected)
				pa_context_disconnect(context);

			pa_context_unref(context);
			pa_threaded_mainloop_unlock(mainloop);
		}
	}
};

class PAStream {
	pa_stream *stream = nullptr;
	pa_threaded_mainloop *mainloop = nullptr;
	bool connected = false;

public:
	PAStream() = default;

	PAStream(const PAStream&) = delete;
	PAStream& operator=(const PAStream&) = delete;
	PAStream(PAStream&&) = delete;
	PAStream& operator=(PAStream&&) = delete;

	void reset(pa_stream *stream, pa_threaded_mainloop *mainloop) {
		this->stream = stream;
		this->mainloop = mainloop;
	}

	template<typename ...Args>
	[[nodiscard]] int connect(Args&& ...args) {
		assert(!connected);
		pa_threaded_mainloop_lock(mainloop);
		int error = pa_stream_connect_playback(stream, std::forward<Args>(args)...);
		pa_threaded_mainloop_unlock(mainloop);
		if (!error)
			connected = true;

		return error;
	}

	pa_stream *get() { return stream; }

	~PAStream() {
		if (stream) {
			pa_threaded_mainloop_lock(mainloop);
			if (connected)
				pa_stream_disconnect(stream);

			pa_stream_unref(stream);
			pa_threaded_mainloop_unlock(mainloop);
		}
	}
};

class PulseAudioPlayer final : public AudioPlayer {
	float volume = 1.f;
	bool is_playing = false;

	volatile unsigned long start_frame = 0;
	volatile unsigned long cur_frame = 0;
	volatile unsigned long end_frame = 0;

	unsigned long bpf = 0; // bytes per frame

	wxSemaphore context_notify{0, 1};
	wxSemaphore stream_notify{0, 1};
	wxSemaphore stream_success{0, 1};
	volatile int stream_success_val;

	PAThreadedMainloop mainloop; // pulseaudio mainloop handle
	PAContext context; // connection context
	volatile pa_context_state_t cstate;

	PAStream stream;
	volatile pa_stream_state_t sstate;

	volatile pa_usec_t play_start_time; // timestamp when playback was started

	int paerror = 0;

	/// Called by PA to notify about other context-related stuff
	static void pa_context_notify(pa_context *c, PulseAudioPlayer *thread);
	/// Called by PA when a stream operation completes
	static void pa_stream_success(pa_stream *p, int success, PulseAudioPlayer *thread);
	/// Called by PA to request more data written to stream
	static void pa_stream_write(pa_stream *p, size_t length, PulseAudioPlayer *thread);
	/// Called by PA to notify about other stream-related stuff
	static void pa_stream_notify(pa_stream *p, PulseAudioPlayer *thread);

public:
	PulseAudioPlayer(agi::AudioProvider *provider);
	~PulseAudioPlayer();

	void Play(int64_t start,int64_t count);
	void Stop();
	bool IsPlaying() { return is_playing; }

	int64_t GetEndPosition() { return end_frame; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);

	void SetVolume(double vol) { volume = vol; }
};

PulseAudioPlayer::PulseAudioPlayer(agi::AudioProvider *provider) : AudioPlayer(provider) {
	// Initialise a mainloop
	mainloop.reset(pa_threaded_mainloop_new());
	if (!mainloop)
		throw AudioPlayerOpenError("Failed to initialise PulseAudio threaded mainloop object");

	pa_threaded_mainloop_start(mainloop.get());

	// Create context
	context.reset(pa_context_new(pa_threaded_mainloop_get_api(mainloop.get()), "Aegisub"), mainloop.get());
	if (!context.get())
		throw AudioPlayerOpenError("Failed to create PulseAudio context");

	pa_context_set_state_callback(context.get(), (pa_context_notify_cb_t)pa_context_notify, this);

	// Connect the context
	if (context.connect(nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr) < 0)
		throw AudioPlayerOpenError("Failed to connect PulseAudio context");

	// Wait for connection
	while (true) {
		context_notify.Wait();
		if (cstate == PA_CONTEXT_READY) {
			break;
		} else if (cstate == PA_CONTEXT_FAILED) {
			// eww
			paerror = context.get_errno();
			throw AudioPlayerOpenError(std::string("PulseAudio reported error: ") + pa_strerror(paerror));
		}
		// otherwise loop once more
	}

	// Set up stream
	bpf = provider->GetChannels() * provider->GetBytesPerSample();
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16LE; // FIXME
	ss.rate = provider->GetSampleRate();
	ss.channels = provider->GetChannels();
	pa_channel_map map;
	pa_channel_map_init_auto(&map, ss.channels, PA_CHANNEL_MAP_DEFAULT);

	stream.reset(pa_stream_new(context.get(), "Sound", &ss, &map), mainloop.get());
	if (!stream.get()) {
		// argh!
		throw AudioPlayerOpenError("PulseAudio could not create stream");
	}
	pa_stream_set_state_callback(stream.get(), (pa_stream_notify_cb_t)pa_stream_notify, this);
	pa_stream_set_write_callback(stream.get(), (pa_stream_request_cb_t)pa_stream_write, this);

	// Connect stream
	paerror = stream.connect(nullptr, nullptr, (pa_stream_flags_t)(PA_STREAM_INTERPOLATE_TIMING|PA_STREAM_NOT_MONOTONOUS|PA_STREAM_AUTO_TIMING_UPDATE), nullptr, nullptr);
	if (paerror) {
		LOG_E("audio/player/pulse") << "Stream connection failed: " << pa_strerror(paerror) << "(" << paerror << ")";
		throw AudioPlayerOpenError(std::string("PulseAudio reported error: ") + pa_strerror(paerror));
	}
	while (true) {
		stream_notify.Wait();
		if (sstate == PA_STREAM_READY) {
			break;
		} else if (sstate == PA_STREAM_FAILED) {
			paerror = context.get_errno();
			LOG_E("audio/player/pulse") << "Stream connection failed: " << pa_strerror(paerror) << "(" << paerror << ")";
			throw AudioPlayerOpenError("PulseAudio player: Something went wrong connecting the stream");
		}
	}
}

PulseAudioPlayer::~PulseAudioPlayer()
{
	if (is_playing) Stop();
}

void PulseAudioPlayer::Play(int64_t start,int64_t count)
{
	if (is_playing) {
		// If we're already playing, do a quick "reset"
		is_playing = false;

		pa_threaded_mainloop_lock(mainloop.get());
		pa_operation *op = pa_stream_flush(stream.get(), (pa_stream_success_cb_t)pa_stream_success, this);
		pa_threaded_mainloop_unlock(mainloop.get());
		stream_success.Wait();
		pa_operation_unref(op);
		if (!stream_success_val) {
			paerror = context.get_errno();
			LOG_E("audio/player/pulse") << "Error flushing stream: " << pa_strerror(paerror) << "(" << paerror << ")";
		}
	}

	start_frame = start;
	cur_frame = start;
	end_frame = start + count;

	is_playing = true;

	play_start_time = 0;
	pa_threaded_mainloop_lock(mainloop.get());
	paerror = pa_stream_get_time(stream.get(), (pa_usec_t*) &play_start_time);
	pa_threaded_mainloop_unlock(mainloop.get());
	if (paerror)
		LOG_E("audio/player/pulse") << "Error getting stream time: " << pa_strerror(paerror) << "(" << paerror << ")";

	PulseAudioPlayer::pa_stream_write(stream.get(), pa_stream_writable_size(stream.get()), this);

	pa_threaded_mainloop_lock(mainloop.get());
	pa_operation *op = pa_stream_trigger(stream.get(), (pa_stream_success_cb_t)pa_stream_success, this);
	pa_threaded_mainloop_unlock(mainloop.get());
	stream_success.Wait();
	pa_operation_unref(op);
	if (!stream_success_val) {
		paerror = context.get_errno();
		LOG_E("audio/player/pulse") << "Error triggering stream: " << pa_strerror(paerror) << "(" << paerror << ")";
	}
}

void PulseAudioPlayer::Stop()
{
	if (!is_playing) return;

	is_playing = false;

	start_frame = 0;
	cur_frame = 0;
	end_frame = 0;

	// Flush the stream of data
	pa_threaded_mainloop_lock(mainloop.get());
	pa_operation *op = pa_stream_flush(stream.get(), (pa_stream_success_cb_t)pa_stream_success, this);
	pa_threaded_mainloop_unlock(mainloop.get());
	stream_success.Wait();
	pa_operation_unref(op);
	if (!stream_success_val) {
		paerror = context.get_errno();
		LOG_E("audio/player/pulse") << "Error flushing stream: " << pa_strerror(paerror) << "(" << paerror << ")";
	}
}

void PulseAudioPlayer::SetEndPosition(int64_t pos)
{
	end_frame = pos;
}

int64_t PulseAudioPlayer::GetCurrentPosition()
{
	if (!is_playing) return 0;

	// FIXME: this should be based on not duration played but actual sample being heard
	// (during video playback, cur_frame might get changed to resync)

	// Calculation duration we have played, in microseconds
	pa_usec_t play_cur_time;
	pa_stream_get_time(stream.get(), &play_cur_time);
	pa_usec_t playtime = play_cur_time - play_start_time;

	return start_frame + playtime * provider->GetSampleRate() / (1000*1000);
}

/// @brief Called by PA to notify about other context-related stuff
void PulseAudioPlayer::pa_context_notify(pa_context *, PulseAudioPlayer *thread)
{
	thread->cstate = pa_context_get_state(thread->context.get());
	thread->context_notify.Post();
}

/// @brief Called by PA when an operation completes
void PulseAudioPlayer::pa_stream_success(pa_stream *, int success, PulseAudioPlayer *thread)
{
	thread->stream_success_val = success;
	thread->stream_success.Post();
}

/// @brief Called by PA to request more data (and other things?)
void PulseAudioPlayer::pa_stream_write(pa_stream *p, size_t length, PulseAudioPlayer *thread)
{
	if (!thread->is_playing) return;

	if (thread->cur_frame >= thread->end_frame + thread->provider->GetSampleRate()) {
		// More than a second past end of stream
		thread->is_playing = false;
		pa_operation *op = pa_stream_drain(p, nullptr, nullptr);
		pa_operation_unref(op);
		return;

	} else if (thread->cur_frame >= thread->end_frame) {
		// Past end of stream, but not a full second, add some silence
		void *buf = calloc(length, 1);
		::pa_stream_write(p, buf, length, free, 0, PA_SEEK_RELATIVE);
		thread->cur_frame += length / thread->bpf;
		return;
	}

	unsigned long bpf = thread->bpf;
	unsigned long frames = length / thread->bpf;
	unsigned long maxframes = thread->end_frame - thread->cur_frame;
	if (frames > maxframes) frames = maxframes;
	void *buf = malloc(frames * bpf);
	thread->provider->GetAudioWithVolume(buf, thread->cur_frame, frames, thread->volume);
	::pa_stream_write(p, buf, frames*bpf, free, 0, PA_SEEK_RELATIVE);
	thread->cur_frame += frames;
}

/// @brief Called by PA to notify about other stuff
void PulseAudioPlayer::pa_stream_notify(pa_stream *, PulseAudioPlayer *thread)
{
	thread->sstate = pa_stream_get_state(thread->stream.get());
	thread->stream_notify.Post();
}
}

std::unique_ptr<AudioPlayer> CreatePulseAudioPlayer(agi::AudioProvider *provider, wxWindow *) {
	return std::make_unique<PulseAudioPlayer>(provider);
}
#endif // WITH_LIBPULSE
