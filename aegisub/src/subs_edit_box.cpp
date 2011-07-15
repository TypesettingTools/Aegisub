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
#ifdef _WIN32
#include <functional>
#else
#include <tr1/functional>
#endif

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

#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "ass_style.h"
#include "audio_controller.h"
#include "dialog_colorpicker.h"
#include "dialog_search_replace.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "timeedit_ctrl.h"
#include "tooltip_manager.h"
#include "utils.h"
#include "validators.h"
#include "video_context.h"
#include "video_display.h"

enum {
	BUTTON_BOLD = 1300,
	BUTTON_ITALICS,
	BUTTON_UNDERLINE,
	BUTTON_STRIKEOUT,
	BUTTON_FONT_NAME,
	BUTTON_COLOR1,
	BUTTON_COLOR2,
	BUTTON_COLOR3,
	BUTTON_COLOR4,
	BUTTON_COMMIT,
	BUTTON_LAST
};
enum {
	BUTTON_FIRST = BUTTON_BOLD
};

template<class T>
struct field_setter : public std::binary_function<AssDialogue*, T, void> {
	T AssDialogue::*field;
	field_setter(T AssDialogue::*field) : field(field) { }
	void operator()(AssDialogue* obj, T value) {
		obj->*field = value;
	}
};

/// @brief Get the selection from a text edit
/// @param[out] start Beginning of selection
/// @param[out] end   End of selection
void get_selection(SubsTextEditCtrl *TextEdit, int &start, int &end) {
	TextEdit->GetSelection(&start, &end);
	int len = TextEdit->GetText().size();
	start = mid(0,TextEdit->GetReverseUnicodePosition(start),len);
	end = mid(0,TextEdit->GetReverseUnicodePosition(end),len);
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

template<class Control>
struct FocusHandler : std::unary_function<wxFocusEvent &, void> {
	wxString value;
	wxString alt;
	wxColor color;
	Control *control;
	void operator()(wxFocusEvent &event) const {
		event.Skip();

		if (control->GetValue() == alt) {
			control->Freeze();
			control->ChangeValue(value);
			control->SetForegroundColour(color);
			control->Thaw();
		}
	}
};

template<class Event, class T>
void bind_focus_handler(T *control, Event event, wxString value, wxString alt, wxColor color) {
	FocusHandler<T> handler;
	handler.value = value;
	handler.alt = alt;
	handler.color = color;
	handler.control = control;
	control->Bind(event, handler);
}

SubsEditBox::SubsEditBox(wxWindow *parent, agi::Context *context)
: wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxRAISED_BORDER, "SubsEditBox")
, line(NULL)
, splitLineMode(false)
, controlState(true)
, c(context)
{
	// Top controls
	wxArrayString styles;
	styles.Add(_T("Default"));
	CommentBox = new wxCheckBox(this,wxID_ANY,_("Comment"));
	StyleBox = new wxComboBox(this,wxID_ANY,_T("Default"),wxDefaultPosition,wxSize(110,-1),styles,wxCB_READONLY | wxTE_PROCESS_ENTER);
	ActorBox = new wxComboBox(this,wxID_ANY,_T("Actor"),wxDefaultPosition,wxSize(110,-1),styles,wxCB_DROPDOWN | wxTE_PROCESS_ENTER);
	Effect = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxSize(80,-1),wxTE_PROCESS_ENTER);

	// Middle controls
	Layer = new wxSpinCtrl(this,wxID_ANY,"",wxDefaultPosition,wxSize(50,-1),wxSP_ARROW_KEYS,0,0x7FFFFFFF,0);
	StartTime = new TimeEdit(this,wxID_ANY,"",wxDefaultPosition,wxSize(75,-1),wxTE_PROCESS_ENTER);
	EndTime = new TimeEdit(this,wxID_ANY,"",wxDefaultPosition,wxSize(75,-1),wxTE_PROCESS_ENTER);
	EndTime->isEnd = true;
	Duration = new TimeEdit(this,wxID_ANY,"",wxDefaultPosition,wxSize(75,-1),wxTE_PROCESS_ENTER);
	MarginL = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxSize(40,-1),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginL->SetMaxLength(4);
	MarginR = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxSize(40,-1),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginR->SetMaxLength(4);
	MarginV = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxSize(40,-1),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginV->SetMaxLength(4);

	// Middle-bottom controls
	ToggableButtons.reserve(10);
	int id = BUTTON_FIRST;
