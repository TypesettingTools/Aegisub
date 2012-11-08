// Copyright (c) 2005, Rodrigo Braz Monteiro
// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file subs_edit_box.cpp
/// @brief Main subtitle editing area, including toolbars around the text control
/// @ingroup main_ui

#include "config.h"

#include <functional>
#include <unordered_set>

#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/hashset.h>
#include <wx/radiobut.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>

#include "subs_edit_box.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "command/command.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "initial_line_state.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "placeholder_ctrl.h"
#include "scintilla_text_selection_controller.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "timeedit_ctrl.h"
#include "tooltip_manager.h"
#include "utils.h"
#include "validators.h"
#include "video_context.h"

#include <libaegisub/of_type_adaptor.h>

namespace {

/// Work around wxGTK's fondness for generating events from ChangeValue
void change_value(wxTextCtrl *ctrl, wxString const& value) {
	if (value != ctrl->GetValue())
		ctrl->ChangeValue(value);
}

void time_edit_char_hook(wxKeyEvent &event) {
	// Force a modified event on Enter
	if (event.GetKeyCode() == WXK_RETURN) {
		TimeEdit *edit = static_cast<TimeEdit*>(event.GetEventObject());
		edit->SetValue(edit->GetValue());
	}
	else
		event.Skip();
}

}

