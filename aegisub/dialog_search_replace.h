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


#pragma once


///////////
// Headers
#include <wx/wxprec.h>


//////////////
// Prototypes
class SubtitlesGrid;


////////////////////
// Search & Replace singleton
class SearchReplaceEngine {
private:
	int curLine;
	size_t pos;
	size_t matchLen;
	size_t replaceLen;
	bool Modified;
	bool LastWasFind;
	bool hasReplace;

	wxString *GetText(int n,int field);
	
public:
	SubtitlesGrid *grid;
	bool isReg;
	bool matchCase;
	bool updateVideo;
	bool CanContinue;
	bool hasFocus;
	int field;
	int affect;
	wxString LookFor;
	wxString ReplaceWith;

	void FindNext();
	void ReplaceNext(bool DoReplace=true);
	void ReplaceAll();
	void OpenDialog(bool HasReplace);
	void OnDialogOpen();
	void OnDialogClose();

	SearchReplaceEngine();
	friend class DialogSearchReplace;
};

// Instance
extern SearchReplaceEngine Search;


//////////////////////////
// Search & Replace class
class DialogSearchReplace : public wxDialog {
	friend class SearchReplaceEngine;

private:
	bool hasReplace;

	wxComboBox *FindEdit;
	wxComboBox *ReplaceEdit;
	wxCheckBox *CheckMatchCase;
	wxCheckBox *CheckRegExp;
	wxCheckBox *CheckUpdateVideo;
	wxRadioBox *Affect;
	wxRadioBox *Field;

	void UpdateDropDowns();

	void OnClose (wxCommandEvent &event);
	void OnFindNext (wxCommandEvent &event);
	void OnReplaceNext (wxCommandEvent &event);
	void OnReplaceAll (wxCommandEvent &event);
	void OnSetFocus (wxFocusEvent &event);
	void OnKillFocus (wxFocusEvent &event);
	void OnKeyDown (wxKeyEvent &event);

public:
	DialogSearchReplace(wxWindow *parent,bool hasReplace,wxString name);
	~DialogSearchReplace();
	void UpdateSettings();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	BUTTON_FIND_NEXT,
	BUTTON_REPLACE_NEXT,
	BUTTON_REPLACE_ALL,
	CHECK_MATCH_CASE,
	CHECK_REGEXP,
	CHECK_UPDATE_VIDEO
};
