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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include <stdio.h>
#include "audio_player.h"
#include "audio_provider.h"
#include "utils.h"
#include "options.h"
#include <pulse/pulseaudio.h>


//////////////
// Prototypes
class PulseAudioPlayer;



//////////////////////
// Pulse Audio player
class PulseAudioPlayer : public AudioPlayer {
private:
	float volume;
	bool open;
	bool is_playing;

	// Audio data info
	unsigned long start_frame;
	unsigned long cur_frame;
	unsigned long end_frame;
	unsigned long bpf; // bytes per frame

	// Used for synchronising with async events
	wxSemaphore context_notify;
	wxSemaphore context_success;
	volatile int context_success_val;
	wxSemaphore stream_notify;
	wxSemaphore stream_success;
	volatile int stream_success_val;

	// PulseAudio data
	pa_threaded_mainloop *mainloop; // pulseaudio mainloop handle
	pa_context *context; // connection context
	volatile pa_context_state_t cstate;
	pa_stream *stream;
	volatile pa_stream_state_t sstate;
	int paerror;

	// Called by PA to notify about contetxt operation completion
	static void pa_context_success(pa_context *c, int success, PulseAudioPlayer *thread);
	// Called by PA to notify about other context-related stuff
	static void pa_context_notify(pa_context *c, PulseAudioPlayer *thread);
	// Called by PA when a stream operation completes
	static void pa_stream_success(pa_stream *p, int success, PulseAudioPlayer *thread);
	// Called by PA to request more data written to stream
	static void pa_stream_write(pa_stream *p, size_t length, PulseAudioPlayer *thread);
	// Called by PA to notify about other stream-related stuff
	static void pa_stream_notify(pa_stream *p, PulseAudioPlayer *thread);

public:
	PulseAudioPlayer();
	~PulseAudioPlayer();

	void OpenStream();
	void CloseStream();

	void Play(__int64 start,__int64 count);
	void Stop(bool timerToo=true);
	bool IsPlaying();

	__int64 GetStartPosition();
	__int64 GetEndPosition();
	__int64 GetCurrentPosition();
	void SetEndPosition(__int64 pos);
	void SetCurrentPosition(__int64 pos);

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }
};



///////////
// Factory
class PulseAudioPlayerFactory : public AudioPlayerFactory {
public:
	AudioPlayer *CreatePlayer() { return new PulseAudioPlayer(); }
	PulseAudioPlayerFactory() : AudioPlayerFactory(_T("pulse")) {}
} registerPulsePlayer;



///////////////
// Constructor
PulseAudioPlayer::PulseAudioPlayer()
{
	volume = 1.0f;
	paerror = 0;
	open = false;
	is_playing = false;
}


//////////////
// Destructor
PulseAudioPlayer::~PulseAudioPlayer()
{
	if (open) CloseStream();
}


///////////////
// Open stream
void PulseAudioPlayer::OpenStream()
{
	printf("Opening PulseAudio stream\n");
	if (open) CloseStream();

	// Get provider
	AudioProvider *provider = GetProvider();

	// Initialise a mainloop
	printf("Initialising threaded main loop\n");
	mainloop = pa_threaded_mainloop_new();
	if (!mainloop) {
		throw _T("Failed to initialise PulseAudio threaded mainloop object");
	}
	printf("Starting main loop\n");
	pa_threaded_mainloop_start(mainloop);

	// Create context
	printf("Creating context\n");
	context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "Aegisub");
	if (!context) {
		pa_threaded_mainloop_free(mainloop);
		throw _T("Failed to create PulseAudio context");
	}
	pa_context_set_state_callback(context, (pa_context_notify_cb_t)pa_context_notify, this);

	// Connect the context
	printf("Connecting context\n");
	pa_context_connect(context, NULL, 0, NULL);
	// Wait for connection
	while (true) {
		context_notify.Wait();
		if (cstate == PA_CONTEXT_READY) {
			break;
		} else if (cstate == PA_CONTEXT_FAILED) {
			// eww
			paerror = pa_context_errno(context);
			pa_context_unref(context);
			pa_threaded_mainloop_stop(mainloop);
			pa_threaded_mainloop_free(mainloop);
			wxString s(pa_strerror(paerror), wxConvUTF8);
			throw s.c_str();
		}
		// otherwise loop once more
	}
	printf("Context connected\n");
	
	// Set up stream
	bpf = provider->GetChannels() * provider->GetBytesPerSample();
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16LE; // FIXME
	ss.rate = provider->GetSampleRate();
	ss.channels = provider->GetChannels();
	pa_channel_map map;
	pa_channel_map_init_auto(&map, ss.channels, PA_CHANNEL_MAP_DEFAULT);
	printf("Creating stream\n");
	stream = pa_stream_new(context, "Sound", &ss, &map);
	if (!stream) {
		// argh!
		pa_context_disconnect(context);
		pa_context_unref(context);
		pa_threaded_mainloop_stop(mainloop);
		pa_threaded_mainloop_free(mainloop);
		throw _T("PulseAudio could not create stream");
	}
	pa_stream_set_state_callback(stream, (pa_stream_notify_cb_t)pa_stream_notify, this);
	pa_stream_set_write_callback(stream, (pa_stream_request_cb_t)pa_stream_write, this);

	// Connext stream
	printf("Connecting playback stream\n");
	paerror = pa_stream_connect_playback(stream, NULL, NULL, PA_STREAM_INTERPOLATE_TIMING|PA_STREAM_NOT_MONOTONOUS|PA_STREAM_AUTO_TIMING_UPDATE, NULL, NULL);
	if (paerror) {
		printf("PulseAudio reported error: %s\n", pa_strerror(paerror));
		wxString s(pa_strerror(paerror), wxConvUTF8);
		throw s.c_str();
	}
	while (true) {
		stream_notify.Wait();
		if (sstate == PA_STREAM_READY) {
			break;
		} else if (sstate == PA_STREAM_FAILED) {
			printf("Stream connection failed for some reason\n");
			throw _T("Something went wrong connecting the stream");
		}
	}
	printf("Connected playback stream, now playing\n\n");

	// Hopefully this marks success
	printf("Finished opening PulseAudio\n\n");
	open = true;
}


