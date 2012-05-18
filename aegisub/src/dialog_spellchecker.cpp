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
//
// $Id$

/// @file dialog_spellchecker.cpp
/// @brief Spell checker dialogue box
/// @ingroup spelling
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#endif

#include "dialog_spellchecker.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "include/aegisub/spellchecker.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"
#include "subs_edit_ctrl.h"
#include "utils.h"

#include <libaegisub/exception.h>

static void save_skip_comments(wxCommandEvent &evt) {
	OPT_SET("Tool/Spell Checker/Skip Comments")->SetBool(!!evt.GetInt());
}

DialogSpellChecker::DialogSpellChecker(agi::Context *context)
: wxDialog(context->parent, -1, _("Spell Checker"))
, context(context)
, spellchecker(SpellCheckerFactory::GetSpellChecker())
, start_line(0)
, active_line(0)
, has_looped(false)
{
	SetIcon(GETICON(spellcheck_toolbutton_16));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer *current_word_sizer = new wxFlexGridSizer(2, 5, 5);
	main_sizer->Add(current_word_sizer, wxSizerFlags().Expand().Border(wxALL, 5));

	wxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	main_sizer->Add(bottom_sizer, wxSizerFlags().Expand().Border(~wxTOP & wxALL, 5));

	wxSizer *bottom_left_sizer = new wxBoxSizer(wxVERTICAL);
	bottom_sizer->Add(bottom_left_sizer, wxSizerFlags().Expand().Border(wxRIGHT, 5));

	wxSizer *actions_sizer = new wxBoxSizer(wxVERTICAL);
	bottom_sizer->Add(actions_sizer, wxSizerFlags().Expand());

	// Misspelled word and currently selected correction
	current_word_sizer->AddGrowableCol(1, 1);
	current_word_sizer->Add(new wxStaticText(this, -1, _("Misspelled word:")), 0, wxALIGN_CENTER_VERTICAL);
	current_word_sizer->Add(orig_word = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY), wxSizerFlags(1).Expand());
	current_word_sizer->Add(new wxStaticText(this, -1, _("Replace with:")), 0, wxALIGN_CENTER_VERTICAL);
	current_word_sizer->Add(replace_word = new wxTextCtrl(this, -1, ""), wxSizerFlags(1).Expand());

	// List of suggested corrections
	suggest_list = new wxListBox(this, -1, wxDefaultPosition, wxSize(300, 150));
	suggest_list->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &DialogSpellChecker::OnChangeSuggestion, this);
	suggest_list->Bind(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, &DialogSpellChecker::OnReplace, this);
	bottom_left_sizer->Add(suggest_list, wxSizerFlags(1).Expand());

	// List of supported spellchecker languages
	{
		if (!spellchecker.get()) {
			wxMessageBox("No spellchecker available.", "Error", wxOK | wxICON_ERROR | wxCENTER);
			throw agi::UserCancelException("No spellchecker available");
		}

		dictionary_lang_codes = spellchecker->GetLanguageList();
		if (dictionary_lang_codes.empty()) {
			wxMessageBox("No spellchecker dictionaries available.", "Error", wxOK | wxICON_ERROR | wxCENTER);
			throw agi::UserCancelException("No spellchecker dictionaries available");
		}

		wxArrayString language_names(dictionary_lang_codes);
		for (size_t i = 0; i < dictionary_lang_codes.size(); ++i) {
			if (const wxLanguageInfo *info = wxLocale::FindLanguageInfo(dictionary_lang_codes[i]))
				language_names[i] = info->Description;
		}

		language = new wxComboBox(this, -1, "", wxDefaultPosition, wxDefaultSize, language_names, wxCB_DROPDOWN | wxCB_READONLY);
		wxString cur_lang = lagi_wxString(OPT_GET("Tool/Spell Checker/Language")->GetString());
		int cur_lang_index = dictionary_lang_codes.Index(cur_lang);
		if (cur_lang_index == wxNOT_FOUND) cur_lang_index = dictionary_lang_codes.Index("en");
		if (cur_lang_index == wxNOT_FOUND) cur_lang_index = dictionary_lang_codes.Index("en_US");
		if (cur_lang_index == wxNOT_FOUND) cur_lang_index = 0;
		language->SetSelection(cur_lang_index);
		language->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &DialogSpellChecker::OnChangeLanguage, this);

		bottom_left_sizer->Add(language, wxSizerFlags().Expand().Border(wxTOP, 5));
	}

	{
		wxSizerFlags button_flags = wxSizerFlags().Expand().Bottom().Border(wxBOTTOM, 5);

		skip_comments = new wxCheckBox(this, -1, _("&Skip Comments"));
		actions_sizer->Add(skip_comments, button_flags);
		skip_comments->SetValue(OPT_GET("Tool/Spell Checker/Skip Comments")->GetBool());
		skip_comments->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, save_skip_comments);

		wxButton *button;

		actions_sizer->Add(button = new wxButton(this, -1, _("&Replace")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSpellChecker::OnReplace, this);

		actions_sizer->Add(button = new wxButton(this, -1, _("Replace &all")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSpellChecker::OnReplaceAll, this);

		actions_sizer->Add(button = new wxButton(this, -1, _("&Ignore")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSpellChecker::OnIgnore, this);

		actions_sizer->Add(button = new wxButton(this, -1, _("Ignore a&ll")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSpellChecker::OnIgnoreAll, this);

		actions_sizer->Add(add_button = new wxButton(this, -1, _("Add to &dictionary")), button_flags);
		add_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSpellChecker::OnAdd, this);

		actions_sizer->Add(new HelpButton(this, "Spell Checker"), button_flags);

		actions_sizer->Add(new wxButton(this, wxID_CANCEL), button_flags.Border(0));
	}

	SetSizerAndFit(main_sizer);
	CenterOnParent();

	if (FindNext())
		Show();
}

DialogSpellChecker::~DialogSpellChecker() {
}

void DialogSpellChecker::OnReplace(wxCommandEvent&) {
	Replace();
	FindNext();
}

void DialogSpellChecker::OnReplaceAll(wxCommandEvent&) {
	auto_replace[orig_word->GetValue()] = replace_word->GetValue();

	Replace();
	FindNext();
}

void DialogSpellChecker::OnIgnore(wxCommandEvent&) {
	FindNext();
}

void DialogSpellChecker::OnIgnoreAll(wxCommandEvent&) {
	auto_ignore.insert(orig_word->GetValue());
	FindNext();
}

void DialogSpellChecker::OnAdd(wxCommandEvent&) {
	spellchecker->AddWord(orig_word->GetValue());
	FindNext();
}

void DialogSpellChecker::OnChangeLanguage(wxCommandEvent&) {
	wxString code = dictionary_lang_codes[language->GetSelection()];
	OPT_SET("Tool/Spell Checker/Language")->SetString(STD_STR(code));

	FindNext();
}

void DialogSpellChecker::OnChangeSuggestion(wxCommandEvent&) {
	replace_word->SetValue(suggest_list->GetStringSelection());
}

bool DialogSpellChecker::FindNext() {
	AssDialogue *real_active_line = context->selectionController->GetActiveLine();
	// User has changed the active line; restart search from this position
	if (real_active_line != active_line) {
		active_line = real_active_line;
		has_looped = false;
		start_line = active_line;
	}

	int start_pos = context->editBox->GetReverseUnicodePosition(context->editBox->GetCurrentPos());
	int commit_id = -1;

	if (CheckLine(active_line, start_pos, &commit_id))
		return true;

	std::list<AssEntry*>::iterator it = find(context->ass->Line.begin(), context->ass->Line.end(), active_line);

	// Note that it is deliberate that the start line is checked twice, as if
	// the cursor is past the first misspelled word in the current line, that
	// word should be hit last
	while(!has_looped || active_line != start_line) {
		do {
			// Wrap around to the beginning if we hit the end
			if (++it == context->ass->Line.end()) {
				it = context->ass->Line.begin();
				has_looped = true;
			}
		} while (!(active_line = dynamic_cast<AssDialogue*>(*it)));

		if (CheckLine(active_line, 0, &commit_id))
			return true;
	}

	if (IsShown()) {
		wxMessageBox(_("Aegisub has finished checking spelling of this script."), _("Spell checking complete."));
		Close();
	}
	else {
		wxMessageBox(_("Aegisub has found no spelling mistakes in this script."), _("Spell checking complete."));
		throw agi::UserCancelException("No spelling mistakes");
	}

	return false;
}

bool DialogSpellChecker::CheckLine(AssDialogue *active_line, int start_pos, int *commit_id) {
	if (active_line->Comment && skip_comments->GetValue()) return false;

	IntPairVector results;
	GetWordBoundaries(active_line->Text, results);

	int shift = 0;
	for (size_t j = 0; j < results.size(); ++j) {
		word_start = results[j].first + shift;
		if (word_start < start_pos) continue;
		word_end = results[j].second + shift;
		wxString word = active_line->Text.Mid(word_start, word_end - word_start);

		if (auto_ignore.count(word) || spellchecker->CheckWord(word)) continue;

		std::map<wxString, wxString>::const_iterator auto_rep = auto_replace.find(word);
		if (auto_rep == auto_replace.end()) {
			SelectionController<AssDialogue>::Selection sel;
			sel.insert(active_line);
			context->selectionController->SetSelectionAndActive(sel, active_line);
			SetWord(word);
			return true;
		}

		active_line->Text = active_line->Text.Left(word_start) + auto_rep->second + active_line->Text.Mid(word_end);
		*commit_id = context->ass->Commit(_("spell check replace"), AssFile::COMMIT_DIAG_TEXT, *commit_id);
		shift += auto_rep->second.size() - auto_rep->first.size();
	}
	return false;
}

void DialogSpellChecker::Replace() {
	AssDialogue *active_line = context->selectionController->GetActiveLine();

	// Only replace if the user hasn't changed the selection to something else
	if (active_line->Text.Mid(word_start, word_end - word_start) == orig_word->GetValue()) {
		active_line->Text = active_line->Text.Left(word_start) + replace_word->GetValue() + active_line->Text.Mid(word_end);
		context->ass->Commit(_("spell check replace"), AssFile::COMMIT_DIAG_TEXT);
		context->editBox->SetCurrentPos(context->editBox->GetUnicodePosition(word_start + replace_word->GetValue().size()));
	}
}

void DialogSpellChecker::SetWord(wxString const& word) {
	orig_word->SetValue(word);

	wxArrayString suggestions = spellchecker->GetSuggestions(word);
	replace_word->SetValue(suggestions.size() ? suggestions[0] : word);
	suggest_list->Clear();
	suggest_list->Append(suggestions);

	context->editBox->SetSelectionU(word_start, word_end);
	context->editBox->SetCurrentPos(context->editBox->GetUnicodePosition(word_end));

	add_button->Enable(spellchecker->CanAddWord(word));
}
