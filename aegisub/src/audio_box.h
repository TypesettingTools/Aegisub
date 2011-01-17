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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_box.h
/// @see audio_box.cpp
/// @ingroup audio_ui
///

#ifndef AGI_PRE
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sashwin.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#endif

class AudioController;
class AudioDisplay;
class AudioKaraoke;
class AudioTimingController;
class ToggleBitmap;
namespace agi {
	struct Context;
	class OptionValue;
}

/// @class AudioBox
/// @brief Panel with audio playback and timing controls, also containing an AudioDisplay
class AudioBox : public wxPanel {
	/// The audio display in the box
	AudioDisplay *audioDisplay;
	
	/// The controller controlling this audio box
	AudioController *controller;

	/// The regular dialogue timing controller
	AudioTimingController *timing_controller_dialogue;

	/// Project context this operates on
	agi::Context *context;

	/// DOCME
	wxSlider *HorizontalZoom;

	/// DOCME
	wxSlider *VerticalZoom;

	/// DOCME
	wxSlider *VolumeBar;

	/// Karaoke box sizer
	wxSizer *karaokeSizer;

	/// Karaoke mode join syllable button.
	wxButton *JoinButton;

	/// Karaoke mode split word button.
	wxButton *SplitButton;

	/// Karaoke mode split/join cancel button.
	wxButton *CancelButton;

	/// Karaoke mode split/join accept button.
	wxButton *AcceptButton;

	/// Join/Split button sizer.
	wxSizer *JoinSplitSizer;

	/// Cancel/Accept sizer.
	wxSizer *CancelAcceptSizer;

	void OnHorizontalZoom(wxScrollEvent &event);
	void OnVerticalZoom(wxScrollEvent &event);
	void OnVolume(wxScrollEvent &event);
	void OnVerticalLink(agi::OptionValue const& opt);

	void OnKaraoke(wxCommandEvent &event);
	void OnJoin(wxCommandEvent &event);
	void OnSplit(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnAccept(wxCommandEvent &event);
	void OnGoto(wxCommandEvent &event);

	void OnCommand(wxCommandEvent &event);

	/// DOCME
	AudioKaraoke *audioKaraoke;

	/// DOCME
	wxBitmapToggleButton *KaraokeButton;

	/// DOCME
	bool karaokeMode;

public:

	AudioBox(wxWindow *parent, agi::Context *context);
	~AudioBox();

	void SetKaraokeButtons();

	DECLARE_EVENT_TABLE()
};