#define MAKE_BUTTON(img, tooltip) \
	ToggableButtons.push_back(new wxBitmapButton(this, id++, GETIMAGE(img))); \
	ToggableButtons.back()->SetToolTip(tooltip);

	MAKE_BUTTON(button_bold_16, _("Bold"));
	MAKE_BUTTON(button_italics_16, _("Italics"));
	MAKE_BUTTON(button_underline_16, _("Underline"));
	MAKE_BUTTON(button_strikeout_16, _("Strikeout"));
	MAKE_BUTTON(button_fontname_16, _("Font Face Name"));
	MAKE_BUTTON(button_color_one_16, _("Primary color"));
	MAKE_BUTTON(button_color_two_16, _("Secondary color"));
	MAKE_BUTTON(button_color_three_16, _("Outline color"));
	MAKE_BUTTON(button_color_four_16, _("Shadow color"));
	MAKE_BUTTON(button_audio_commit_16, _("Commits the text (Enter). Hold Ctrl to stay in line (%KEY%)."));
#undef MAKE_BUTTON

	ByTime = new wxRadioButton(this,wxID_ANY,_("Time"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	ByFrame = new wxRadioButton(this,wxID_ANY,_("Frame"));
	ByFrame->Enable(false);

	// Tooltips
	CommentBox->SetToolTip(_("Comment this line out. Commented lines don't show up on screen."));
	StyleBox->SetToolTip(_("Style for this line."));
	ActorBox->SetToolTip(_("Actor name for this speech. This is only for reference, and is mainly useless."));
	Effect->SetToolTip(_("Effect for this line. This can be used to store extra information for karaoke scripts, or for the effects supported by the renderer."));
	Layer->SetToolTip(_("Layer number"));
	StartTime->SetToolTip(_("Start time"));
	EndTime->SetToolTip(_("End time"));
	Duration->SetToolTip(_("Line duration"));
	MarginL->SetToolTip(_("Left Margin (0 = default)"));
	MarginR->SetToolTip(_("Right Margin (0 = default)"));
	MarginV->SetToolTip(_("Vertical Margin (0 = default)"));
	ByTime->SetToolTip(_("Time by h:mm:ss.cs"));
	ByFrame->SetToolTip(_("Time by frame number"));

	// Top sizer
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(CommentBox,0,wxRIGHT | wxALIGN_CENTER,5);
	TopSizer->Add(StyleBox,2,wxRIGHT|wxALIGN_CENTER,5);
	TopSizer->Add(ActorBox,2,wxRIGHT|wxALIGN_CENTER,5);
	TopSizer->Add(Effect,3,wxALIGN_CENTER,5);

	// Middle sizer
	splitLineMode = true;
	MiddleSizer = new wxBoxSizer(wxHORIZONTAL);
	MiddleSizer->Add(Layer,0,wxRIGHT|wxALIGN_CENTER,5);
	MiddleSizer->Add(StartTime,0,wxRIGHT|wxALIGN_CENTER,0);
	MiddleSizer->Add(EndTime,0,wxRIGHT|wxALIGN_CENTER,5);
	MiddleSizer->Add(Duration,0,wxRIGHT|wxALIGN_CENTER,5);
	MiddleSizer->Add(MarginL,0,wxALIGN_CENTER,0);
	MiddleSizer->Add(MarginR,0,wxALIGN_CENTER,0);
	MiddleSizer->Add(MarginV,0,wxALIGN_CENTER,0);
	MiddleSizer->AddSpacer(5);

	// Middle-bottom sizer
	MiddleBotSizer = new wxBoxSizer(wxHORIZONTAL);
	for (size_t i = 0; i < ToggableButtons.size(); ++i) {
		MiddleBotSizer->Add(ToggableButtons[i],0,wxALIGN_CENTER|wxEXPAND,0);
		if (i == 4 || i == 8)
			MiddleBotSizer->AddSpacer(5);
	}
	MiddleBotSizer->AddSpacer(10);
	MiddleBotSizer->Add(ByTime,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,5);
	MiddleBotSizer->Add(ByFrame,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,5);

	// Text editor
	TextEdit = new SubsTextEditCtrl(this, wxSize(300,50), wxBORDER_SUNKEN, c->subsGrid);
	TextEdit->Bind(wxEVT_KEY_DOWN, &SubsEditBox::OnKeyDown, this);
	TextEdit->SetUndoCollection(false);
	BottomSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer->Add(TextEdit,1,wxEXPAND,0);

	// Main sizer
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxEXPAND | wxALL,3);
	MainSizer->Add(MiddleSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	MainSizer->Add(MiddleBotSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	MainSizer->Add(BottomSizer,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);

	origBgColour = TextEdit->GetBackgroundColour();
	disabledBgColour = GetBackgroundColour();

	wxColor text = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
	wxColor grey((text.Red() + origBgColour.Red()) / 2,
	             (text.Green() + origBgColour.Green()) / 2,
	             (text.Blue() + origBgColour.Blue()) / 2);

	// Setup placeholders for effect and actor boxes
	bind_focus_handler(Effect, wxEVT_SET_FOCUS, "", L"Effect", text);
	bind_focus_handler(Effect, wxEVT_KILL_FOCUS, L"Effect", "", grey);
	Effect->SetForegroundColour(grey);

	bind_focus_handler(ActorBox, wxEVT_SET_FOCUS, "", L"Actor", text);
	bind_focus_handler(ActorBox, wxEVT_KILL_FOCUS, L"Actor", "", grey);
	ActorBox->SetForegroundColour(grey);

	TextEdit->Bind(wxEVT_STC_STYLENEEDED, &SubsEditBox::OnNeedStyle, this);
	TextEdit->Bind(wxEVT_STC_MODIFIED, &SubsEditBox::OnChange, this);
	TextEdit->SetModEventMask(wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT);

	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &SubsEditBox::OnFrameTimeRadio, this, ByFrame->GetId());
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &SubsEditBox::OnFrameTimeRadio, this, ByTime->GetId());

	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnStyleChange, this, StyleBox->GetId());
	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &SubsEditBox::OnActorChange, this, ActorBox->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnActorChange, this, ActorBox->GetId());

	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnLayerEnter, this, Layer->GetId());
	Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &SubsEditBox::OnLayerChange, this, Layer->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnStartTimeChange, this, StartTime->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnEndTimeChange, this, EndTime->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnDurationChange, this, Duration->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnMarginLChange, this, MarginL->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnMarginRChange, this, MarginR->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnMarginVChange, this, MarginV->GetId());
	Bind(wxEVT_COMMAND_TEXT_UPDATED, &SubsEditBox::OnEffectChange, this, Effect->GetId());
	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &SubsEditBox::OnCommentChange, this, CommentBox->GetId());

	Bind(wxEVT_SIZE, &SubsEditBox::OnSize, this);

	for (int i = 0; i < 4; i++) {
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SubsEditBox::OnFlagButton, this, BUTTON_FIRST + i);
	}
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SubsEditBox::OnFontButton, this, BUTTON_FONT_NAME);
	for (int i = 5; i < 9; i++) {
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SubsEditBox::OnColorButton, this, BUTTON_FIRST + i);
	}
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SubsEditBox::OnCommitButton, this, BUTTON_COMMIT);

	wxSizeEvent evt;
	OnSize(evt);

	c->subsGrid->AddSelectionListener(this);
	c->ass->AddCommitListener(&SubsEditBox::Update, this);
	context->videoController->AddTimecodesListener(&SubsEditBox::UpdateFrameTiming, this);
}
SubsEditBox::~SubsEditBox() {
	c->subsGrid->RemoveSelectionListener(this);
}