SubsEditBox::SubsEditBox(wxWindow *parent, agi::Context *context)
: wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxRAISED_BORDER, "SubsEditBox")
, line(0)
, button_bar_split(true)
, controls_enabled(true)
, c(context)
, commit_id(-1)
, undo_timer(GetEventHandler())
{
	using std::bind;

	// Top controls
	top_sizer = new wxBoxSizer(wxHORIZONTAL);

	comment_box = new wxCheckBox(this,-1,_("&Comment"));
	comment_box->SetToolTip(_("Comment this line out. Commented lines don't show up on screen."));
#ifdef __WXGTK__
	// Only supported in wxgtk
	comment_box->SetCanFocus(false);
#endif
	top_sizer->Add(comment_box, 0, wxRIGHT | wxALIGN_CENTER, 5);

	style_box = MakeComboBox("Default", wxCB_READONLY, &SubsEditBox::OnStyleChange, _("Style for this line"));

	actor_box = new Placeholder<wxComboBox>(this, _("Actor"), wxSize(110, -1), wxCB_DROPDOWN | wxTE_PROCESS_ENTER, _("Actor name for this speech. This is only for reference, and is mainly useless."));
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnActorChange, this, actor_box->GetId());
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnActorChange, this, actor_box->GetId());
	top_sizer->Add(actor_box, wxSizerFlags(2).Center().Border(wxRIGHT));

	effect_box = new Placeholder<wxComboBox>(this, _("Effect"), wxSize(80,-1), wxCB_DROPDOWN | wxTE_PROCESS_ENTER, _("Effect for this line. This can be used to store extra information for karaoke scripts, or for the effects supported by the renderer."));
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnEffectChange, this, effect_box->GetId());
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnEffectChange, this, effect_box->GetId());
	top_sizer->Add(effect_box, 3, wxALIGN_CENTER, 5);

	char_count = new wxTextCtrl(this, -1, "0", wxDefaultPosition, wxSize(30, -1), wxTE_READONLY | wxTE_CENTER);
	char_count->SetToolTip(_("Number of characters in the longest line of this subtitle."));
	top_sizer->Add(char_count, 0, wxALIGN_CENTER, 5);

	// Middle controls
	middle_left_sizer = new wxBoxSizer(wxHORIZONTAL);

	layer = new wxSpinCtrl(this,-1,"",wxDefaultPosition,wxSize(50,-1), wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER,0,0x7FFFFFFF,0);
	layer->SetToolTip(_("Layer number"));
	middle_left_sizer->Add(layer, wxSizerFlags().Center());
	middle_left_sizer->AddSpacer(5);

	start_time = MakeTimeCtrl(_("Start time"), TIME_START);
	end_time   = MakeTimeCtrl(_("End time"), TIME_END);
	middle_left_sizer->AddSpacer(5);
	duration   = MakeTimeCtrl(_("Line duration"), TIME_DURATION);
	middle_left_sizer->AddSpacer(5);

	margin[0] = MakeMarginCtrl(_("Left Margin (0 = default)"), 0, _("left margin change"));
	margin[1] = MakeMarginCtrl(_("Right Margin (0 = default)"), 1, _("right margin change"));
	margin[2] = MakeMarginCtrl(_("Vertical Margin (0 = default)"), 2, _("vertical margin change"));
	middle_left_sizer->AddSpacer(5);

	// Middle-bottom controls
	middle_right_sizer = new wxBoxSizer(wxHORIZONTAL);
	MakeButton("edit/style/bold");
	MakeButton("edit/style/italic");
	MakeButton("edit/style/underline");
	MakeButton("edit/style/strikeout");
	MakeButton("edit/font");
	middle_right_sizer->AddSpacer(5);
	MakeButton("edit/color/primary");
	MakeButton("edit/color/secondary");
	MakeButton("edit/color/outline");
	MakeButton("edit/color/shadow");
	middle_right_sizer->AddSpacer(5);
	MakeButton("grid/line/next/create");
	middle_right_sizer->AddSpacer(10);

	by_time = MakeRadio(_("T&ime"), true, _("Time by h:mm:ss.cs"));
	by_frame = MakeRadio(_("F&rame"), false, _("Time by frame number"));
	by_frame->Enable(false);

	split_box = new wxCheckBox(this,-1,_("Split"));
	split_box->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SubsEditBox::OnSplit, this);
	middle_right_sizer->Add(split_box, wxSizerFlags().Center().Left());

	// Main sizer
	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(top_sizer,0,wxEXPAND | wxALL,3);
	main_sizer->Add(middle_left_sizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	main_sizer->Add(middle_right_sizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);

	// Text editor
	edit_ctrl = new SubsTextEditCtrl(this, wxSize(300,50), wxBORDER_SUNKEN, c);
	edit_ctrl->Bind(wxEVT_CHAR_HOOK, &SubsEditBox::OnKeyDown, this);

	secondary_editor = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(300,50), wxBORDER_SUNKEN | wxTE_MULTILINE | wxTE_READONLY);

	main_sizer->Add(secondary_editor,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	main_sizer->Add(edit_ctrl,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	main_sizer->Hide(secondary_editor);

	bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	bottom_sizer->Add(MakeBottomButton("edit/revert"), wxSizerFlags().Border(wxRIGHT));
	bottom_sizer->Add(MakeBottomButton("edit/clear"), wxSizerFlags().Border(wxRIGHT));
	bottom_sizer->Add(MakeBottomButton("edit/clear/text"), wxSizerFlags().Border(wxRIGHT));
	bottom_sizer->Add(MakeBottomButton("edit/insert_original"));
	main_sizer->Add(bottom_sizer);
	main_sizer->Hide(bottom_sizer);

	SetSizerAndFit(main_sizer);

	edit_ctrl->Bind(wxEVT_STC_MODIFIED, &SubsEditBox::OnChange, this);
	edit_ctrl->SetModEventMask(wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT | wxSTC_STARTACTION);

	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnLayerEnter, this, layer->GetId());
	Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &SubsEditBox::OnLayerEnter, this, layer->GetId());
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SubsEditBox::OnCommentChange, this, comment_box->GetId());

	Bind(wxEVT_CHAR_HOOK, &SubsEditBox::OnKeyDown, this);
	Bind(wxEVT_SIZE, &SubsEditBox::OnSize, this);
	Bind(wxEVT_TIMER, [=](wxTimerEvent&) { commit_id = -1; });

	wxSizeEvent evt;
	OnSize(evt);

	file_changed_slot = c->ass->AddCommitListener(&SubsEditBox::OnCommit, this);
	connections.push_back(context->videoController->AddTimecodesListener(&SubsEditBox::UpdateFrameTiming, this));
	connections.push_back(context->selectionController->AddActiveLineListener(&SubsEditBox::OnActiveLineChanged, this));
	connections.push_back(context->selectionController->AddSelectionListener(&SubsEditBox::OnSelectedSetChanged, this));
	connections.push_back(context->initialLineState->AddChangeListener(&SubsEditBox::OnLineInitialTextChanged, this));

	textSelectionController.reset(new ScintillaTextSelectionController(edit_ctrl));
	context->textSelectionController = textSelectionController.get();
	edit_ctrl->SetFocus();
}

SubsEditBox::~SubsEditBox() {
}

wxTextCtrl *SubsEditBox::MakeMarginCtrl(wxString const& tooltip, int margin, wxString const& commit_msg) {
	wxTextCtrl *ctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(40,-1), wxTE_CENTRE | wxTE_PROCESS_ENTER, NumValidator());
	ctrl->SetMaxLength(4);
	ctrl->SetToolTip(tooltip);
	middle_left_sizer->Add(ctrl, wxSizerFlags().Center());

	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent&) {
		wxString value = ctrl->GetValue();
		SetSelectedRows([&](AssDialogue *d) { d->SetMarginString(value, margin); }, commit_msg, AssFile::COMMIT_DIAG_META);
		if (line) change_value(ctrl, line->GetMarginString(margin));
	}, ctrl->GetId());

	return ctrl;
}

