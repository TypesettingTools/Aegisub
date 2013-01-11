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

/// @file dialog_search_replace.cpp
/// @brief Find and Search/replace dialogue box and logic
/// @ingroup secondary_ui
///

#include "config.h"

#include "dialog_search_replace.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "selection_controller.h"
#include "text_selection_controller.h"
#include "subs_grid.h"
#include "utils.h"

#include <libaegisub/of_type_adaptor.h>

#include <functional>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/regex.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

enum {
	BUTTON_FIND_NEXT,
	BUTTON_REPLACE_NEXT,
	BUTTON_REPLACE_ALL
};

enum {
	FIELD_TEXT = 0,
	FIELD_STYLE,
	FIELD_ACTOR,
	FIELD_EFFECT
};

enum {
	LIMIT_ALL = 0,
	LIMIT_SELECTED
};

DialogSearchReplace::DialogSearchReplace(agi::Context* c, bool withReplace)
: wxDialog(c->parent, -1, withReplace ? _("Replace") : _("Find"))
, hasReplace(withReplace)
{
	wxSizer *FindSizer = new wxFlexGridSizer(2, 2, 5, 15);
	FindEdit = new wxComboBox(this, -1, "", wxDefaultPosition, wxSize(300, -1), lagi_MRU_wxAS("Find"), wxCB_DROPDOWN);
	if (!FindEdit->IsListEmpty())
		FindEdit->SetSelection(0);
	FindSizer->Add(new wxStaticText(this, -1, _("Find what:")), 0, wxALIGN_CENTER_VERTICAL);
	FindSizer->Add(FindEdit);
	if (hasReplace) {
		ReplaceEdit = new wxComboBox(this, -1, "", wxDefaultPosition, wxSize(300, -1), lagi_MRU_wxAS("Replace"), wxCB_DROPDOWN);
		FindSizer->Add(new wxStaticText(this, -1, _("Replace with:")), 0, wxALIGN_CENTER_VERTICAL);
		FindSizer->Add(ReplaceEdit);
		if (!ReplaceEdit->IsListEmpty())
			ReplaceEdit->SetSelection(0);
	}

	wxSizer *OptionsSizer = new wxBoxSizer(wxVERTICAL);
	CheckMatchCase = new wxCheckBox(this, -1, _("&Match case"));
	CheckRegExp = new wxCheckBox(this, -1, _("&Use regular expressions"));
	CheckMatchCase->SetValue(OPT_GET("Tool/Search Replace/Match Case")->GetBool());
	CheckRegExp->SetValue(OPT_GET("Tool/Search Replace/RegExp")->GetBool());
	OptionsSizer->Add(CheckMatchCase, wxSizerFlags().Border(wxBOTTOM));
	OptionsSizer->Add(CheckRegExp);

	// Left sizer
	wxSizer *LeftSizer = new wxBoxSizer(wxVERTICAL);
	LeftSizer->Add(FindSizer, wxSizerFlags().DoubleBorder(wxBOTTOM));
	LeftSizer->Add(OptionsSizer);

	// Limits sizer
	wxString field[] = { _("Text"), _("Style"), _("Actor"), _("Effect") };
	wxString affect[] = { _("All rows"), _("Selected rows") };
	Field = new wxRadioBox(this, -1, _("In Field"), wxDefaultPosition, wxDefaultSize, countof(field), field);
	Affect = new wxRadioBox(this, -1, _("Limit to"), wxDefaultPosition, wxDefaultSize, countof(affect), affect);
	wxSizer *LimitSizer = new wxBoxSizer(wxHORIZONTAL);
	LimitSizer->Add(Field, wxSizerFlags().Border(wxRIGHT));
	LimitSizer->Add(Affect);
	Field->SetSelection(OPT_GET("Tool/Search Replace/Field")->GetInt());
	Affect->SetSelection(OPT_GET("Tool/Search Replace/Affect")->GetInt());

	// Buttons
	wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
	wxButton *FindNext = new wxButton(this, BUTTON_FIND_NEXT, _("&Find next"));
	FindNext->SetDefault();
	ButtonSizer->Add(FindNext, wxSizerFlags().Border(wxBOTTOM));
	if (hasReplace) {
		ButtonSizer->Add(new wxButton(this, BUTTON_REPLACE_NEXT, _("Replace &next")), wxSizerFlags().Border(wxBOTTOM));
		ButtonSizer->Add(new wxButton(this, BUTTON_REPLACE_ALL, _("Replace &all")), wxSizerFlags().Border(wxBOTTOM));
	}
	ButtonSizer->Add(new wxButton(this, wxID_CANCEL));

	wxSizer *TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(LeftSizer, wxSizerFlags().Border());
	TopSizer->Add(ButtonSizer, wxSizerFlags().Border());

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer);
	MainSizer->Add(LimitSizer, wxSizerFlags().Border());
	SetSizerAndFit(MainSizer);
	CenterOnParent();

	Search.OnDialogOpen();

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogSearchReplace::FindReplace, this, 0), BUTTON_FIND_NEXT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogSearchReplace::FindReplace, this, 1), BUTTON_REPLACE_NEXT);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogSearchReplace::FindReplace, this, 2), BUTTON_REPLACE_ALL);
}

