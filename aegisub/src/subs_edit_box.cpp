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

#ifndef AGI_PRE
#include <tr1/functional>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/colordlg.h>
#include <wx/combobox.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/fontdlg.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#endif

#include "subs_edit_box.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "command/command.h"
#include "dialog_search_replace.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "placeholder_ctrl.h"
#include "scintilla_text_selection_controller.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "timeedit_ctrl.h"
#include "tooltip_manager.h"
#include "utils.h"
#include "validators.h"
#include "video_context.h"

namespace {
template<class T>
struct field_setter : public std::binary_function<AssDialogue*, T, void> {
	T AssDialogue::*field;
	field_setter(T AssDialogue::*field) : field(field) { }
	void operator()(AssDialogue* obj, T const& value) {
		obj->*field = value;
	}
};

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
, splitLineMode(true)
, controlState(true)
, c(context)
, commitId(-1)
, undoTimer(GetEventHandler())
{
	using std::tr1::bind;

	// Top controls
	TopSizer = new wxBoxSizer(wxHORIZONTAL);

	CommentBox = new wxCheckBox(this,-1,_("&Comment"));
	CommentBox->SetToolTip(_("Comment this line out. Commented lines don't show up on screen."));
#ifdef __WXGTK__
	// Only supported in wxgtk
	CommentBox->SetCanFocus(false);
#endif
	TopSizer->Add(CommentBox, 0, wxRIGHT | wxALIGN_CENTER, 5);

	StyleBox = MakeComboBox("Default", wxCB_READONLY, &SubsEditBox::OnStyleChange, _("Style for this line"));

	ActorBox = new Placeholder<wxComboBox>(this, _("Actor"), wxSize(110, -1), wxCB_DROPDOWN | wxTE_PROCESS_ENTER, _("Actor name for this speech. This is only for reference, and is mainly useless."));
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnActorChange, this, ActorBox->GetId());
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnActorChange, this, ActorBox->GetId());
	TopSizer->Add(ActorBox, wxSizerFlags(2).Center().Border(wxRIGHT));

	Effect = new Placeholder<wxComboBox>(this, _("Effect"), wxSize(80,-1), wxCB_DROPDOWN | wxTE_PROCESS_ENTER, _("Effect for this line. This can be used to store extra information for karaoke scripts, or for the effects supported by the renderer."));
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnEffectChange, this, Effect->GetId());
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnEffectChange, this, Effect->GetId());
	TopSizer->Add(Effect, 3, wxALIGN_CENTER, 5);

	// Middle controls
	MiddleSizer = new wxBoxSizer(wxHORIZONTAL);

	Layer = new wxSpinCtrl(this,-1,"",wxDefaultPosition,wxSize(50,-1), wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER,0,0x7FFFFFFF,0);
	Layer->SetToolTip(_("Layer number"));
	MiddleSizer->Add(Layer, wxSizerFlags().Center());
	MiddleSizer->AddSpacer(5);

	StartTime = MakeTimeCtrl(false, _("Start time"), &SubsEditBox::OnStartTimeChange);
	EndTime   = MakeTimeCtrl(true, _("End time"), &SubsEditBox::OnEndTimeChange);
	MiddleSizer->AddSpacer(5);
	Duration  = MakeTimeCtrl(false, _("Line duration"), &SubsEditBox::OnDurationChange);
	MiddleSizer->AddSpacer(5);

	MarginL = MakeMarginCtrl(_("Left Margin (0 = default)"), &SubsEditBox::OnMarginLChange);
	MarginR = MakeMarginCtrl(_("Right Margin (0 = default)"), &SubsEditBox::OnMarginRChange);
	MarginV = MakeMarginCtrl(_("Vertical Margin (0 = default)"), &SubsEditBox::OnMarginVChange);
	MiddleSizer->AddSpacer(5);

	// Middle-bottom controls
	MiddleBotSizer = new wxBoxSizer(wxHORIZONTAL);
	MakeButton("edit/style/bold");
	MakeButton("edit/style/italic");
	MakeButton("edit/style/underline");
	MakeButton("edit/style/strikeout");
	MakeButton("edit/font");
	MiddleBotSizer->AddSpacer(5);
	MakeButton("edit/color/primary");
	MakeButton("edit/color/secondary");
	MakeButton("edit/color/outline");
	MakeButton("edit/color/shadow");
	MiddleBotSizer->AddSpacer(5);
	MakeButton("grid/line/next/create");
	MiddleBotSizer->AddSpacer(10);

	ByTime = MakeRadio(_("T&ime"), true, _("Time by h:mm:ss.cs"));
	ByFrame = MakeRadio(_("F&rame"), false, _("Time by frame number"));
	ByFrame->Enable(false);

	// Text editor
	TextEdit = new SubsTextEditCtrl(this, wxSize(300,50), wxBORDER_SUNKEN, c);
	TextEdit->Bind(wxEVT_CHAR_HOOK, &SubsEditBox::OnKeyDown, this);
	Bind(wxEVT_CHAR_HOOK, &SubsEditBox::OnKeyDown, this);
	BottomSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer->Add(TextEdit,1,wxEXPAND,0);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxEXPAND | wxALL,3);
	MainSizer->Add(MiddleSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	MainSizer->Add(MiddleBotSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	MainSizer->Add(BottomSizer,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);

	SetSizerAndFit(MainSizer);

	TextEdit->Bind(wxEVT_STC_MODIFIED, &SubsEditBox::OnChange, this);
	TextEdit->SetModEventMask(wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT | wxSTC_STARTACTION);

	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnLayerEnter, this, Layer->GetId());
	Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &SubsEditBox::OnLayerEnter, this, Layer->GetId());
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SubsEditBox::OnCommentChange, this, CommentBox->GetId());

	Bind(wxEVT_SIZE, &SubsEditBox::OnSize, this);
	Bind(wxEVT_TIMER, &SubsEditBox::OnUndoTimer, this);

	wxSizeEvent evt;
	OnSize(evt);

	file_changed_slot = c->ass->AddCommitListener(&SubsEditBox::OnCommit, this);
	connections.push_back(context->videoController->AddTimecodesListener(&SubsEditBox::UpdateFrameTiming, this));
	connections.push_back(context->selectionController->AddActiveLineListener(&SubsEditBox::OnActiveLineChanged, this));
	connections.push_back(context->selectionController->AddSelectionListener(&SubsEditBox::OnSelectedSetChanged, this));

	textSelectionController.reset(new ScintillaTextSelectionController(TextEdit));
	context->textSelectionController = textSelectionController.get();
	TextEdit->SetFocus();
}

