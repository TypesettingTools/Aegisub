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
//
// $Id$

/// @file audio_player_openal.cpp
/// @brief OpenAL-based audio output
/// @ingroup audio_output
///

#include "config.h"

#ifdef WITH_OPENAL

#include <libaegisub/log.h>

#include "audio_player_openal.h"

#include "audio_controller.h"
#include "utils.h"

// Auto-link to OpenAL lib for MSVC
#ifdef _MSC_VER
#pragma comment(lib, "openal32.lib")
#endif

DEFINE_SIMPLE_EXCEPTION(OpenALException, agi::AudioPlayerOpenError, "audio/open/player/openal")

OpenALPlayer::OpenALPlayer()
: open(false)
, playing(false)
, volume(1.f)
, samplerate(0)
, bpf(0)
, start_frame(0)
, cur_frame(0)
, end_frame(0)
, device(0)
, context(0)
{
}

OpenALPlayer::~OpenALPlayer()
{
	CloseStream();
}

void OpenALPlayer::OpenStream()
{
	CloseStream();

	bpf = provider->GetChannels() * provider->GetBytesPerSample();
	try {
		// Open device
		device = alcOpenDevice(0);
		if (!device) throw OpenALException("Failed opening default OpenAL device", 0);

		// Create context
		context = alcCreateContext(device, 0);
		if (!context) throw OpenALException("Failed creating OpenAL context", 0);
		if (!alcMakeContextCurrent(context)) throw OpenALException("Failed selecting OpenAL context", 0);

		// Clear error code
		alGetError();

		// Generate buffers
		alGenBuffers(num_buffers, buffers);
		if (alGetError() != AL_NO_ERROR) throw OpenALException("Error generating OpenAL buffers", 0);

		// Generate source
		alGenSources(1, &source);
		if (alGetError() != AL_NO_ERROR) {
			alDeleteBuffers(num_buffers, buffers);
			throw OpenALException("Error generating OpenAL source", 0);
		}
	}
	catch (...)
	{
		alcDestroyContext(context);
		alcCloseDevice(device);
		context = 0;
		device = 0;
		throw;
	}

	// Determine buffer length
	samplerate = provider->GetSampleRate();
	decode_buffer.resize(samplerate * bpf / num_buffers / 2); // buffers for half a second of audio

	// Now ready
	open = true;
}

void OpenALPlayer::CloseStream()
{
	if (!open) return;

	Stop();

	alDeleteSources(1, &source);
	alDeleteBuffers(num_buffers, buffers);
	alcDestroyContext(context);
	alcCloseDevice(device);

	context = 0;
	device = 0;

	// No longer working
	open = false;
}

void OpenALPlayer::Play(int64_t start, int64_t count)
{
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
	playing = true;

	// Prepare buffers
	buffers_free = num_buffers;
	buffers_played = 0;
	buf_first_free = 0;
	buf_first_queued = 0;
	FillBuffers(num_buffers);

	// And go!
	alSourcePlay(source);
	wxTimer::Start(100);
	playback_segment_timer.Start();
}

void OpenALPlayer::Stop()
{
	if (!open) return;
	if (!playing) return;

	// Reset data
	wxTimer::Stop();
	playing = false;
	start_frame = 0;
	cur_frame = 0;
	end_frame = 0;

	// Then drop the playback
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
}

void OpenALPlayer::FillBuffers(ALsizei count)
{
	// Do the actual filling/queueing
	for (count = mid(1, count, buffers_free); count > 0; --count) {
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
		playback_segment_timer.Start();

		// Fill more buffers
		FillBuffers(newplayed);
	}

	LOG_D("player/audio/openal") << "frames played=" << (buffers_played - num_buffers) * decode_buffer.size() / bpf << " num frames=" << end_frame - start_frame;
	// Check that all of the selected audio plus one full set of buffers has been queued
	if ((buffers_played - num_buffers) * (int64_t)decode_buffer.size() > (end_frame - start_frame) * bpf) {
		Stop();
	}
}

void OpenALPlayer::SetEndPosition(int64_t pos)
{
	end_frame = pos;
}

void OpenALPlayer::SetCurrentPosition(int64_t pos)
{
	cur_frame = pos;
}

int64_t OpenALPlayer::GetCurrentPosition()
{
	// FIXME: this should be based on not duration played but actual sample being heard
	// (during video playback, cur_frame might get changed to resync)
	long extra = playback_segment_timer.Time();
	return buffers_played * decode_buffer.size() / bpf + start_frame + extra * samplerate / 1000;
}

#endif // WITH_OPENAL