DialogSearchReplace::~DialogSearchReplace() {
	Search.isReg = CheckRegExp->IsChecked() && CheckRegExp->IsEnabled();
	Search.matchCase = CheckMatchCase->IsChecked();
	OPT_SET("Tool/Search Replace/Match Case")->SetBool(CheckMatchCase->IsChecked());
	OPT_SET("Tool/Search Replace/RegExp")->SetBool(CheckRegExp->IsChecked());
	OPT_SET("Tool/Search Replace/Field")->SetInt(Field->GetSelection());
	OPT_SET("Tool/Search Replace/Affect")->SetInt(Affect->GetSelection());
}

void DialogSearchReplace::FindReplace(int mode) {
	if (mode < 0 || mode > 2) return;

	wxString LookFor = FindEdit->GetValue();
	if (!LookFor) return;

	Search.isReg = CheckRegExp->IsChecked() && CheckRegExp->IsEnabled();
	Search.matchCase = CheckMatchCase->IsChecked();
	Search.LookFor = LookFor;
	Search.initialized = true;
	Search.affect = Affect->GetSelection();
	Search.field = Field->GetSelection();

	if (hasReplace) {
		wxString ReplaceWith = ReplaceEdit->GetValue();
		Search.ReplaceWith = ReplaceWith;
		config::mru->Add("Replace", from_wx(ReplaceWith));
	}

	if (mode == 0)
		Search.FindNext();
	else if (mode == 1)
		Search.ReplaceNext();
	else
		Search.ReplaceAll();

	config::mru->Add("Find", from_wx(LookFor));
	UpdateDropDowns();
}

static void update_mru(wxComboBox *cb, const char *mru_name) {
	cb->Freeze();
	cb->Clear();
	cb->Append(lagi_MRU_wxAS(mru_name));
	if (!cb->IsListEmpty())
		cb->SetSelection(0);
	cb->Thaw();
}

void DialogSearchReplace::UpdateDropDowns() {
	update_mru(FindEdit, "Find");

	if (hasReplace)
		update_mru(ReplaceEdit, "Replace");
}

SearchReplaceEngine::SearchReplaceEngine()
: curLine(0)
, pos(0)
, matchLen(0)
, replaceLen(0)
, LastWasFind(true)
, hasReplace(false)
, isReg(false)
, matchCase(false)
, initialized(false)
, field(FIELD_TEXT)
, affect(LIMIT_ALL)
, context(nullptr)
{
}

static boost::flyweight<wxString> *get_text(AssDialogue *cur, int field) {
	switch (field) {
		case FIELD_TEXT: return &cur->Text;
		case FIELD_STYLE: return &cur->Style;
		case FIELD_ACTOR: return &cur->Actor;
		case FIELD_EFFECT: return &cur->Effect;
		default: throw agi::InternalError("Bad find/replace field", 0);
	}
}