TimeEdit *SubsEditBox::MakeTimeCtrl(wxString const& tooltip, TimeField field) {
	TimeEdit *ctrl = new TimeEdit(this, -1, c, "", wxSize(GetTextExtent(wxS(" 0:00:00.000 ")).GetWidth(),-1), field == TIME_END);
	ctrl->SetToolTip(tooltip);
	Bind(wxEVT_COMMAND_TEXT_UPDATED, [=](wxCommandEvent&) { CommitTimes(field); }, ctrl->GetId());
	ctrl->Bind(wxEVT_CHAR_HOOK, time_edit_char_hook);
	middle_left_sizer->Add(ctrl, wxSizerFlags().Center());
	return ctrl;
}

void SubsEditBox::MakeButton(const char *cmd_name) {
	cmd::Command *command = cmd::get(cmd_name);
	wxBitmapButton *btn = new wxBitmapButton(this, -1, command->Icon(16));
	ToolTipManager::Bind(btn, command->StrHelp(), "Subtitle Edit Box", cmd_name);

	middle_right_sizer->Add(btn, wxSizerFlags().Center().Expand());
	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&SubsEditBox::CallCommand, this, cmd_name));
}

wxButton *SubsEditBox::MakeBottomButton(const char *cmd_name) {
	cmd::Command *command = cmd::get(cmd_name);
	wxButton *btn = new wxButton(this, -1, command->StrDisplay(c));
	ToolTipManager::Bind(btn, command->StrHelp(), "Subtitle Edit Box", cmd_name);

	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&SubsEditBox::CallCommand, this, cmd_name));
	return btn;
}

wxComboBox *SubsEditBox::MakeComboBox(wxString const& initial_text, int style, void (SubsEditBox::*handler)(wxCommandEvent&), wxString const& tooltip) {
	wxString styles[] = { "Default" };
	wxComboBox *ctrl = new wxComboBox(this, -1, initial_text, wxDefaultPosition, wxSize(110,-1), 1, styles, style | wxTE_PROCESS_ENTER);
	ctrl->SetToolTip(tooltip);
	top_sizer->Add(ctrl, wxSizerFlags(2).Center().Border(wxRIGHT));
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, handler, this, ctrl->GetId());
	return ctrl;
}

wxRadioButton *SubsEditBox::MakeRadio(wxString const& text, bool start, wxString const& tooltip) {
	wxRadioButton *ctrl = new wxRadioButton(this, -1, text, wxDefaultPosition, wxDefaultSize, start ? wxRB_GROUP : 0);
	ctrl->SetToolTip(tooltip);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &SubsEditBox::OnFrameTimeRadio, this, ctrl->GetId());
	middle_right_sizer->Add(ctrl, wxSizerFlags().Center().Expand().Border(wxRIGHT));
	return ctrl;
}

void SubsEditBox::OnCommit(int type) {
	wxEventBlocker blocker(this);

	initial_times.clear();

	if (type == AssFile::COMMIT_NEW || type & AssFile::COMMIT_STYLES) {
		wxString style = style_box->GetValue();
		style_box->Clear();
		style_box->Append(to_wx(c->ass->GetStyles()));
		style_box->Select(style_box->FindString(style));
	}

	if (type == AssFile::COMMIT_NEW) {
		/// @todo maybe preserve selection over undo?
		PopulateList(effect_box, &AssDialogue::Effect);
		PopulateList(actor_box, &AssDialogue::Actor);

		edit_ctrl->SetSelection(0,0);
		return;
	}
	else if (type & AssFile::COMMIT_STYLES)
		style_box->Select(style_box->FindString(line->Style));

	if (!(type ^ AssFile::COMMIT_ORDER)) return;

	SetControlsState(!!line);
	if (!line) return;

	if (type & AssFile::COMMIT_DIAG_TIME) {
		start_time->SetTime(line->Start);
		end_time->SetTime(line->End);
		duration->SetTime(line->End - line->Start);
	}

	if (type & AssFile::COMMIT_DIAG_TEXT) {
		edit_ctrl->SetTextTo(line->Text);
		UpdateCharacterCount(line->Text);
	}

	if (type & AssFile::COMMIT_DIAG_META) {
		layer->SetValue(line->Layer);
		for (size_t i = 0; i < margin.size(); ++i)
			change_value(margin[i], line->GetMarginString(i));
		comment_box->SetValue(line->Comment);
		style_box->Select(style_box->FindString(line->Style));

		PopulateList(effect_box, &AssDialogue::Effect);
		effect_box->ChangeValue(line->Effect);
		effect_box->SetStringSelection(line->Effect);

		PopulateList(actor_box, &AssDialogue::Actor);
		actor_box->ChangeValue(line->Actor);
		actor_box->SetStringSelection(line->Actor);
		edit_ctrl->SetTextTo(line->Text);
	}
}

