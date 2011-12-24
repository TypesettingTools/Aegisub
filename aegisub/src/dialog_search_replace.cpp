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

/// @file dialog_search_replace.cpp
/// @brief Find and Search/replace dialogue box and logic
/// @ingroup secondary_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <functional>

#include <wx/regex.h>
#include <wx/string.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "command/command.h"
#include "compat.h"
#include "dialog_search_replace.h"
#include "include/aegisub/context.h"
#include "main.h"
#include "selection_controller.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "video_context.h"

enum {
	BUTTON_FIND_NEXT,
	BUTTON_REPLACE_NEXT,
	BUTTON_REPLACE_ALL,
	CHECK_MATCH_CASE,
	CHECK_REGEXP,
	CHECK_UPDATE_VIDEO
};

DialogSearchReplace::DialogSearchReplace(agi::Context* c, bool withReplace)
: wxDialog(c->parent, -1, withReplace ? _("Replace") : _("Find"))
, hasReplace(withReplace)
{
	wxSizer *FindSizer = new wxFlexGridSizer(2,2,5,15);
	FindEdit = new wxComboBox(this,-1,"",wxDefaultPosition,wxSize(300,-1),lagi_MRU_wxAS("Find"),wxCB_DROPDOWN);
	FindEdit->SetSelection(0);
	FindSizer->Add(new wxStaticText(this,-1,_("Find what:")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,0);
	FindSizer->Add(FindEdit,0,wxRIGHT,0);
	if (hasReplace) {
		ReplaceEdit = new wxComboBox(this,-1,"",wxDefaultPosition,wxSize(300,-1),lagi_MRU_wxAS("Replace"),wxCB_DROPDOWN);
		FindSizer->Add(new wxStaticText(this,-1,_("Replace with:")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,0);
		FindSizer->Add(ReplaceEdit,0,wxRIGHT,0);
		ReplaceEdit->SetSelection(0);
	}

	wxSizer *OptionsSizer = new wxBoxSizer(wxVERTICAL);
	CheckMatchCase = new wxCheckBox(this,CHECK_MATCH_CASE,_("&Match case"));
	CheckRegExp = new wxCheckBox(this,CHECK_MATCH_CASE,_("&Use regular expressions"));
	CheckUpdateVideo = new wxCheckBox(this,CHECK_UPDATE_VIDEO,_("Update &Video"));
	CheckMatchCase->SetValue(OPT_GET("Tool/Search Replace/Match Case")->GetBool());
	CheckRegExp->SetValue(OPT_GET("Tool/Search Replace/RegExp")->GetBool());
	CheckUpdateVideo->SetValue(OPT_GET("Tool/Search Replace/Video Update")->GetBool());
	CheckUpdateVideo->Enable(c->videoController->IsLoaded());
	OptionsSizer->Add(CheckMatchCase,0,wxBOTTOM,5);
	OptionsSizer->Add(CheckRegExp,0,wxBOTTOM,5);
	OptionsSizer->Add(CheckUpdateVideo,0,wxBOTTOM,0);

	// Limits sizer
	wxArrayString field;
	field.Add(_("Text"));
	field.Add(_("Style"));
	field.Add(_("Actor"));
	field.Add(_("Effect"));
	wxArrayString affect;
	affect.Add(_("All rows"));
	affect.Add(_("Selected rows"));
	Field = new wxRadioBox(this,-1,_("In Field"),wxDefaultPosition,wxDefaultSize,field);
	Affect = new wxRadioBox(this,-1,_("Limit to"),wxDefaultPosition,wxDefaultSize,affect);
	wxSizer *LimitSizer = new wxBoxSizer(wxHORIZONTAL);
	LimitSizer->Add(Field,1,wxEXPAND | wxRIGHT,5);
	LimitSizer->Add(Affect,0,wxEXPAND | wxRIGHT,0);
	Field->SetSelection(OPT_GET("Tool/Search Replace/Field")->GetInt());
	Affect->SetSelection(OPT_GET("Tool/Search Replace/Affect")->GetInt());

	// Left sizer
	wxSizer *LeftSizer = new wxBoxSizer(wxVERTICAL);
	LeftSizer->Add(FindSizer,0,wxBOTTOM,10);
	LeftSizer->Add(OptionsSizer,0,wxBOTTOM,5);
	LeftSizer->Add(LimitSizer,0,wxEXPAND | wxBOTTOM,0);

	// Buttons
	wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
	wxButton *FindNext = new wxButton(this,BUTTON_FIND_NEXT,_("&Find next"));
	FindNext->SetDefault();
	ButtonSizer->Add(FindNext,0,wxEXPAND | wxBOTTOM,3);
	if (hasReplace) {
		ButtonSizer->Add(new wxButton(this,BUTTON_REPLACE_NEXT,_("Replace &next")),0,wxEXPAND | wxBOTTOM,3);
		ButtonSizer->Add(new wxButton(this,BUTTON_REPLACE_ALL,_("Replace &all")),0,wxEXPAND | wxBOTTOM,3);
	}
	ButtonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxEXPAND | wxBOTTOM,20);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxHORIZONTAL);
	MainSizer->Add(LeftSizer,0,wxEXPAND | wxALL,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxALL,5);
	SetSizerAndFit(MainSizer);
	CenterOnParent();

	Search.OnDialogOpen();

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&DialogSearchReplace::FindReplace, this, 0), BUTTON_FIND_NEXT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&DialogSearchReplace::FindReplace, this, 1), BUTTON_REPLACE_NEXT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&DialogSearchReplace::FindReplace, this, 2), BUTTON_REPLACE_ALL);
	Bind(wxEVT_SET_FOCUS, std::tr1::bind(&SearchReplaceEngine::SetFocus, &Search, true));
	Bind(wxEVT_KILL_FOCUS, std::tr1::bind(&SearchReplaceEngine::SetFocus, &Search, false));
}