SubsEditBox::~SubsEditBox() {
}

wxTextCtrl *SubsEditBox::MakeMarginCtrl(wxString const& tooltip, void (SubsEditBox::*handler)(wxCommandEvent&)) {
	wxTextCtrl *ctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(40,-1), wxTE_CENTRE | wxTE_PROCESS_ENTER, NumValidator());
	ctrl->SetMaxLength(4);
	ctrl->SetToolTip(tooltip);
	Bind(wxEVT_COMMAND_TEXT_UPDATED, handler, this, ctrl->GetId());
	MiddleSizer->Add(ctrl, wxSizerFlags().Center());
	return ctrl;
}

TimeEdit *SubsEditBox::MakeTimeCtrl(bool end, wxString const& tooltip, void (SubsEditBox::*handler)(wxCommandEvent&)) {
	TimeEdit *ctrl = new TimeEdit(this, -1, c, "", wxSize(75,-1), end);
	ctrl->SetToolTip(tooltip);
	Bind(wxEVT_COMMAND_TEXT_UPDATED, handler, this, ctrl->GetId());
	ctrl->Bind(wxEVT_CHAR_HOOK, time_edit_char_hook);
	MiddleSizer->Add(ctrl, wxSizerFlags().Center());
	return ctrl;
}

void SubsEditBox::MakeButton(const char *cmd_name) {
	cmd::Command *command = cmd::get(cmd_name);
	wxBitmapButton *btn = new wxBitmapButton(this, -1, command->Icon(16));
	ToolTipManager::Bind(btn, command->StrHelp(), "Subtitle Edit Box", cmd_name);

	MiddleBotSizer->Add(btn, wxSizerFlags().Center().Expand());
	btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&SubsEditBox::CallCommand, this, cmd_name));
}