void SubsEditBox::Update(int type) {
	SetEvtHandlerEnabled(false);
	if (type == AssFile::COMMIT_FULL || type == AssFile::COMMIT_UNDO) {
		/// @todo maybe preserve selection over undo?
		StyleBox->Clear();
		StyleBox->Append(c->ass->GetStyles());

		ActorBox->Freeze();
		ActorBox->Clear();
		int nrows = c->subsGrid->GetRows();
		for (int i=0;i<nrows;i++) {
			wxString actor = c->subsGrid->GetDialogue(i)->Actor;
			// OSX doesn't like combo boxes that are empty.
			if (actor.empty()) actor = "Actor";
			if (ActorBox->FindString(actor) == wxNOT_FOUND) {
				ActorBox->Append(actor);
			}
		}
		ActorBox->Thaw();

		TextEdit->SetSelection(0,0);
		SetEvtHandlerEnabled(true);
		return;
	}

	SetControlsState(!!line);
	if (!line) {
		SetEvtHandlerEnabled(true);
		return;
	}

	StartTime->SetTime(line->Start);
	EndTime->SetTime(line->End);
	Duration->SetTime(line->End-line->Start);
	if (type != AssFile::COMMIT_TIMES) {
		TextEdit->SetTextTo(line->Text);
		Layer->SetValue(line->Layer);
		MarginL->ChangeValue(line->GetMarginString(0,false));
		MarginR->ChangeValue(line->GetMarginString(1,false));
		MarginV->ChangeValue(line->GetMarginString(2,false));
		Effect->ChangeValue(line->Effect.empty() ?  L"Effect" : line->Effect);
		CommentBox->SetValue(line->Comment);
		StyleBox->Select(StyleBox->FindString(line->Style));
		ActorBox->ChangeValue(line->Actor.empty() ?  L"Actor" : line->Actor);
		ActorBox->SetStringSelection(line->Actor);
	}

	SetEvtHandlerEnabled(true);
}

