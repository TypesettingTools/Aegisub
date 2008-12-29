// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


////////////
// Includes
#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "ass_override.h"
#include "timeedit_ctrl.h"
#include "vfr.h"
#include "options.h"
#include "audio_display.h"
#include "hilimod_textctrl.h"
#include "video_display.h"
#include "validators.h"
#include "dialog_colorpicker.h"
#include "main.h"
#include "frame_main.h"
#include "utils.h"
#include "dialog_search_replace.h"
#include "idle_field_event.h"
#include "float_spin.h"
#include "tooltip_manager.h"
#include "osx_bevelButton.h"


///////////////
// Constructor
SubsEditBox::SubsEditBox (wxWindow *parent,SubtitlesGrid *gridp) : wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxRAISED_BORDER, _T("SubsEditBox"))
{
	// Setup
	audio = NULL;
	grid = gridp;
	grid->editBox = this;
	enabled = false;
	textEditReady = true;
	controlState = true;
	setupDone = false;
	linen = -2;

	// Top controls
	wxArrayString styles;
	styles.Add(_T(""));
	CommentBox = new wxCheckBox(this,COMMENT_CHECKBOX,_("Comment"));
	CommentBox->SetToolTip(_("Comment this line out. Commented lines don't show up on screen."));
	StyleBox = new wxComboBox(this,STYLE_COMBOBOX,_T(""),wxDefaultPosition,wxSize(110,-1),styles,wxCB_READONLY | wxTE_PROCESS_ENTER);
	StyleBox->SetToolTip(_("Style for this line."));
	ActorBox = new wxComboBox(this,ACTOR_COMBOBOX,_T(""),wxDefaultPosition,wxSize(110,-1),styles,wxCB_DROPDOWN | wxTE_PROCESS_ENTER);
	ActorBox->SetToolTip(_("Actor name for this speech. This is only for reference, and is mainly useless."));
	ActorBox->PushEventHandler(new IdleFieldHandler(ActorBox,_("Actor")));
	Effect = new HiliModTextCtrl(this,EFFECT_BOX,_T(""),wxDefaultPosition,wxSize(80,-1),wxTE_PROCESS_ENTER);
	Effect->SetToolTip(_("Effect for this line. This can be used to store extra information for karaoke scripts, or for the effects supported by the renderer."));
	Effect->PushEventHandler(new IdleFieldHandler(Effect,_("Effect")));

	// Middle controls
	Layer = new wxSpinCtrl(this,LAYER_BOX,_T(""),wxDefaultPosition,wxSize(50,-1),wxSP_ARROW_KEYS,0,0x7FFFFFFF,0);
	Layer->SetToolTip(_("Layer number"));
	StartTime = new TimeEdit(this,STARTTIME_BOX,_T(""),wxDefaultPosition,wxSize(75,-1),wxTE_PROCESS_ENTER);
	StartTime->SetToolTip(_("Start time"));
	StartTime->showModified = true;
	EndTime = new TimeEdit(this,ENDTIME_BOX,_T(""),wxDefaultPosition,wxSize(75,-1),wxTE_PROCESS_ENTER);
	EndTime->SetToolTip(_("End time"));
	EndTime->isEnd = true;
	EndTime->showModified = true;
	Duration = new TimeEdit(this,DURATION_BOX,_T(""),wxDefaultPosition,wxSize(75,-1),wxTE_PROCESS_ENTER);
	Duration->SetToolTip(_("Line duration"));
	Duration->showModified = true;
	MarginL = new HiliModTextCtrl(this,MARGINL_BOX,_T(""),wxDefaultPosition,wxSize(40,-1),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginL->SetToolTip(_("Left Margin (0 = default)"));
	MarginL->SetMaxLength(4);
	MarginR = new HiliModTextCtrl(this,MARGINR_BOX,_T(""),wxDefaultPosition,wxSize(40,-1),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginR->SetToolTip(_("Right Margin (0 = default)"));
	MarginR->SetMaxLength(4);
	MarginV = new HiliModTextCtrl(this,MARGINV_BOX,_T(""),wxDefaultPosition,wxSize(40,-1),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginV->SetToolTip(_("Vertical Margin (0 = default)"));
	MarginV->SetMaxLength(4);

	// Middle-bottom controls
	Bold = new wxBitmapButton(this,BUTTON_BOLD,wxBITMAP(button_bold),wxDefaultPosition,wxSize(20,20));
	Bold->SetToolTip(_("Bold"));
	Italics = new wxBitmapButton(this,BUTTON_ITALICS,wxBITMAP(button_italics),wxDefaultPosition,wxSize(20,20));
	Italics->SetToolTip(_("Italics"));
	Underline = new wxBitmapButton(this,BUTTON_UNDERLINE,wxBITMAP(button_underline),wxDefaultPosition,wxSize(20,20));
	Underline->SetToolTip(_("Underline"));
	Strikeout = new wxBitmapButton(this,BUTTON_STRIKEOUT,wxBITMAP(button_strikeout),wxDefaultPosition,wxSize(20,20));
	Strikeout->SetToolTip(_("Strikeout"));
	FontName = new wxBitmapButton(this,BUTTON_FONT_NAME,wxBITMAP(button_fontname),wxDefaultPosition,wxSize(30,20));
	FontName->SetToolTip(_("Font Face Name"));
	Color1 = new wxBitmapButton(this,BUTTON_COLOR1,wxBITMAP(button_color_one),wxDefaultPosition,wxSize(30,20));
	Color1->SetToolTip(_("Primary color"));
	Color2 = new wxBitmapButton(this,BUTTON_COLOR2,wxBITMAP(button_color_two),wxDefaultPosition,wxSize(30,20));
	Color2->SetToolTip(_("Secondary color"));
	Color3 = new wxBitmapButton(this,BUTTON_COLOR3,wxBITMAP(button_color_three),wxDefaultPosition,wxSize(30,20));
	Color3->SetToolTip(_("Outline color"));
	Color4 = new wxBitmapButton(this,BUTTON_COLOR4,wxBITMAP(button_color_four),wxDefaultPosition,wxSize(30,20));
	Color4->SetToolTip(_("Shadow color"));
#ifdef __WXMAC__
	CommitButton = new wxBevelButton(this,BUTTON_COMMIT,_("Commit"),wxDefaultPosition,wxDefaultSize);
#else
	CommitButton = new wxButton(this,BUTTON_COMMIT,_("Commit"),wxDefaultPosition,wxDefaultSize);
#endif
	ToolTipManager::Bind(CommitButton,_("Commits the text (Enter). Hold Ctrl to stay in line (%KEY%)."),_T("Edit Box Commit"));
	ByTime = new wxRadioButton(this,RADIO_TIME_BY_TIME,_("Time"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	ByTime->SetToolTip(_("Time by h:mm:ss.cs"));
	ByFrame = new wxRadioButton(this,RADIO_TIME_BY_FRAME,_("Frame"));
	ByFrame->SetToolTip(_("Time by frame number"));

	// Top sizer
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	//TopSizer->Add(new FloatSpinCtrl(this,-1,wxDefaultPosition,wxSize(40,20),0,-20.0,50.0,0.0,0.5));
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
	MiddleBotSizer->Add(Bold,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(Italics,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(Underline,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(Strikeout,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(FontName,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->AddSpacer(5);
	MiddleBotSizer->Add(Color1,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(Color2,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(Color3,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->Add(Color4,0,wxALIGN_CENTER|wxEXPAND,0);
	MiddleBotSizer->AddSpacer(5);
	MiddleBotSizer->Add(CommitButton,0,wxALIGN_CENTER,0);
	MiddleBotSizer->AddSpacer(10);
	MiddleBotSizer->Add(ByTime,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,5);
	MiddleBotSizer->Add(ByFrame,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,5);

	// Text editor
	int textStyle = 0;
#if wxCHECK_VERSION(2,9,0)
	textStyle = wxBORDER_SUNKEN;
#else
#ifdef _WIN32
	textStyle = wxWindow::GetThemedBorderStyle();
#else
	textStyle = wxBORDER_SUNKEN;
#endif
#endif
	TextEdit = new SubsTextEditCtrl(this,EDIT_BOX,_T(""),wxDefaultPosition,wxSize(300,50),textStyle);
	TextEdit->PushEventHandler(new SubsEditBoxEvent(this));
	TextEdit->control = this;
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

	// HACK: Fix colour of bg of editbox
	origBgColour = TextEdit->GetBackgroundColour();
	disabledBgColour = GetBackgroundColour();

	// Set split mode
	setupDone = true;
	SetSplitLineMode();
	Update();
}


/////////////////////////////////
// Set split or single line mode
void SubsEditBox::SetSplitLineMode(wxSize newSize) {
	// Widths
	int topWidth;
	if (newSize.GetWidth() == -1) topWidth = TopSizer->GetSize().GetWidth();
	else topWidth = newSize.GetWidth()-GetSize().GetWidth()+GetClientSize().GetWidth();
	int midMin = MiddleSizer->GetMinSize().GetWidth();
	int botMin = MiddleBotSizer->GetMinSize().GetWidth();

	// Currently split
	if (splitLineMode) {

		if (topWidth >= midMin + botMin) {
			MainSizer->Detach(MiddleBotSizer);
			MiddleSizer->Add(MiddleBotSizer,0,wxALIGN_CENTER_VERTICAL);
			Layout();
			splitLineMode = false;
		}
	}

	// Currently joined
	else {
		if (topWidth < midMin) {
			MiddleSizer->Detach(MiddleBotSizer);
			MainSizer->Insert(2,MiddleBotSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
			Layout();
			splitLineMode = true;
		}
	}
}


///////////////////
// Update function
void SubsEditBox::Update (bool timeOnly,bool weak) {
	if (enabled) {
		AssDialogue *curdiag = grid->GetDialogue(linen);
		if (curdiag) {
			// Controls
			SetControlsState(true);
			int start = curdiag->Start.GetMS();
			int end = curdiag->End.GetMS();
			StartTime->SetTime(start);
			EndTime->SetTime(end);
			Duration->SetTime(end-start);
			if (!timeOnly) {
				TextEdit->SetTextTo(curdiag->Text);
				Layer->SetValue(wxString::Format(_T("%i"),curdiag->Layer));
				MarginL->SetValue(curdiag->GetMarginString(0,false));
				MarginR->SetValue(curdiag->GetMarginString(1,false));
				MarginV->SetValue(curdiag->GetMarginString(2,false));
				Effect->SetValue(curdiag->Effect);
				CommentBox->SetValue(curdiag->Comment);
				StyleBox->Select(StyleBox->FindString(curdiag->Style));
				ActorBox->SetValue(curdiag->Actor);
				ActorBox->SetStringSelection(curdiag->Actor);

				// Force actor box to update its idle status
				wxCommandEvent changeEvent(wxEVT_COMMAND_TEXT_UPDATED,ActorBox->GetId());
				ActorBox->GetEventHandler()->AddPendingEvent(changeEvent);
			}

			// Audio
			if (!weak) audio->SetDialogue(grid,curdiag,linen);

			// Video
			VideoContext::Get()->curLine = curdiag;
			VideoContext::Get()->UpdateDisplays(false);
		}
		else enabled = false;
	}
	
	else {
		SetControlsState(false);
	}
}


//////////////////
// Update globals
void SubsEditBox::UpdateGlobals () {
	// Styles
	StyleBox->Clear();
	StyleBox->Append(grid->ass->GetStyles());

	// Actors
	ActorBox->Freeze();
	ActorBox->Clear();
	int nrows = grid->GetRows();
	wxString actor;
	for (int i=0;i<nrows;i++) {
		actor = grid->GetDialogue(i)->Actor;
		if (ActorBox->FindString(actor) == wxNOT_FOUND) {
			ActorBox->Append(actor);
		}
	}
	ActorBox->Thaw();

	// Set subs update
	linen = -2;
	TextEdit->SetSelection(0,0);
	SetToLine(grid->GetFirstSelRow());
}


//////////////////
// Jump to a line
void SubsEditBox::SetToLine(int n,bool weak) {
	// Set to nothing
	if (n == -1) {
		enabled = false;
	}

	// Set line
	else if (grid->GetDialogue(n)) {
		enabled = true;
		if (n != linen) {
			linen = n;
			StartTime->Update();
			EndTime->Update();
			Duration->Update();
		}
	}

	// Update controls
	Update();

	// Set video
	if (VideoContext::Get()->IsLoaded() && !weak) {
		wxString sync;
		if (Search.hasFocus) sync = _T("Find update video");
		else sync = _T("Sync video with subs");
		
		if (Options.AsBool(sync) == true) {
			VideoContext::Get()->Stop();
			AssDialogue *cur = grid->GetDialogue(n);
			if (cur) VideoContext::Get()->JumpToFrame(VFR_Output.GetFrameAtTime(cur->Start.GetMS(),true));
		}
	}
}


///////////////
// Event table
BEGIN_EVENT_TABLE(SubsEditBox, wxPanel)
	EVT_STC_MODIFIED(EDIT_BOX,SubsEditBox::OnEditText)
	EVT_STC_STYLENEEDED(EDIT_BOX,SubsEditBox::OnNeedStyle)
	EVT_STC_KEY(EDIT_BOX,SubsEditBox::OnKeyDown)
	EVT_STC_CHARADDED(EDIT_BOX,SubsEditBox::OnCharAdded)
	EVT_STC_UPDATEUI(EDIT_BOX,SubsEditBox::OnUpdateUI)

	EVT_CHECKBOX(SYNTAX_BOX, SubsEditBox::OnSyntaxBox)
	EVT_RADIOBUTTON(RADIO_TIME_BY_FRAME, SubsEditBox::OnFrameRadio)
	EVT_RADIOBUTTON(RADIO_TIME_BY_TIME, SubsEditBox::OnTimeRadio)
	EVT_COMBOBOX(STYLE_COMBOBOX, SubsEditBox::OnStyleChange)
	EVT_COMBOBOX(ACTOR_COMBOBOX, SubsEditBox::OnActorChange)
	EVT_TEXT_ENTER(ACTOR_COMBOBOX, SubsEditBox::OnActorChange)
	EVT_TEXT_ENTER(LAYER_BOX, SubsEditBox::OnLayerEnter)
	EVT_SPINCTRL(LAYER_BOX, SubsEditBox::OnLayerChange)
	EVT_TEXT_ENTER(STARTTIME_BOX, SubsEditBox::OnStartTimeChange)
	EVT_TEXT_ENTER(ENDTIME_BOX, SubsEditBox::OnEndTimeChange)
	EVT_TEXT_ENTER(DURATION_BOX, SubsEditBox::OnDurationChange)
	EVT_TEXT_ENTER(MARGINL_BOX, SubsEditBox::OnMarginLChange)
	EVT_TEXT_ENTER(MARGINR_BOX, SubsEditBox::OnMarginRChange)
	EVT_TEXT_ENTER(MARGINV_BOX, SubsEditBox::OnMarginVChange)
	EVT_TEXT_ENTER(EFFECT_BOX, SubsEditBox::OnEffectChange)
	EVT_CHECKBOX(COMMENT_CHECKBOX, SubsEditBox::OnCommentChange)

	EVT_BUTTON(BUTTON_COLOR1,SubsEditBox::OnButtonColor1)
	EVT_BUTTON(BUTTON_COLOR2,SubsEditBox::OnButtonColor2)
	EVT_BUTTON(BUTTON_COLOR3,SubsEditBox::OnButtonColor3)
	EVT_BUTTON(BUTTON_COLOR4,SubsEditBox::OnButtonColor4)
	EVT_BUTTON(BUTTON_FONT_NAME,SubsEditBox::OnButtonFontFace)
	EVT_BUTTON(BUTTON_BOLD,SubsEditBox::OnButtonBold)
	EVT_BUTTON(BUTTON_ITALICS,SubsEditBox::OnButtonItalics)
	EVT_BUTTON(BUTTON_UNDERLINE,SubsEditBox::OnButtonUnderline)
	EVT_BUTTON(BUTTON_STRIKEOUT,SubsEditBox::OnButtonStrikeout)
	EVT_BUTTON(BUTTON_COMMIT,SubsEditBox::OnButtonCommit)

	EVT_SIZE(SubsEditBox::OnSize)
END_EVENT_TABLE()


///////////
// On size
void SubsEditBox::OnSize(wxSizeEvent &event) {
	if (setupDone) SetSplitLineMode(event.GetSize());
	event.Skip();
}


/////////////////////
// Text edited event
void SubsEditBox::OnEditText(wxStyledTextEvent &event) {
	int modType = event.GetModificationType();
	if (modType == (wxSTC_MOD_INSERTTEXT | wxSTC_PERFORMED_USER) || modType == (wxSTC_MOD_DELETETEXT | wxSTC_PERFORMED_USER)) {
		//TextEdit->UpdateCallTip();
	}
}


//////////////////////////
// User Interface updated
void SubsEditBox::OnUpdateUI(wxStyledTextEvent &event) {
	TextEdit->UpdateCallTip();
}


//////////////
// Need style
void SubsEditBox::OnNeedStyle(wxStyledTextEvent &event) {
	// Check if it needs to fix text
	wxString text = TextEdit->GetText();
	if (text.Contains(_T("\n")) || text.Contains(_T("\r"))) {
		TextEdit->SetTextTo(TextEdit->GetText());
	}

	// Just update style
	else TextEdit->UpdateStyle();
}


///////////////////
// Character added
void SubsEditBox::OnCharAdded(wxStyledTextEvent &event) {
	//int character = event.GetKey();
}


////////////
// Key down
void SubsEditBox::OnKeyDown(wxStyledTextEvent &event) {
}


/////////////////////////////
// Syntax highlight checkbox
void SubsEditBox::OnSyntaxBox(wxCommandEvent &event) {
	TextEdit->UpdateStyle();
	Options.SetBool(_T("Syntax Highlight Enabled"),SyntaxHighlight->GetValue());
	Options.Save();
	event.Skip();
}


//////////////////////////
// Time by frame radiobox
void SubsEditBox::OnFrameRadio(wxCommandEvent &event) {
	if (ByFrame->GetValue()) {
		StartTime->SetByFrame(true);
		EndTime->SetByFrame(true);
		Duration->SetByFrame(true);
		grid->SetByFrame(true);
	}
	event.Skip();
}


//////////////////////////
// Standard time radiobox
void SubsEditBox::OnTimeRadio(wxCommandEvent &event) {
	if (ByTime->GetValue()) {
		StartTime->SetByFrame(false);
		EndTime->SetByFrame(false);
		Duration->SetByFrame(false);
		grid->SetByFrame(false);
	}
	event.Skip();
}


//////////////////////////////////////////////////
// Sets state (enabled/disabled) for all controls
void SubsEditBox::SetControlsState (bool state) {
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
	//SyntaxHighlight->Enable(state);
	Bold->Enable(state);
	Italics->Enable(state);
	Underline->Enable(state);
	Strikeout->Enable(state);
	Color1->Enable(state);
	Color2->Enable(state);
	Color3->Enable(state);
	Color4->Enable(state);
	FontName->Enable(state);
	CommitButton->Enable(state);

	UpdateFrameTiming();

	// Clear values if it's false
	if (state==false) {
		TextEdit->SetTextTo(_T(""));
		StartTime->SetTime(0);
		EndTime->SetTime(0);
		Duration->SetTime(0);
		Layer->SetValue(_T(""));
		MarginL->SetValue(_T(""));
		MarginR->SetValue(_T(""));
		MarginV->SetValue(_T(""));
		Effect->SetValue(_T(""));
		CommentBox->SetValue(false);
	}
}


////////////////////////////////////
// Disables or enables frame timing
void SubsEditBox::UpdateFrameTiming () {
	if (VFR_Output.IsLoaded()) ByFrame->Enable(enabled);
	else {
		ByFrame->Enable(false);
		ByTime->SetValue(true);
		StartTime->SetByFrame(false);
		EndTime->SetByFrame(false);
		grid->SetByFrame(false);
	}
}


/////////////////
// Style changed
void SubsEditBox::OnStyleChange(wxCommandEvent &event) {
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->Style = StyleBox->GetValue();
			cur->UpdateData();
		}
	}
	grid->ass->FlagAsModified(_("style change"));
	grid->CommitChanges();
	grid->EndBatch();
}


/////////////////
// Style changed
void SubsEditBox::OnActorChange(wxCommandEvent &event) {
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	AssDialogue *cur;
	wxString actor = ActorBox->GetValue();

	// Update rows
	int n = sel.Count();
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->Actor = actor;
			cur->UpdateData();
		}
	}

	// Add actor to list
	if (ActorBox->GetString(0).IsEmpty()) ActorBox->Delete(0);
	if (ActorBox->FindString(actor) == wxNOT_FOUND) {
		ActorBox->Append(actor);
	}

	// Update grid
	grid->ass->FlagAsModified(_("actor change"));
	grid->CommitChanges();
	grid->EndBatch();
}


///////////////////////////
// Layer changed with spin
void SubsEditBox::OnLayerChange(wxSpinEvent &event) {
	// Value
	long temp = event.GetPosition();

	// Get selection
	wxArrayInt sel = grid->GetSelection();

	// Update
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->Layer = temp;
			cur->UpdateData();
		}
	}

	// Done
	grid->ass->FlagAsModified(_("layer change"));
	grid->CommitChanges();
}


////////////////////////////
// Layer changed with enter
void SubsEditBox::OnLayerEnter(wxCommandEvent &event) {
	// Value
	long temp = Layer->GetValue();

	// Get selection
	wxArrayInt sel = grid->GetSelection();

	// Update
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->Layer = temp;
			cur->UpdateData();
		}
	}

	// Done
	grid->ass->FlagAsModified(_("layer change"));
	grid->CommitChanges();
}


//////////////////////
// Start time changed
void SubsEditBox::OnStartTimeChange(wxCommandEvent &event) {
	if (StartTime->time > EndTime->time) StartTime->SetTime(EndTime->time.GetMS());
	bool join = Options.AsBool(_T("Link Time Boxes Commit")) && EndTime->HasBeenModified();
	StartTime->Update();
	Duration->Update();
	if (join) EndTime->Update();
	CommitTimes(true,join,true);
}


////////////////////
// End time changed
void SubsEditBox::OnEndTimeChange(wxCommandEvent &event) {
	if (StartTime->time > EndTime->time) EndTime->SetTime(StartTime->time.GetMS());
	bool join = Options.AsBool(_T("Link Time Boxes Commit")) && StartTime->HasBeenModified();
	EndTime->Update();
	Duration->Update();
	if (join) StartTime->Update();
	CommitTimes(join,true,false);
}


////////////////////
// Duration changed
void SubsEditBox::OnDurationChange(wxCommandEvent &event) {
	EndTime->SetTime(StartTime->time.GetMS() + Duration->time.GetMS());
	StartTime->Update();
	EndTime->Update();
	Duration->Update();
	CommitTimes(false,true,true);
}


///////////////////////
// Commit time changes
void SubsEditBox::CommitTimes(bool start,bool end,bool fromStart,bool commit) {
	// Get selection
	if (!start && !end) return;
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	if (n == 0) return;
	AssDialogue *cur;
	Duration->SetTime(EndTime->time.GetMS() - StartTime->time.GetMS());

	// Update lines
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			// Set times
			if (start) cur->Start = StartTime->time;
			if (end) cur->End = EndTime->time;

			// Ensure that they have positive length
			if (cur->Start > cur->End) {
				if (fromStart) cur->End = cur->Start;
				else cur->Start = cur->End;
			}
		}
	}

	// Commit
	if (commit) {
		grid->ass->FlagAsModified(_("modify times"));
		grid->CommitChanges();
		audio->SetDialogue(grid,grid->GetDialogue(sel[0]),sel[0]);
		VideoContext::Get()->UpdateDisplays(false);
	}
}


////////////////////
// Margin L changed
void SubsEditBox::OnMarginLChange(wxCommandEvent &event) {
	MarginL->Commited();
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur = NULL;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->SetMarginString(MarginL->GetValue(),0);
			cur->UpdateData();
		}
	}
	MarginL->SetValue(cur->GetMarginString(0,false));
	grid->ass->FlagAsModified(_("MarginL change"));
	grid->CommitChanges();
	grid->EndBatch();
}


////////////////////
// Margin R changed
void SubsEditBox::OnMarginRChange(wxCommandEvent &event) {
	MarginR->Commited();
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur = NULL;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->SetMarginString(MarginR->GetValue(),1);
			cur->UpdateData();
		}
	}
	MarginR->SetValue(cur->GetMarginString(1,false));
	grid->ass->FlagAsModified(_("MarginR change"));
	grid->CommitChanges();
	grid->EndBatch();
}


