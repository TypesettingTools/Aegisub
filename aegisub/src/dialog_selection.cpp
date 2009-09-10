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

/// @file dialog_selection.cpp
/// @brief Select Lines dialogue box and logic
/// @ingroup secondary_ui
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/regex.h>
#include <wx/string.h>
#endif

#include "ass_dialogue.h"
#include "dialog_selection.h"
#include "help_button.h"
#include "options.h"
#include "subs_grid.h"


/// @brief Constructor 
/// @param parent 
/// @param _grid  
///
DialogSelection::DialogSelection(wxWindow *parent, SubtitlesGrid *_grid) :
wxDialog (parent,-1,_("Select"),wxDefaultPosition,wxDefaultSize,wxCAPTION)
{
	// Variables
	grid = _grid;

	// Static-box sizers before anything else
	wxSizer *MatchSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Match"));
	wxSizer *MatchTopSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *DialogueSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Match dialogues/comments"));
	
	// Matches box
	Matches = new wxRadioButton(this,-1,_("Matches"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	DoesntMatch = new wxRadioButton(this,-1,_("Doesn't Match"),wxDefaultPosition,wxDefaultSize,0);
	Match = new wxTextCtrl(this,-1,Options.AsText(_T("Select Text")),wxDefaultPosition,wxSize(200,-1));
	MatchCase = new wxCheckBox(this,-1,_("Match case"));
	Exact = new wxRadioButton(this,-1,_("Exact match"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	Contains = new wxRadioButton(this,-1,_("Contains"));
	RegExp = new wxRadioButton(this,-1,_("Regular Expression match"));

	// Fields box
	wxArrayString field;
	field.Add(_("Text"));
	field.Add(_("Style"));
	field.Add(_("Actor"));
	field.Add(_("Effect"));
	Field = new wxRadioBox(this,-1,_("In Field"),wxDefaultPosition,wxDefaultSize,field);

	// Dialogues/comments box
	MatchDialogues = new wxCheckBox(this,MATCH_DIALOGUES_CHECKBOX,_("Dialogues"));
	MatchComments = new wxCheckBox(this,MATCH_COMMENTS_CHECKBOX,_("Comments"));

	// Action box
	wxArrayString actions;
	actions.Add(_("Set selection"));
	actions.Add(_("Add to selection"));
	actions.Add(_("Subtract from selection"));
	actions.Add(_("Intersect with selection"));
	Action = new wxRadioBox(this,-1,_("Action"),wxDefaultPosition,wxDefaultSize,actions,1);

	// Matches box sizer
	MatchTopSizer->Add(Matches,0,wxEXPAND|wxRIGHT,5);
	MatchTopSizer->Add(DoesntMatch,0,wxEXPAND,0);
	MatchTopSizer->AddStretchSpacer(1);
	MatchSizer->Add(MatchTopSizer,0,wxEXPAND,0);
	MatchSizer->Add(Match,1,wxTOP|wxEXPAND,5);
	MatchSizer->Add(MatchCase,0,wxTOP|wxEXPAND,5);
	MatchSizer->Add(Exact,0,wxTOP|wxEXPAND,5);
	MatchSizer->Add(Contains,0,wxTOP|wxEXPAND,5);
	MatchSizer->Add(RegExp,0,wxTOP|wxEXPAND,5);

	// Dialogues / Comments box
	DialogueSizer->Add(MatchDialogues,0, wxRIGHT|wxEXPAND,5);
	DialogueSizer->Add(MatchComments,0, wxEXPAND);
	
	// Buttons sizer
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(new wxButton(this,wxID_OK));
	ButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,_T("Select Lines")));
	ButtonSizer->Realize();

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(MatchSizer,0,wxEXPAND|wxLEFT|wxTOP|wxRIGHT,5);
	MainSizer->Add(Field,0,wxEXPAND|wxLEFT|wxRIGHT|wxTOP,5);
	MainSizer->Add(DialogueSizer,0,wxEXPAND|wxLEFT|wxRIGHT|wxTOP,5);
	MainSizer->Add(Action,0,wxEXPAND|wxLEFT|wxRIGHT|wxTOP,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND|wxALL,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	CenterOnParent();

	// Load settings
	Field->SetSelection(Options.AsInt(_T("Select Field")));
	Action->SetSelection(Options.AsInt(_T("Select Action")));
	MatchCase->SetValue(Options.AsBool(_T("Select Match case")));
	MatchDialogues->SetValue(Options.AsBool(_T("Select Match dialogues")));
	MatchComments->SetValue(Options.AsBool(_T("Select Match comments")));
	int condition = Options.AsInt(_T("Select Condition"));
	int mode = Options.AsInt(_T("Select Mode"));
	if (condition == 1) DoesntMatch->SetValue(true);
	if (mode == 1) Contains->SetValue(true);
	else if (mode == 2) RegExp->SetValue(true);
}



/// @brief Matching function 
/// @param diag 
/// @return 
///
bool DialogSelection::StringMatches(AssDialogue *diag) {
	// Variables
	wxString text;
	wxString matching = Match->GetValue();
	bool result = false;
	int field = Field->GetSelection();

	// Get text
	if (field == 0) text = diag->Text;
	else if (field == 1) text = diag->Style;
	else if (field == 2) text = diag->Actor;
	else if (field == 3) text = diag->Effect;

	// RegExp?
	bool isReg = false;
	if (RegExp->GetValue()) {
		isReg = true;
	}

	// Case sensitivity
	int regFlags = wxRE_ADVANCED;
	if (!MatchCase->IsChecked()) {
		if (isReg) regFlags |= wxRE_ICASE;
		else {
			text.LowerCase();
			matching.LowerCase();
		}
	}
	
	// Dialogue/Comment
	bool dial = MatchDialogues->GetValue();
	bool comm = MatchComments->GetValue();	
	if ((diag->Comment && !comm) || (!diag->Comment && !dial)) {
		result = false;
	}

	// Exact
	else if (Exact->GetValue()) {
		if (text == matching) result = true;
	}
	
	// Contains
	else if (Contains->GetValue()) {
		if (text.Contains(matching)) result = true;
	}

	// Regular expression
	else if (isReg) {
		wxRegEx regex (matching,regFlags);
		if (regex.IsValid()) {
			if (regex.Matches(text)) {
				result = true;
			}
			}
		else result = false;
	}

	// wtf?
	else {
		throw _T("Invalid mode");
	}

	// Result
	if (Matches->GetValue()) return result;
	else return !result;
}



/// @brief Process 
///
void DialogSelection::Process() {
	// Prepare
	AssDialogue *current;
	int rows = grid->GetRows();
	int action = Action->GetSelection();
	bool replaceSel = false;
	if (action == 0) replaceSel = true;
	int count = 0;

	// Build current selection list
	wxArrayInt sels;
	if (action == 2 || action == 3) {
		sels = grid->GetSelection();
	}

	// Iterate
	for (int i=0;i<rows;i++) {
		current = grid->GetDialogue(i);
		if (StringMatches(current)) {
			// Set/Add to selection
			if (action == 0 || action == 1) {
				grid->SelectRow(i,!replaceSel);
				replaceSel = false;
				count++;
			}

			// Subtract from selection
			if (action == 2) {
				if (sels.Index(i) != wxNOT_FOUND) {
					sels.Remove(i);
					count++;
				}
			}
		}
		else {
			// Intersect with selection
			if (action == 3) {
				if (sels.Index(i) != wxNOT_FOUND) {
					sels.Remove(i);
					count++;
				}
			}
		}
	}

	// Select for modes 2 and 3
	if (action == 2 || action == 3) {
		grid->ClearSelection();
		int count = sels.Count();
		for (int i=0;i<count;i++) {
			grid->SelectRow(sels[i],true);
		}
	}

	// Message saying number selected
	if (action == 0) wxMessageBox(wxString::Format(_("Selection was set to %i lines"),count), _("Selection"), wxOK);
	else if (action == 1) wxMessageBox(wxString::Format(_("%i lines were added to selection"),count), _("Selection"), wxOK);
	else wxMessageBox(wxString::Format(_("%i lines were removed from selection"),count), _("Selection"), wxOK);
}



/// @brief Save settings 
///
void DialogSelection::SaveSettings() {
	// Prepare settings
	int action = Action->GetSelection();
	int mode;
	if (Exact->GetValue()) mode = 0;
	else if (Contains->GetValue()) mode = 1;
	else mode = 2;
	int field = Field->GetSelection();
	int condition;
	if (Matches->GetValue()) condition = 0;
	else condition = 1;

	// Store
	Options.SetText(_T("Select Text"),Match->GetValue());
	Options.SetInt(_T("Select Condition"),condition);
	Options.SetInt(_T("Select Field"),field);
	Options.SetInt(_T("Select Action"),action);
	Options.SetInt(_T("Select Mode"),mode);
	Options.SetBool(_T("Select Match case"),MatchCase->IsChecked());
	Options.SetBool(_T("Select Match dialogues"),MatchDialogues->IsChecked());
	Options.SetBool(_T("Select Match comments"),MatchComments->IsChecked());
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogSelection,wxDialog)
	EVT_BUTTON(wxID_OK,DialogSelection::OnOK)
	EVT_BUTTON(wxID_CANCEL,DialogSelection::OnCancel)
	EVT_CHECKBOX(MATCH_DIALOGUES_CHECKBOX, DialogSelection::OnDialogueCheckbox)	
	EVT_CHECKBOX(MATCH_COMMENTS_CHECKBOX, DialogSelection::OnCommentCheckbox)	
END_EVENT_TABLE()




/// @brief Dialogue/Comment checkboxes 
/// @param event 
///
void DialogSelection::OnDialogueCheckbox(wxCommandEvent &event) {
	if(!event.IsChecked() && !MatchComments->GetValue())
		MatchComments->SetValue(true);
}


/// @brief DOCME
/// @param event 
///
void DialogSelection::OnCommentCheckbox(wxCommandEvent &event) {
	if(!event.IsChecked() && !MatchDialogues->GetValue())
		MatchDialogues->SetValue(true);
}	
	

/// @brief OK pressed 
/// @param event 
///
void DialogSelection::OnOK(wxCommandEvent &event) {
	Process();
	SaveSettings();
	EndModal(0);
}



/// @brief Cancel pressed 
/// @param event 
///
void DialogSelection::OnCancel(wxCommandEvent &event) {
	SaveSettings();
	EndModal(0);
}


