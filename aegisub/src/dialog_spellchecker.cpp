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

/// @file dialog_spellchecker.cpp
/// @brief Spell checker dialogue box
/// @ingroup spelling
///

#include "config.h"

#include "dialog_spellchecker.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "include/aegisub/spellchecker.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "selection_controller.h"
#include "text_selection_controller.h"
#include "subs_edit_ctrl.h"
#include "utils.h"

#include <libaegisub/ass/dialogue_parser.h>
#include <libaegisub/exception.h>
#include <libaegisub/spellchecker.h>

#include <boost/locale/conversion.hpp>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

DialogSpellChecker::DialogSpellChecker(agi::Context *context)
: wxDialog(context->parent, -1, _("Spell Checker"))
, context(context)
, spellchecker(SpellCheckerFactory::GetSpellChecker())
, start_line(nullptr)
, active_line(nullptr)
, has_looped(false)
{
	SetIcon(GETICON(spellcheck_toolbutton_16));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	auto current_word_sizer = new wxFlexGridSizer(2, 5, 5);
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

	replace_word->Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent&) {
		remove_button->Enable(spellchecker->CanRemoveWord(from_wx(replace_word->GetValue())));
	});

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

		dictionary_lang_codes = to_wx(spellchecker->GetLanguageList());
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
		wxString cur_lang = to_wx(OPT_GET("Tool/Spell Checker/Language")->GetString());
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

		auto make_checkbox = [&](wxString const& text, const char *opt) {
			auto checkbox = new wxCheckBox(this, -1, text);
			actions_sizer->Add(checkbox, button_flags);
			checkbox->SetValue(OPT_GET(opt)->GetBool());
			checkbox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED,
				[=](wxCommandEvent &evt) { OPT_SET(opt)->SetBool(!!evt.GetInt()); });
		};

		make_checkbox(_("&Skip Comments"), "Tool/Spell Checker/Skip Comments");
		make_checkbox(_("Ignore &UPPERCASE words"), "Tool/Spell Checker/Skip Uppercase");

		wxButton *button;

		actions_sizer->Add(button = new wxButton(this, -1, _("&Replace")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogSpellChecker::OnReplace, this);

		actions_sizer->Add(button = new wxButton(this, -1, _("Replace &all")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) {
			auto_replace[from_wx(orig_word->GetValue())] = from_wx(replace_word->GetValue());
			Replace();
			FindNext();
		});

		actions_sizer->Add(button = new wxButton(this, -1, _("&Ignore")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) { FindNext(); });

		actions_sizer->Add(button = new wxButton(this, -1, _("Ignore a&ll")), button_flags);
		button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) {
			auto_ignore.insert(from_wx(orig_word->GetValue()));
			FindNext();
		});

		actions_sizer->Add(add_button = new wxButton(this, -1, _("Add to &dictionary")), button_flags);
		add_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) {
			spellchecker->AddWord(from_wx(orig_word->GetValue()));
			FindNext();
		});

		actions_sizer->Add(remove_button = new wxButton(this, -1, _("Remove fro&m dictionary")), button_flags);
		remove_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) {
			spellchecker->RemoveWord(from_wx(replace_word->GetValue()));
			SetWord(from_wx(orig_word->GetValue()));
		});

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

void DialogSpellChecker::OnChangeLanguage(wxCommandEvent&) {
	wxString code = dictionary_lang_codes[language->GetSelection()];
	OPT_SET("Tool/Spell Checker/Language")->SetString(from_wx(code));

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

	int start_pos = context->textSelectionController->GetInsertionPoint();
	int commit_id = -1;

	if (CheckLine(active_line, start_pos, &commit_id))
		return true;

	entryIter it = context->ass->Line.iterator_to(*active_line);

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
		} while (!(active_line = dynamic_cast<AssDialogue*>(&*it)));

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
	if (active_line->Comment && OPT_GET("Tool/Spell Checker/Skip Comments")->GetBool()) return false;

	std::string text = active_line->Text;
	auto tokens = agi::ass::TokenizeDialogueBody(text);
	agi::ass::SplitWords(text, tokens);

	bool ignore_uppercase = OPT_GET("Tool/Spell Checker/Skip Uppercase")->GetBool();

	word_start = 0;
	for (auto const& tok : tokens) {
		if (tok.type != agi::ass::DialogueTokenType::WORD || word_start < start_pos) {
			word_start += tok.length;
			continue;
		}

		word_len = tok.length;
		std::string word = text.substr(word_start, word_len);

		if (auto_ignore.count(word) || spellchecker->CheckWord(word) || (ignore_uppercase && word == boost::locale::to_upper(word))) {
			word_start += tok.length;
			continue;
		}

		auto auto_rep = auto_replace.find(word);
		if (auto_rep == auto_replace.end()) {
#ifdef __WXGTK__
			// http://trac.wxwidgets.org/ticket/14369
			orig_word->Remove(0, -1);
			replace_word->Remove(0, -1);
#endif

			SubtitleSelection sel;
			sel.insert(active_line);
			context->selectionController->SetSelectionAndActive(sel, active_line);
			SetWord(word);
			return true;
		}

		text.replace(word_start, word_len, auto_rep->second);
		active_line->Text = text;
		*commit_id = context->ass->Commit(_("spell check replace"), AssFile::COMMIT_DIAG_TEXT, *commit_id);
		word_start += auto_rep->second.size();
	}
	return false;
}

void DialogSpellChecker::Replace() {
	AssDialogue *active_line = context->selectionController->GetActiveLine();

	// Only replace if the user hasn't changed the selection to something else
	if (to_wx(active_line->Text.get().substr(word_start, word_len)) == orig_word->GetValue()) {
		std::string text = active_line->Text;
		text.replace(word_start, word_len, from_wx(replace_word->GetValue()));
		active_line->Text = text;
		context->ass->Commit(_("spell check replace"), AssFile::COMMIT_DIAG_TEXT);
		context->textSelectionController->SetInsertionPoint(word_start + replace_word->GetValue().size());
	}
}

void DialogSpellChecker::SetWord(std::string const& word) {
	orig_word->SetValue(to_wx(word));

	wxArrayString suggestions = to_wx(spellchecker->GetSuggestions(word));
	replace_word->SetValue(suggestions.size() ? suggestions[0] : to_wx(word));
	suggest_list->Clear();
	suggest_list->Append(suggestions);

	context->textSelectionController->SetSelection(word_start, word_start + word_len);
	context->textSelectionController->SetInsertionPoint(word_start + word_len);

	add_button->Enable(spellchecker->CanAddWord(word));
}
