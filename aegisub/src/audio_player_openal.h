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

/// @file audio_player_openal.h
/// @see audio_player_openal.cpp
/// @ingroup audio_output
///

#ifdef WITH_OPENAL
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"

#ifdef __WINDOWS__
#include <al.h>
#include <alc.h>
#elif defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

/// DOCME
/// @class OpenALPlayer
/// @brief DOCME
///
/// DOCME
class OpenALPlayer : public AudioPlayer, wxTimer {
private:

	/// DOCME
	bool open;

	/// DOCME
	volatile bool playing;

	/// DOCME
	volatile float volume;

	/// DOCME
	static const ALsizei num_buffers = 8;

	/// DOCME
	ALsizei buffer_length;

	/// DOCME
	ALsizei samplerate;

	/// DOCME
	volatile unsigned long start_frame; // first frame of playback

	/// DOCME
	volatile unsigned long cur_frame; // last written frame + 1

	/// DOCME
	volatile unsigned long end_frame; // last frame to play

	/// DOCME
	unsigned long bpf; // bytes per frame

	/// DOCME
	AudioProvider *provider;

	/// DOCME
	ALCdevice *device; // device handle

	/// DOCME
	ALCcontext *context; // sound context

	/// DOCME
	ALuint buffers[num_buffers]; // sound buffers

	/// DOCME
	ALuint source; // playback source

	/// DOCME
	ALsizei buf_first_free; // index into buffers, first free (unqueued) buffer

	/// DOCME
	ALsizei buf_first_queued; // index into buffers, first queued (non-free) buffer

	/// DOCME
	ALsizei buffers_free; // number of free buffers

	/// DOCME
	ALsizei buffers_played;

	/// DOCME
	wxStopWatch playback_segment_timer;

	void FillBuffers(ALsizei count);

protected:
	void Notify(); // from wxTimer

public:
	OpenALPlayer();
	~OpenALPlayer();

	void OpenStream();
	void CloseStream();

	void Play(int64_t start,int64_t count);
	void Stop(bool timerToo=true);
	bool IsPlaying();

	int64_t GetStartPosition();
	int64_t GetEndPosition();
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);
	void SetCurrentPosition(int64_t pos);

	/// @brief DOCME
	/// @param vol 
	/// @return 
	///
	void SetVolume(double vol) { volume = vol; }

	/// @brief DOCME
	/// @return 
	///
	double GetVolume() { return volume; }
};
#endif