wxComboBox *SubsEditBox::MakeComboBox(wxString const& initial_text, int style, void (SubsEditBox::*handler)(wxCommandEvent&), wxString const& tooltip) {
	wxString styles[] = { "Default" };
	wxComboBox *ctrl = new wxComboBox(this, -1, initial_text, wxDefaultPosition, wxSize(110,-1), 1, styles, style | wxTE_PROCESS_ENTER);
	ctrl->SetToolTip(tooltip);
	TopSizer->Add(ctrl, wxSizerFlags(2).Center().Border(wxRIGHT));
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, handler, this, ctrl->GetId());
	return ctrl;
}

wxRadioButton *SubsEditBox::MakeRadio(wxString const& text, bool start, wxString const& tooltip) {
	wxRadioButton *ctrl = new wxRadioButton(this, -1, text, wxDefaultPosition, wxDefaultSize, start ? wxRB_GROUP : 0);
	ctrl->SetToolTip(tooltip);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &SubsEditBox::OnFrameTimeRadio, this, ctrl->GetId());
	MiddleBotSizer->Add(ctrl, wxSizerFlags().Center().Expand().Border(wxRIGHT));
	return ctrl;
}

void SubsEditBox::OnCommit(int type) {
	wxEventBlocker blocker(this);

	initialTimes.clear();

	if (type == AssFile::COMMIT_NEW || type & AssFile::COMMIT_STYLES) {
		wxString style = StyleBox->GetValue();
		StyleBox->Clear();
		StyleBox->Append(c->ass->GetStyles());
		StyleBox->Select(StyleBox->FindString(style));
	}

	if (type == AssFile::COMMIT_NEW) {
		/// @todo maybe preserve selection over undo?
		PopulateList(Effect, &AssDialogue::Effect);
		PopulateList(ActorBox, &AssDialogue::Actor);

		TextEdit->SetSelection(0,0);
		return;
	}
	else if (type & AssFile::COMMIT_STYLES)
		StyleBox->Select(StyleBox->FindString(line->Style));

	if (!(type ^ AssFile::COMMIT_ORDER)) return;

	SetControlsState(!!line);
	if (!line) return;

	if (type & AssFile::COMMIT_DIAG_TIME) {
		StartTime->SetTime(line->Start);
		EndTime->SetTime(line->End);
		Duration->SetTime(line->End - line->Start);
	}

	if (type & AssFile::COMMIT_DIAG_TEXT) {
		TextEdit->SetTextTo(line->Text);
	}

	if (type & AssFile::COMMIT_DIAG_META) {
		Layer->SetValue(line->Layer);
		change_value(MarginL, line->GetMarginString(0,false));
		change_value(MarginR, line->GetMarginString(1,false));
		change_value(MarginV, line->GetMarginString(2,false));
		CommentBox->SetValue(line->Comment);
		StyleBox->Select(StyleBox->FindString(line->Style));

		PopulateList(Effect, &AssDialogue::Effect);
		Effect->ChangeValue(line->Effect);
		Effect->SetStringSelection(line->Effect);

		PopulateList(ActorBox, &AssDialogue::Actor);
		ActorBox->ChangeValue(line->Actor);
		ActorBox->SetStringSelection(line->Actor);
	}
}