void SubsEditBox::OnActiveLineChanged(AssDialogue *new_line) {
	SetEvtHandlerEnabled(false);
	line = new_line;

	Update(AssFile::COMMIT_TEXT);

	/// @todo VideoContext should be doing this
	if (c->videoController->IsLoaded()) {
		bool sync;
		if (Search.hasFocus) sync = OPT_GET("Tool/Search Replace/Video Update")->GetBool();
		else sync = OPT_GET("Video/Subtitle Sync")->GetBool();

		if (sync) {
			c->videoController->Stop();
			c->videoController->JumpToTime(line->Start.GetMS());
		}
	}
	SetEvtHandlerEnabled(true);
}
void SubsEditBox::OnSelectedSetChanged(const Selection &, const Selection &) {
	sel = c->subsGrid->GetSelectedSet();
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
	event.StopPropagation();
	if (hotkey::check("Subtitle Edit Box", event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		return;

	int key = event.GetKeyCode();
	if (line && (key == WXK_RETURN || key == WXK_NUMPAD_ENTER)) {
		NextLine();
	}
	else {
		event.Skip();
	}
}

void SubsEditBox::OnCommitButton(wxCommandEvent &) {
	if (line) NextLine();
}

void SubsEditBox::NextLine() {
	int next = c->subsGrid->GetLastSelRow() + 1;
	if (next >= c->subsGrid->GetRows()) {
		AssDialogue *cur = c->subsGrid->GetDialogue(next-1);
		AssDialogue *newline = new AssDialogue;
		newline->Start = cur->End;
		newline->End = cur->End + OPT_GET("Timing/Default Duration")->GetInt();
		newline->Style = cur->Style;
		c->subsGrid->InsertLine(newline,next-1,true,true);
	}
	c->subsGrid->NextLine();
}

void SubsEditBox::OnChange(wxStyledTextEvent &event) {
	if (line && TextEdit->GetText() != line->Text) {
		if (event.GetModificationType() & wxSTC_MOD_INSERTTEXT) {
			CommitText(_("insert text"));
		}
		else {
			CommitText(_("delete text"));
		}
	}
}

template<class T, class setter>
void SubsEditBox::SetSelectedRows(setter set, T value, wxString desc, bool amend) {
	using namespace std::tr1::placeholders;

	for_each(sel.begin(), sel.end(), std::tr1::bind(set, _1, value));

	commitId = c->ass->Commit(desc, AssFile::COMMIT_TEXT, (amend && desc == lastCommitType) ? commitId : -1);
	lastCommitType = desc;
}

template<class T>
void SubsEditBox::SetSelectedRows(T AssDialogue::*field, T value, wxString desc, bool amend) {
	SetSelectedRows(field_setter<T>(field), value, desc, amend);
}

void SubsEditBox::CommitText(wxString desc) {
	SetSelectedRows(&AssDialogue::Text, TextEdit->GetText(), desc, true);
}

void SubsEditBox::CommitTimes(TimeField field) {
	Duration->SetTime(EndTime->time - StartTime->time);

	// Update lines
	for (Selection::iterator cur = sel.begin(); cur != sel.end(); ++cur) {
		switch (field) {
			case TIME_START:
				(*cur)->Start = StartTime->time;
				if ((*cur)->Start > (*cur)->End) (*cur)->End = (*cur)->Start;
				break;
			case TIME_END:
				(*cur)->End = EndTime->time;
				if ((*cur)->Start > (*cur)->End) (*cur)->Start = (*cur)->End;
				break;
			case TIME_DURATION:
				(*cur)->End = (*cur)->Start + Duration->time;
				break;
		}
	}

	timeCommitId[field] = c->ass->Commit(_("modify times"), AssFile::COMMIT_TIMES, timeCommitId[field]);
}

void SubsEditBox::OnSize(wxSizeEvent &evt) {
	int topWidth = TopSizer->GetSize().GetWidth();
	int midMin = MiddleSizer->GetMinSize().GetWidth();
	int botMin = MiddleBotSizer->GetMinSize().GetWidth();

	if (splitLineMode) {
		if (topWidth >= midMin + botMin) {
			MainSizer->Detach(MiddleBotSizer);
			MiddleSizer->Add(MiddleBotSizer,0,wxALIGN_CENTER_VERTICAL);
			splitLineMode = false;
		}
	}
	else {
		if (topWidth < midMin) {
			MiddleSizer->Detach(MiddleBotSizer);
			MainSizer->Insert(2,MiddleBotSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
			splitLineMode = true;
		}
	}

	evt.Skip();
}

void SubsEditBox::OnNeedStyle(wxStyledTextEvent &event) {
	// Check if it needs to fix text
	wxString text = TextEdit->GetText();
	if (text.Contains("\n") || text.Contains("\r")) {
		TextEdit->SetTextTo(text);
	}
	// Just update style
	else TextEdit->UpdateStyle();
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

	// HACK: TextEdit workaround the stupid colour lock bug
	TextEdit->SetReadOnly(!state);
	if (state) TextEdit->SetBackgroundColour(origBgColour);
	else TextEdit->SetBackgroundColour(disabledBgColour);

	// Sets controls
	StartTime->Enable(state);
	EndTime->Enable(state);
	Duration->Enable(state);
	Layer->Enable(state);
	MarginL->Enable(state);
	MarginR->Enable(state);
	MarginV->Enable(state);
	Effect->Enable(state);
	CommentBox->Enable(state);
	StyleBox->Enable(state);
	ActorBox->Enable(state);
	ByTime->Enable(state);
	for (size_t i = 0; i < ToggableButtons.size(); ++i)
		ToggableButtons[i]->Enable(state);

	if (!state) {
		SetEvtHandlerEnabled(false);
		TextEdit->SetTextTo("");
		StartTime->SetTime(0);
		EndTime->SetTime(0);
		Duration->SetTime(0);
		Layer->SetValue("");
		MarginL->ChangeValue("");
		MarginR->ChangeValue("");
		MarginV->ChangeValue("");
		Effect->ChangeValue("");
		CommentBox->SetValue(false);
		SetEvtHandlerEnabled(true);
	}
}


void SubsEditBox::OnStyleChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Style, StyleBox->GetValue(), _("style change"));
}

void SubsEditBox::OnActorChange(wxCommandEvent &) {
	wxString actor = ActorBox->GetValue();
	SetSelectedRows(&AssDialogue::Actor, actor, _("actor change"));

	// Add actor to list
	if (!actor.empty() && ActorBox->GetCount() && ActorBox->GetString(0).empty()) {
		ActorBox->Delete(0);
	}
	if (ActorBox->FindString(actor) == wxNOT_FOUND) {
		ActorBox->Append(actor);
	}
}

void SubsEditBox::OnLayerChange(wxSpinEvent &event) {
	OnLayerEnter(event);
}

void SubsEditBox::OnLayerEnter(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Layer, Layer->GetValue(), _("layer change"));
}

