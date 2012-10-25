// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

/// @file dialog_selection.cpp
/// @brief Select Lines dialogue box and logic
/// @ingroup secondary_ui
///

#include "config.h"

#include "dialog_selection.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/regex.h>
#include <wx/textctrl.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "frame_main.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"

enum {
	ACTION_SET = 0,
	ACTION_ADD,
	ACTION_SUB,
	ACTION_INTERSECT
};

enum {
	FIELD_TEXT = 0,
	FIELD_STYLE,
	FIELD_ACTOR,
	FIELD_EFFECT
};

enum {
	MODE_EXACT = 0,
	MODE_CONTAINS,
	MODE_REGEXP
};

DEFINE_SIMPLE_EXCEPTION(BadRegex, agi::InvalidInputException, "bad_regex")

static wxString AssDialogue::* get_field(int field_n) {
	switch(field_n) {
		case FIELD_TEXT:   return &AssDialogue::Text; break;
		case FIELD_STYLE:  return &AssDialogue::Style; break;
		case FIELD_ACTOR:  return &AssDialogue::Actor; break;
		case FIELD_EFFECT: return &AssDialogue::Effect; break;
		default: throw agi::InternalError("Bad field", 0);
	}
}

std::tr1::function<bool (wxString)> get_predicate(int mode, wxRegEx *re, bool match_case, wxString const& match_text) {
	using std::tr1::placeholders::_1;

	switch (mode) {
		case MODE_REGEXP:
			return bind(static_cast<bool (wxRegEx::*)(wxString const&,int) const>(&wxRegEx::Matches), re, _1, 0);
		case MODE_EXACT:
			if (match_case)
				return bind(std::equal_to<wxString>(), match_text, _1);
			else
				return bind(std::equal_to<wxString>(), match_text.Lower(), bind(&wxString::Lower, _1));
		case MODE_CONTAINS:
			if (match_case)
				return bind(&wxString::Contains, _1, match_text);
			else
				return bind(&wxString::Contains, bind(&wxString::Lower, _1), match_text.Lower());
			break;
		default: throw agi::InternalError("Bad mode", 0);
	}
}

static std::set<AssDialogue*> process(wxString match_text, bool match_case, int mode, bool invert, bool comments, bool dialogue, int field_n, AssFile *ass) {
	wxRegEx re;
	if (mode == MODE_REGEXP) {
		int flags = wxRE_ADVANCED;
		if (!match_case)
			flags |= wxRE_ICASE;
		if (!re.Compile(match_text, flags))
			throw BadRegex("Syntax error in regular expression", 0);
		match_case = false;
	}

	wxString AssDialogue::*field = get_field(field_n);
	std::tr1::function<bool (wxString)> pred = get_predicate(mode, &re, match_case, match_text);

	std::set<AssDialogue*> matches;
	for (entryIter it = ass->Line.begin(); it != ass->Line.end(); ++it) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(*it);
		if (!diag) continue;
		if (diag->Comment && !comments) continue;
		if (!diag->Comment && !dialogue) continue;

		if (pred(diag->*field) != invert)
			matches.insert(diag);
	}

	return matches;
}

DialogSelection::DialogSelection(agi::Context *c) :
wxDialog (c->parent, -1, _("Select"), wxDefaultPosition, wxDefaultSize, wxCAPTION)
, con(c)
{
	SetIcon(GETICON(select_lines_button_16));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxSizerFlags main_flags = wxSizerFlags().Expand().Border();

	wxRadioButton *select_matching_lines = 0;
	{
		wxSizer *match_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Match"));
		{
			wxSizerFlags radio_flags = wxSizerFlags().Border(wxLEFT | wxRIGHT);
			wxSizer *match_radio_line = new wxBoxSizer(wxHORIZONTAL);
			match_radio_line->Add(select_matching_lines = new wxRadioButton(this, -1, _("&Matches"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP), radio_flags);
			match_radio_line->Add(select_unmatching_lines = new wxRadioButton(this, -1, _("&Doesn't Match")), radio_flags);
			match_radio_line->Add(case_sensitive = new wxCheckBox(this, -1, _("Match c&ase")), radio_flags);
			match_sizer->Add(match_radio_line);
		}
		match_sizer->Add(match_text = new wxTextCtrl(this, -1, lagi_wxString(OPT_GET("Tool/Select Lines/Text")->GetString())), main_flags);

		main_sizer->Add(match_sizer, main_flags);
	}

	{
		wxString modes[] = { _("&Exact match"), _("&Contains"), _("&Regular Expression match") };
		main_sizer->Add(match_mode = new wxRadioBox(this, -1, _("Mode"), wxDefaultPosition, wxDefaultSize, 3, modes, 1), main_flags);
	}

	{
		wxString fields[] = { _("&Text"), _("&Style"), _("Act&or"), _("E&ffect") };
		main_sizer->Add(dialogue_field = new wxRadioBox(this, -1, _("In Field"), wxDefaultPosition, wxDefaultSize, 4, fields), main_flags);
	}

	{
		wxSizer *comment_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Match dialogues/comments"));
		comment_sizer->Add(apply_to_dialogue = new wxCheckBox(this, -1, _("D&ialogues")), wxSizerFlags().Border());
		comment_sizer->Add(apply_to_comments = new wxCheckBox(this, -1, _("Comme&nts")), wxSizerFlags().Border());
		main_sizer->Add(comment_sizer, main_flags);
	}

	{
		wxString actions[] = { _("Set se&lection"), _("&Add to selection"), _("S&ubtract from selection"), _("Intersect &with selection") };
		main_sizer->Add(selection_change_type = new wxRadioBox(this, -1, _("Action"), wxDefaultPosition, wxDefaultSize, 4, actions, 1), main_flags);
	}

	main_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL | wxHELP), main_flags);

	SetSizerAndFit(main_sizer);
	CenterOnParent();

	dialogue_field->SetSelection(OPT_GET("Tool/Select Lines/Field")->GetInt());
	selection_change_type->SetSelection(OPT_GET("Tool/Select Lines/Action")->GetInt());
	case_sensitive->SetValue(OPT_GET("Tool/Select Lines/Match/Case")->GetBool());
	apply_to_dialogue->SetValue(OPT_GET("Tool/Select Lines/Match/Dialogue")->GetBool());
	apply_to_comments->SetValue(OPT_GET("Tool/Select Lines/Match/Comment")->GetBool());
	select_unmatching_lines->SetValue(!!OPT_GET("Tool/Select Lines/Condition")->GetInt());
	select_matching_lines->SetValue(!select_unmatching_lines->GetValue());
	match_mode->SetSelection(OPT_GET("Tool/Select Lines/Mode")->GetInt());

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSelection::Process, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&HelpButton::OpenPage, "Select Lines"), wxID_HELP);
	apply_to_comments->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, std::tr1::bind(&DialogSelection::OnDialogueCheckbox, this, apply_to_dialogue));
	apply_to_dialogue->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, std::tr1::bind(&DialogSelection::OnDialogueCheckbox, this, apply_to_comments));
}