void SubsEditBox::PopulateList(wxComboBox *combo, wxString AssDialogue::*field) {
	wxEventBlocker blocker(this);

	std::set<wxString> values;
	for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ++it) {
		if (AssDialogue *diag = dynamic_cast<AssDialogue*>(*it))
			values.insert(diag->*field);
	}
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
	commitId = -1;

	OnCommit(AssFile::COMMIT_DIAG_FULL);

	/// @todo VideoContext should be doing this
	if (c->videoController->IsLoaded()) {
		bool sync;
		if (Search.HasFocus()) sync = OPT_GET("Tool/Search Replace/Video Update")->GetBool();
		else sync = OPT_GET("Video/Subtitle Sync")->GetBool();

		if (sync) {
			c->videoController->Stop();
			c->videoController->JumpToTime(line->Start);
		}
	}
}
void SubsEditBox::OnSelectedSetChanged(const SubtitleSelection &, const SubtitleSelection &) {
	sel = c->selectionController->GetSelectedSet();
	initialTimes.clear();
}

void SubsEditBox::UpdateFrameTiming(agi::vfr::Framerate const& fps) {
	if (fps.IsLoaded()) {
		ByFrame->Enable(true);
	}
	else {
		ByFrame->Enable(false);
		ByTime->SetValue(true);
		StartTime->SetByFrame(false);
		EndTime->SetByFrame(false);
		Duration->SetByFrame(false);
		c->subsGrid->SetByFrame(false);
	}
}

void SubsEditBox::OnKeyDown(wxKeyEvent &event) {
	hotkey::check("Subtitle Edit Box", c, event);
}

void SubsEditBox::OnChange(wxStyledTextEvent &event) {
	if (line && TextEdit->GetText() != line->Text) {
		if (event.GetModificationType() & wxSTC_STARTACTION)
			commitId = -1;
		CommitText(_("modify text"));
	}
}

void SubsEditBox::OnUndoTimer(wxTimerEvent&) {
	commitId = -1;
}

template<class T, class setter>
void SubsEditBox::SetSelectedRows(setter set, T value, wxString desc, int type, bool amend) {
	for_each(sel.begin(), sel.end(), bind(set, std::tr1::placeholders::_1, value));

	file_changed_slot.Block();
	commitId = c->ass->Commit(desc, type, (amend && desc == lastCommitType) ? commitId : -1, sel.size() == 1 ? *sel.begin() : 0);
	file_changed_slot.Unblock();
	lastCommitType = desc;
	lastTimeCommitType = -1;
	initialTimes.clear();
	undoTimer.Start(30000, wxTIMER_ONE_SHOT);
}

template<class T>
void SubsEditBox::SetSelectedRows(T AssDialogue::*field, T value, wxString desc, int type, bool amend) {
	SetSelectedRows(field_setter<T>(field), value, desc, type, amend);
}

void SubsEditBox::CommitText(wxString const& desc) {
	SetSelectedRows(&AssDialogue::Text, TextEdit->GetText(), desc, AssFile::COMMIT_DIAG_TEXT, true);
}

void SubsEditBox::CommitTimes(TimeField field) {
	for (SubtitleSelection::iterator cur = sel.begin(); cur != sel.end(); ++cur) {
		AssDialogue *d = *cur;

		if (!initialTimes.count(d))
			initialTimes[d] = std::make_pair(d->Start, d->End);

		switch (field) {
			case TIME_START:
				initialTimes[d].first = d->Start = StartTime->GetTime();
				d->End = std::max(d->Start, initialTimes[d].second);
				break;

			case TIME_END:
				initialTimes[d].second = d->End = EndTime->GetTime();
				d->Start = std::min(d->End, initialTimes[d].first);
				break;

			case TIME_DURATION:
				if (ByFrame->GetValue())
					d->End = c->videoController->TimeAtFrame(c->videoController->FrameAtTime(d->Start, agi::vfr::START) + Duration->GetFrame(), agi::vfr::END);
				else
					d->End = d->Start + Duration->GetTime();
				initialTimes[d].second = d->End;
				break;
		}
	}

	StartTime->SetTime(line->Start);
	EndTime->SetTime(line->End);

	if (ByFrame->GetValue())
		Duration->SetFrame(EndTime->GetFrame() - StartTime->GetFrame() + 1);
	else
		Duration->SetTime(EndTime->GetTime() - StartTime->GetTime());

	if (field != lastTimeCommitType)
		commitId = -1;

	lastTimeCommitType = field;
	file_changed_slot.Block();
	commitId = c->ass->Commit(_("modify times"), AssFile::COMMIT_DIAG_TIME, commitId, sel.size() == 1 ? *sel.begin() : 0);
	file_changed_slot.Unblock();
}

