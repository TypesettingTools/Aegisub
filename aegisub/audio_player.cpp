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


///////////
// Headers
#include <wx/wxprec.h>
#include "audio_player.h"
#include "options.h"
#ifdef WITH_ALSA
#include "audio_player_alsa.h"
#endif
#ifdef WITH_DIRECTSOUND
#include "audio_player_dsound.h"
#endif
#ifdef WITH_OPENAL
#include "audio_player_openal.h"
#endif
#ifdef WITH_PORTAUDIO
#include "audio_player_portaudio.h"
#endif
#ifdef WITH_PULSEAUDIO
#include "audio_player_pulse.h"
#endif


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
	AddPendingEvent(event); // thread safe
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
AudioPlayer* AudioPlayerFactory::GetAudioPlayer() {
	// Register factories
	// HACK: fix me
	static bool init = false;
	if (!init) RegisterFactories();
	init = true;

	// List of providers
	wxArrayString list = GetFactoryList(Options.AsText(_T("Audio player")));

	// None available
	if (list.Count() == 0) throw _T("No audio players are available.");

	// Get provider
	wxString error;
	for (unsigned int i=0;i<list.Count();i++) {
		try {
			AudioPlayer *player = GetFactory(list[i])->CreatePlayer();
			if (player) return player;
		}
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}

	// Failed
	throw error;
}


//////////////////////////
// Register all factories
void AudioPlayerFactory::RegisterFactories() {
#ifdef WITH_ALSA
	new AlsaPlayerFactory();
#endif
#ifdef WITH_DIRECTSOUND
	new DirectSoundPlayerFactory();
#endif
#ifdef WITH_OPENAL
	new OpenALPlayerFactory();
#endif
#ifdef WITH_PORTAUDIO
	new PortAudioPlayerFactory();
#endif
#ifdef WITH_PULSEAUDIO
	new PulseAudioPlayerFactory();
#endif
}


//////////
// Static
template <class AudioPlayerFactory> std::map<wxString,AudioPlayerFactory*>* AegisubFactory<AudioPlayerFactory>::factories=NULL;