////////////////////
// Margin V changed
void SubsEditBox::OnMarginVChange(wxCommandEvent &event) {
	MarginV->Commited();
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur = NULL;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->SetMarginString(MarginV->GetValue(),2);
			cur->SetMarginString(MarginV->GetValue(),3); // also bottom margin for now
			cur->UpdateData();
		}
	}
	MarginV->SetValue(cur->GetMarginString(2,false));
	grid->ass->FlagAsModified(_("MarginV change"));
	grid->CommitChanges();
	grid->EndBatch();
}


//////////////////
// Effect changed
void SubsEditBox::OnEffectChange(wxCommandEvent &event) {
	Effect->Commited();
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->Effect = Effect->GetValue();
			cur->UpdateData();
		}
	}
	grid->ass->FlagAsModified(_("effect change"));
	grid->CommitChanges();
	grid->EndBatch();
}


/////////////////////////
// Comment state changed
void SubsEditBox::OnCommentChange(wxCommandEvent &event) {
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->Comment = CommentBox->GetValue();
			cur->UpdateData();
		}
	}
	grid->ass->FlagAsModified(_("comment change"));
	grid->CommitChanges();
	grid->EndBatch();
}


///////////////////////
// Event handler class
SubsEditBoxEvent::SubsEditBoxEvent(SubsEditBox *_control) {
	control = _control;
}

