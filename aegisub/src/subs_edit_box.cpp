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
//
// $Id$

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
#include "ass_override.h"
#include "ass_style.h"
#include "command/command.h"
#include "dialog_colorpicker.h"
#include "dialog_search_replace.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "placeholder_ctrl.h"
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
	void operator()(AssDialogue* obj, T value) {
		obj->*field = value;
	}
};

/// @brief Get the selection from a text edit
/// @return Pair of selection start and end positions
std::pair<int, int> get_selection(SubsTextEditCtrl *TextEdit) {
	int start, end;
	TextEdit->GetSelection(&start, &end);
	int len = TextEdit->GetText().size();
	return std::make_pair(
		mid(0, TextEdit->GetReverseUnicodePosition(start), len),
		mid(0, TextEdit->GetReverseUnicodePosition(end), len));
}

/// @brief Get the value of a tag at a specified position in a line
/// @param line    Line to get the value from
/// @param blockn  Block number in the line
/// @param initial Value from style to use if the tag does not exist
/// @param tag     Tag to get the value of
/// @param alt     Alternate name of the tag, if any
template<class T>
static T get_value(AssDialogue const& line, int blockn, T initial, wxString tag, wxString alt = "") {
	for (int i = blockn; i >= 0; i--) {
		AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride*>(line.Blocks[i]);
		if (!ovr) continue;

		for (int j = (int)ovr->Tags.size() - 1; j >= 0; j--) {
			if (ovr->Tags[j]->Name == tag || ovr->Tags[j]->Name == alt) {
				return ovr->Tags[j]->Params[0]->Get<T>(initial);
			}
		}
	}
	return initial;
}

/// Get the block index in the text of the position
int block_at_pos(wxString const& text, int pos) {
	int n = 0;
	int max = text.size() - 1;
	for (int i = 0; i <= pos && i <= max; ++i) {
		if (i > 0 && text[i] == '{')
			n++;
		if (text[i] == '}' && i != max && i != pos && i != pos -1 && (i+1 == max || text[i+1] != '{'))
			n++;
	}

	return n;
}

/// Work around wxGTK's fondness for generating events from ChangeValue
void change_value(wxTextCtrl *ctrl, wxString const& value) {
	if (value != ctrl->GetValue())
		ctrl->ChangeValue(value);
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
	TopSizer->Add(CommentBox, 0, wxRIGHT | wxALIGN_CENTER, 5);

	StyleBox = MakeComboBox("Default", wxCB_READONLY, &SubsEditBox::OnStyleChange, _("Style for this line."));

	ActorBox = new Placeholder<wxComboBox>(this, "Actor", wxSize(110, -1), wxCB_DROPDOWN | wxTE_PROCESS_ENTER, _("Actor name for this speech. This is only for reference, and is mainly useless."));
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnActorChange, this, ActorBox->GetId());
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnActorChange, this, ActorBox->GetId());
	TopSizer->Add(ActorBox, wxSizerFlags(2).Center().Border(wxRIGHT));

	Effect = new Placeholder<wxTextCtrl>(this, "Effect", wxSize(80,-1), wxTE_PROCESS_ENTER, _("Effect for this line. This can be used to store extra information for karaoke scripts, or for the effects supported by the renderer."));
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnEffectChange, this, Effect->GetId());
	TopSizer->Add(Effect, 3, wxALIGN_CENTER, 5);

	// Middle controls
	MiddleSizer = new wxBoxSizer(wxHORIZONTAL);

	Layer = new wxSpinCtrl(this,-1,"",wxDefaultPosition,wxSize(50,-1),wxSP_ARROW_KEYS,0,0x7FFFFFFF,0);
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
	MakeButton(GETIMAGE(button_bold_16), _("Bold"), bind(&SubsEditBox::OnFlagButton, this, &AssStyle::bold, "\\b", _("toggle bold")));
	MakeButton(GETIMAGE(button_italics_16), _("Italics"), bind(&SubsEditBox::OnFlagButton, this, &AssStyle::italic, "\\i", _("toggle italic")));
	MakeButton(GETIMAGE(button_underline_16), _("Underline"), bind(&SubsEditBox::OnFlagButton, this, &AssStyle::underline, "\\u", _("toggle underline")));
	MakeButton(GETIMAGE(button_strikeout_16), _("Strikeout"), bind(&SubsEditBox::OnFlagButton, this, &AssStyle::strikeout, "\\s", _("toggle strikeout")));
	MakeButton(GETIMAGE(button_fontname_16), _("Font Face"), bind(&SubsEditBox::OnFontButton, this));
	MiddleBotSizer->AddSpacer(5);
	MakeButton(GETIMAGE(button_color_one_16), _("Primary color"), bind(&SubsEditBox::OnColorButton, this, &AssStyle::primary, "\\c", "\\1c"));
	MakeButton(GETIMAGE(button_color_two_16), _("Secondary color"), bind(&SubsEditBox::OnColorButton, this, &AssStyle::secondary, "\\2c", ""));
	MakeButton(GETIMAGE(button_color_three_16), _("Outline color"), bind(&SubsEditBox::OnColorButton, this, &AssStyle::outline, "\\3c", ""));
	MakeButton(GETIMAGE(button_color_four_16), _("Shadow color"), bind(&SubsEditBox::OnColorButton, this, &AssStyle::shadow, "\\4c", ""));
	MiddleBotSizer->AddSpacer(5);
	MakeButton(GETIMAGE(button_audio_commit_16), _("Commits the text (Enter)"), bind(&cmd::call, "grid/line/next/create", c));
	MiddleBotSizer->AddSpacer(10);

	ByTime = MakeRadio(_("&Time"), true, _("Time by h:mm:ss.cs"));
	ByFrame = MakeRadio(_("F&rame"), false, _("Time by frame number"));
	ByFrame->Enable(false);

	// Text editor
	TextEdit = new SubsTextEditCtrl(this, wxSize(300,50), wxBORDER_SUNKEN, c);
	TextEdit->Bind(wxEVT_KEY_DOWN, &SubsEditBox::OnKeyDown, this);
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
	Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &SubsEditBox::OnLayerChange, this, Layer->GetId());
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SubsEditBox::OnCommentChange, this, CommentBox->GetId());

	Bind(wxEVT_SIZE, &SubsEditBox::OnSize, this);
	Bind(wxEVT_TIMER, &SubsEditBox::OnUndoTimer, this);

	wxSizeEvent evt;
	OnSize(evt);

	c->selectionController->AddSelectionListener(this);
	file_changed_slot = c->ass->AddCommitListener(&SubsEditBox::OnCommit, this);
	context->videoController->AddTimecodesListener(&SubsEditBox::UpdateFrameTiming, this);
}