DialogSelection::~DialogSelection() {
	OPT_SET("Tool/Select Lines/Text")->SetString(STD_STR(match_text->GetValue()));
	OPT_SET("Tool/Select Lines/Condition")->SetInt(select_unmatching_lines->GetValue());
	OPT_SET("Tool/Select Lines/Field")->SetInt(dialogue_field->GetSelection());
	OPT_SET("Tool/Select Lines/Action")->SetInt(selection_change_type->GetSelection());
	OPT_SET("Tool/Select Lines/Mode")->SetInt(match_mode->GetSelection());
	OPT_SET("Tool/Select Lines/Match/Case")->SetBool(case_sensitive->IsChecked());
	OPT_SET("Tool/Select Lines/Match/Dialogue")->SetBool(apply_to_dialogue->IsChecked());
	OPT_SET("Tool/Select Lines/Match/Comment")->SetBool(apply_to_comments->IsChecked());
}

void DialogSelection::Process(wxCommandEvent&) {
	std::set<AssDialogue*> matches;

	try {
		matches = process(
			match_text->GetValue(), case_sensitive->IsChecked(),
			match_mode->GetSelection(), select_unmatching_lines->GetValue(),
			apply_to_comments->IsChecked(), apply_to_dialogue->IsChecked(),
			dialogue_field->GetSelection(), con->ass);
	}
	catch (agi::Exception const&) {
		Close();
		return;
	}

	int action = selection_change_type->GetSelection();

	SubtitleSelection old_sel, new_sel;
	if (action != ACTION_SET)
		con->selectionController->GetSelectedSet(old_sel);

	wxString message;
	size_t count = 0;
	switch (action) {
		case ACTION_SET:
			new_sel = matches;
			switch (count = new_sel.size()) {
				case 0:  message = _("Selection was set to no lines"); break;
				case 1:  message = _("Selection was set to one line"); break;
				default: message = wxString::Format(_("Selection was set to %u lines"), (unsigned)count);
			}
			break;

		case ACTION_ADD:
			set_union(old_sel.begin(), old_sel.end(), matches.begin(), matches.end(), inserter(new_sel, new_sel.begin()));
			switch (count = new_sel.size() - old_sel.size()) {
				case 0:  message = _("No lines were added to selection"); break;
				case 1:  message = _("One line was added to selection"); break;
				default: message = wxString::Format(_("%u lines were added to selection"), (unsigned)count);
			}
			break;

		case ACTION_SUB:
			set_difference(old_sel.begin(), old_sel.end(), matches.begin(), matches.end(), inserter(new_sel, new_sel.begin()));
			switch (count = old_sel.size() - new_sel.size()) {
				case 0:  message = _("No lines were removed from selection"); break;
				case 1:  message = _("One line was removed from selection"); break;
				default: message = wxString::Format(_("%u lines were removed from selection"), (unsigned)count);
			}
			break;

		case ACTION_INTERSECT:
			set_intersection(old_sel.begin(), old_sel.end(), matches.begin(), matches.end(), inserter(new_sel, new_sel.begin()));
			switch (count = old_sel.size() - new_sel.size()) {
				case 0:  message = _("No lines were removed from selection"); break;
				case 1:  message = _("One line was removed from selection"); break;
				default: message = wxString::Format(_("%u lines were removed from selection"), (unsigned)count);
			}
			break;
	}

	if (count == 0)
		wxMessageBox(message, _("Selection"), wxOK | wxCENTER, this);
	else
		wxGetApp().frame->StatusTimeout(message);

	if (new_sel.size() && !new_sel.count(con->selectionController->GetActiveLine()))
		con->selectionController->SetActiveLine(*new_sel.begin());
	con->selectionController->SetSelectedSet(new_sel);

	AssDialogue *new_active = con->selectionController->GetActiveLine();
	if (new_sel.size() && !new_sel.count(new_active))
		new_active = *new_sel.begin();
	con->selectionController->SetSelectionAndActive(new_sel, new_active);

	Close();
}

void DialogSelection::OnDialogueCheckbox(wxCheckBox *chk) {
	if(!apply_to_dialogue->IsChecked() && !apply_to_comments->GetValue())
		chk->SetValue(true);
}