BEGIN_EVENT_TABLE(SubsEditBoxEvent, wxEvtHandler)
	EVT_KEY_DOWN(SubsEditBoxEvent::OnKeyPress)
END_EVENT_TABLE()

void SubsEditBoxEvent::OnKeyPress(wxKeyEvent &event) {
	control->DoKeyPress(event);
}


///////////////////////
// Actual text changed
void SubsEditBox::DoKeyPress(wxKeyEvent &event) {
	int key = event.GetKeyCode();

	if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
		if (enabled) {
#ifdef __APPLE__
			Commit(event.m_metaDown);
#else
			Commit(event.m_controlDown);
#endif
			return;
		}
	}

	event.Skip();
}


//////////
// Commit
void SubsEditBox::Commit(bool stay) {
	// Update line
	CommitText();

	// Next line if control is not held down
	bool updated = false;
	if (!stay) {
		AssDialogue *cur = grid->GetDialogue(linen);
		int nrows = grid->GetRows();
		int next = linen+1;
		if (next >= nrows) {
			AssDialogue *newline = new AssDialogue;
			newline->Start = cur->End;
			newline->End.SetMS(cur->End.GetMS()+Options.AsInt(_T("Timing Default Duration")));
			newline->Style = cur->Style;
			newline->UpdateData();
			grid->InsertLine(newline,next-1,true,true);
			updated = true;
		}
		grid->SelectRow(next);
		grid->MakeCellVisible(next,0);
		SetToLine(next);
		if (next >= nrows) return;
	}

	// Update file
	if (!updated) {
		grid->ass->FlagAsModified(_("editing"));
		grid->CommitChanges();
	}
}


