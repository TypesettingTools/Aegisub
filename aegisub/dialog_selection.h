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


#ifndef DIALOG_SELECTION_H
#define DIALOG_SELECTION_H


///////////
// Headers
#include <wx/wxprec.h>


//////////////
// Prototypes
class SubtitlesGrid;
class AssDialogue;


//////////////////////////
// Selection dialog class
class DialogSelection : public wxDialog {
private:
	SubtitlesGrid *grid;
	wxTextCtrl *Match;
	wxCheckBox *MatchCase;
	wxCheckBox *MatchDialogues;
	wxCheckBox *MatchComments;
	wxRadioButton *Matches;
	wxRadioButton *DoesntMatch;
	wxRadioBox *Action;
	wxRadioBox *Field;
	wxRadioButton *Exact;
	wxRadioButton *Contains;
	wxRadioButton *RegExp;

	void Process();
	void SaveSettings();
	bool StringMatches (AssDialogue *diag);

	void OnOK (wxCommandEvent &event);
	void OnCancel (wxCommandEvent &event);
	void OnDialogueCheckbox(wxCommandEvent &event);
	void OnCommentCheckbox(wxCommandEvent &event);

public:
	DialogSelection(wxWindow *parent, SubtitlesGrid *grid);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	MATCH_DIALOGUES_CHECKBOX = 3000,
	MATCH_COMMENTS_CHECKBOX
};

#endif
