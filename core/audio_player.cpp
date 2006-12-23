// Copyright (c) 2005, Rodrigo Braz Monteiro
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


///////////
// Headers
#include <wx/wxprec.h>
#include "setup.h"
#include "audio_player_portaudio.h"
#if USE_DIRECTSOUND == 1
#include "audio_player_dsound.h"
#endif
#include "audio_provider.h"


///////////////
// Constructor
AudioPlayer::AudioPlayer() {
	provider = NULL;
	displayTimer = NULL;
}


//////////////
// Destructor
AudioPlayer::~AudioPlayer() {
	if (displayTimer) {
		displayTimer->Stop();
	}
	CloseStream();
}


////////////////
// Set provider
void AudioPlayer::SetProvider(AudioProvider *_provider) {
	provider = _provider;
}


////////////////
// Get provider
AudioProvider *AudioPlayer::GetProvider() {
	return provider;
}


/////////////
// Get mutex
wxMutex *AudioPlayer::GetMutex() {
	return NULL;
}


/////////////
// Set timer
void AudioPlayer::SetDisplayTimer(wxTimer *timer) {
	displayTimer = timer;
}


/////////////////////
// Ask to stop later
void AudioPlayer::RequestStop() {
	wxCommandEvent event(wxEVT_STOP_AUDIO, 1000);
    event.SetEventObject(this);
	wxMutexGuiEnter();
	AddPendingEvent(event);
	wxMutexGuiLeave();
}


/////////
// Event
DEFINE_EVENT_TYPE(wxEVT_STOP_AUDIO)

BEGIN_EVENT_TABLE(AudioPlayer, wxEvtHandler)
	EVT_COMMAND (1000, wxEVT_STOP_AUDIO, AudioPlayer::OnStopAudio)
END_EVENT_TABLE()

void AudioPlayer::OnStopAudio(wxCommandEvent &event) {
	Stop(false);
}


//////////////
// Get player
AudioPlayer* AudioPlayer::GetAudioPlayer() {
	// Prepare
	AudioPlayer *player = NULL;

	try {
		// Get DirectSound player
		#if USE_DIRECTSOUND == 1
		player = new DirectSoundPlayer;
		#endif

		// Get PortAudio player
		if (!player) player = new PortAudioPlayer;
	}
	catch (...) {
		delete player;
		player = NULL;
		throw;
	}

	// Got player?
	if (!player) throw _T("Unable to create audio player.");

	// Return
	return player;
}
