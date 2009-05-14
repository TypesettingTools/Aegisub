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


#ifndef DIALOG_STYLING_ASSISTANT_H
#define DIALOG_STYLING_ASSISTANT_H


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/dialog.h>
#include <wx/colour.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/checkbox.h>


//////////////
// Prototypes
class AssFile;
class AssDialogue;
class SubtitlesGrid;
class DialogStyling;
class AudioDisplay;
class VideoContext;


/////////////////
// Editbox class
class StyleEditBox : public wxTextCtrl {
private:
	DialogStyling *diag;
	void OnKeyDown(wxKeyEvent &event);

public:
	StyleEditBox(DialogStyling *parent);

	DECLARE_EVENT_TABLE()
};


/////////
// Class
class DialogStyling : public wxDialog {
	friend class StyleEditBox;

private:
	SubtitlesGrid *grid;
	wxColour origColour;
	bool needCommit;

	wxTextCtrl *CurLine;
	wxListBox *Styles;
	StyleEditBox *TypeBox;
	wxCheckBox *PreviewCheck;
	wxButton *PlayVideoButton;
	wxButton *PlayAudioButton;

	void OnStyleBoxModified (wxCommandEvent &event);
	void OnStyleBoxEnter (wxCommandEvent &event);
	void OnListClicked (wxCommandEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnPlayVideoButton(wxCommandEvent &event);
	void OnPlayAudioButton(wxCommandEvent &event);
	void OnActivate(wxActivateEvent &event);

	void SetStyle (wxString curName,bool jump=true);

	static int lastx, lasty;

public:
	int linen;
	AssDialogue *line;
	AudioDisplay *audio;
	VideoContext *video;

	DialogStyling (wxWindow *parent,SubtitlesGrid *grid);
	~DialogStyling ();

	void JumpToLine(int n);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	ENTER_STYLE_BOX,
	STYLE_LIST,
	BUTTON_PLAY_VIDEO,
	BUTTON_PLAY_AUDIO
};


#endif
