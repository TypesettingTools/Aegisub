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

/// @file audio_player_openal.cpp
/// @brief OpenAL-based audio output
/// @ingroup audio_output
///

#ifdef WITH_OPENAL
#include "include/aegisub/audio_player.h"

#include "utils.h"

#include <libaegisub/audio/provider.h>
#include <libaegisub/log.h>

#ifdef __WINDOWS__
#include <al.h>
#include <alc.h>
#elif defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <CoreAudio/CoreAudio.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <algorithm>
#include <chrono>
#include <vector>
#include <wx/timer.h>

// Auto-link to OpenAL lib for MSVC
#ifdef _MSC_VER
#pragma comment(lib, "openal32.lib")
#endif

namespace {
#ifdef __APPLE__
// Apple's OpenAL framework lacks AL_SOFT_source_latency. Estimate the delay
// between its mixer and the device output using Core Audio properties.
double GetOutputLatency()
{
	auto get = [](AudioObjectID object, AudioObjectPropertySelector selector,
	              AudioObjectPropertyScope scope, auto &value) {
		AudioObjectPropertyAddress address{selector, scope, kAudioObjectPropertyElementMain};
		UInt32 size = sizeof(value);
		return AudioObjectGetPropertyData(object, &address, 0, nullptr, &size, &value) == noErr;
	};

	AudioDeviceID output = kAudioObjectUnknown;
	if (!get(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice,
	         kAudioObjectPropertyScopeGlobal, output)) return 0;

	// Core Audio reports delay in device frames, which may run at a different
	// rate from the OpenAL source.
	Float64 rate = 0;
	if (!get(output, kAudioDevicePropertyNominalSampleRate,
	         kAudioObjectPropertyScopeGlobal, rate) || rate <= 0) return 0;

	// Add the device buffer and safety offset to the reported device latency.
	uint64_t frames = 0;
	const AudioObjectPropertySelector selectors[] = {kAudioDevicePropertyLatency,
		kAudioDevicePropertySafetyOffset, kAudioDevicePropertyBufferFrameSize};
	for (auto selector : selectors) {
		UInt32 value = 0;
		if (get(output, selector, kAudioDevicePropertyScopeOutput, value)) frames += value;
	}

	// Streams on the same device run in parallel. Use the largest reported
	// stream latency as a conservative estimate.
	AudioObjectPropertyAddress address{kAudioDevicePropertyStreams,
		kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMain};
	UInt32 size = 0;

	if (AudioObjectGetPropertyDataSize(output, &address, 0, nullptr, &size) == noErr && size) {
		std::vector<AudioStreamID> streams(size / sizeof(AudioStreamID));

		if (AudioObjectGetPropertyData(output, &address, 0, nullptr, &size, streams.data()) == noErr) {
			UInt32 latency = 0;
			for (auto stream : streams) {
				UInt32 value = 0;
				if (get(stream, kAudioStreamPropertyLatency, kAudioObjectPropertyScopeGlobal, value))
					latency = std::max(latency, value);
			}
			frames += latency;
		}
	}

	return frames / rate;
}
#endif

class OpenALPlayer final : public AudioPlayer, wxTimer {
	/// Number of OpenAL buffers to use
	static const ALsizei num_buffers = 8;

	bool playing = false; ///< Is audio currently playing?

	float volume = 1.f; ///< Current audio volume
	ALsizei samplerate; ///< Sample rate of the audio
	int bpf; ///< Bytes per frame

	int64_t start_frame = 0; ///< First frame of playback
	int64_t cur_frame = 0; ///< Next frame to write to playback buffers
	int64_t end_frame = 0; ///< Last frame to play

	ALCdevice *device = nullptr; ///< OpenAL device handle
	ALCcontext *context = nullptr; ///< OpenAL sound context
	ALuint buffers[num_buffers]; ///< OpenAL sound buffers
	ALuint source = 0; ///< OpenAL playback source

	/// Index into buffers, first free (unqueued) buffer to be filled
	ALsizei buf_first_free = 0;

	/// Index into buffers, first queued (non-free) buffer
	ALsizei buf_first_queued = 0;

	/// Number of free buffers
	ALsizei buffers_free = 0;

	/// Number of buffers which have been fully played since playback was last started
	int64_t buffers_played = 0;
	int64_t last_position = 0;
	int64_t last_mix_position = 0;
	std::chrono::steady_clock::time_point last_mix_time;

