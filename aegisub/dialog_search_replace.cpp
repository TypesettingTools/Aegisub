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


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/regex.h>
#include "dialog_search_replace.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "subs_grid.h"
#include "options.h"
#include "subs_edit_box.h"
#include "video_display.h"
#include "frame_main.h"
#include "main.h"


///////////////
// Constructor
DialogSearchReplace::DialogSearchReplace (wxWindow *parent,bool _hasReplace,wxString name)
: wxDialog(parent, -1, name, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("SearchReplace"))
{
	// Setup
	hasReplace = _hasReplace;

	// Find sizer
	wxSizer *FindSizer = new wxFlexGridSizer(2,2,5,15);
	wxArrayString FindHistory = Options.GetRecentList(_T("Recent find"));
	FindEdit = new wxComboBox(this,-1,_T(""),wxDefaultPosition,wxSize(300,20),FindHistory,wxCB_DROPDOWN);
	//if (FindHistory.Count()) FindEdit->SetStringSelection(FindHistory[0]);
	FindEdit->SetSelection(0);
	FindSizer->Add(new wxStaticText(this,-1,_("Find what:")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,0);
	FindSizer->Add(FindEdit,0,wxRIGHT,0);
	if (hasReplace) {
		wxArrayString ReplaceHistory = Options.GetRecentList(_T("Recent replace"));
		ReplaceEdit = new wxComboBox(this,-1,_T(""),wxDefaultPosition,wxSize(300,20),ReplaceHistory,wxCB_DROPDOWN);
		FindSizer->Add(new wxStaticText(this,-1,_("Replace with:")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,0);
		FindSizer->Add(ReplaceEdit,0,wxRIGHT,0);
		ReplaceEdit->SetSelection(0);
	}

	// Options sizer
	wxSizer *OptionsSizer = new wxBoxSizer(wxVERTICAL);
	CheckMatchCase = new wxCheckBox(this,CHECK_MATCH_CASE,_("Match case"));
	CheckRegExp = new wxCheckBox(this,CHECK_MATCH_CASE,_("Use regular expressions"));
	CheckUpdateVideo = new wxCheckBox(this,CHECK_UPDATE_VIDEO,_("Update Video (slow)"));
	CheckMatchCase->SetValue(Options.AsBool(_T("Find Match Case")));
	CheckRegExp->SetValue(Options.AsBool(_T("Find RegExp")));
	//CheckRegExp->Enable(false);
	CheckUpdateVideo->SetValue(Options.AsBool(_T("Find Update Video")));
//	CheckUpdateVideo->Enable(Search.grid->video->loaded);
	OptionsSizer->Add(CheckMatchCase,0,wxBOTTOM,5);
	OptionsSizer->Add(CheckRegExp,0,wxBOTTOM,5);
	OptionsSizer->Add(CheckUpdateVideo,0,wxBOTTOM,0);

	// Limits sizer
	wxArrayString field;
	field.Add(_("Text"));
	field.Add(_("Style"));
	field.Add(_("Actor"));
	wxArrayString affect;
	affect.Add(_("All rows"));
	affect.Add(_("Selected rows"));
	Field = new wxRadioBox(this,-1,_("In Field"),wxDefaultPosition,wxDefaultSize,field);
	Affect = new wxRadioBox(this,-1,_("Limit to"),wxDefaultPosition,wxDefaultSize,affect);
	wxSizer *LimitSizer = new wxBoxSizer(wxHORIZONTAL);
	LimitSizer->Add(Field,1,wxEXPAND | wxRIGHT,5);
	LimitSizer->Add(Affect,0,wxEXPAND | wxRIGHT,0);
	Field->SetSelection(Options.AsInt(_T("Find Field")));
	Affect->SetSelection(Options.AsInt(_T("Find Affect")));

	// Left sizer
	wxSizer *LeftSizer = new wxBoxSizer(wxVERTICAL);
	LeftSizer->Add(FindSizer,0,wxBOTTOM,10);
	LeftSizer->Add(OptionsSizer,0,wxBOTTOM,5);
	LeftSizer->Add(LimitSizer,0,wxEXPAND | wxBOTTOM,0);

	// Buttons
	wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
	wxButton *FindNext = new wxButton(this,BUTTON_FIND_NEXT,_("Find next"));
	FindNext->SetDefault();
	ButtonSizer->Add(FindNext,0,wxEXPAND | wxBOTTOM,3);
	if (hasReplace) {
		ButtonSizer->Add(new wxButton(this,BUTTON_REPLACE_NEXT,_("Replace next")),0,wxEXPAND | wxBOTTOM,3);
		ButtonSizer->Add(new wxButton(this,BUTTON_REPLACE_ALL,_("Replace all")),0,wxEXPAND | wxBOTTOM,3);
	}
	ButtonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxEXPAND | wxBOTTOM,20);
	//ButtonSizer->Add(new wxButton(this,wxID_HELP),0,wxEXPAND | wxBOTTOM,0);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxHORIZONTAL);
	MainSizer->Add(LeftSizer,0,wxEXPAND | wxALL,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxALL,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	CenterOnParent();

	// Open
	Search.OnDialogOpen();
}


//////////////
// Destructor
DialogSearchReplace::~DialogSearchReplace() {
	// Save options
	UpdateSettings();
}


/////////////////
// Update search
void DialogSearchReplace::UpdateSettings() {
	Search.isReg = CheckRegExp->IsChecked() && CheckRegExp->IsEnabled();
	Search.matchCase = CheckMatchCase->IsChecked();
	Search.updateVideo = CheckUpdateVideo->IsChecked() && CheckUpdateVideo->IsEnabled();
	Options.SetBool(_T("Find Match Case"),CheckMatchCase->IsChecked());
	Options.SetBool(_T("Find RegExp"),CheckRegExp->IsChecked());
	Options.SetBool(_T("Find Update Video"),CheckUpdateVideo->IsChecked());
	Options.SetInt(_T("Find Field"),Field->GetSelection());
	Options.SetInt(_T("Find Affect"),Affect->GetSelection());
	Options.Save();
}	


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogSearchReplace,wxDialog)
	EVT_BUTTON(wxID_CANCEL,DialogSearchReplace::OnClose)
	EVT_BUTTON(BUTTON_FIND_NEXT,DialogSearchReplace::OnFindNext)
	EVT_BUTTON(BUTTON_REPLACE_NEXT,DialogSearchReplace::OnReplaceNext)
	EVT_BUTTON(BUTTON_REPLACE_ALL,DialogSearchReplace::OnReplaceAll)
	EVT_SET_FOCUS(DialogSearchReplace::OnSetFocus)
	EVT_KILL_FOCUS(DialogSearchReplace::OnKillFocus)
END_EVENT_TABLE()

	
/////////
// Close
void DialogSearchReplace::OnClose (wxCommandEvent &event) {
	Search.OnDialogClose();
	// Just hide
	Show(false);
}


///////
// Key
void DialogSearchReplace::OnKeyDown (wxKeyEvent &event) {
	//if (event.GetKeyCode() == WXK_ESCAPE) {
	//	Search.OnDialogClose();
	//	// Just hide
	//	Show(false);
	//}
	event.Skip();
}


///////////////////
// Find or replace
void DialogSearchReplace::FindReplace(int mode) {
	// Check mode
	if (mode < 0 || mode > 2) return;

	// Variables
	wxString LookFor = FindEdit->GetValue();
	if (LookFor.IsEmpty()) return;

	// Setup
	Search.isReg = CheckRegExp->IsChecked() && CheckRegExp->IsEnabled();
	Search.matchCase = CheckMatchCase->IsChecked();
	Search.updateVideo = CheckUpdateVideo->IsChecked() && CheckUpdateVideo->IsEnabled();
	Search.LookFor = LookFor;
	Search.CanContinue = true;
	Search.affect = Affect->GetSelection();
	Search.field = Field->GetSelection();

	// Find
	if (mode == 0) {
		Search.FindNext();
		if (hasReplace) {
			wxString ReplaceWith = ReplaceEdit->GetValue();
			Search.ReplaceWith = ReplaceWith;
			Options.AddToRecentList(ReplaceWith,_T("Recent replace"));
		}	
	}

	// Replace
	else {
		wxString ReplaceWith = ReplaceEdit->GetValue();
		Search.ReplaceWith = ReplaceWith;
		if (mode == 1) Search.ReplaceNext();
		else Search.ReplaceAll();
		Options.AddToRecentList(ReplaceWith,_T("Recent replace"));
	}
	
	// Add to history
	Options.AddToRecentList(LookFor,_T("Recent find"));
	UpdateDropDowns();
}


/////////////
// Find next
void DialogSearchReplace::OnFindNext (wxCommandEvent &event) {
	FindReplace(0);
}


////////////////
// Replace next
void DialogSearchReplace::OnReplaceNext (wxCommandEvent &event) {
	FindReplace(1);
}


///////////////
// Replace all
void DialogSearchReplace::OnReplaceAll (wxCommandEvent &event) {
	FindReplace(2);
}


//////////////////////////
// Update drop down boxes
void DialogSearchReplace::UpdateDropDowns() {
	// Find
	FindEdit->Freeze();
	FindEdit->Clear();
	FindEdit->Append(Options.GetRecentList(_T("Recent find")));
	FindEdit->SetSelection(0);
	FindEdit->Thaw();

	// Replace
	if (hasReplace) {
		ReplaceEdit->Freeze();
		ReplaceEdit->Clear();
		ReplaceEdit->Append(Options.GetRecentList(_T("Recent replace")));
		ReplaceEdit->SetSelection(0);
		ReplaceEdit->Thaw();
	}
}


void DialogSearchReplace::OnSetFocus (wxFocusEvent &event) {
	Search.hasFocus = true;
}

void DialogSearchReplace::OnKillFocus (wxFocusEvent &event) {
	Search.hasFocus = false;
}


/////////////////////// SearchReplaceEngine ///////////////////////
///////////////
// Constructor
SearchReplaceEngine::SearchReplaceEngine () {
	CanContinue = false;
}


//////////////////////
// Find next instance
void SearchReplaceEngine::FindNext() {
	ReplaceNext(false);
}


////////////////////////////////
// Find & Replace next instance
void SearchReplaceEngine::ReplaceNext(bool DoReplace) {
	// Check if it's OK to go on
	if (!CanContinue) {
		OpenDialog(DoReplace);
		return;
	}
	
	wxArrayInt sels = grid->GetSelection();
	// if selection has changed reset values
	if (sels[0] < curLine) {
		curLine = sels[0];
		Modified = false;
		LastWasFind = true;
		pos = 0;
		matchLen = 0;
		replaceLen = 0;
	}

	// Setup
	int start = curLine;
	int nrows = grid->GetRows();
	bool found = false;
	wxString *Text = NULL;
	size_t tempPos;
	int regFlags = wxRE_ADVANCED;
	if (!matchCase) {
		if (isReg) regFlags |= wxRE_ICASE;
		else LookFor.MakeLower();
	}

	// Search for it
	while (!found) {
		Text = GetText(curLine,field);
		if (DoReplace && LastWasFind) tempPos = pos;
		else tempPos = pos+replaceLen;

		// RegExp
		if (isReg) {
			wxRegEx regex (LookFor,regFlags);
			if (regex.IsValid()) {
				if (regex.Matches(Text->Mid(tempPos))) {
					size_t match_start;
					regex.GetMatch(&match_start,&matchLen,0);
					pos = match_start + tempPos;
					//matchLen++;
					found = true;
				}
			}
		}

		// Normal
		else {
			wxString src = Text->Mid(tempPos);
			if (!matchCase) src.MakeLower();
			pos = src.Find(LookFor);
			if (pos != -1) {
				pos += tempPos;
				found = true;
				matchLen = LookFor.Length();
			}
		}

		// Didn't find, go to next line
		if (!found) {
			curLine++;
			pos = 0;
			matchLen = 0;
			replaceLen = 0;
			if (curLine == nrows) curLine = 0;
			if (curLine == start) break;
		}
	}

	// Found
	if (found) {
		grid->BeginBatch();

		// If replacing
		if (DoReplace) {
			// Replace with regular expressions
			if (isReg) {
				wxString toReplace = Text->Mid(pos,matchLen);
				wxRegEx regex(LookFor,regFlags);
				regex.ReplaceFirst(&toReplace,ReplaceWith);
				*Text = Text->Left(pos) + toReplace + Text->Mid(pos+matchLen);
				replaceLen = toReplace.Length();
			}

			// Normal replace
			else {
				*Text = Text->Left(pos) + ReplaceWith + Text->Mid(pos+matchLen);
				replaceLen = ReplaceWith.Length();
			}

			// Update
			AssDialogue *cur = grid->GetDialogue(curLine);
			//cur->ParseASSTags();
			cur->UpdateData();

			// Commit
			grid->ass->FlagAsModified(_("replace"));
		}

		else {
			replaceLen = matchLen;
		}

		// Select
		grid->SelectRow(curLine,false);
		grid->MakeCellVisible(curLine,0);
		if (field == 0) {
			grid->editBox->SetToLine(curLine);
			grid->editBox->TextEdit->SetSelectionU(pos,pos+replaceLen);
		}
		grid->EndBatch();

		// Update video
		if (updateVideo) {
			grid->CommitChanges();
			grid->SetVideoToSubs(true);
		}
		else if (DoReplace) Modified = true;

		// hAx to prevent double match on style/actor
		if (field != 0) replaceLen = 99999;
	}
	LastWasFind = !DoReplace;
}


/////////////////////////
// Replace all instances
void SearchReplaceEngine::ReplaceAll() {
	// Setup
	wxString *Text;
	int nrows = grid->GetRows();
	size_t count = 0;
	int regFlags = wxRE_ADVANCED;
	if (!matchCase) {
		if (isReg) regFlags |= wxRE_ICASE;
		//else LookFor.MakeLower();
	}
	bool replaced;
	grid->BeginBatch();

	// Selection
	bool hasSelection = false;
	wxArrayInt sels = grid->GetSelection();
	if (sels.Count() > 0) hasSelection = true;
	bool inSel = false;
	if (affect == 1) inSel = true;

	// Scan
	for (int i=0;i<nrows;i++) {
		// Check if row is selected
		if (inSel && hasSelection && sels.Index(i) == wxNOT_FOUND) {
			continue;
		}

		// Prepare
		replaced = false;
		Text = GetText(i,field);

		// Regular expressions
		if (isReg) {
			wxRegEx reg(LookFor,regFlags);
			if (reg.IsValid()) {
				size_t reps = reg.ReplaceAll(Text,ReplaceWith);
				if (reps > 0) replaced = true;
				count += reps;
			}
		}

		// Normal replace
		else {
			if (Text->Contains(LookFor)) {
				count += Text->Replace(LookFor,ReplaceWith);
				replaced = true;
			}
		}

		// Replaced?
		if (replaced) {
			AssDialogue *cur = grid->GetDialogue(i);
			cur->UpdateData();
			//cur->ParseASSTags();
		}
	}

	// Commit
	if (count > 0) {
		grid->ass->FlagAsModified(_("replace"));
		grid->CommitChanges();
		grid->editBox->Update();
		wxMessageBox(wxString::Format(_("%i matches were replaced."),count));
	}

	// None found
	else {
		wxMessageBox(_("No matches found."));
	}
	grid->EndBatch();
	LastWasFind = false;
}


////////////////////////
// Search dialog opened
void SearchReplaceEngine::OnDialogOpen() {
	// Set curline
	wxArrayInt sels = grid->GetSelection();
	curLine = 0;
	if (sels.Count() > 0) curLine = sels[0];

	// Reset values
	Modified = false;
	LastWasFind = true;
	pos = 0;
	matchLen = 0;
	replaceLen = 0;
}


////////////////////////
// Search dialog closed
void SearchReplaceEngine::OnDialogClose() {
	if (Modified) grid->CommitChanges();
}


///////////////
// Open dialog
void SearchReplaceEngine::OpenDialog (bool replace) {
	static DialogSearchReplace *diag = NULL;
	wxString title = replace? _("Replace") : _("Find");

	// already opened
	if (diag) {
		// it's the right type so give focus
		if(replace == hasReplace) {
			diag->FindEdit->SetFocus();
			diag->Show();
			OnDialogOpen();
			return;
		}
		// wrong type - destroy and create the right one
		diag->Destroy();
	}
	// create new one
	diag = new DialogSearchReplace(((AegisubApp*)wxTheApp)->frame,replace,title);
	diag->FindEdit->SetFocus();
	diag->Show();
	hasReplace = replace;
}


////////////////////
// Get text pointer
wxString *SearchReplaceEngine::GetText(int n,int field) {
	AssDialogue *cur = grid->GetDialogue(n);
	if (field == 0) return &cur->Text;
	else if (field == 1) return &cur->Style;
	else if (field == 2) return &cur->Actor;
	else throw wxString(_T("Invalid field"));
}


///////////////////
// Global instance
SearchReplaceEngine Search;
