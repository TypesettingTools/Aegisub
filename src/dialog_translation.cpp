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

/// @file dialog_translation.cpp
/// @brief Translation Assistant dialogue box and logic
/// @ingroup tools_ui
///

#include "dialog_translation.h"

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "command/command.h"
#include "compat.h"
#include "format.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "persist_location.h"
#include "project.h"
#include "subs_edit_ctrl.h"
#include "selection_controller.h"
#include "video_controller.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>

static void add_hotkey(wxSizer *sizer, wxWindow *parent, const char *command, wxString const& text) {
	sizer->Add(new wxStaticText(parent, -1, text));
	sizer->Add(new wxStaticText(parent, -1, to_wx(hotkey::get_hotkey_str_first("Translation Assistant", command))));
}

// Skip over override blocks, comments, and whitespace between blocks
static bool bad_block(std::unique_ptr<AssDialogueBlock> &block) {
	bool is_whitespace = boost::all(block->GetText(), boost::is_space());
	return block->GetType() != AssBlockType::PLAIN || (is_whitespace && OPT_GET("Tool/Translation Assistant/Skip Whitespace")->GetBool());
}

DialogTranslation::DialogTranslation(agi::Context *c)
: wxDialog(c->parent, -1, _("Translation Assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX)
, c(c)
, file_change_connection(c->ass->AddCommitListener(&DialogTranslation::OnExternalCommit, this))
, active_line_connection(c->selectionController->AddActiveLineListener(&DialogTranslation::OnActiveLineChanged, this))
, active_line(c->selectionController->GetActiveLine())
, line_count(c->ass->Events.size())
{
	SetIcons(GETICONS(translation_toolbutton));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxSizer *translation_sizer = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer *original_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Original"));

		line_number_display = new wxStaticText(original_box->GetStaticBox(), -1, "");
		original_box->Add(line_number_display, 0, wxBOTTOM, 5);

		original_text = new wxStyledTextCtrl(original_box->GetStaticBox(), -1, wxDefaultPosition, wxSize(320, 80));
		original_text->SetWrapMode(wxSTC_WRAP_WORD);
		original_text->SetMarginWidth(1, 0);
		original_text->StyleSetForeground(1, wxColour(10, 60, 200));
		original_text->SetReadOnly(true);
		original_box->Add(original_text, 1, wxEXPAND, 0);

		translation_sizer->Add(original_box, 1, wxEXPAND, 0);
	}

	{
		wxStaticBoxSizer *translated_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Translation"));

		translated_text = new SubsTextEditCtrl(translated_box->GetStaticBox(), wxSize(320, 80), 0, nullptr);
		translated_text->SetWrapMode(wxSTC_WRAP_WORD);
		translated_text->SetMarginWidth(1, 0);
		translated_text->SetFocus();
		translated_text->Bind(wxEVT_CHAR_HOOK, &DialogTranslation::OnKeyDown, this);
		translated_text->CmdKeyAssign(wxSTC_KEY_RETURN, wxSTC_KEYMOD_SHIFT, wxSTC_CMD_NEWLINE);

		translated_box->Add(translated_text, 1, wxEXPAND, 0);
		translation_sizer->Add(translated_box, 1, wxTOP|wxEXPAND, 5);
	}
	main_sizer->Add(translation_sizer, 1, wxALL | wxEXPAND, 5);

	wxSizer *right_box = new wxBoxSizer(wxHORIZONTAL);
	{
		wxStaticBoxSizer *hotkey_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Keys"));
		wxWindow *hotkey_sizer_box = hotkey_sizer->GetStaticBox();

		wxSizer *hotkey_grid = new wxGridSizer(2, 0, 5);
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/translation_assistant/commit", _("Accept changes"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/translation_assistant/preview", _("Preview changes"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/translation_assistant/prev", _("Previous line"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/translation_assistant/next", _("Next line"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/translation_assistant/insert_original", _("Insert original"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "video/play/line", _("Play video"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "audio/play/selection", _("Play audio"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "edit/line/delete", _("Delete line"));
		hotkey_sizer->Add(hotkey_grid, 0, wxEXPAND, 0);

		seek_video = new wxCheckBox(hotkey_sizer_box, -1, _("Enable &preview"));
		seek_video->SetValue(true);
		hotkey_sizer->Add(seek_video, 0, wxTOP, 5);

		right_box->Add(hotkey_sizer, 1, wxRIGHT | wxEXPAND, 5);
	}

	{
		wxStaticBoxSizer *actions_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Actions"));

		wxButton *play_audio = new wxButton(actions_box->GetStaticBox(), -1, _("Play &Audio"));
		play_audio->Enable(!!c->project->AudioProvider());
		play_audio->Bind(wxEVT_BUTTON, &DialogTranslation::OnPlayAudioButton, this);
		actions_box->Add(play_audio, 0, wxALL, 5);

		wxButton *play_video = new wxButton(actions_box->GetStaticBox(), -1, _("Play &Video"));
		play_video->Enable(!!c->project->VideoProvider());
		play_video->Bind(wxEVT_BUTTON, &DialogTranslation::OnPlayVideoButton, this);
		actions_box->Add(play_video, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

		right_box->Add(actions_box, 0);
	}
	main_sizer->Add(right_box, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

	{
		auto standard_buttons = new wxStdDialogButtonSizer();
		standard_buttons->AddButton(new wxButton(this, wxID_CANCEL));
		standard_buttons->AddButton(new HelpButton(this, "Translation Assistant"));
		standard_buttons->Realize();
		main_sizer->Add(standard_buttons, 0, wxALIGN_RIGHT | wxLEFT | wxBOTTOM | wxRIGHT, 5);
	}

	SetSizerAndFit(main_sizer);

	persist = std::make_unique<PersistLocation>(this, "Tool/Translation Assistant");

	Bind(wxEVT_KEY_DOWN, &DialogTranslation::OnKeyDown, this);

	blocks = active_line->ParseTags();
	if (bad_block(blocks[0])) {
		if (!NextBlock())
			throw NothingToTranslate();
	}
	else
		UpdateDisplay();
}

DialogTranslation::~DialogTranslation() { }

void DialogTranslation::OnActiveLineChanged(AssDialogue *new_line) {
	if (switching_lines) return;

	active_line = new_line;
	blocks = active_line->ParseTags();
	cur_block = 0;

	if (bad_block(blocks[cur_block]) && !NextBlock()) {
		wxMessageBox(_("No more lines to translate."));
		EndModal(1);
	}
}

void DialogTranslation::OnExternalCommit(int commit_type) {
	if (commit_type == AssFile::COMMIT_NEW || commit_type & AssFile::COMMIT_DIAG_ADDREM) {
		line_count = c->ass->Events.size();
		line_number_display->SetLabel(fmt_tl("Current line: %d/%d", active_line->Row + 1, line_count));
	}

	if (commit_type & AssFile::COMMIT_DIAG_TEXT)
		OnActiveLineChanged(active_line);
}

bool DialogTranslation::NextBlock() {
	switching_lines = true;
	do {
		if (cur_block == blocks.size() - 1) {
			c->selectionController->NextLine();
			AssDialogue *new_line = c->selectionController->GetActiveLine();
			if (active_line == new_line || !new_line) return false;

			active_line = new_line;
			blocks = active_line->ParseTags();
			cur_block = 0;
		}
		else
			++cur_block;
	} while (bad_block(blocks[cur_block]));
	switching_lines = false;

	UpdateDisplay();
	return true;
}

bool DialogTranslation::PrevBlock() {
	switching_lines = true;
	do {
		if (cur_block == 0) {
			c->selectionController->PrevLine();
			AssDialogue *new_line = c->selectionController->GetActiveLine();
			if (active_line == new_line || !new_line) return false;

			active_line = new_line;
			blocks = active_line->ParseTags();
			cur_block = blocks.size() - 1;
		}
		else
			--cur_block;
	} while (bad_block(blocks[cur_block]));
	switching_lines = false;

	UpdateDisplay();
	return true;
}

void DialogTranslation::UpdateDisplay() {
	line_number_display->SetLabel(fmt_tl("Current line: %d/%d", active_line->Row, line_count));

	original_text->SetReadOnly(false);
	original_text->ClearAll();

	size_t i = 0;
	for (auto& block : blocks) {
		if (block->GetType() == AssBlockType::PLAIN) {
			int initial_pos = original_text->GetLength();
			original_text->AppendTextRaw(block->GetText().c_str());
			if (i == cur_block) {
				original_text->StartStyling(initial_pos);
				original_text->SetStyling(block->GetText().size(), 1);
			}
		}
		else
			original_text->AppendTextRaw(block->GetText().c_str());
		++i;
	}

	original_text->SetReadOnly(true);

	if (seek_video->IsChecked()) c->videoController->JumpToTime(active_line->Start);

	translated_text->ClearAll();
	translated_text->SetFocus();
}

void DialogTranslation::Commit(bool next) {
	std::string new_value = translated_text->GetTextRaw().data();
	boost::replace_all(new_value, "\r\n", "\\N");
	boost::replace_all(new_value, "\r", "\\N");
	boost::replace_all(new_value, "\n", "\\N");
	*blocks[cur_block] = AssDialogueBlockPlain(new_value);
	active_line->UpdateText(blocks);

	file_change_connection.Block();
	c->ass->Commit(_("translation assistant"), AssFile::COMMIT_DIAG_TEXT);
	file_change_connection.Unblock();

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
	auto const& text = blocks[cur_block]->GetText();
	translated_text->AddTextRaw(text.data(), text.size());
}

void DialogTranslation::OnKeyDown(wxKeyEvent &evt) {
	hotkey::check("Translation Assistant", c, evt);
}

void DialogTranslation::OnPlayVideoButton(wxCommandEvent &) {
	c->videoController->PlayLine();
	translated_text->SetFocus();
}

void DialogTranslation::OnPlayAudioButton(wxCommandEvent &) {
	cmd::call("audio/play/selection", c);
	translated_text->SetFocus();
}