	// Load the optional extension at runtime for OpenAL 1.1 implementations.
	using GetSourcedv = void (AL_APIENTRY *)(ALuint, ALenum, ALdouble *);
	GetSourcedv get_source_latency = nullptr;

#ifdef __APPLE__
	double output_latency = 0;
	std::chrono::steady_clock::time_point next_latency_update;

	double GetCachedOutputLatency();
#endif

	/// Buffer to decode audio into
	std::vector<char> decode_buffer;

	/// Fill count OpenAL buffers
	void FillBuffers(ALsizei count);

	/// wxTimer override to periodically fill available buffers
	void Notify() override;

	void InitContext();
	void TeardownContext();

public:
	OpenALPlayer(agi::AudioProvider *provider);
	~OpenALPlayer();

	void Play(int64_t start,int64_t count) override;
	void Stop() override;
	bool IsPlaying() override { return playing; }

	int64_t GetEndPosition() override { return end_frame; }
	int64_t GetCurrentPosition() override;
	void SetEndPosition(int64_t pos) override;

	void SetVolume(double vol) override { volume = vol; }
};

OpenALPlayer::OpenALPlayer(agi::AudioProvider *provider)
: AudioPlayer(provider)
, samplerate(provider->GetSampleRate())
, bpf(provider->GetChannels() * provider->GetBytesPerSample())
{
	device = alcOpenDevice(nullptr);
	if (!device) throw AudioPlayerOpenError("Failed opening default OpenAL device");

	// Determine buffer length
	decode_buffer.resize((samplerate / num_buffers / 2) * bpf); // half a second, in whole sample frames
}

OpenALPlayer::~OpenALPlayer()
{
	Stop();
	alcCloseDevice(device);
}

void OpenALPlayer::InitContext()
{
	if (context) return;

	try {
		// Create context
		context = alcCreateContext(device, nullptr);
		if (!context) throw AudioPlayerOpenError("Failed creating OpenAL context");
		if (!alcMakeContextCurrent(context)) throw AudioPlayerOpenError("Failed selecting OpenAL context");

		// Clear error code
		alGetError();

		// Generate buffers
		alGenBuffers(num_buffers, buffers);
		if (alGetError() != AL_NO_ERROR) throw AudioPlayerOpenError("Error generating OpenAL buffers");

		// Generate source
		alGenSources(1, &source);
		if (alGetError() != AL_NO_ERROR) {
			alDeleteBuffers(num_buffers, buffers);
			throw AudioPlayerOpenError("Error generating OpenAL source");
		}

		get_source_latency = alIsExtensionPresent("AL_SOFT_source_latency")
			? reinterpret_cast<GetSourcedv>(alGetProcAddress("alGetSourcedvSOFT")) : nullptr;
	}
	catch (...)
	{
		alcDestroyContext(context);
		context = nullptr;
		throw;
	}
}

void OpenALPlayer::TeardownContext()
{
	if (!context) return;
	alcMakeContextCurrent(context);
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
	alDeleteSources(1, &source);
	alDeleteBuffers(num_buffers, buffers);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	context = nullptr;
}

void OpenALPlayer::Play(int64_t start, int64_t count)
{
	InitContext();
	alcMakeContextCurrent(context);
	if (playing) {
		// Quick reset
		playing = false;
		alSourceStop(source);
		alSourcei(source, AL_BUFFER, 0);
	}

	// Set params
	start_frame = start;
	cur_frame = start;
	end_frame = start + count;
	last_position = start;
	playing = true;

#ifdef __APPLE__
	next_latency_update = {};
#endif

	// Prepare buffers
	buffers_free = num_buffers;
	buffers_played = 0;
	buf_first_free = 0;
	buf_first_queued = 0;
	FillBuffers(num_buffers);

	// And go!
	alSourcePlay(source);
	last_mix_position = 0;
	last_mix_time = std::chrono::steady_clock::now();
	wxTimer::Start(20);
}

void OpenALPlayer::Stop()
{
	wxTimer::Stop();
	if (playing) GetCurrentPosition();
	playing = false;
	start_frame = 0;
	cur_frame = 0;
	end_frame = 0;

	TeardownContext();
}

void OpenALPlayer::FillBuffers(ALsizei count)
{
	InitContext();
	// Do the actual filling/queueing
	for (count = std::min(count, buffers_free); count > 0; --count) {
		ALsizei fill_len = mid<ALsizei>(0, decode_buffer.size() / bpf, end_frame - cur_frame);

		if (fill_len > 0)
			// Get fill_len frames of audio
			provider->GetAudioWithVolume(&decode_buffer[0], cur_frame, fill_len, volume);
		if ((size_t)fill_len * bpf < decode_buffer.size())
			// And zerofill the rest
			memset(&decode_buffer[fill_len * bpf], 0, decode_buffer.size() - fill_len * bpf);

		cur_frame += fill_len;

		alBufferData(buffers[buf_first_free], AL_FORMAT_MONO16, &decode_buffer[0], decode_buffer.size(), samplerate);
		alSourceQueueBuffers(source, 1, &buffers[buf_first_free]); // FIXME: collect buffer handles and queue all at once instead of one at a time?
		buf_first_free = (buf_first_free + 1) % num_buffers;
		--buffers_free;
	}
}

void OpenALPlayer::Notify()
{
	if (!playing) return;
	alcMakeContextCurrent(context);

	// Feed silence until the last requested sample reaches the output.
	if (GetCurrentPosition() >= end_frame) {
		Stop();
		return;
	}

	ALsizei newplayed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &newplayed);