DialogSearchReplace::~DialogSearchReplace() {
	UpdateSettings();
}

void DialogSearchReplace::UpdateSettings() {
	Search.isReg = CheckRegExp->IsChecked() && CheckRegExp->IsEnabled();
	Search.matchCase = CheckMatchCase->IsChecked();
	Search.updateVideo = CheckUpdateVideo->IsChecked() && CheckUpdateVideo->IsEnabled();
	OPT_SET("Tool/Search Replace/Match Case")->SetBool(CheckMatchCase->IsChecked());
	OPT_SET("Tool/Search Replace/RegExp")->SetBool(CheckRegExp->IsChecked());
	OPT_SET("Tool/Search Replace/Video Update")->SetBool(CheckUpdateVideo->IsChecked());
	OPT_SET("Tool/Search Replace/Field")->SetInt(Field->GetSelection());
	OPT_SET("Tool/Search Replace/Affect")->SetInt(Affect->GetSelection());
}

void DialogSearchReplace::FindReplace(int mode) {
	if (mode < 0 || mode > 2) return;

	// Variables
	wxString LookFor = FindEdit->GetValue();
	if (!LookFor) return;

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
			config::mru->Add("Replace", STD_STR(ReplaceWith));
		}
	}

	// Replace
	else {
		wxString ReplaceWith = ReplaceEdit->GetValue();
		Search.ReplaceWith = ReplaceWith;
		if (mode == 1) Search.ReplaceNext();
		else Search.ReplaceAll();
		config::mru->Add("Replace", STD_STR(ReplaceWith));
	}
	
	// Add to history
	config::mru->Add("Find", STD_STR(LookFor));
	UpdateDropDowns();
}

void DialogSearchReplace::UpdateDropDowns() {
	FindEdit->Freeze();
	FindEdit->Clear();
	FindEdit->Append(lagi_MRU_wxAS("Find"));
	FindEdit->SetSelection(0);
	FindEdit->Thaw();

	if (hasReplace) {
		ReplaceEdit->Freeze();
		ReplaceEdit->Clear();
		ReplaceEdit->Append(lagi_MRU_wxAS("Replace"));
		ReplaceEdit->SetSelection(0);
		ReplaceEdit->Thaw();
	}
}

/// @brief Constructor  SearchReplaceEngine ///////////////////////
SearchReplaceEngine::SearchReplaceEngine () {
	CanContinue = false;
}

void SearchReplaceEngine::FindNext() {
	ReplaceNext(false);
}


