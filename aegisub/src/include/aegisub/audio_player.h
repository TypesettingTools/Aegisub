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

/// @file audio_player.h
/// @brief Declaration of base-class for audio players
/// @ingroup main_headers audio_output
///


#pragma once


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/event.h>
#include <wx/timer.h>
#include <wx/thread.h>
#include "aegisub.h"


//////////////
// Prototypes
class AudioProvider;


///////////////////////////
// Audio Player base class
class AudioPlayer : public wxEvtHandler {
private:
	void OnStopAudio(wxCommandEvent &event);

protected:
	AudioProvider *provider;
	wxTimer *displayTimer;

public:
	AudioPlayer();
	virtual ~AudioPlayer();

	virtual void OpenStream() {}
	virtual void CloseStream() {}

	virtual void Play(int64_t start,int64_t count)=0;	// Play sample range
	virtual void Stop(bool timerToo=true)=0;			// Stop playing
	virtual void RequestStop();							// Request it to stop playing in a thread-safe way
	virtual bool IsPlaying()=0;

	virtual void SetVolume(double volume)=0;
	virtual double GetVolume()=0;

	virtual int64_t GetStartPosition()=0;
	virtual int64_t GetEndPosition()=0;
	virtual int64_t GetCurrentPosition()=0;
	virtual void SetEndPosition(int64_t pos)=0;
	virtual void SetCurrentPosition(int64_t pos)=0;

	virtual wxMutex *GetMutex();

	virtual void SetProvider(AudioProvider *provider);
	AudioProvider *GetProvider();

	void SetDisplayTimer(wxTimer *timer);

	DECLARE_EVENT_TABLE()
};


///////////
// Factory
class AudioPlayerFactory {
public:
	virtual ~AudioPlayerFactory() {}
	virtual AudioPlayer *CreatePlayer()=0;
};

