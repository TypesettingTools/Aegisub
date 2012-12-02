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

#include <vector>

#include <wx/timer.h>

class OpenALPlayer : public AudioPlayer, wxTimer {
	/// Number of OpenAL buffers to use
	static const ALsizei num_buffers = 8;

	bool playing; ///< Is audio currently playing?

	float volume; ///< Current audio volume
	ALsizei samplerate; ///< Sample rate of the audio
	int bpf; ///< Bytes per frame

	int64_t start_frame; ///< First frame of playbacka
	int64_t cur_frame; ///< Next frame to write to playback buffers
	int64_t end_frame; ///< Last frame to play

	ALCdevice *device; ///< OpenAL device handle
	ALCcontext *context; ///< OpenAL sound context
	ALuint buffers[num_buffers]; ///< OpenAL sound buffers
	ALuint source; ///< OpenAL playback source

	/// Index into buffers, first free (unqueued) buffer to be filled
	ALsizei buf_first_free;

	/// Index into buffers, first queued (non-free) buffer
	ALsizei buf_first_queued;

	/// Number of free buffers
	ALsizei buffers_free;

	/// Number of buffers which have been fully played since playback was last started
	ALsizei buffers_played;

	wxStopWatch playback_segment_timer;

	/// Buffer to decode audio into
	std::vector<char> decode_buffer;

	/// Fill count OpenAL buffers
	void FillBuffers(ALsizei count);

protected:
	/// wxTimer override to periodically fill available buffers
	void Notify();

public:
	OpenALPlayer(AudioProvider *provider);
	~OpenALPlayer();

	void Play(int64_t start,int64_t count);
	void Stop();
	bool IsPlaying() { return playing; }

	int64_t GetStartPosition() { return start_frame; }
	int64_t GetEndPosition() { return end_frame; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);
	void SetCurrentPosition(int64_t pos);

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }
};
#endif
