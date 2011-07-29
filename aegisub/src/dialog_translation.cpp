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

/// @file dialog_translation.cpp
/// @brief Translation Assistant dialogue box and logic
/// @ingroup tools_ui
///

#include "config.h"

#include "dialog_translation.h"

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "audio_controller.h"
#include "command/command.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "persist_location.h"
#include "scintilla_text_ctrl.h"
#include "selection_controller.h"
#include "utils.h"
#include "video_context.h"

#ifndef AGI_PRE
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#endif

static void add_hotkey(wxSizer *sizer, wxWindow *parent, const char *command, const char *text) {
	sizer->Add(new wxStaticText(parent, -1, _(text)));
	sizer->Add(new wxStaticText(parent, -1, hotkey::get_hotkey_str_first("Translation Assistant", command)));
}

DialogTranslation::DialogTranslation(agi::Context *c)
: wxDialog(c->parent, -1, _("Translation Assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX, "TranslationAssistant")
, c(c)
, active_line(c->selectionController->GetActiveLine())
, cur_block(0)
, line_count(count_if(c->ass->Line.begin(), c->ass->Line.end(), cast<AssDialogue*>()))
, line_number(count_if(c->ass->Line.begin(), find(c->ass->Line.begin(), c->ass->Line.end(), active_line), cast<AssDialogue*>()) + 1)
{
	SetIcon(BitmapToIcon(GETIMAGE(translation_toolbutton_16)));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxSizer *translation_sizer = new wxBoxSizer(wxVERTICAL);
	{
		wxSizer *original_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Original"));

		line_number_display = new wxStaticText(this, -1, "");
		original_box->Add(line_number_display, 0, wxBOTTOM, 5);

		original_text = new ScintillaTextCtrl(this, -1, "", wxDefaultPosition, wxSize(320, 80));
		original_text->SetWrapMode(wxSTC_WRAP_WORD);
		original_text->SetMarginWidth(1, 0);
		original_text->StyleSetForeground(1, wxColour(10, 60, 200));
		original_text->SetReadOnly(true);
		original_box->Add(original_text, 1, wxEXPAND, 0);

		translation_sizer->Add(original_box, 1, wxEXPAND, 0);
	}

	{
		translated_text = new ScintillaTextCtrl(this, -1, "", wxDefaultPosition, wxSize(320, 80));
		translated_text->SetWrapMode(wxSTC_WRAP_WORD);
		translated_text->SetMarginWidth(1, 0);
		translated_text->SetFocus();
		translated_text->Bind(wxEVT_KEY_DOWN, &DialogTranslation::OnKeyDown, this);

		wxSizer *translated_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Translation"));
		translated_box->Add(translated_text, 1, wxEXPAND, 0);
		translation_sizer->Add(translated_box, 1, wxTOP|wxEXPAND, 5);
	}
	main_sizer->Add(translation_sizer, 1, wxALL | wxEXPAND, 5);

	wxSizer *right_box = new wxBoxSizer(wxHORIZONTAL);
	{
		wxSizer *hotkey_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Keys"));

		wxSizer *hotkey_grid = new wxGridSizer(2, 0, 5);
		add_hotkey(hotkey_grid, this, "tool/translation_assistant/commit", "Accept changes");
		add_hotkey(hotkey_grid, this, "tool/translation_assistant/preview", "Preview changes");
		add_hotkey(hotkey_grid, this, "tool/translation_assistant/prev", "Previous line");
		add_hotkey(hotkey_grid, this, "tool/translation_assistant/next", "Next line");
		add_hotkey(hotkey_grid, this, "tool/translation_assistant/insert_original", "Insert original");
		add_hotkey(hotkey_grid, this, "video/play/line", "Play Video");
		add_hotkey(hotkey_grid, this, "audio/play/selection", "Play Audio");
		hotkey_box->Add(hotkey_grid, 0, wxEXPAND, 0);

		seek_video = new wxCheckBox(this, -1, _("Enable preview"));
		seek_video->SetValue(true);
		hotkey_box->Add(seek_video, 0, wxTOP, 5);

		right_box->Add(hotkey_box, 1, wxRIGHT | wxEXPAND, 5);
	}

	{
		wxStaticBoxSizer *actions_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Actions"));

		wxButton *play_audio = new wxButton(this, -1, _("Play Audio"));
		play_audio->Enable(c->audioController->IsAudioOpen());
		play_audio->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogTranslation::OnPlayAudioButton, this);
		actions_box->Add(play_audio, 0, wxALL, 5);

		wxButton *play_video = new wxButton(this, -1, _("Play Video"));
		play_video->Enable(c->videoController->IsLoaded());
		play_video->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogTranslation::OnPlayVideoButton, this);
		actions_box->Add(play_video, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

		right_box->Add(actions_box, 0);
	}
	main_sizer->Add(right_box, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

	{
		wxStdDialogButtonSizer *standard_buttons = new wxStdDialogButtonSizer();
		standard_buttons->AddButton(new wxButton(this, wxID_CANCEL));
		standard_buttons->AddButton(new HelpButton(this, "Translation Assistant"));
		standard_buttons->Realize();
		main_sizer->Add(standard_buttons, 0, wxALIGN_RIGHT | wxLEFT | wxBOTTOM | wxRIGHT, 5);
	}

	SetSizer(main_sizer);
	main_sizer->SetSizeHints(this);

	persist.reset(new PersistLocation(this, "Tool/Translation Assistant"));

	Bind(wxEVT_KEY_DOWN, &DialogTranslation::OnKeyDown, this);
	active_line->ParseASSTags();
	UpdateDisplay();
}

DialogTranslation::~DialogTranslation() { }

// Skip over override blocks, comments, and whitespace between blocks
static bool bad_block(AssDialogueBlock *block) {
	if (block->GetType() != BLOCK_PLAIN) return true;
	wxString text = block->GetText();
	if (text.Trim().Trim(false).empty()) return true;
	if (text[0] == '{' && text.Last() == '}') return true;
	return false;
}

bool DialogTranslation::NextBlock() {
	do {
		if (cur_block == active_line->Blocks.size() - 1) {
			c->selectionController->NextLine();
			AssDialogue *new_line = c->selectionController->GetActiveLine();
			if (active_line == new_line || !new_line) return false;

			active_line->ClearBlocks();
			active_line = new_line;
			active_line->ParseASSTags();
			cur_block = 0;
			++line_number;
		}
		else
			++cur_block;
	} while (bad_block(active_line->Blocks[cur_block]));

	UpdateDisplay();
	return true;
}

bool DialogTranslation::PrevBlock() {
	do {
		if (cur_block == 0) {
			c->selectionController->PrevLine();
			AssDialogue *new_line = c->selectionController->GetActiveLine();
			if (active_line == new_line || !new_line) return false;

			active_line->ClearBlocks();
			active_line = new_line;
			active_line->ParseASSTags();
			cur_block = active_line->Blocks.size() - 1;
			--line_number;
		}
		else
			--cur_block;
	} while (bad_block(active_line->Blocks[cur_block]));

	UpdateDisplay();
	return true;
}

void DialogTranslation::UpdateDisplay() {
	line_number_display->SetLabel(wxString::Format(_("Current line: %d/%d"), line_number, line_count));

	original_text->SetReadOnly(false);
	original_text->ClearAll();

	for (size_t i = 0; i < active_line->Blocks.size(); ++i) {
		AssDialogueBlock *block = active_line->Blocks[i];
		if (block->GetType() == BLOCK_PLAIN) {
			int cur_size = original_text->GetReverseUnicodePosition(original_text->GetLength());
			original_text->AppendText(block->text);
			if (i == cur_block) {
				original_text->StartUnicodeStyling(cur_size);
				original_text->SetUnicodeStyling(cur_size, block->text.size(), 1);
			}
		}
		else if (block->GetType() == BLOCK_OVERRIDE)
			original_text->AppendText("{" + block->text + "}");
	}

	original_text->SetReadOnly(true);

	if (seek_video->IsChecked()) c->videoController->JumpToTime(active_line->Start.GetMS());

	translated_text->ClearAll();
	translated_text->SetFocus();
}

void DialogTranslation::Commit(bool next) {
	active_line->Blocks[cur_block]->text = translated_text->GetValue();
	active_line->UpdateText();
	c->ass->Commit(_("translation assistant"), AssFile::COMMIT_TEXT);

	if (next) {
		if (!NextBlock()) {
			wxMessageBox(_("No more lines to translate."));
			EndModal(1);
		}
	}
	else {
		UpdateDisplay();
	}
}

void DialogTranslation::InsertOriginal() {
	translated_text->AddText(active_line->Blocks[cur_block]->GetText());
}


void DialogTranslation::OnKeyDown(wxKeyEvent &evt) {
	if (!hotkey::check("Translation Assistant", c, evt.GetKeyCode(), evt.GetUnicodeKey(), evt.GetModifiers()))
		evt.Skip();
	evt.StopPropagation();
}

void DialogTranslation::OnPlayVideoButton(wxCommandEvent &) {
	c->videoController->PlayLine();
	translated_text->SetFocus();
}

void DialogTranslation::OnPlayAudioButton(wxCommandEvent &) {
	cmd::call("audio/play/selection", c);
	translated_text->SetFocus();
}
