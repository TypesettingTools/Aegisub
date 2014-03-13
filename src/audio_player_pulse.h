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

/// @file audio_player_pulse.h
/// @see audio_player_pulse.cpp
/// @ingroup audio_output
///

#ifdef WITH_LIBPULSE
#include <pulse/pulseaudio.h>

#include "include/aegisub/audio_player.h"

class PulseAudioPlayer;

class PulseAudioPlayer final : public AudioPlayer {
	float volume = 1.f;
	bool is_playing = false;

	volatile unsigned long start_frame = 0;
	volatile unsigned long cur_frame = 0;
	volatile unsigned long end_frame = 0;

	unsigned long bpf = 0; // bytes per frame


	wxSemaphore context_notify{0, 1};
	wxSemaphore context_success{0, 1};
	volatile int context_success_val;

	wxSemaphore stream_notify{0, 1};
	wxSemaphore stream_success{0, 1};
	volatile int stream_success_val;

	pa_threaded_mainloop *mainloop = nullptr; // pulseaudio mainloop handle
	pa_context *context = nullptr; // connection context
	volatile pa_context_state_t cstate;

	pa_stream *stream = nullptr;
	volatile pa_stream_state_t sstate;

	volatile pa_usec_t play_start_time; // timestamp when playback was started

	int paerror = 0;

	/// Called by PA to notify about contetxt operation completion
	static void pa_context_success(pa_context *c, int success, PulseAudioPlayer *thread);
	/// Called by PA to notify about other context-related stuff
	static void pa_context_notify(pa_context *c, PulseAudioPlayer *thread);
	/// Called by PA when a stream operation completes
	static void pa_stream_success(pa_stream *p, int success, PulseAudioPlayer *thread);
	/// Called by PA to request more data written to stream
	static void pa_stream_write(pa_stream *p, size_t length, PulseAudioPlayer *thread);
	/// Called by PA to notify about other stream-related stuff
	static void pa_stream_notify(pa_stream *p, PulseAudioPlayer *thread);

public:
	PulseAudioPlayer(AudioProvider *provider);
	~PulseAudioPlayer();

	void Play(int64_t start,int64_t count);
	void Stop();
	bool IsPlaying() { return is_playing; }

	int64_t GetEndPosition() { return end_frame; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);

	void SetVolume(double vol) { volume = vol; }
};

#endif
