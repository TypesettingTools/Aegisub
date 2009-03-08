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


#ifdef WITH_ALSA


///////////
// Headers
#include <wx/wxprec.h>
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "utils.h"
#include "main.h"
#include "frame_main.h"
#include <alsa/asoundlib.h>
#include "options.h"


///////////////
// Alsa player
class AlsaPlayer : public AudioPlayer {
private:
	bool open;
	volatile bool playing;
	volatile float volume;

	volatile unsigned long start_frame; // first frame of playback
	volatile unsigned long cur_frame; // last written frame + 1
	volatile unsigned long end_frame; // last frame to play
	unsigned long bpf; // bytes per frame

	AudioProvider *provider;
	snd_pcm_t *pcm_handle; // device handle
	snd_pcm_stream_t stream; // stream direction
	snd_async_handler_t *pcm_callback;

	snd_pcm_format_t sample_format;
	unsigned int rate; // sample rate of audio
	unsigned int real_rate; // actual sample rate played back
	unsigned int period_len; // length of period in microseconds
	unsigned int buflen; // length of buffer in microseconds
	snd_pcm_uframes_t period; // size of period in frames
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

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }
};



///////////
// Factory
class AlsaPlayerFactory : public AudioPlayerFactory {
public:
	AudioPlayer *CreatePlayer() { return new AlsaPlayer(); }
};

#endif
