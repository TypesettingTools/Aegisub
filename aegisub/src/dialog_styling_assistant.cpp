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

/// @file dialog_styling_assistant.cpp
/// @brief Styling Assistant dialogue box and logic
/// @ingroup tools_ui
///


#include "config.h"

#include "dialog_styling_assistant.h"

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "audio_controller.h"
#include "command/command.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "persist_location.h"
#include "utils.h"
#include "video_context.h"

#ifndef AGI_PRE
#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/listbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#endif

static void add_hotkey(wxSizer *sizer, wxWindow *parent, const char *command, const char *text) {
	sizer->Add(new wxStaticText(parent, -1, _(text)));
	sizer->Add(new wxStaticText(parent, -1, hotkey::get_hotkey_str_first("Styling Assistant", command)));
}

DialogStyling::DialogStyling(agi::Context *context)
: wxDialog(context->parent, -1, _("Styling assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX)
, c(context)
, active_line(0)
{
	SetIcon(BitmapToIcon(GETIMAGE(styling_toolbutton_24)));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);

	{
		wxSizer *cur_line_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Current line"));
		current_line_text = new wxTextCtrl(this, -1, _("Current line"), wxDefaultPosition, wxSize(300, 60), wxTE_MULTILINE | wxTE_READONLY);
		cur_line_box->Add(current_line_text, 1, wxEXPAND, 0);
		main_sizer->Add(cur_line_box, 0, wxEXPAND | wxALL, 5);
	}

	{
		wxSizer *styles_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Styles available"));
		style_list = new wxListBox(this, -1, wxDefaultPosition, wxSize(150, 180), context->ass->GetStyles());
		styles_box->Add(style_list, 1, wxEXPAND, 0);
		bottom_sizer->Add(styles_box, 1, wxEXPAND | wxRIGHT, 5);
	}

	wxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
	{
		wxSizer *style_text_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Set style"));
		style_name = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(180, -1), wxTE_PROCESS_ENTER);
		style_text_box->Add(style_name, 1, wxEXPAND);
		right_sizer->Add(style_text_box, 0, wxEXPAND | wxBOTTOM, 5);
	}

	{
		wxSizer *hotkey_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Keys"));

		wxSizer *hotkey_grid = new wxGridSizer(2, 0, 5);
		add_hotkey(hotkey_grid, this, "tool/styling_assistant/commit", "Accept changes:");
		add_hotkey(hotkey_grid, this, "tool/styling_assistant/preview", "Preview changes:");
		add_hotkey(hotkey_grid, this, "grid/line/prev", "Previous line:");
		add_hotkey(hotkey_grid, this, "grid/line/next", "Next line:");
		add_hotkey(hotkey_grid, this, "video/play/line", "Play Video:");
		add_hotkey(hotkey_grid, this, "audio/play/selection", "Play Audio:");
		hotkey_grid->Add(new wxStaticText(this, -1, _("Click on list:")));
		hotkey_grid->Add(new wxStaticText(this, -1, _("Select style")));

		hotkey_box->Add(hotkey_grid, 0, wxEXPAND | wxBOTTOM, 5);

		auto_seek = new wxCheckBox(this, -1, _("Seek video to line start time"));
		auto_seek->SetValue(true);
		hotkey_box->Add(auto_seek, 0, 0, 0);
		hotkey_box->AddStretchSpacer(1);

		right_sizer->Add(hotkey_box, 0, wxEXPAND | wxBOTTOM, 5);
	}

	{
		wxSizer *actions_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Actions"));
		actions_box->AddStretchSpacer(1);

		play_audio = new wxButton(this, -1, _("Play Audio"));
		play_audio->Enable(c->audioController->IsAudioOpen());
		actions_box->Add(play_audio, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

		play_video = new wxButton(this, -1, _("Play Video"));
		play_video->Enable(c->videoController->IsLoaded());
		actions_box->Add(play_video, 0, wxBOTTOM | wxRIGHT, 5);

		actions_box->AddStretchSpacer(1);
		right_sizer->Add(actions_box, 0, wxEXPAND, 5);
	}
	bottom_sizer->Add(right_sizer);
	main_sizer->Add(bottom_sizer, 1, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 5);

	{
		wxStdDialogButtonSizer *button_sizer = new wxStdDialogButtonSizer;
		button_sizer->AddButton(new wxButton(this, wxID_CANCEL));
		button_sizer->AddButton(new HelpButton(this, "Styling Assistant"));
		button_sizer->Realize();

		main_sizer->Add(button_sizer, 0, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 5);
	}

	main_sizer->SetSizeHints(this);
	SetSizer(main_sizer);

	persist.reset(new PersistLocation(this, "Tool/Styling Assistant"));

	c->selectionController->AddSelectionListener(this);
	Bind(wxEVT_ACTIVATE, &DialogStyling::OnActivate, this);
	Bind(wxEVT_KEY_DOWN, &DialogStyling::OnKeyDown, this);
	Bind(wxEVT_SHOW, &DialogStyling::OnShow, this);
	style_name->Bind(wxEVT_KEY_DOWN, &DialogStyling::OnKeyDown, this);
	play_video->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogStyling::OnPlayVideoButton, this);
	play_audio->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogStyling::OnPlayAudioButton, this);
	style_list->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &DialogStyling::OnListClicked, this);
	style_list->Bind(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, &DialogStyling::OnListDoubleClicked, this);
	style_name->Bind(wxEVT_COMMAND_TEXT_UPDATED, &DialogStyling::OnStyleBoxModified, this);

	OnActiveLineChanged(c->selectionController->GetActiveLine());
}