SubsEditBox::~SubsEditBox() {
	c->selectionController->RemoveSelectionListener(this);
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
	MiddleSizer->Add(ctrl, wxSizerFlags().Center());
	return ctrl;
}

template<class Handler>
void SubsEditBox::MakeButton(wxBitmap const& icon, wxString const& tooltip, Handler const& handler) {
	wxBitmapButton *btn = new wxBitmapButton(this, -1, icon);
	btn->SetToolTip(tooltip);

	MiddleBotSizer->Add(btn, wxSizerFlags().Center().Expand());
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, handler, btn->GetId());
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

	if (type == AssFile::COMMIT_NEW || type & AssFile::COMMIT_STYLES) {
		wxString style = StyleBox->GetValue();
		StyleBox->Clear();
		StyleBox->Append(c->ass->GetStyles());
		StyleBox->Select(StyleBox->FindString(style));
	}

	if (type == AssFile::COMMIT_NEW) {
		/// @todo maybe preserve selection over undo?
		PopulateActorList();

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
		Effect->ChangeValue(line->Effect);
		CommentBox->SetValue(line->Comment);
		StyleBox->Select(StyleBox->FindString(line->Style));

		PopulateActorList();
		ActorBox->ChangeValue(line->Actor);
		ActorBox->SetStringSelection(line->Actor);
	}
}

void SubsEditBox::PopulateActorList() {
	wxEventBlocker blocker(this);

	std::set<wxString> actors;
	for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ++it) {
		if (AssDialogue *diag = dynamic_cast<AssDialogue*>(*it))
			actors.insert(diag->Actor);
	}
#ifdef __APPLE__
	// OSX doesn't like combo boxes that are empty.
	actors.insert("Actor");
#endif
	actors.erase("");
	wxArrayString arrstr;
	arrstr.reserve(actors.size());
	copy(actors.begin(), actors.end(), std::back_inserter(arrstr));

	ActorBox->Freeze();
	long pos = ActorBox->GetInsertionPoint();
	wxString value = ActorBox->GetValue();

	ActorBox->Set(arrstr);
	ActorBox->ChangeValue(value);
	ActorBox->SetStringSelection(value);
	ActorBox->SetInsertionPoint(pos);
	ActorBox->Thaw();
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
void SubsEditBox::OnSelectedSetChanged(const Selection &, const Selection &) {
	sel = c->selectionController->GetSelectedSet();
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
		c->subsGrid->SetByFrame(false);
	}
}