void SubsEditBox::OnStartTimeChange(wxCommandEvent &) {
	if (StartTime->time > EndTime->time) EndTime->SetTime(StartTime->time);
	CommitTimes(TIME_START);
}

void SubsEditBox::OnEndTimeChange(wxCommandEvent &) {
	if (StartTime->time > EndTime->time) StartTime->SetTime(EndTime->time);
	CommitTimes(TIME_END);
}

void SubsEditBox::OnDurationChange(wxCommandEvent &) {
	EndTime->SetTime(StartTime->time + Duration->time);
	CommitTimes(TIME_DURATION);
}
void SubsEditBox::OnMarginLChange(wxCommandEvent &) {
	SetSelectedRows(std::mem_fun(&AssDialogue::SetMarginString<0>), MarginL->GetValue(), _("MarginL change"));
	AssDialogue *cur = c->subsGrid->GetDialogue(c->subsGrid->GetFirstSelRow());
	if (cur)
		MarginL->ChangeValue(cur->GetMarginString(0,false));
}

void SubsEditBox::OnMarginRChange(wxCommandEvent &) {
	SetSelectedRows(std::mem_fun(&AssDialogue::SetMarginString<1>), MarginR->GetValue(), _("MarginR change"));
	AssDialogue *cur = c->subsGrid->GetDialogue(c->subsGrid->GetFirstSelRow());
	if (cur)
		MarginR->ChangeValue(cur->GetMarginString(1,false));
}

