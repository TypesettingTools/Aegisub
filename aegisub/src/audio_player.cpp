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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_player.cpp
/// @brief Baseclass for audio players
/// @ingroup audio_output
///

#include "config.h"

#ifdef WITH_ALSA
#include "audio_player_alsa.h"
#endif
#ifdef WITH_DIRECTSOUND
#include "audio_player_dsound.h"
#include "audio_player_dsound2.h"
#endif
#ifdef WITH_OPENAL
#include "audio_player_openal.h"
#endif
#ifdef WITH_OSS
#include "audio_player_oss.h"
#endif
#ifdef WITH_PORTAUDIO
#include "audio_player_portaudio.h"
#endif
#ifdef WITH_PULSEAUDIO
#include "audio_player_pulse.h"
#endif
#include "compat.h"
#include "main.h"

/// @brief Constructor 
///
AudioPlayer::AudioPlayer() {
	provider = NULL;
	displayTimer = NULL;
}

/// @brief Destructor 
///
AudioPlayer::~AudioPlayer() {
	if (displayTimer) {
		displayTimer->Stop();
	}
	CloseStream();
}

/// @brief Ask to stop later 
///
void AudioPlayer::RequestStop() {
	wxCommandEvent event(wxEVT_STOP_AUDIO, 1000);
	event.SetEventObject(this);
	AddPendingEvent(event); // thread safe
}

DEFINE_EVENT_TYPE(wxEVT_STOP_AUDIO)

BEGIN_EVENT_TABLE(AudioPlayer, wxEvtHandler)
	EVT_COMMAND (1000, wxEVT_STOP_AUDIO, AudioPlayer::OnStopAudio)
END_EVENT_TABLE()

/// @brief DOCME
/// @param event 
///
void AudioPlayer::OnStopAudio(wxCommandEvent &event) {
	Stop(false);
}

/// @brief Get player 
/// @return 
///
AudioPlayer* AudioPlayerFactory::GetAudioPlayer() {
	std::vector<std::string> list = GetClasses(OPT_GET("Audio/Player")->GetString());
	if (list.empty()) throw _T("No audio players are available.");

	wxString error;
	for (unsigned int i=0;i<list.size();i++) {
		try {
			AudioPlayer *player = Create(list[i]);
			if (player) return player;
		}
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}
	throw error;
}



/// @brief Register all factories 
///
void AudioPlayerFactory::RegisterProviders() {
#ifdef WITH_ALSA
	Register<AlsaPlayer>("ALSA");
#endif
#ifdef WITH_DIRECTSOUND
	Register<DirectSoundPlayer>("DirectSound-old");
	Register<DirectSoundPlayer2>("DirectSound");
#endif
#ifdef WITH_OPENAL
	Register<OpenALPlayer>("OpenAL");
#endif
#ifdef WITH_PORTAUDIO
	Register<PortAudioPlayer>("PortAudio");
#endif
#ifdef WITH_PULSEAUDIO
	Register<PulseAudioPlayer>("PulseAudio");
#endif
#ifdef WITH_OSS
	Register<OSSPlayer>("OSS");
#endif
}

template<> AudioPlayerFactory::map *FactoryBase<AudioPlayer *(*)()>::classes = NULL;
