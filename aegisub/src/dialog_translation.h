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


#ifndef DIALOG_TRANSLATION_H
#define DIALOG_TRANSLATION_H


///////////
// Headers
#include <wx/wxprec.h>
#include "scintilla_text_ctrl.h"


//////////////
// Prototypes
class AssFile;
class AssDialogue;
class SubtitlesGrid;
class AudioDisplay;
class VideoContext;

/////////
// Class
class DialogTranslation : public wxDialog {
private:
	AudioDisplay *audio;
	VideoContext *video;
	AssFile *subs;
	SubtitlesGrid *grid;
	AssDialogue *current;
	wxWindow *main;
	int curline;
	int curblock;

	wxStaticText *LineCount;
	ScintillaTextCtrl *OrigText;
	ScintillaTextCtrl *TransText;
	wxCheckBox *PreviewCheck;

	void OnMinimize(wxIconizeEvent &event);
	void OnPlayAudioButton(wxCommandEvent &event);
	void OnPlayVideoButton(wxCommandEvent &event);
	void OnClose(wxCommandEvent &event);

	bool JumpToLine(int n,int block);
	void UpdatePreview();

	static int lastx, lasty;

public:
	bool enablePreview;
	DialogTranslation (wxWindow *parent,AssFile *subs,SubtitlesGrid *grid,int startrow=0,bool preview=false);
	~DialogTranslation();

	void OnTransBoxKey(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};


/////////////////
// Event handler
class DialogTranslationEvent : public wxEvtHandler {
private:
	DialogTranslation *control;

	void OnPreviewCheck(wxCommandEvent &event);
	void OnTransBoxKey(wxKeyEvent &event);

public:
	DialogTranslationEvent(DialogTranslation *control);
	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	TEXT_ORIGINAL = 1100,
	TEXT_TRANS,
	PREVIEW_CHECK,
	BUTTON_TRANS_PLAY_AUDIO,
	BUTTON_TRANS_PLAY_VIDEO
};


#endif