void SearchReplaceEngine::ReplaceNext(bool DoReplace) {
	if (!initialized) {
		OpenDialog(DoReplace);
		return;
	}

	wxArrayInt sels = context->subsGrid->GetSelection();
	int firstLine = 0;
	if (sels.Count() > 0) firstLine = sels[0];
	// if selection has changed reset values
	if (firstLine != curLine) {
		curLine = firstLine;
		LastWasFind = true;
		pos = 0;
		matchLen = 0;
		replaceLen = 0;
	}

	// Setup
	int start = curLine;
	int nrows = context->subsGrid->GetRows();
	bool found = false;
	size_t tempPos;
	int regFlags = wxRE_ADVANCED;
	if (!matchCase) {
		if (isReg)
			regFlags |= wxRE_ICASE;
		else
			LookFor.MakeLower();
	}
	wxRegEx regex;
	if (isReg) {
		regex.Compile(LookFor, regFlags);

		if (!regex.IsValid()) {
			LastWasFind = !DoReplace;
			return;
		}
	}

	// Search for it
	boost::flyweight<wxString> *Text = nullptr;
	while (!found) {
		Text = get_text(context->subsGrid->GetDialogue(curLine), field);
		if (DoReplace && LastWasFind)
			tempPos = pos;
		else
			tempPos = pos+replaceLen;

		if (isReg) {
			if (regex.Matches(Text->get().Mid(tempPos))) {
				size_t match_start;
				regex.GetMatch(&match_start,&matchLen,0);
				pos = match_start + tempPos;
				found = true;
			}
		}
		else {
			wxString src = Text->get().Mid(tempPos);
			if (!matchCase) src.MakeLower();
			int textPos = src.Find(LookFor);
			if (textPos != -1) {
				pos = tempPos+textPos;
				found = true;
				matchLen = LookFor.Length();
			}
		}

		// Didn't find, go to next line
		if (!found) {
			curLine = (curLine + 1) % nrows;
			pos = 0;
			matchLen = 0;
			replaceLen = 0;
			if (curLine == start) break;
		}
	}

	if (found) {
		if (!DoReplace)
			replaceLen = matchLen;
		else {
			if (isReg) {
				wxString toReplace = Text->get().Mid(pos,matchLen);
				regex.ReplaceFirst(&toReplace,ReplaceWith);
				*Text = Text->get().Left(pos) + toReplace + Text->get().Mid(pos+matchLen);
				replaceLen = toReplace.Length();
			}
			else {
				*Text = Text->get().Left(pos) + ReplaceWith + Text->get().Mid(pos+matchLen);
				replaceLen = ReplaceWith.Length();
			}

			context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		}

		context->subsGrid->SelectRow(curLine,false);
		context->subsGrid->MakeCellVisible(curLine,0);
		if (field == FIELD_TEXT) {
			context->selectionController->SetActiveLine(context->subsGrid->GetDialogue(curLine));
			context->textSelectionController->SetSelection(pos, pos + replaceLen);
		}
		// hAx to prevent double match on style/actor
		else
			replaceLen = 99999;
	}
	LastWasFind = !DoReplace;
}

void SearchReplaceEngine::ReplaceAll() {
	size_t count = 0;

	int regFlags = wxRE_ADVANCED;
	if (!matchCase)
		regFlags |= wxRE_ICASE;
	wxRegEx reg;
	if (isReg)
		reg.Compile(LookFor, regFlags);

	SubtitleSelection const& sel = context->selectionController->GetSelectedSet();
	bool hasSelection = !sel.empty();
	bool inSel = affect == LIMIT_SELECTED;

	for (auto diag : context->ass->Line | agi::of_type<AssDialogue>()) {
		if (inSel && hasSelection && !sel.count(diag))
			continue;

		boost::flyweight<wxString> *Text = get_text(diag, field);

		if (isReg) {
			if (reg.Matches(*Text)) {
				size_t start, len;
				reg.GetMatch(&start, &len);

				// A zero length match (such as '$') will always be replaced
				// maxMatches times, which is almost certainly not what the user
				// wanted, so limit it to one replacement in that situation
				wxString repl(*Text);
				count += reg.Replace(&repl, ReplaceWith, len > 0 ? 1000 : 1);
				*Text = repl;
			}
		}
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
			else if(Text->get().Contains(LookFor)) {
				wxString repl(*Text);
				count += repl.Replace(LookFor, ReplaceWith);
				*Text = repl;
			}
		}
	}

	if (count > 0) {
		context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		wxMessageBox(wxString::Format(_("%i matches were replaced."), (int)count));
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

	LastWasFind = true;
	pos = 0;
	matchLen = 0;
	replaceLen = 0;
}

void SearchReplaceEngine::OpenDialog(bool replace) {
	static DialogSearchReplace *diag = nullptr;

	if (diag && replace != hasReplace) {
		// Already opened, but wrong type - destroy and create the right one
		diag->Destroy();
		diag = nullptr;
	}

	if (!diag)
		diag = new DialogSearchReplace(context, replace);

	diag->FindEdit->SetFocus();
	diag->Show();
	hasReplace = replace;
}

SearchReplaceEngine Search;