void SubsEditBox::PopulateList(wxComboBox *combo, boost::flyweight<wxString> AssDialogue::*field) {
	wxEventBlocker blocker(this);

	std::unordered_set<wxString, wxStringHash, wxStringEqual> values;
	for (auto diag : c->ass->Line | agi::of_type<AssDialogue>())
		values.insert(diag->*field);
	values.erase("");
	wxArrayString arrstr;
	arrstr.reserve(values.size());
	copy(values.begin(), values.end(), std::back_inserter(arrstr));

	combo->Freeze();
	long pos = combo->GetInsertionPoint();
	wxString value = combo->GetValue();

	combo->Set(arrstr);
	combo->ChangeValue(value);
	combo->SetStringSelection(value);
	combo->SetInsertionPoint(pos);
	combo->Thaw();
}

void SubsEditBox::OnActiveLineChanged(AssDialogue *new_line) {
	wxEventBlocker blocker(this);
	line = new_line;
	commit_id = -1;

	OnCommit(AssFile::COMMIT_DIAG_FULL);

	/// @todo VideoContext should be doing this
	if (c->videoController->IsLoaded()) {
		if (OPT_GET("Video/Subtitle Sync")->GetBool()) {
			c->videoController->Stop();
			c->videoController->JumpToTime(line->Start);
		}
	}
}

void SubsEditBox::OnSelectedSetChanged(const SubtitleSelection &, const SubtitleSelection &) {
	sel = c->selectionController->GetSelectedSet();
	initial_times.clear();
}

void SubsEditBox::OnLineInitialTextChanged(wxString const& new_text) {
	if (split_box->IsChecked())
		secondary_editor->SetValue(new_text);
}

void SubsEditBox::UpdateFrameTiming(agi::vfr::Framerate const& fps) {
	if (fps.IsLoaded()) {
		by_frame->Enable(true);
	}
	else {
		by_frame->Enable(false);
		by_time->SetValue(true);
		start_time->SetByFrame(false);
		end_time->SetByFrame(false);
		duration->SetByFrame(false);
		c->subsGrid->SetByFrame(false);
	}
}

void SubsEditBox::OnKeyDown(wxKeyEvent &event) {
	hotkey::check("Subtitle Edit Box", c, event);
}

void SubsEditBox::OnChange(wxStyledTextEvent &event) {
	if (line && edit_ctrl->GetText() != line->Text) {
		if (event.GetModificationType() & wxSTC_STARTACTION)
			commit_id = -1;
		CommitText(_("modify text"));
		UpdateCharacterCount(line->Text);
	}
}

template<class setter>
void SubsEditBox::SetSelectedRows(setter set, wxString const& desc, int type, bool amend) {
	for_each(sel.begin(), sel.end(), set);

	file_changed_slot.Block();
	commit_id = c->ass->Commit(desc, type, (amend && desc == last_commit_type) ? commit_id : -1, sel.size() == 1 ? *sel.begin() : 0);
	file_changed_slot.Unblock();
	last_commit_type = desc;
	last_time_commit_type = -1;
	initial_times.clear();
	undo_timer.Start(30000, wxTIMER_ONE_SHOT);
}

template<class T>
void SubsEditBox::SetSelectedRows(T AssDialogue::*field, T value, wxString const& desc, int type, bool amend) {
	SetSelectedRows([&](AssDialogue *d) { d->*field = value; }, desc, type, amend);
}

void SubsEditBox::CommitText(wxString const& desc) {
	SetSelectedRows(&AssDialogue::Text, boost::flyweight<wxString>(edit_ctrl->GetText()), desc, AssFile::COMMIT_DIAG_TEXT, true);
}