	LOG_D("player/audio/openal") << "buffers_played=" << buffers_played << " newplayed=" << newplayed;

	if (newplayed > 0) {
		// Reclaim buffers
		ALuint bufs[num_buffers];
		for (ALsizei i = 0; i < newplayed; ++i) {
			bufs[i] = buffers[buf_first_queued];
			buf_first_queued = (buf_first_queued + 1) % num_buffers;
		}
		alSourceUnqueueBuffers(source, newplayed, bufs);
		buffers_free += newplayed;

		// Update
		buffers_played += newplayed;

		// Fill more buffers
		FillBuffers(newplayed);
	}

	ALint state = AL_INITIAL;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if (state == AL_STOPPED) alSourcePlay(source); // Recover after an underrun.
}

void OpenALPlayer::SetEndPosition(int64_t pos)
{
	end_frame = pos;
}

int64_t OpenALPlayer::GetCurrentPosition()
{
	if (!playing) return last_position;

	alcMakeContextCurrent(context);

	double offset = 0;
	double latency = 0;
	if (get_source_latency) {
		// OpenAL Soft measures latency for the offset returned by this same
		// call. Read both together on each position query.
		ALdouble values[2] = {};
		get_source_latency(source, 0x1201 /* AL_SEC_OFFSET_LATENCY_SOFT */, values);
		offset = values[0] * samplerate;
		latency = values[1];
	}
	else {
#ifdef __APPLE__
		latency = GetCachedOutputLatency();
#endif
		ALint samples = 0;
		alGetSourcei(source, AL_SAMPLE_OFFSET, &samples);
		offset = samples;
	}

	const int64_t frames_per_buffer = decode_buffer.size() / bpf;
	ALint state = AL_INITIAL;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if (state == AL_STOPPED) {
		// OpenAL resets the offset to zero on underrun, but every queued buffer
		// has been consumed. Preserve that position when restarting the source.
		ALint processed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		offset = processed * frames_per_buffer;
	}

	const int64_t mixed = buffers_played * frames_per_buffer + static_cast<int64_t>(offset);
	auto now = std::chrono::steady_clock::now();
	if (state == AL_STOPPED) {
		// The device continues draining after the mixer runs out of buffers.
		// Estimate when that happened from the last observed mixer position.
		double elapsed = std::chrono::duration<double>(now - last_mix_time).count();
		double mixed_time = static_cast<double>(mixed - last_mix_position) / samplerate;
		latency = std::max(0., latency - std::max(0., elapsed - mixed_time));
	}
	else {
		last_mix_position = mixed;
		last_mix_time = now;
	}

	// Hold at the start while the output pipeline fills and avoid backwards
	// motion when the reported latency changes.
	const int64_t position = start_frame + mixed - static_cast<int64_t>(latency * samplerate);
	last_position = std::max(last_position, position);
	return last_position;
}

#ifdef __APPLE__
double OpenALPlayer::GetCachedOutputLatency()
{
	// Core Audio provides a separate device estimate. Cache the property
	// queries and stream list allocation, but refresh during playback to
	// pick up changes such as switching to Bluetooth.
	auto now = std::chrono::steady_clock::now();
	if (now >= next_latency_update) {
		output_latency = GetOutputLatency();
		next_latency_update = now + std::chrono::milliseconds(100);
	}
	return output_latency;
}
#endif
}

std::unique_ptr<AudioPlayer> CreateOpenALPlayer(agi::AudioProvider *provider, wxWindow *)
{
	return std::make_unique<OpenALPlayer>(provider);
}

#endif // WITH_OPENAL