static void set_margin_v(AssDialogue* diag, wxString value) {
	diag->SetMarginString(value, 2);
	diag->SetMarginString(value, 3);
}

void SubsEditBox::OnMarginVChange(wxCommandEvent &) {
	SetSelectedRows(set_margin_v, MarginV->GetValue(), _("MarginV change"));
	AssDialogue *cur = c->subsGrid->GetDialogue(c->subsGrid->GetFirstSelRow());
	if (cur)
		MarginV->ChangeValue(cur->GetMarginString(2,false));
}

void SubsEditBox::OnEffectChange(wxCommandEvent &) {
	SetSelectedRows(&AssDialogue::Effect, Effect->GetValue(), _("effect change"));
}

void SubsEditBox::OnCommentChange(wxCommandEvent &event) {
	SetSelectedRows(&AssDialogue::Comment, CommentBox->GetValue(), _("comment change"));
}

int SubsEditBox::BlockAtPos(int pos) const {
	int n=0;
	wxString text = TextEdit->GetText();;
	int max = text.Length()-1;

	for (int i=0;i<=pos && i<=max;i++) {
		if (i > 0 && text[i] == '{') n++;
		if (text[i] == '}' && i != max && i != pos && i != pos -1 && (i+1 == max || text[i+1] != '{')) n++;
	}

	return n;
}