DialogStyling::~DialogStyling () {
	c->stylingAssistant = 0;
	c->selectionController->RemoveSelectionListener(this);
}

void DialogStyling::OnShow(wxShowEvent &evt) {
	if (evt.IsShown())
		evt.Skip();
	else
		Destroy();
}

void DialogStyling::OnActiveLineChanged(AssDialogue *new_line) {
	if (!new_line) return;
	active_line = new_line;

	current_line_text->SetValue(active_line->Text);
	style_name->SetValue(active_line->Style);
	style_name->SetSelection(0, active_line->Style.size());
	style_name->SetFocus();

	style_list->SetStringSelection(active_line->Style);

	if (auto_seek->IsChecked() && IsActive())
		c->videoController->JumpToTime(active_line->Start.GetMS());
}

void DialogStyling::Commit(bool next) {
	if (!c->ass->GetStyle(style_name->GetValue())) return;

	active_line->Style = style_name->GetValue();
	c->ass->Commit(_("styling assistant"), AssFile::COMMIT_TEXT);

	if (next) cmd::call("grid/line/next", c);
}

void DialogStyling::OnActivate(wxActivateEvent &) {
	if (!IsActive()) return;

	play_video->Enable(c->videoController->IsLoaded());
	play_audio->Enable(c->audioController->IsAudioOpen());

	style_list->Set(c->ass->GetStyles());

	if (auto_seek->IsChecked())
		c->videoController->JumpToTime(active_line->Start.GetMS());

	style_name->SetFocus();
}

void DialogStyling::OnStyleBoxModified(wxCommandEvent &event) {
	long from, to;
	style_name->GetSelection(&from, &to);
	wxString prefix = style_name->GetValue().Left(from).Lower();

	if (prefix.empty()) {
		style_name->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		return;
	}

	// Find the first style name which the contents of the box could be the
	// beginning of
	for (size_t i = 0; i < style_list->GetCount(); ++i) {
		wxString style = style_list->GetString(i);
		if (style.Lower().StartsWith(prefix)) {
			style_name->ChangeValue(style);
			style_name->SetSelection(prefix.size(), style.size());
			style_name->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
			style_name->Refresh();
			return;
		}
	}

	style_name->SetBackgroundColour(wxColour(255, 108, 108));
	style_name->Refresh();
}

void DialogStyling::OnListClicked(wxCommandEvent &evt) {
	style_name->ChangeValue(style_list->GetString(evt.GetInt()));
	Commit(false);
	style_name->SetFocus();
}

void DialogStyling::OnListDoubleClicked(wxCommandEvent &evt) {
	style_name->ChangeValue(style_list->GetString(evt.GetInt()));
	Commit(true);
	style_name->SetFocus();
}

void DialogStyling::OnPlayVideoButton(wxCommandEvent &) {
	c->videoController->PlayLine();
	style_name->SetFocus();
}

void DialogStyling::OnPlayAudioButton(wxCommandEvent &) {
	cmd::call("audio/play/selection", c);
	style_name->SetFocus();
}

void DialogStyling::OnKeyDown(wxKeyEvent &evt) {
	if (!hotkey::check("Styling Assistant", c, evt.GetKeyCode(), evt.GetUnicodeKey(), evt.GetModifiers())) {
		// Move the beginning of the selection back one character so that backspace
		// actually does something
		if (evt.GetKeyCode() == WXK_BACK && !evt.GetModifiers()) {
			long from, to;
			style_name->GetSelection(&from, &to);
			if (from > 0)
				style_name->SetSelection(from - 1, to);
		}
		evt.Skip();
	}
	evt.StopPropagation();
}
