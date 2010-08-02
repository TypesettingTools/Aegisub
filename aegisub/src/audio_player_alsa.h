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

/// @file audio_player_alsa.h
/// @see audio_player_alsa.cpp
/// @ingroup audio_output
///


#ifdef WITH_ALSA

#include <alsa/asoundlib.h>

#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "utils.h"



/// DOCME
/// @class AlsaPlayer
/// @brief DOCME
///
/// DOCME
class AlsaPlayer : public AudioPlayer {
private:

	/// DOCME
	bool open;

	/// DOCME
	volatile bool playing;

	/// DOCME
	volatile float volume;


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
	snd_pcm_t *pcm_handle; // device handle

	/// DOCME
	snd_pcm_stream_t stream; // stream direction

	/// DOCME
	snd_async_handler_t *pcm_callback;


	/// DOCME
	snd_pcm_format_t sample_format;

	/// DOCME
	unsigned int rate; // sample rate of audio

	/// DOCME
	unsigned int real_rate; // actual sample rate played back

	/// DOCME
	unsigned int period_len; // length of period in microseconds

	/// DOCME
	unsigned int buflen; // length of buffer in microseconds

	/// DOCME
	snd_pcm_uframes_t period; // size of period in frames

	/// DOCME
	snd_pcm_uframes_t bufsize; // size of buffer in frames

	void SetUpHardware();
	void SetUpAsync();

	static void async_write_handler(snd_async_handler_t *pcm_callback);

public:
	AlsaPlayer();
	~AlsaPlayer();

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