void SubsEditBox::SetTag(wxString tag, wxString value, bool atEnd) {
	assert(line);
	assert(line->Valid);
	if (line->Blocks.empty())
		line->ParseASSTags();

	int selstart, selend;
	get_selection(TextEdit, selstart, selend);
	int start = atEnd ? selend : selstart;
	int blockn = BlockAtPos(start);

	AssDialogueBlock *block = line->Blocks[blockn];
	AssDialogueBlockPlain *plain = dynamic_cast<AssDialogueBlockPlain*>(block);
	AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride*>(block);

	// Drawings should always be preceded by an override block (with the \pX)
	if (dynamic_cast<AssDialogueBlockDrawing*>(block)) {
		assert(blockn > 0);
		ovr = dynamic_cast<AssDialogueBlockOverride*>(line->Blocks[blockn - 1]);
		assert(ovr);
	}

	wxString insert = tag + value;
	int shift = insert.size();
	if (plain) {
		line->Text = line->Text.Left(start) + "{" + insert + "}" + line->Text.Mid(start);
		shift += 2;
		line->ParseASSTags();
	}
	else if (ovr) {
		wxString alt;
		if (tag == L"\\c") alt = L"\\1c";
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
	TextEdit->SetTextTo(line->Text);
	if (!atEnd) TextEdit->SetSelectionU(selstart+shift,selend+shift);
	TextEdit->SetFocus();
}
void SubsEditBox::OnFlagButton(wxCommandEvent &evt) {
	int id = evt.GetId();
	assert(id < BUTTON_LAST && id >= BUTTON_FIRST);

	wxString tagname;
	wxString desc;
	bool state = false;
	AssStyle *style = c->ass->GetStyle(line->Style);
	AssStyle defStyle;
	if (!style) style = &defStyle;
	if (id == BUTTON_BOLD) {
		tagname = L"\\b";
		desc = _("toggle bold");
		state = style->bold;
	}
	else if (id == BUTTON_ITALICS) {
		tagname = L"\\i";
		desc = _("toggle italic");
		state = style->italic;
	}
	else if (id == BUTTON_UNDERLINE) {
		tagname = L"\\u";
		desc = _("toggle underline");
		state = style->underline;
	}
	else if (id == BUTTON_STRIKEOUT) {
		tagname = L"\\s";
		desc = _("toggle strikeout");
		state = style->strikeout;
	}
	else {
		return;
	}

	line->ParseASSTags();
	int selstart, selend;
	get_selection(TextEdit, selstart, selend);
	int blockn = BlockAtPos(selstart);

	state = get_value(*line, blockn, state, tagname);

	SetTag(tagname, wxString::Format("%i", !state));
	if (selend != selstart) {
		SetTag(tagname, wxString::Format("%i", state), true);
	}

	line->ClearBlocks();
	commitId = -1;
	CommitText(desc);
}
void SubsEditBox::OnFontButton(wxCommandEvent &) {
	int selstart, selend;
	get_selection(TextEdit, selstart, selend);

	line->ParseASSTags();
	int blockn = BlockAtPos(selstart);

	wxFont startfont;
	AssStyle *style = c->ass->GetStyle(line->Style);
	AssStyle defStyle;
	if (!style) style = &defStyle;

	startfont.SetFaceName(get_value(*line, blockn, style->font, L"\\fn"));
	startfont.SetPointSize(get_value(*line, blockn, (int)style->fontsize, L"\\fs"));
	startfont.SetWeight(get_value(*line, blockn, style->bold, L"\\b") ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
	startfont.SetStyle(get_value(*line, blockn, style->italic, L"\\i") ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
	startfont.SetUnderlined(get_value(*line, blockn, style->underline, L"\\u"));

	wxFont font = wxGetFontFromUser(this, startfont);
	if (!font.Ok() || font == startfont) {
		line->ClearBlocks();
		return;
	}

	if (font.GetFaceName() != startfont.GetFaceName()) {
		SetTag(L"\\fn", font.GetFaceName());
	}
	if (font.GetPointSize() != startfont.GetPointSize()) {
		SetTag(L"\\fs", wxString::Format("%i", font.GetPointSize()));
	}
	if (font.GetWeight() != startfont.GetWeight()) {
		SetTag(L"\\b", wxString::Format("%i", font.GetWeight() == wxFONTWEIGHT_BOLD));
	}
	if (font.GetStyle() != startfont.GetStyle()) {
		SetTag(L"\\i", wxString::Format("%i", font.GetStyle() == wxFONTSTYLE_ITALIC));
	}
	if (font.GetUnderlined() != startfont.GetUnderlined()) {
		SetTag(L"\\i", wxString::Format("%i", font.GetUnderlined()));
	}
	line->ClearBlocks();
	commitId = -1;
	CommitText(_("set font"));
}
void SubsEditBox::OnColorButton(wxCommandEvent &evt) {
	int id = evt.GetId();
	assert(id < BUTTON_LAST && id >= BUTTON_FIRST);
	wxString alt;

	wxColor color;
	AssStyle *style = c->ass->GetStyle(line->Style);
	AssStyle defStyle;
	if (!style) style = &defStyle;
	if (id == BUTTON_COLOR1) {
		color = style->primary.GetWXColor();
		colorTag = L"\\c";
		alt = L"\\c1";
	}
	else if (id == BUTTON_COLOR2) {
		color = style->secondary.GetWXColor();
		colorTag = L"\\2c";
	}
	else if (id == BUTTON_COLOR3) {
		color = style->outline.GetWXColor();
		colorTag = L"\\3c";
	}
	else if (id == BUTTON_COLOR4) {
		color = style->shadow.GetWXColor();
		colorTag = L"\\4c";
	}
	else {
		return;
	}

	commitId = -1;

	line->ParseASSTags();
	int selstart, selend;
	get_selection(TextEdit, selstart, selend);
	int blockn = BlockAtPos(selstart);

	color = get_value(*line, blockn, color, colorTag, alt);
	wxString initialText = line->Text;
	wxColor newColor = GetColorFromUser<SubsEditBox, &SubsEditBox::SetColorCallback>(c->parent, color, this);
	if (newColor == color) {
		TextEdit->SetTextTo(initialText);
		TextEdit->SetSelectionU(selstart, selend);
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
