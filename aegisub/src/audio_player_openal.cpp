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


///////////
// Headers
#include <wx/wxprec.h>
#include "audio_player_manager.h"
#include "audio_provider_manager.h"
#include "utils.h"
#include "main.h"
#include "frame_main.h"
#include "audio_player_openal.h"
#include "options.h"

#ifdef __WINDOWS__
#include <al.h>
#include <alc.h>
#elif defined(__APPLE__)
#include <OpenAL/AL.h>
#include <OpenAL/ALC.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif


// Auto-link to OpenAL lib for MSVC
#ifdef _MSC_VER
#pragma comment(lib, "openal32.lib")
#endif


///////////////
// Constructor
OpenALPlayer::OpenALPlayer()
{
	volume = 1.0f;
	open = false;
	playing = false;
	start_frame = cur_frame = end_frame = bpf = 0;
	provider = 0;
}


//////////////
// Destructor
OpenALPlayer::~OpenALPlayer()
{
	CloseStream();
}


///////////////
// Open stream
void OpenALPlayer::OpenStream()
{
	CloseStream();

	// Get provider
	provider = GetProvider();
	bpf = provider->GetChannels() * provider->GetBytesPerSample();

	// Open device
	device = alcOpenDevice(0);
	if (!device) {
		throw _T("Failed opening default OpenAL device");
	}

	// Create context
	context = alcCreateContext(device, 0);
	if (!context) {
		alcCloseDevice(device);
		throw _T("Failed creating OpenAL context");
	}
	if (!alcMakeContextCurrent(context)) {
		alcDestroyContext(context);
		alcCloseDevice(device);
		throw _T("Failed selecting OpenAL context");
	}
	// Clear error code
	alGetError();

	// Generate buffers
	alGenBuffers(num_buffers, buffers);
	if (alGetError() != AL_NO_ERROR) {
		alcDestroyContext(context);
		alcCloseDevice(device);
		throw _T("Error generating OpenAL buffers");
	}

	// Generate source
	alGenSources(1, &source);
	if (alGetError() != AL_NO_ERROR) {
		alDeleteBuffers(num_buffers, buffers);
		alcDestroyContext(context);
		alcCloseDevice(device);
		throw _T("Error generating OpenAL source");
	}

	// Determine buffer length
	samplerate = provider->GetSampleRate();
	buffer_length = samplerate / num_buffers / 2; // buffers for half a second of audio

	// Now ready
	open = true;
}


////////////////
// Close stream
void OpenALPlayer::CloseStream()
{
	if (!open) return;

	Stop();

	alDeleteSources(1, &source);
	alDeleteBuffers(num_buffers, buffers);
	alcDestroyContext(context);
	alcCloseDevice(device);

	// No longer working
	open = false;
}


////////
// Play
void OpenALPlayer::Play(int64_t start,int64_t count)
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

	// Update timer
	if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(15);
}


////////
// Stop
void OpenALPlayer::Stop(bool timerToo)
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

        if (timerToo && displayTimer) {
                displayTimer->Stop();
        }
}


void OpenALPlayer::FillBuffers(ALsizei count)
{
	wxLogDebug(_T("FillBuffers: count=%d, buffers_free=%d"), count, buffers_free);
	if (count > buffers_free) count = buffers_free;
	if (count < 1) count = 1;

	// Get memory to hold sound buffers
	void *data = malloc(buffer_length * bpf);

	// Do the actual filling/queueing
	ALuint bufid = buf_first_free;
	while (count > 0) {
		ALsizei fill_len = buffer_length;
		if (fill_len > (ALsizei)(end_frame - cur_frame)) fill_len = (ALsizei)(end_frame - cur_frame);
		if (fill_len < 0) fill_len = 0;
		wxLogDebug(_T("buffer_length=%d, fill_len=%d, end_frame-cur_frame=%d"), buffer_length, fill_len, end_frame-cur_frame);

		if (fill_len > 0)
			// Get fill_len frames of audio
			provider->GetAudioWithVolume(data, cur_frame, fill_len, volume);
		if (fill_len < buffer_length)
			// And zerofill the rest
			memset((char*)data+(fill_len*bpf), 0, (buffer_length-fill_len)*bpf);

		cur_frame += fill_len;

		alBufferData(buffers[bufid], AL_FORMAT_MONO16, data, buffer_length*bpf, samplerate);
		alSourceQueueBuffers(source, 1, &buffers[bufid]); // FIXME: collect buffer handles and queue all at once instead of one at a time?
		if (++bufid >= num_buffers) bufid = 0;
		count--; buffers_free--;
	}
	buf_first_free = bufid;

	// Free buffer memory again
	free(data);
}


void OpenALPlayer::Notify()
{
	ALsizei newplayed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &newplayed);

	wxLogDebug(_T("OpenAL Player notify: buffers_played=%d, newplayed=%d, playeddiff=%d"), buffers_played, newplayed);

	if (newplayed > 0) {
		// Reclaim buffers
		ALuint *bufs = new ALuint[newplayed];
		ALsizei i = 0;
		while (i < newplayed) {
			bufs[i++] = buffers[buf_first_queued];
			if (++buf_first_queued >= num_buffers) buf_first_queued = 0;
		}
		alSourceUnqueueBuffers(source, newplayed, bufs);
		delete[] bufs;
		buffers_free += newplayed;

		// Update
		buffers_played += newplayed;
		playback_segment_timer.Start();

		// Fill more buffers
		FillBuffers(newplayed);
	}

	wxLogDebug(_T("frames played=%d, num frames=%d"), (buffers_played - num_buffers) * buffer_length, end_frame - start_frame);
	// Check that all of the selected audio plus one full set of buffers has been queued
	if ((buffers_played - num_buffers) * buffer_length > (ALsizei)(end_frame - start_frame)) {
		// Then stop
		Stop(true);
	}
}


bool OpenALPlayer::IsPlaying()
{
	return playing;
}


///////////
// Set end
void OpenALPlayer::SetEndPosition(int64_t pos)
{
	end_frame = pos;
}


////////////////////////
// Set current position
void OpenALPlayer::SetCurrentPosition(int64_t pos)
{
	cur_frame = pos;
}


int64_t OpenALPlayer::GetStartPosition()
{
	return start_frame;
}


int64_t OpenALPlayer::GetEndPosition()
{
	return end_frame;
}


////////////////////////
// Get current position
int64_t OpenALPlayer::GetCurrentPosition()
{
	// FIXME: this should be based on not duration played but actual sample being heard
	// (during video playback, cur_frame might get changed to resync)
	long extra = playback_segment_timer.Time();
	return buffers_played * buffer_length + start_frame + extra * samplerate / 1000;
}


#endif // WITH_OPENAL


