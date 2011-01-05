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

/// @file dialog_styling_assistant.h
/// @see dialog_styling_assistant.cpp
/// @ingroup tools_ui
///

#pragma once


///////////
// Headers
#ifndef AGI_PRE
#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#endif


//////////////
// Prototypes
class AssFile;
class AssDialogue;
class SubtitlesGrid;
class DialogStyling;
class AudioDisplay;
class VideoContext;
class AudioController;



/// DOCME
/// @class StyleEditBox
/// @brief DOCME
///
/// DOCME
class StyleEditBox : public wxTextCtrl {
private:

	/// DOCME
	DialogStyling *diag;
	void OnKeyDown(wxKeyEvent &event);

public:
	StyleEditBox(DialogStyling *parent);

	DECLARE_EVENT_TABLE()
};



/// DOCME
/// @class DialogStyling
/// @brief DOCME
///
/// DOCME
class DialogStyling : public wxDialog {
	friend class StyleEditBox;

private:

	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	wxColour origColour;

	/// DOCME
	bool needCommit;


	/// DOCME
	wxTextCtrl *CurLine;

	/// DOCME
	wxListBox *Styles;

	/// DOCME
	StyleEditBox *TypeBox;

	/// DOCME
	wxCheckBox *PreviewCheck;

	/// DOCME
	wxButton *PlayVideoButton;

	/// DOCME
	wxButton *PlayAudioButton;

	void OnStyleBoxModified (wxCommandEvent &event);
	void OnStyleBoxEnter (wxCommandEvent &event);
	void OnListClicked (wxCommandEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnPlayVideoButton(wxCommandEvent &event);
	void OnPlayAudioButton(wxCommandEvent &event);
	void OnActivate(wxActivateEvent &event);

	void SetStyle (wxString curName,bool jump=true);


	/// DOCME

	/// DOCME
	static int lastx, lasty;

public:

	/// DOCME
	int linen;

	/// DOCME
	AssDialogue *line;

	/// DOCME
	AudioController *audio;

	/// DOCME
	VideoContext *video;

	DialogStyling (wxWindow *parent,SubtitlesGrid *grid);
	~DialogStyling ();

	void JumpToLine(int n);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	ENTER_STYLE_BOX,

	/// DOCME
	STYLE_LIST,

	/// DOCME
	BUTTON_PLAY_VIDEO,

	/// DOCME
	BUTTON_PLAY_AUDIO
};