///////////////
// Commit text
void SubsEditBox::CommitText(bool weak) {
	AssDialogue *cur = grid->GetDialogue(linen);

	// Update line
	if (cur) {
		// Update text
		cur->Text = TextEdit->GetText();

		// Update times
		cur->Start = StartTime->time;
		cur->End = EndTime->time;
		if (cur->Start > cur->End) cur->End = cur->Start;
		StartTime->Update();
		EndTime->Update();
		Duration->Update();

		// Update audio
		if (!weak) {
			grid->Refresh(false);
			audio->SetDialogue(grid,cur,linen);
		}
	}
}


//////////////////////////////////////
// Gets block number at text position
int SubsEditBox::BlockAtPos(int pos) {
	// Prepare
	int n=0;
	wxString text = TextEdit->GetText();;
	int max = text.Length()-1;

	// Find block number at pos
	for (int i=0;i<=pos && i<=max;i++) {
		if (i > 0 && text[i] == _T('{')) n++;
		if (text[i] == _T('}') && i != max && i != pos && i != pos -1 && (i+1 == max || text[i+1] != _T('{'))) n++;
	}

	return n;
}


////////////////
// Set override
void SubsEditBox::SetOverride (wxString tagname,wxString preValue,int forcePos,bool getFocus) {
	// Selection
	int selstart, selend;
	if (forcePos != -1) {
		selstart = forcePos;
		selend = forcePos;
	}
	else TextEdit->GetSelection(&selstart,&selend);
	int len = TextEdit->GetText().Length();
	selstart = MID(0,TextEdit->GetReverseUnicodePosition(selstart),len);
	selend = MID(0,TextEdit->GetReverseUnicodePosition(selend),len);

	// Current tag name
	wxString alttagname = tagname;
	wxString removeTag;
	if (tagname == _T("\\1c")) tagname = _T("\\c");
	if (tagname == _T("\\fr")) tagname = _T("\\frz");
	if (tagname == _T("\\pos")) removeTag = _T("\\move");
	if (tagname == _T("\\move")) removeTag = _T("\\pos");

	// Get block at start
	size_t blockn = BlockAtPos(selstart);
	AssDialogue *line = new AssDialogue();
	line->Text = TextEdit->GetText();
	line->ParseASSTags();
	AssDialogueBlock *block = line->Blocks.at(blockn);

	// Insert variables
	wxString insert;
	wxString insert2;
	int shift = 0;
	int nInserted = 1;

	// Default value
	wxColour startcolor;
	wxFont startfont;
	float startangle;
	float startScale;
	bool isColor = false;
	bool isFont = false;
	bool isGeneric = false;
	bool isFlag = false;
	bool isAngle = false;
	bool isScale = false;
	bool state = false;
	AssStyle *style = grid->ass->GetStyle(grid->GetDialogue(linen)->Style);
	AssStyle defStyle;
	if (style == NULL) style = &defStyle; 
	if (tagname == _T("\\b")) {
		state = style->bold;
		isFlag = true;
	}
	else if (tagname == _T("\\i")) {
		state = style->italic;
		isFlag = true;
	}
	else if (tagname == _T("\\u")) {
		state = style->underline;
		isFlag = true;
	}
	else if (tagname == _T("\\s")) {
		state = style->strikeout;
		isFlag = true;
	}
	else if (tagname == _T("\\fn")) {
		startfont.SetFaceName(style->font);
		startfont.SetPointSize(int(style->fontsize));
		startfont.SetWeight(style->bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
		startfont.SetStyle(style->italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
		startfont.SetUnderlined(style->underline);
		isFont = true;
	}
	else if (tagname == _T("\\c")) {
		startcolor = style->primary.GetWXColor();
		isColor = true;
	}
	else if (tagname == _T("\\2c")) {
		startcolor = style->secondary.GetWXColor();
		isColor = true;
	}
	else if (tagname == _T("\\3c")) {
		startcolor = style->outline.GetWXColor();
		isColor = true;
	}
	else if (tagname == _T("\\4c")) {
		startcolor = style->shadow.GetWXColor();
		isColor = true;
	}
	else if (tagname == _T("\\frz")) {
		startangle = style->angle;
		isAngle = true;
	}
	else if (tagname == _T("\\frx") || tagname == _T("\\fry")) {
		startangle = 0.0;
		isAngle = true;
	}
	else if (tagname == _T("\\fscx")) {
		startScale = style->scalex;
		isScale = true;
	}
	else if (tagname == _T("\\fscy")) {
		startScale = style->scaley;
		isScale = true;
	}
	else isGeneric = true;

	bool hasEnd = isFlag;

	// Find current value of style
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	if (isFont || isColor || isFlag || isAngle) {
		for (size_t i=0;i<=blockn;i++) {
			override = AssDialogueBlock::GetAsOverride(line->Blocks.at(i));
			if (override) {
				for (size_t j=0;j<override->Tags.size();j++) {
					tag = override->Tags.at(j);
					if (tag->Name == tagname || tag->Name == alttagname || tagname == _T("\\fn")) {
						if (isColor) startcolor = tag->Params.at(0)->AsColour();
						if (isFlag) state = tag->Params.at(0)->AsBool();
						if (isFont) {
							if (tag->Name == _T("\\fn")) startfont.SetFaceName(tag->Params.at(0)->AsText());
							if (tag->Name == _T("\\fs")) startfont.SetPointSize(tag->Params.at(0)->AsInt());
							if (tag->Name == _T("\\b")) startfont.SetWeight((tag->Params.at(0)->AsInt() > 0) ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
							if (tag->Name == _T("\\i")) startfont.SetStyle(tag->Params.at(0)->AsBool() ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
							if (tag->Name == _T("\\u")) startfont.SetUnderlined(tag->Params.at(0)->AsBool());
						}
						if (isAngle) startangle = tag->Params.at(0)->AsFloat();
						if (isScale) startScale = tag->Params.at(0)->AsFloat();
					}
				}
			}
		}
	}

	// Overrides being inserted
	wxArrayString insertTags;

	// Toggle value
	if (isFlag) {
		state = !state;
		int stateval = 0;
		if (state) stateval = 1;

		// Generate insert string
		insert = tagname + wxString::Format(_T("%i"),stateval);
		insert2 = tagname + wxString::Format(_T("%i"),1-stateval);
		insertTags.Add(tagname);
	}

	// Choose color
	if (isColor) {
		// Pick from dialog
		//wxColour color = wxGetColourFromUser(this,startcolor);
		wxColour color = GetColorFromUser(((AegisubApp*)wxTheApp)->frame, startcolor);
		if (!color.Ok() || color == startcolor) {
			delete line;
			return;
		}

		// Generate insert string 
		AssColor asscolor(color);
		insert = tagname + asscolor.GetASSFormatted(false);
		insertTags.Add(tagname);
	}

	// Choose font
	if (isFont) {
		// Pick from dialog
		wxFont font = wxGetFontFromUser(this,startfont);
		if (!font.Ok()) {
			delete line;
			return;
		}

		// Generate insert string
		nInserted = 0;
		if (font.GetFaceName() != startfont.GetFaceName()) {
			insert = _T("\\fn") + font.GetFaceName();
			nInserted++;
			insertTags.Add(_T("\\fn"));
		}
		if (font.GetPointSize() != startfont.GetPointSize()) {
			insert += _T("\\fs") + wxString::Format(_T("%i"),font.GetPointSize());
			nInserted++;
			insertTags.Add(_T("\\fs"));
		}
		if (font.GetWeight() != startfont.GetWeight()) {
			insert += _T("\\b") + wxString::Format(_T("%i"),font.GetWeight() == wxFONTWEIGHT_BOLD ? 1 : 0);
			nInserted++;
			insertTags.Add(_T("\\b"));
		}
		if (font.GetStyle() != startfont.GetStyle()) {
			insert += _T("\\i") + wxString::Format(_T("%i"),font.GetStyle() == wxFONTSTYLE_ITALIC ? 1 : 0);
			nInserted++;
			insertTags.Add(_T("\\i"));
		}
		if (font.GetUnderlined() != startfont.GetUnderlined()) {
			insert += _T("\\u") + wxString::Format(_T("%i"),font.GetUnderlined() ? 1 : 0);
			nInserted++;
			insertTags.Add(_T("\\u"));
		}
		if (insert.IsEmpty()) {
			delete line;
			return;
		}
	}

	// Generic tag
	if (isGeneric) {
		insert = tagname + preValue;
		insertTags.Add(tagname);
	}

	// Angle
	if (isAngle) {
		insert = tagname + preValue;
		insertTags.Add(tagname);
	}

	// Scale
	if (isScale) {
		insert = tagname + preValue;
		insertTags.Add(tagname);
	}

	// Get current block as plain or override
	AssDialogueBlockPlain *plain = AssDialogueBlock::GetAsPlain(block);
	override = AssDialogueBlock::GetAsOverride(block);

	// Plain
	if (plain) {
		// Insert in text
		line->Text = line->Text.Left(selstart) + _T("{") + insert + _T("}") + line->Text.Mid(selstart);
		shift = 2 + insert.Length();
		line->ParseASSTags();
	}

	// Override
	else if (override) {
		// Insert new tag
		override->text += insert;
		override->ParseTags();
		shift = insert.Length();

		// Remove old of same
		for (size_t i=0;i<override->Tags.size()-nInserted;i++) {
			//if (insert.Contains(override->Tags.at(i)->Name)) {
			wxString name = override->Tags.at(i)->Name;
			if (insertTags.Index(name) != wxNOT_FOUND || removeTag == name) {
				shift -= override->Tags.at(i)->ToString().Length();
				override->Tags.erase(override->Tags.begin() + i);
				i--;
			}
		}

		// Update line
		line->UpdateText();
	}

	// End
	if (hasEnd && selend != selstart) {
		// Prepare variables again
		int origStart = selstart;
		selstart = selend + shift;
		insert = insert2;
		TextEdit->SetTextTo(line->Text);
		blockn = BlockAtPos(selstart);
		block = line->Blocks.at(blockn);
		plain = AssDialogueBlock::GetAsPlain(block);
		override = AssDialogueBlock::GetAsOverride(block);

		// Plain
		if (plain) {
			// Insert in text
			line->Text = line->Text.Left(selstart) + _T("{") + insert + _T("}") + line->Text.Mid(selstart);
		}

		// Override
		else if (override) {
			// Insert new tag
			override->text += insert;
			override->ParseTags();

			// Remove old of same
			for (size_t i=0;i<override->Tags.size()-nInserted;i++) {
				wxString name = override->Tags.at(i)->Name;
				if (insert.Contains(name) || removeTag == name) {
					shift -= override->Tags.at(i)->ToString().Length();
					override->Tags.erase(override->Tags.begin() + i);
					i--;
				}
			}

			// Update line
			line->UpdateText();
		}

		// Shift selection
		selstart = origStart;
		TextEdit->SetSelectionU(origStart+shift,selend+shift);
	}

	// Commit changes and shift selection
	TextEdit->SetTextTo(line->Text);
	delete line;
	TextEdit->SetSelectionU(selstart+shift,selend+shift);
	if (getFocus) TextEdit->SetFocus();
}


/////////////////////
// Set primary color
void SubsEditBox::OnButtonColor1(wxCommandEvent &event) {
	SetOverride(_T("\\1c"));
}


///////////////////////
// Set secondary color
void SubsEditBox::OnButtonColor2(wxCommandEvent &event) {
	SetOverride(_T("\\2c"));
}


/////////////////////
// Set outline color
void SubsEditBox::OnButtonColor3(wxCommandEvent &event) {
	SetOverride(_T("\\3c"));
}


////////////////////
// Set shadow color
void SubsEditBox::OnButtonColor4(wxCommandEvent &event) {
	SetOverride(_T("\\4c"));
}


/////////////////
// Set font face
void SubsEditBox::OnButtonFontFace(wxCommandEvent &event) {
	SetOverride(_T("\\fn"));
}


////////
// Bold
void SubsEditBox::OnButtonBold(wxCommandEvent &event) {
	SetOverride(_T("\\b"));
}


///////////
// Italics
void SubsEditBox::OnButtonItalics(wxCommandEvent &event) {
	SetOverride(_T("\\i"));
}


/////////////
// Underline
void SubsEditBox::OnButtonUnderline(wxCommandEvent &event) {
	SetOverride(_T("\\u"));
}


/////////////
// Strikeout
void SubsEditBox::OnButtonStrikeout(wxCommandEvent &event) {
	SetOverride(_T("\\s"));
}


//////////
// Commit
void SubsEditBox::OnButtonCommit(wxCommandEvent &event) {
#ifdef __APPLE__
	Commit(wxGetMouseState().CmdDown());
#else
	Commit(wxGetMouseState().ControlDown());
#endif
}