////////////////
// Close stream
void PulseAudioPlayer::CloseStream()
{
	if (!open) return;
	printf("Closing PuseAudio\n");

	if (is_playing) Stop();

	// Hope for the best and just do things as quickly as possible
	pa_stream_disconnect(stream);
	pa_stream_unref(stream);
	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_threaded_mainloop_stop(mainloop);
	pa_threaded_mainloop_free(mainloop);

	printf("Closed PulseAudio\n");
	open = false;
}


////////
// Play
void PulseAudioPlayer::Play(__int64 start,__int64 count)
{
	printf("Starting PulseAudio playback\n");
	if (!open) OpenStream();
	if (is_playing) Stop();

	start_frame = start;
	cur_frame = start;
	end_frame = start + count;
	printf("start=%lu end=%lu\n", start_frame, end_frame);

	is_playing = true;

	PulseAudioPlayer::pa_stream_write(stream, pa_stream_writable_size(stream), this);

	pa_operation *op = pa_stream_trigger(stream, (pa_stream_success_cb_t)pa_stream_success, this);
	stream_success.Wait();
	pa_operation_unref(op);
}


////////
// Stop
void PulseAudioPlayer::Stop(bool timerToo)
{
	if (!is_playing) return;
	printf("Stopping PulseAudio\n");

	is_playing = false;

	pa_operation *op = pa_stream_cork(stream, 0, (pa_stream_success_cb_t)pa_stream_success, this);
	stream_success.Wait();
	pa_operation_unref(op);

	start_frame = 0;
	cur_frame = 0;
	end_frame = 0;

	// Flush the stream of data
	printf("Draining stream\n");
	op = pa_stream_flush(stream, (pa_stream_success_cb_t)pa_stream_success, this);
	stream_success.Wait();
	pa_operation_unref(op);

	// Then disconnect it
	/*printf("Disconnecting stream\n");
	paerror = pa_stream_disconnect(stream);
	if (paerror) {
		printf("PulseAudio reported error: %s\n", pa_strerror(paerror));
		wxString s(pa_strerror(paerror), wxConvUTF8);
		throw s.c_str();
	}
	while (true) {
		stream_notify.Wait();
		if (sstate == PA_STREAM_TERMINATED) {
			break;
		} else if (sstate == PA_STREAM_FAILED) {
			printf("Stream stopping failed\n");
			throw _T("Something went wrong disconnecting the stream?!");
		}
	}*/

	// And unref it
	printf("Stopped stream\n\n");
}


bool PulseAudioPlayer::IsPlaying()
{
	return is_playing;
}


///////////
// Set end
void PulseAudioPlayer::SetEndPosition(__int64 pos)
{
	end_frame = pos;
}


////////////////////////
// Set current position
void PulseAudioPlayer::SetCurrentPosition(__int64 pos)
{
	cur_frame = pos;
}


__int64 PulseAudioPlayer::GetStartPosition()
{
	return start_frame;
}


__int64 PulseAudioPlayer::GetEndPosition()
{
	return end_frame;
}


////////////////////////
// Get current position
__int64 PulseAudioPlayer::GetCurrentPosition()
{
	// TODO: use pulse functions
	return cur_frame;
}


// Called by PA to notify about contetxt operation completion
void PulseAudioPlayer::pa_context_success(pa_context *c, int success, PulseAudioPlayer *thread)
{
	thread->context_success_val = success;
	thread->context_success.Post();
}


// Called by PA to notify about other context-related stuff
void PulseAudioPlayer::pa_context_notify(pa_context *c, PulseAudioPlayer *thread)
{
	thread->cstate = pa_context_get_state(thread->context);
	//printf("Context state change: %d\n", thread->cstate);
	thread->context_notify.Post();
}


// Called by PA when an operation completes
void PulseAudioPlayer::pa_stream_success(pa_stream *p, int success, PulseAudioPlayer *thread)
{
	thread->stream_success_val = success;
	thread->stream_success.Post();
}


// Called by PA to request more data (and other things?)
void PulseAudioPlayer::pa_stream_write(pa_stream *p, size_t length, PulseAudioPlayer *thread)
{
	if (!thread->is_playing) return;
	if (thread->cur_frame >= thread->end_frame) {
		thread->is_playing = false;
		printf("PA requested more buffer, but no more to stream\n");
		return;
	}

	printf("PA requested more buffer, %lu bytes\n", (unsigned long)length);
	unsigned long bpf = thread->bpf;
	unsigned long frames = length / thread->bpf;
	unsigned long maxframes = thread->end_frame - thread->cur_frame;
	if (frames > maxframes) frames = maxframes;
	printf("Handing it %lu frames\n", frames);
	void *buf = malloc(frames * bpf);
	thread->provider->GetAudioWithVolume(buf, thread->cur_frame, frames, thread->volume);
	::pa_stream_write(p, buf, frames*bpf, free, 0, PA_SEEK_RELATIVE);
	thread->cur_frame += frames;
}


// Called by PA to notify about other stuff
void PulseAudioPlayer::pa_stream_notify(pa_stream *p, PulseAudioPlayer *thread)
{
	thread->sstate = pa_stream_get_state(thread->stream);
	//printf("Stream state change: %d\n", thread->sstate);
	thread->stream_notify.Post();
}




