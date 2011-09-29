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
#include <wx/sashwin.h>
#endif

namespace agi {
	struct Context;
	class OptionValue;
}

class AudioController;
class AudioDisplay;
class wxBitmapToggleButton;
class wxButton;
class wxCommandEvent;
class wxScrollEvent;
class wxSizer;
class wxSlider;

/// @class AudioBox
/// @brief Panel with audio playback and timing controls, also containing an AudioDisplay
class AudioBox : public wxSashWindow {
	/// The controller controlling this audio box
	AudioController *controller;

	/// Project context this operates on
	agi::Context *context;


	/// Panel containing the children
	wxPanel *panel;

	/// The audio display in the box
	AudioDisplay *audioDisplay;

	/// DOCME
	wxSlider *HorizontalZoom;

	/// DOCME
	wxSlider *VerticalZoom;

	/// DOCME
	wxSlider *VolumeBar;

	void OnHorizontalZoom(wxScrollEvent &event);
	void OnVerticalZoom(wxScrollEvent &event);
	void OnVolume(wxScrollEvent &event);
	void OnVerticalLink(agi::OptionValue const& opt);
	void OnSashDrag(wxSashEvent &event);

public:
	AudioBox(wxWindow *parent, agi::Context *context);
	~AudioBox();

	DECLARE_EVENT_TABLE()
};