void SubsEditBox::CommitTimes(TimeField field) {
	for (AssDialogue *d : sel) {
		if (!initial_times.count(d))
			initial_times[d] = std::make_pair(d->Start, d->End);

		switch (field) {
			case TIME_START:
				initial_times[d].first = d->Start = start_time->GetTime();
				d->End = std::max(d->Start, initial_times[d].second);
				break;

			case TIME_END:
				initial_times[d].second = d->End = end_time->GetTime();
				d->Start = std::min(d->End, initial_times[d].first);
				break;

			case TIME_DURATION:
				if (by_frame->GetValue())
					d->End = c->videoController->TimeAtFrame(c->videoController->FrameAtTime(d->Start, agi::vfr::START) + duration->GetFrame(), agi::vfr::END);
				else
					d->End = d->Start + duration->GetTime();
				initial_times[d].second = d->End;
				break;
		}
	}

	start_time->SetTime(line->Start);
	end_time->SetTime(line->End);

	if (by_frame->GetValue())
		duration->SetFrame(end_time->GetFrame() - start_time->GetFrame() + 1);
	else
		duration->SetTime(end_time->GetTime() - start_time->GetTime());

	if (field != last_time_commit_type)
		commit_id = -1;

	last_time_commit_type = field;
	file_changed_slot.Block();
	commit_id = c->ass->Commit(_("modify times"), AssFile::COMMIT_DIAG_TIME, commit_id, sel.size() == 1 ? *sel.begin() : 0);
	file_changed_slot.Unblock();
}

void SubsEditBox::OnSize(wxSizeEvent &evt) {
	int availableWidth = GetVirtualSize().GetWidth();
	int midMin = middle_left_sizer->GetMinSize().GetWidth();
	int botMin = middle_right_sizer->GetMinSize().GetWidth();

	if (button_bar_split) {
		if (availableWidth > midMin + botMin) {
			GetSizer()->Detach(middle_right_sizer);
			middle_left_sizer->Add(middle_right_sizer,0,wxALIGN_CENTER_VERTICAL);
			button_bar_split = false;
		}
	}
	else {
		if (availableWidth < midMin) {
			middle_left_sizer->Detach(middle_right_sizer);
			GetSizer()->Insert(2,middle_right_sizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
			button_bar_split = true;
		}
	}

	evt.Skip();
}

void SubsEditBox::OnFrameTimeRadio(wxCommandEvent &event) {
	bool byFrame = by_frame->GetValue();
	start_time->SetByFrame(byFrame);
	end_time->SetByFrame(byFrame);
	duration->SetByFrame(byFrame);
	c->subsGrid->SetByFrame(byFrame);
	event.Skip();
}

void SubsEditBox::SetControlsState(bool state) {
	if (state == controls_enabled) return;
	controls_enabled = state;

	Enable(state);
	if (!state) {
		wxEventBlocker blocker(this);
		edit_ctrl->SetTextTo("");
	}
}

void SubsEditBox::OnSplit(wxCommandEvent&) {
	Freeze();
	GetSizer()->Show(secondary_editor, split_box->IsChecked());
	GetSizer()->Show(bottom_sizer, split_box->IsChecked());
	Fit();
	SetMinSize(GetSize());
	GetParent()->GetSizer()->Layout();
	Thaw();

	if (split_box->IsChecked())
		secondary_editor->SetValue(c->initialLineState->GetInitialText());
}

void SubsEditBox::OnStyleChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Style,  boost::flyweight<wxString>(style_box->GetValue()), _("style change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::OnActorChange(wxCommandEvent &evt) {
	bool amend = evt.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED;
	SetSelectedRows(&AssDialogue::Actor, boost::flyweight<wxString>(actor_box->GetValue()), _("actor change"), AssFile::COMMIT_DIAG_META, amend);
	PopulateList(actor_box, &AssDialogue::Actor);
}

void SubsEditBox::OnLayerEnter(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Layer, layer->GetValue(), _("layer change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::OnEffectChange(wxCommandEvent &evt) {
	bool amend = evt.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED;
	SetSelectedRows(&AssDialogue::Effect, boost::flyweight<wxString>(effect_box->GetValue()), _("effect change"), AssFile::COMMIT_DIAG_META, amend);
	PopulateList(effect_box, &AssDialogue::Effect);
}

void SubsEditBox::OnCommentChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Comment, comment_box->GetValue(), _("comment change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::CallCommand(const char *cmd_name) {
	cmd::call(cmd_name, c);
	edit_ctrl->SetFocus();
}

void SubsEditBox::UpdateCharacterCount(wxString const& text) {
	size_t length = MaxLineLength(text);
	char_count->SetValue(wxString::Format("%lu", (unsigned long)length));
	size_t limit = (size_t)OPT_GET("Subtitle/Character Limit")->GetInt();
	if (limit && length > limit)
		char_count->SetBackgroundColour(to_wx(OPT_GET("Colour/Subtitle/Syntax/Background/Error")->GetColor()));
	else
		char_count->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}
