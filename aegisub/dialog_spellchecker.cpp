// Copyright (c) 2007, Rodrigo Braz Monteiro
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
#include "dialog_spellchecker.h"
#include "spellchecker.h"


///////
// IDs
enum {
	BUTTON_REPLACE = 1720,
	BUTTON_IGNORE,
	BUTTON_REPLACE_ALL,
	BUTTON_IGNORE_ALL,
	BUTTON_ADD
};


///////////////
// Constructor
DialogSpellChecker::DialogSpellChecker(wxFrame *parent)
: wxDialog(parent, -1, _T("Spell Checker"), wxDefaultPosition, wxDefaultSize)
{
	// Top sizer
	origWord = new wxTextCtrl(this,-1,_T("original"));
	replaceWord = new wxTextCtrl(this,-1,_T("replace with"));
	wxFlexGridSizer *topSizer = new wxFlexGridSizer(2,2,5,5);
	topSizer->Add(new wxStaticText(this,-1,_("Misspelled word:")),0,wxALIGN_CENTER_VERTICAL);
	topSizer->Add(origWord,1,wxEXPAND);
	topSizer->Add(new wxStaticText(this,-1,_("Replace with:")),0,wxALIGN_CENTER_VERTICAL);
	topSizer->Add(replaceWord,1,wxEXPAND);
	topSizer->AddGrowableCol(1,1);

	// Actions sizer
	wxSizer *actionsSizer = new wxBoxSizer(wxVERTICAL);
	actionsSizer->Add(new wxButton(this,BUTTON_REPLACE,_("Replace")),0,wxEXPAND | wxBOTTOM,2);
	actionsSizer->Add(new wxButton(this,BUTTON_REPLACE_ALL,_("Replace All")),0,wxEXPAND | wxBOTTOM,2);
	actionsSizer->Add(new wxButton(this,BUTTON_IGNORE,_("Ignore")),0,wxEXPAND | wxBOTTOM,2);
	actionsSizer->Add(new wxButton(this,BUTTON_IGNORE_ALL,_("Ignore all")),0,wxEXPAND | wxBOTTOM,2);
	actionsSizer->Add(new wxButton(this,BUTTON_ADD,_("Add to dictionary")),0,wxEXPAND | wxBOTTOM,0);

	// Middle sizer
	suggestList = new wxListBox(this,-1,wxDefaultPosition,wxSize(150,100));
	wxBoxSizer *midSizer = new wxBoxSizer(wxHORIZONTAL);
	midSizer->Add(suggestList,1,wxEXPAND);
	midSizer->Add(actionsSizer,0,wxEXPAND | wxLEFT,5);

	// Bottom sizer
	wxBoxSizer *botSizer = new wxBoxSizer(wxHORIZONTAL);
	botSizer->AddStretchSpacer(1);
	botSizer->Add(new wxButton(this,wxID_CLOSE));

	// Main sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(topSizer,0,wxEXPAND | wxALL,5);
	mainSizer->Add(midSizer,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	mainSizer->Add(botSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	SetSizer(mainSizer);
	CenterOnParent();

	// Get spell checker
	spellchecker = SpellChecker::GetSpellChecker();
	if (!spellchecker) {
		wxMessageBox(_T("No spellchecker available."),_T("Error"),wxICON_ERROR);
		Destroy();
		return;
	}

	// Go to first match
	FindNext(0,0);
}


///////////////////
// Find next match
void DialogSpellChecker::FindNext(int startLine,int startPos) {

}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogSpellChecker,wxDialog)
	EVT_BUTTON(wxID_CLOSE,DialogSpellChecker::OnClose)
	EVT_BUTTON(BUTTON_REPLACE,DialogSpellChecker::OnReplace)
	EVT_BUTTON(BUTTON_REPLACE_ALL,DialogSpellChecker::OnReplaceAll)
	EVT_BUTTON(BUTTON_IGNORE,DialogSpellChecker::OnIgnore)
	EVT_BUTTON(BUTTON_IGNORE_ALL,DialogSpellChecker::OnIgnoreAll)
	EVT_BUTTON(BUTTON_ADD,DialogSpellChecker::OnAdd)
END_EVENT_TABLE()


/////////
// Close
void DialogSpellChecker::OnClose(wxCommandEvent &event) {
	Destroy();
}


///////////
// Replace
void DialogSpellChecker::OnReplace(wxCommandEvent &event) {
}


//////////////////////
// Replace all errors
void DialogSpellChecker::OnReplaceAll(wxCommandEvent &event) {
}


/////////////////////
// Ignore this error
void DialogSpellChecker::OnIgnore(wxCommandEvent &event) {
}


/////////////////////
// Ignore all errors
void DialogSpellChecker::OnIgnoreAll(wxCommandEvent &event) {
}


/////////////////////
// Add to dictionary
void DialogSpellChecker::OnAdd(wxCommandEvent &event) {
}