/// @brief Find & Replace next instance 
void SearchReplaceEngine::ReplaceNext(bool DoReplace) {
	if (!CanContinue) {
		OpenDialog(DoReplace);
		return;
	}
	
	wxArrayInt sels = context->subsGrid->GetSelection();
	int firstLine = 0;
	if (sels.Count() > 0) firstLine = sels[0];
	// if selection has changed reset values
	if (firstLine != curLine) {
		curLine = firstLine;
		Modified = false;
		LastWasFind = true;
		pos = 0;
		matchLen = 0;
		replaceLen = 0;
	}

	// Setup
	int start = curLine;
	int nrows = context->subsGrid->GetRows();
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
		Text = GetText(context->subsGrid->GetDialogue(curLine), field);
		if (DoReplace && LastWasFind)
			tempPos = pos;
		else
			tempPos = pos+replaceLen;

		// RegExp
		if (isReg) {
			wxRegEx regex (LookFor,regFlags);
			if (regex.IsValid()) {
				if (regex.Matches(Text->Mid(tempPos))) {
					size_t match_start;
					regex.GetMatch(&match_start,&matchLen,0);
					pos = match_start + tempPos;
					found = true;
				}
			}
		}

		// Normal
		else {
			int textPos = tempPos;
			wxString src = Text->Mid(tempPos);
			if (!matchCase) src.MakeLower();
			int tempPos = src.Find(LookFor);
			if (tempPos != -1) {
				pos = textPos+tempPos;
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

			// Commit
			context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		}

		else {
			replaceLen = matchLen;
		}

		// Select
		context->subsGrid->SelectRow(curLine,false);
		context->subsGrid->MakeCellVisible(curLine,0);
		if (field == 0) {
			context->subsGrid->SetActiveLine(context->subsGrid->GetDialogue(curLine));
			context->editBox->SetSelectionU(pos,pos+replaceLen);
		}

		// Update video
		if (updateVideo) {
			(*cmd::get("video/jump/start"))(context);
		}
		else if (DoReplace) Modified = true;

		// hAx to prevent double match on style/actor
		if (field != 0) replaceLen = 99999;
	}
	LastWasFind = !DoReplace;
}

/// @brief Replace all instances
void SearchReplaceEngine::ReplaceAll() {
	size_t count = 0;

	int regFlags = wxRE_ADVANCED;
	if (!matchCase)
		regFlags |= wxRE_ICASE;
	wxRegEx reg;
	if (isReg)
		reg.Compile(LookFor, regFlags);

	// Selection
	std::set<AssDialogue*> sel = context->selectionController->GetSelectedSet();
	bool hasSelection = !sel.empty();
	bool inSel = affect == 1;

	// Scan
	for (std::list<AssEntry*>::iterator it = context->ass->Line.begin(); it != context->ass->Line.end(); ++it) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(*it);
		if (!diag) continue;

		// Check if row is selected
		if (inSel && hasSelection && !sel.count(diag))
			continue;

		wxString *Text = GetText(diag, field);

		// Regular expressions
		if (isReg) {
			if (reg.Matches(*Text)) {
				size_t start, len;
				reg.GetMatch(&start, &len);

				// A zero length match (such as '$') will always be replaced
				// maxMatches times, which is almost certainly not what the user
				// wanted, so limit it to one replacement in that situation
				count += reg.Replace(Text, ReplaceWith, len > 0 ? 1000 : 1);
			}
		}
		// Normal replace
		else {
			if (!Search.matchCase) {
				bool replaced = false;
				wxString Left, Right = *Text;
				size_t pos = 0;
				Left.reserve(Right.size());
				while (pos + LookFor.size() <= Right.size()) {
					if (Right.Mid(pos, LookFor.size()).CmpNoCase(LookFor) == 0) {
						Left.Append(Right.Left(pos)).Append(ReplaceWith);
						Right = Right.Mid(pos + LookFor.Len());
						++count;
						replaced = true;
						pos = 0;
					}
					else {
						pos++;
					}
				}
				if (replaced) {
					*Text = Left + Right;
				}
			}
			else if(Text->Contains(LookFor)) {
				count += Text->Replace(LookFor, ReplaceWith);
			}
		}
	}

	if (count > 0) {
		context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		wxMessageBox(wxString::Format(_("%i matches were replaced."),count));
	}
	else {
		wxMessageBox(_("No matches found."));
	}
	LastWasFind = false;
}

void SearchReplaceEngine::OnDialogOpen() {
	wxArrayInt sels = context->subsGrid->GetSelection();
	curLine = 0;
	if (sels.Count() > 0) curLine = sels[0];

	// Reset values
	Modified = false;
	LastWasFind = true;
	pos = 0;
	matchLen = 0;
	replaceLen = 0;
}

void SearchReplaceEngine::OpenDialog (bool replace) {
	static DialogSearchReplace *diag = NULL;

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
	diag = new DialogSearchReplace(context, replace);
	diag->FindEdit->SetFocus();
	diag->Show();
	hasReplace = replace;
}

wxString *SearchReplaceEngine::GetText(AssDialogue *cur, int field) {
	if (field == 0) return &cur->Text;
	else if (field == 1) return &cur->Style;
	else if (field == 2) return &cur->Actor;
	else if (field == 3) return &cur->Effect;
	else throw wxString("Invalid field");
}

SearchReplaceEngine Search;