void SubsEditBox::OnKeyDown(wxKeyEvent &event) {
	if (!hotkey::check("Subtitle Edit Box", c, event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		event.Skip();
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
	undoTimer.Start(10000, wxTIMER_ONE_SHOT);
}

template<class T>
void SubsEditBox::SetSelectedRows(T AssDialogue::*field, T value, wxString desc, int type, bool amend) {
	SetSelectedRows(field_setter<T>(field), value, desc, type, amend);
}

void SubsEditBox::CommitText(wxString desc) {
	SetSelectedRows(&AssDialogue::Text, TextEdit->GetText(), desc, AssFile::COMMIT_DIAG_TEXT, true);
}

void SubsEditBox::CommitTimes(TimeField field) {
	if (ByFrame->GetValue())
		Duration->SetFrame(EndTime->GetFrame() - StartTime->GetFrame() + 1);
	else
		Duration->SetTime(EndTime->GetTime() - StartTime->GetTime());

	// Update lines
	for (Selection::iterator cur = sel.begin(); cur != sel.end(); ++cur) {
		AssDialogue *d = *cur;
		switch (field) {
			case TIME_START:
				d->Start = StartTime->GetTime();
				if (d->Start > d->End)
					d->End = d->Start;
				break;
			case TIME_END:
				d->End = EndTime->GetTime();
				if (d->Start > d->End)
					d->Start = d->End;
				break;
			case TIME_DURATION:
				if (ByFrame->GetValue())
					d->End = c->videoController->TimeAtFrame(c->videoController->FrameAtTime(d->Start, agi::vfr::START) + Duration->GetFrame(), agi::vfr::END);
				else
					d->End = d->Start + Duration->GetTime();
				break;
		}
	}

	timeCommitId[field] = c->ass->Commit(_("modify times"), AssFile::COMMIT_DIAG_TIME, timeCommitId[field], sel.size() == 1 ? *sel.begin() : 0);
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

void SubsEditBox::OnActorChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Actor, ActorBox->GetValue(), _("actor change"), AssFile::COMMIT_DIAG_META);
	PopulateActorList();
}

void SubsEditBox::OnLayerChange(wxSpinEvent &event) {
	OnLayerEnter(event);
}

void SubsEditBox::OnLayerEnter(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Layer, Layer->GetValue(), _("layer change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::OnStartTimeChange(wxCommandEvent &) {
	if (StartTime->GetTime() > EndTime->GetTime()) EndTime->SetTime(StartTime->GetTime());
	CommitTimes(TIME_START);
}

void SubsEditBox::OnEndTimeChange(wxCommandEvent &) {
	if (StartTime->GetTime() > EndTime->GetTime()) StartTime->SetTime(EndTime->GetTime());
	CommitTimes(TIME_END);
}

void SubsEditBox::OnDurationChange(wxCommandEvent &) {
	if (ByFrame->GetValue())
		EndTime->SetFrame(StartTime->GetFrame() + Duration->GetFrame() - 1);
	else
		EndTime->SetTime(StartTime->GetTime() + Duration->GetTime());
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

static void set_margin_v(AssDialogue* diag, wxString value) {
	diag->SetMarginString(value, 2);
	diag->SetMarginString(value, 3);
}

void SubsEditBox::OnMarginVChange(wxCommandEvent &) {
	SetSelectedRows(set_margin_v, MarginV->GetValue(), _("MarginV change"), AssFile::COMMIT_DIAG_META);
	if (line) change_value(MarginV, line->GetMarginString(2, false));
}

void SubsEditBox::OnEffectChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Effect, Effect->GetValue(), _("effect change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::OnCommentChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Comment, CommentBox->GetValue(), _("comment change"), AssFile::COMMIT_DIAG_META);
}

void SubsEditBox::SetTag(wxString tag, wxString value, bool atEnd) {
	assert(line);
	if (line->Blocks.empty())
		line->ParseASSTags();

	std::pair<int, int> sel = get_selection(TextEdit);
	int start = atEnd ? sel.second : sel.first;
	int blockn = block_at_pos(line->Text, start);

	AssDialogueBlockPlain *plain = 0;
	AssDialogueBlockOverride *ovr = 0;
	while (blockn >= 0) {
		AssDialogueBlock *block = line->Blocks[blockn];
		if (dynamic_cast<AssDialogueBlockDrawing*>(block))
			--blockn;
		else if ((plain = dynamic_cast<AssDialogueBlockPlain*>(block))) {
			// Cursor is in a comment block, so try the previous block instead
			if (plain->GetText().StartsWith("{")) {
				--blockn;
				start = line->Text.rfind('{', start);
			}
			else
				break;
		}
		else {
			ovr = dynamic_cast<AssDialogueBlockOverride*>(block);
			assert(ovr);
			break;
		}
	}

	// If we didn't hit a suitable block for inserting the override just put
	// it at the beginning of the line
	if (blockn < 0)
		start = 0;

	wxString insert = tag + value;
	int shift = insert.size();
	if (plain || blockn < 0) {
		line->Text = line->Text.Left(start) + "{" + insert + "}" + line->Text.Mid(start);
		shift += 2;
		line->ParseASSTags();
	}
	else if(ovr) {
		wxString alt;
		if (tag == "\\c") alt = "\\1c";
		// Remove old of same
		bool found = false;
		for (size_t i = 0; i < ovr->Tags.size(); i++) {
			wxString name = ovr->Tags[i]->Name;
			if (tag == name || alt == name) {
				shift -= ((wxString)*ovr->Tags[i]).size();
				if (found) {
					delete ovr->Tags[i];
					ovr->Tags.erase(ovr->Tags.begin() + i);
					i--;
				}
				else {
					ovr->Tags[i]->Params[0]->Set(value);
					ovr->Tags[i]->Params[0]->omitted = false;
					found = true;
				}
			}
		}
		if (!found) {
			ovr->AddTag(insert);
		}

		line->UpdateText();
	}
	else
		assert(false);

	TextEdit->SetTextTo(line->Text);
	if (!atEnd) TextEdit->SetSelectionU(sel.first+shift,sel.second+shift);
	TextEdit->SetFocus();
}

void SubsEditBox::OnFlagButton(bool (AssStyle::*field), const char *tag, wxString const& undo_msg) {
	AssStyle *style = c->ass->GetStyle(line->Style);
	bool state = style ? style->*field : AssStyle().*field;

	line->ParseASSTags();
	std::pair<int, int> sel = get_selection(TextEdit);
	int blockn = block_at_pos(line->Text, sel.first);

	state = get_value(*line, blockn, state, tag);

	SetTag(tag, state ? "0" : "1");
	if (sel.first != sel.second)
		SetTag(tag, state ? "1" : "0", true);

	line->ClearBlocks();
	commitId = -1;
	CommitText(undo_msg);
}

void SubsEditBox::OnFontButton() {
	line->ParseASSTags();
	int blockn = block_at_pos(line->Text, get_selection(TextEdit).first);

	AssStyle *style = c->ass->GetStyle(line->Style);
	AssStyle defStyle;
	if (!style) style = &defStyle;

	wxFont startfont(
		get_value(*line, blockn, (int)style->fontsize, "\\fs"),
		wxFONTFAMILY_DEFAULT,
		get_value(*line, blockn, style->italic, "\\i") ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
		get_value(*line, blockn, style->bold, "\\b") ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
		get_value(*line, blockn, style->underline, "\\u"),
		get_value(*line, blockn, style->font, "\\fn"));

	wxFont font = wxGetFontFromUser(this, startfont);
	if (!font.Ok() || font == startfont) {
		line->ClearBlocks();
		return;
	}

	if (font.GetFaceName() != startfont.GetFaceName())
		SetTag("\\fn", font.GetFaceName());
	if (font.GetPointSize() != startfont.GetPointSize())
		SetTag("\\fs", wxString::Format("%d", font.GetPointSize()));
	if (font.GetWeight() != startfont.GetWeight())
		SetTag("\\b", wxString::Format("%d", font.GetWeight() == wxFONTWEIGHT_BOLD));
	if (font.GetStyle() != startfont.GetStyle())
		SetTag("\\i", wxString::Format("%d", font.GetStyle() == wxFONTSTYLE_ITALIC));
	if (font.GetUnderlined() != startfont.GetUnderlined())
		SetTag("\\i", wxString::Format("%d", font.GetUnderlined()));

	line->ClearBlocks();
	commitId = -1;
	CommitText(_("set font"));
}

void SubsEditBox::OnColorButton(AssColor (AssStyle::*field), const char *tag, const char *alt) {
	AssStyle *style = c->ass->GetStyle(line->Style);
	wxColor color = (style ? style->*field : AssStyle().*field).GetWXColor();

	colorTag = tag;
	commitId = -1;

	line->ParseASSTags();

	std::pair<int, int> sel = get_selection(TextEdit);
	int blockn = block_at_pos(line->Text, sel.first);

	color = get_value(*line, blockn, color, colorTag, alt);
	wxString initialText = line->Text;
	wxColor newColor = GetColorFromUser<SubsEditBox, &SubsEditBox::SetColorCallback>(c->parent, color, this);
	if (newColor == color) {
		TextEdit->SetTextTo(initialText);
		TextEdit->SetSelectionU(sel.first, sel.second);
	}

	line->ClearBlocks();
	CommitText(_("set color"));
}

void SubsEditBox::SetColorCallback(wxColor newColor) {
	if (newColor.Ok()) {
		SetTag(colorTag, AssColor(newColor).GetASSFormatted(false));
		CommitText(_("set color"));
	}
}
