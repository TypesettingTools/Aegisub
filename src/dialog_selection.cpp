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

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "dialog_manager.h"
#include "frame_main.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "search_replace_engine.h"
#include "selection_controller.h"
#include "utils.h"

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm.hpp>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace {
class DialogSelection final : public wxDialog {
	agi::Context *con; ///< Project context

	wxTextCtrl *match_text; ///< Text to search for
	wxCheckBox *case_sensitive; ///< Should the search be case-sensitive
	wxCheckBox *apply_to_dialogue; ///< Select/deselect uncommented lines
	wxCheckBox *apply_to_comments; ///< Select/deselect commented lines
	wxRadioButton *select_unmatching_lines; ///< Select lines which don't match instead
	wxRadioBox *selection_change_type; ///< What sort of action to take on the selection
	wxRadioBox *dialogue_field; ///< Which dialogue field to look at
	wxRadioBox *match_mode;

	void Process(wxCommandEvent&);

	/// Dialogue/Comment check handler to ensure at least one is always checked
	/// @param chk The checkbox to check if both are clear
	void OnDialogueCheckbox(wxCheckBox *chk);

public:
	DialogSelection(agi::Context *c);
	~DialogSelection();
};

enum class Action {
	SET = 0,
	ADD,
	SUB,
	INTERSECT
};

enum Mode {
	EXACT = 0,
	CONTAINS,
	REGEXP
};

std::set<AssDialogue*> process(std::string const& match_text, bool match_case, Mode mode, bool invert, bool comments, bool dialogue, int field_n, AssFile *ass) {
	SearchReplaceSettings settings = {
		match_text,
		std::string(),
		static_cast<SearchReplaceSettings::Field>(field_n),
		SearchReplaceSettings::Limit::ALL,
		match_case,
		mode == Mode::REGEXP,
		false,
		false,
		mode == Mode::EXACT
	};

	auto predicate = SearchReplaceEngine::GetMatcher(settings);

	std::set<AssDialogue*> matches;
	for (auto& diag : ass->Events) {
		if (diag.Comment && !comments) continue;
		if (!diag.Comment && !dialogue) continue;

		if (invert != predicate(&diag, 0))
			matches.insert(&diag);
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

	wxRadioButton *select_matching_lines = nullptr;
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
		match_sizer->Add(match_text = new wxTextCtrl(this, -1, to_wx(OPT_GET("Tool/Select Lines/Text")->GetString())), main_flags);

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

	Bind(wxEVT_BUTTON, &DialogSelection::Process, this, wxID_OK);
	Bind(wxEVT_BUTTON, std::bind(&HelpButton::OpenPage, "Select Lines"), wxID_HELP);
	apply_to_comments->Bind(wxEVT_CHECKBOX, std::bind(&DialogSelection::OnDialogueCheckbox, this, apply_to_dialogue));
	apply_to_dialogue->Bind(wxEVT_CHECKBOX, std::bind(&DialogSelection::OnDialogueCheckbox, this, apply_to_comments));
}

DialogSelection::~DialogSelection() {
	OPT_SET("Tool/Select Lines/Text")->SetString(from_wx(match_text->GetValue()));
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
			from_wx(match_text->GetValue()), case_sensitive->IsChecked(),
			static_cast<Mode>(match_mode->GetSelection()), select_unmatching_lines->GetValue(),
			apply_to_comments->IsChecked(), apply_to_dialogue->IsChecked(),
			dialogue_field->GetSelection(), con->ass.get());
	}
	catch (agi::Exception const&) {
		Close();
		return;
	}

	auto action = static_cast<Action>(selection_change_type->GetSelection());

	Selection old_sel, new_sel;
	if (action != Action::SET)
		old_sel = con->selectionController->GetSelectedSet();

	wxString message;
	size_t count = 0;
	switch (action) {
		case Action::SET:
			new_sel = std::move(matches);
			message = (count = new_sel.size())
				? wxString::Format(wxPLURAL("Selection was set to one line", "Selection was set to %u lines", count), (unsigned)count)
				: _("Selection was set to no lines");
			break;

		case Action::ADD:
			boost::set_union(old_sel, matches, inserter(new_sel, new_sel.begin()));
			message = (count = new_sel.size() - old_sel.size())
				? wxString::Format(wxPLURAL("One line was added to selection", "%u lines were added to selection", count), (unsigned)count)
				: _("No lines were added to selection");
			break;

		case Action::SUB:
			boost::set_difference(old_sel, matches, inserter(new_sel, new_sel.begin()));
			goto sub_message;

		case Action::INTERSECT:
			boost::set_intersection(old_sel, matches, inserter(new_sel, new_sel.begin()));
			sub_message:
			message = (count = old_sel.size() - new_sel.size())
				? wxString::Format(wxPLURAL("One line was removed from selection", "%u lines were removed from selection", count), (unsigned)count)
				: _("No lines were removed from selection");
			break;
	}

	if (count == 0)
		wxMessageBox(message, _("Selection"), wxOK | wxCENTER, this);
	else
		con->frame->StatusTimeout(message);

	AssDialogue *new_active = con->selectionController->GetActiveLine();
	if (new_sel.size() && !new_sel.count(new_active))
		new_active = *new_sel.begin();
	con->selectionController->SetSelectionAndActive(std::move(new_sel), new_active);

	Close();
}

void DialogSelection::OnDialogueCheckbox(wxCheckBox *chk) {
	if(!apply_to_dialogue->IsChecked() && !apply_to_comments->GetValue())
		chk->SetValue(true);
}
}

void ShowSelectLinesDialog(agi::Context *c) {
	c->dialog->Show<DialogSelection>(c);
}