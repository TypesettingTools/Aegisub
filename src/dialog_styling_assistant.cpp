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

/// @file dialog_styling_assistant.cpp
/// @brief Styling Assistant dialogue box and logic
/// @ingroup tools_ui
///

#include "dialog_styling_assistant.h"

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "command/command.h"
#include "compat.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "persist_location.h"
#include "project.h"
#include "selection_controller.h"
#include "video_controller.h"

#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/event.h>
#include <wx/listbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

static void add_hotkey(wxSizer *sizer, wxWindow *parent, const char *command, wxString const& text) {
	sizer->Add(new wxStaticText(parent, -1, text));
	sizer->Add(new wxStaticText(parent, -1, to_wx(hotkey::get_hotkey_str_first("Styling Assistant", command))));
}

DialogStyling::DialogStyling(agi::Context *context)
: wxDialog(context->parent, -1, _("Styling Assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX)
, c(context)
, active_line_connection(context->selectionController->AddActiveLineListener(&DialogStyling::OnActiveLineChanged, this))
{
	SetIcons(GETICONS(styling_toolbutton));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);

	{
		wxStaticBoxSizer *cur_line_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Current line"));
		current_line_text = new wxTextCtrl(cur_line_box->GetStaticBox(), -1, _("Current line"), wxDefaultPosition, wxSize(300, 60), wxTE_MULTILINE | wxTE_READONLY);
		cur_line_box->Add(current_line_text, wxSizerFlags(1).Expand());
		main_sizer->Add(cur_line_box, wxSizerFlags().Expand().Border());
	}

	{
		wxStaticBoxSizer *styles_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Styles available"));
		style_list = new wxListBox(styles_box->GetStaticBox(), -1, wxDefaultPosition, wxSize(150, 180), to_wx(context->ass->GetStyles()));
		styles_box->Add(style_list, wxSizerFlags(1).Expand());
		bottom_sizer->Add(styles_box, wxSizerFlags(1).Expand().Border(wxRIGHT));
	}

	wxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer *style_text_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Set style"));
		style_name = new wxTextCtrl(style_text_box->GetStaticBox(), -1, "", wxDefaultPosition, wxSize(180, -1), wxTE_PROCESS_ENTER);
		style_text_box->Add(style_name, wxSizerFlags(1).Expand());
		right_sizer->Add(style_text_box, wxSizerFlags().Expand().Border(wxBOTTOM));
	}

	{
		wxStaticBoxSizer *hotkey_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Keys"));
		wxWindow *hotkey_sizer_box = hotkey_sizer->GetStaticBox();

		wxSizer *hotkey_grid = new wxGridSizer(2, 0, 5);
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/styling_assistant/commit", _("Accept changes"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "tool/styling_assistant/preview", _("Preview changes"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "grid/line/prev", _("Previous line"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "grid/line/next", _("Next line"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "video/play/line", _("Play video"));
		add_hotkey(hotkey_grid, hotkey_sizer_box, "audio/play/selection", _("Play audio"));
		hotkey_grid->Add(new wxStaticText(hotkey_sizer_box, -1, _("Click on list")));
		hotkey_grid->Add(new wxStaticText(hotkey_sizer_box, -1, _("Select style")));

		hotkey_sizer->Add(hotkey_grid, wxSizerFlags().Expand().Border(wxBOTTOM));

		auto_seek = new wxCheckBox(hotkey_sizer_box, -1, _("&Seek video to line start time"));
		auto_seek->SetValue(true);
		hotkey_sizer->Add(auto_seek);
		hotkey_sizer->AddStretchSpacer(1);

		right_sizer->Add(hotkey_sizer, wxSizerFlags().Expand().Border(wxBOTTOM));
	}

	{
		wxStaticBoxSizer *actions_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Actions"));
		actions_box->AddStretchSpacer(1);

		play_audio = new wxButton(actions_box->GetStaticBox(), -1, _("Play &Audio"));
		play_audio->Enable(!!c->project->AudioProvider());
		actions_box->Add(play_audio, wxSizerFlags().Border(wxALL & ~wxTOP));

		play_video = new wxButton(actions_box->GetStaticBox(), -1, _("Play &Video"));
		play_video->Enable(!!c->project->VideoProvider());
		actions_box->Add(play_video, wxSizerFlags().Border(wxBOTTOM | wxRIGHT));

		actions_box->AddStretchSpacer(1);
		right_sizer->Add(actions_box, wxSizerFlags().Expand());
	}
	bottom_sizer->Add(right_sizer);
	main_sizer->Add(bottom_sizer, wxSizerFlags(1).Expand().Border(wxALL & ~wxTOP));

	{
		auto button_sizer = new wxStdDialogButtonSizer;
		button_sizer->AddButton(new wxButton(this, wxID_CANCEL));
		button_sizer->AddButton(new HelpButton(this, "Styling Assistant"));
		button_sizer->Realize();

		main_sizer->Add(button_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	}

	SetSizerAndFit(main_sizer);

	persist = std::make_unique<PersistLocation>(this, "Tool/Styling Assistant");

	Bind(wxEVT_ACTIVATE, &DialogStyling::OnActivate, this);
	Bind(wxEVT_CHAR_HOOK, &DialogStyling::OnCharHook, this);
	style_name->Bind(wxEVT_CHAR_HOOK, &DialogStyling::OnCharHook, this);
	style_name->Bind(wxEVT_KEY_DOWN, &DialogStyling::OnKeyDown, this);
	play_video->Bind(wxEVT_BUTTON, &DialogStyling::OnPlayVideoButton, this);
	play_audio->Bind(wxEVT_BUTTON, &DialogStyling::OnPlayAudioButton, this);
	style_list->Bind(wxEVT_LISTBOX, &DialogStyling::OnListClicked, this);
	style_list->Bind(wxEVT_LISTBOX_DCLICK, &DialogStyling::OnListDoubleClicked, this);
	style_name->Bind(wxEVT_TEXT, &DialogStyling::OnStyleBoxModified, this);

	OnActiveLineChanged(c->selectionController->GetActiveLine());
}

DialogStyling::~DialogStyling () {
}

void DialogStyling::OnActiveLineChanged(AssDialogue *new_line) {
	if (!new_line) return;
	active_line = new_line;

	current_line_text->SetValue(to_wx(active_line->Text));
	style_name->SetValue(to_wx(active_line->Style));
	style_name->SetSelection(0, active_line->Style.get().size());
	style_name->SetFocus();

	style_list->SetStringSelection(to_wx(active_line->Style));

	if (auto_seek->IsChecked() && IsActive())
		c->videoController->JumpToTime(active_line->Start);
}

void DialogStyling::Commit(bool next) {
	if (!c->ass->GetStyle(from_wx(style_name->GetValue()))) return;

	active_line->Style = from_wx(style_name->GetValue());
	c->ass->Commit(_("styling assistant"), AssFile::COMMIT_DIAG_META);

	if (next) cmd::call("grid/line/next", c);
}

void DialogStyling::OnActivate(wxActivateEvent &) {
	if (!IsActive()) return;

	play_video->Enable(!!c->project->VideoProvider());
	play_audio->Enable(!!c->project->AudioProvider());

	style_list->Set(to_wx(c->ass->GetStyles()));

	if (auto_seek->IsChecked())
		c->videoController->JumpToTime(active_line->Start);

	style_name->SetFocus();
}

void DialogStyling::OnStyleBoxModified(wxCommandEvent &) {
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

void DialogStyling::OnCharHook(wxKeyEvent &evt) {
	hotkey::check("Styling Assistant", c, evt);
}

void DialogStyling::OnKeyDown(wxKeyEvent &evt) {
	// Move the beginning of the selection back one character so that backspace
	// actually does something
	if (evt.GetKeyCode() == WXK_BACK && !evt.GetModifiers()) {
		long from, to;
		style_name->GetSelection(&from, &to);
		if (from > 0)
			style_name->SetSelection(from - 1, to);
	}
	else
		evt.Skip();
}
