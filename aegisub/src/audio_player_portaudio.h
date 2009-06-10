// Copyright (c) 2005-2007, Rodrigo Braz Monteiro
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
// Contact: mailto:zeratul@cellosoft.com
//


#ifdef WITH_PORTAUDIO


///////////
// Headers
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "utils.h"
extern "C" {
#include <portaudio.h>
}



////////////////////
// Portaudio player
class PortAudioPlayer : public AudioPlayer {
private:
	static int pa_refcount;
	wxMutex PAMutex;
	volatile bool stopping;
	//bool softStop;
	bool playing;
	float volume;

	volatile int64_t playPos;
	volatile int64_t startPos;
	volatile int64_t endPos;
	void *stream;
	PaTime paStart;

	static int paCallback(
		const void *inputBuffer,
		void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo*
		timeInfo,
		PaStreamCallbackFlags
		statusFlags,
		void *userData);

	static void paStreamFinishedCallback(void *userData);

public:
	PortAudioPlayer();
	~PortAudioPlayer();

	void OpenStream();
	void CloseStream();

	void Play(int64_t start,int64_t count);
	void Stop(bool timerToo=true);
	bool IsPlaying() { return playing; }

	int64_t GetStartPosition() { return startPos; }
	int64_t GetEndPosition() { return endPos; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos) { endPos = pos; }
	void SetCurrentPosition(int64_t pos) { playPos = pos; }

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }

	wxArrayString GetOutputDevices(wxString favorite);
	wxMutex *GetMutex() { return &PAMutex; }
};


///////////
// Factory
class PortAudioPlayerFactory : public AudioPlayerFactory {
public:
	AudioPlayer *CreatePlayer() { return new PortAudioPlayer(); }
};

#endif //ifdef WITH_PORTAUDIO