void SubsEditBox::OnSize(wxSizeEvent &evt) {
	int availableWidth = GetVirtualSize().GetWidth();
	int midMin = MiddleSizer->GetMinSize().GetWidth();
	int botMin = MiddleBotSizer->GetMinSize().GetWidth();

	if (splitLineMode) {
		if (availableWidth > midMin + botMin) {
			GetSizer()->Detach(MiddleBotSizer);
			MiddleSizer->Add(MiddleBotSizer,0,wxALIGN_CENTER_VERTICAL);
			splitLineMode = false;
		}
	}
	else {
		if (availableWidth < midMin) {
			MiddleSizer->Detach(MiddleBotSizer);
			GetSizer()->Insert(2,MiddleBotSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
			splitLineMode = true;
		}
	}

	evt.Skip();
}

void SubsEditBox::OnFrameTimeRadio(wxCommandEvent &event) {
	bool byFrame = ByFrame->GetValue();
	StartTime->SetByFrame(byFrame);
	EndTime->SetByFrame(byFrame);
	Duration->SetByFrame(byFrame);
	c->subsGrid->SetByFrame(byFrame);
	event.Skip();
}

void SubsEditBox::SetControlsState(bool state) {
	if (state == controlState) return;
	controlState = state;

	Enable(state);
	if (!state) {
		wxEventBlocker blocker(this);
		TextEdit->SetTextTo("");
	}
}

void SubsEditBox::OnStyleChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Style, StyleBox->GetValue(), _("style change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::OnActorChange(wxCommandEvent &evt) {
	bool amend = evt.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED;
	SetSelectedRows(&AssDialogue::Actor, ActorBox->GetValue(), _("actor change"), AssFile::COMMIT_DIAG_META, amend);
	PopulateList(ActorBox, &AssDialogue::Actor);
}

void SubsEditBox::OnLayerEnter(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Layer, Layer->GetValue(), _("layer change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::OnStartTimeChange(wxCommandEvent &) {
	CommitTimes(TIME_START);
}

void SubsEditBox::OnEndTimeChange(wxCommandEvent &) {
	CommitTimes(TIME_END);
}

void SubsEditBox::OnDurationChange(wxCommandEvent &) {
	CommitTimes(TIME_DURATION);
}

void SubsEditBox::OnMarginLChange(wxCommandEvent &) {
	SetSelectedRows(std::mem_fun(&AssDialogue::SetMarginString<0>), MarginL->GetValue(), _("MarginL change"), AssFile::COMMIT_DIAG_META);
	if (line) change_value(MarginL, line->GetMarginString(0, false));
}

void SubsEditBox::OnMarginRChange(wxCommandEvent &) {
	SetSelectedRows(std::mem_fun(&AssDialogue::SetMarginString<1>), MarginR->GetValue(), _("MarginR change"), AssFile::COMMIT_DIAG_META);
	if (line) change_value(MarginR, line->GetMarginString(1, false));
}

void SubsEditBox::OnMarginVChange(wxCommandEvent &) {
	SetSelectedRows(std::mem_fun(&AssDialogue::SetMarginString<2>), MarginV->GetValue(), _("MarginV change"), AssFile::COMMIT_DIAG_META);
	if (line) change_value(MarginV, line->GetMarginString(2, false));
}

void SubsEditBox::OnEffectChange(wxCommandEvent &evt) {
	bool amend = evt.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED;
	SetSelectedRows(&AssDialogue::Effect, Effect->GetValue(), _("effect change"), AssFile::COMMIT_DIAG_META, amend);
	PopulateList(Effect, &AssDialogue::Effect);
}

void SubsEditBox::OnCommentChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Comment, CommentBox->GetValue(), _("comment change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::CallCommand(const char *cmd_name) {
	cmd::call(cmd_name, c);
	TextEdit->SetFocus();
}
