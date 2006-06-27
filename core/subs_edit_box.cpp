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
#include "subs_edit_box.h"
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
#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include "dialog_colorpicker.h"
#include "main.h"
#include "frame_main.h"
#include "utils.h"
#include "dialog_search_replace.h"



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
	linen = -2;

	// Top controls
	wxArrayString styles;
	styles.Add(_T(""));
	CommentBox = new wxCheckBox(this,COMMENT_CHECKBOX,_("Comment"));
	CommentBox->SetToolTip(_("Comment this line out. Commented lines don't show up on screen."));
	StyleBox = new wxComboBox(this,STYLE_COMBOBOX,_T(""),wxDefaultPosition,wxSize(110,25),styles,wxCB_READONLY);
	StyleBox->SetToolTip(_("Style for this line."));
	ActorBox = new wxComboBox(this,ACTOR_COMBOBOX,_T(""),wxDefaultPosition,wxSize(110,25),styles,wxCB_DROPDOWN);
	ActorBox->SetToolTip(_("Actor name for this speech. This is only for reference, and is mainly useless."));
	MarginL = new HiliModTextCtrl(this,MARGINL_BOX,_T(""),wxDefaultPosition,wxSize(40,20),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginL->SetToolTip(_("Left Margin (0000 = default)"));
	MarginR = new HiliModTextCtrl(this,MARGINR_BOX,_T(""),wxDefaultPosition,wxSize(40,20),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginR->SetToolTip(_("Right Margin (0000 = default)"));
	MarginV = new HiliModTextCtrl(this,MARGINV_BOX,_T(""),wxDefaultPosition,wxSize(40,20),wxTE_CENTRE | wxTE_PROCESS_ENTER,NumValidator());
	MarginV->SetToolTip(_("Vertical Margin (0000 = default)"));

	// Middle controls
	Layer = new HiliModTextCtrl(this,LAYER_BOX,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator());
	Layer->SetToolTip(_("Layer number"));
	StartTime = new TimeEdit(this,STARTTIME_BOX,_T(""),wxDefaultPosition,wxSize(75,20));
	StartTime->SetToolTip(_("Start time"));
	StartTime->showModified = true;
	EndTime = new TimeEdit(this,ENDTIME_BOX,_T(""),wxDefaultPosition,wxSize(75,20),0,NumValidator());
	EndTime->SetToolTip(_("End time"));
	EndTime->isEnd = true;
	EndTime->showModified = true;
	Duration = new TimeEdit(this,DURATION_BOX,_T(""),wxDefaultPosition,wxSize(75,20),0,NumValidator());
	Duration->SetToolTip(_("Line duration"));
	Duration->showModified = true;
	ByTime = new wxRadioButton(this,RADIO_TIME_BY_TIME,_("Time"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	ByTime->SetToolTip(_("Time by h:mm:ss.cs"));
	ByFrame = new wxRadioButton(this,RADIO_TIME_BY_FRAME,_("Frame"));
	ByFrame->SetToolTip(_("Time by frame number"));
	//SyntaxHighlight = new wxCheckBox(this,SYNTAX_BOX,_("Syntax"));
	//SyntaxHighlight->SetToolTip(_("Enable syntax highlighting"));
	//SyntaxHighlight->SetValue(Options.AsBool(_T("Syntax Highlight Enabled")));

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
	Color1 = new wxBitmapButton(this,BUTTON_COLOR1,wxBITMAP(button_color1),wxDefaultPosition,wxSize(30,20));
	Color1->SetToolTip(_("Primary color"));
	Color2 = new wxBitmapButton(this,BUTTON_COLOR2,wxBITMAP(button_color2),wxDefaultPosition,wxSize(30,20));
	Color2->SetToolTip(_("Secondary color"));
	Color3 = new wxBitmapButton(this,BUTTON_COLOR3,wxBITMAP(button_color3),wxDefaultPosition,wxSize(30,20));
	Color3->SetToolTip(_("Outline color"));
	Color4 = new wxBitmapButton(this,BUTTON_COLOR4,wxBITMAP(button_color4),wxDefaultPosition,wxSize(30,20));
	Color4->SetToolTip(_("Shadow color"));
	Effect = new HiliModTextCtrl(this,EFFECT_BOX,_T(""),wxDefaultPosition,wxSize(120,20),0);
	Effect->SetToolTip(_("Effect for this line. This can be used to store extra information for karaoke scripts, or for the effects supported by the renderer."));

	// Top sizer
	wxSizer *TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(CommentBox,0,wxRIGHT | wxALIGN_CENTER,5);
	TopSizer->Add(StyleBox,0,wxRIGHT,5);
	TopSizer->Add(ActorBox,0,wxRIGHT,5);
	TopSizer->Add(Effect,0,0,0);

	// Middle sizer
	wxSizer *MiddleSizer = new wxBoxSizer(wxHORIZONTAL);
	MiddleSizer->Add(Layer,0,wxRIGHT,5);
	MiddleSizer->Add(StartTime,0,wxRIGHT,0);
	MiddleSizer->Add(EndTime,0,wxRIGHT,5);
	MiddleSizer->Add(Duration,0,wxRIGHT,5);
	MiddleSizer->Add(MarginL,0,0,0);
	MiddleSizer->Add(MarginR,0,0,0);
	MiddleSizer->Add(MarginV,0,0,0);
	//MiddleSizer->Add(SyntaxHighlight,0,wxRIGHT | wxALIGN_CENTER,5);

	// Middle-bottom sizer
	wxSizer *MiddleBotSizer = new wxBoxSizer(wxHORIZONTAL);
	MiddleBotSizer->Add(Bold);
	MiddleBotSizer->Add(Italics);
	MiddleBotSizer->Add(Underline);
	MiddleBotSizer->Add(Strikeout);
	MiddleBotSizer->Add(FontName);
	MiddleBotSizer->AddSpacer(5);
	MiddleBotSizer->Add(Color1);
	MiddleBotSizer->Add(Color2);
	MiddleBotSizer->Add(Color3);
	MiddleBotSizer->Add(Color4,0,wxRIGHT,10);
	MiddleBotSizer->Add(ByTime,0,wxRIGHT | wxALIGN_CENTER,5);
	MiddleBotSizer->Add(ByFrame,0,wxRIGHT | wxALIGN_CENTER,5);

	// Text editor
	TextEdit = new SubsTextEditCtrl(this,EDIT_BOX,_T(""),wxDefaultPosition,wxSize(300,50),wxTE_MULTILINE | wxTE_RICH2 | wxTE_NOHIDESEL);
	TextEdit->PushEventHandler(new SubsEditBoxEvent(this));
	TextEdit->control = this;
	wxSizer *BottomSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer->Add(TextEdit,1,wxEXPAND,0);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxTOP,3);
	MainSizer->Add(MiddleSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	MainSizer->Add(MiddleBotSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);
	MainSizer->Add(BottomSizer,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,3);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);

	// HACK: Fix colour of bg of editbox
	origBgColour = TextEdit->GetBackgroundColour();
	disabledBgColour = GetBackgroundColour();
	Update();
}


///////////////////
// Update function
void SubsEditBox::Update (bool timeOnly) {
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
				SetText(curdiag->Text);
				Layer->SetValue(wxString::Format(_T("%i"),curdiag->Layer));
				MarginL->SetValue(curdiag->GetMarginString(1));
				MarginR->SetValue(curdiag->GetMarginString(2));
				MarginV->SetValue(curdiag->GetMarginString(3));
				Effect->SetValue(curdiag->Effect);
				CommentBox->SetValue(curdiag->Comment);
				StyleBox->Select(StyleBox->FindString(curdiag->Style));
				ActorBox->SetValue(curdiag->Actor);
				ActorBox->SetStringSelection(curdiag->Actor);
			}

			// Audio
			audio->SetDialogue(grid,curdiag,linen);

			// Video
			video->curLine = curdiag;
			video->UpdateSubsRelativeTime();
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
void SubsEditBox::SetToLine(int n) {
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
		}
	}

	// Update controls
	Update();

	// Set video
	if (video->loaded) {
		wxString sync;
		if (Search.hasFocus) sync = _T("Find update video");
		else sync = _T("Sync video with subs");
		
		if (Options.AsBool(sync) == true) {
			video->Stop();
			AssDialogue *cur = grid->GetDialogue(n);
			if (cur) video->JumpToFrame(VFR_Output.GetFrameAtTime(cur->Start.GetMS(),true));
		}
	}
}


///////////////////////////
// Set text to a new value
void SubsEditBox::SetText(const wxString _text) {
	// Setup
	textEditReady = false;
	TextEdit->Freeze();
	wxString text = _text;
	text.Replace(_T("\r\n"),_T("\\N"));
	text.Replace(_T("\n\r"),_T("\\N"));
	text.Replace(_T("\r"),_T("\\N"));
	text.Replace(_T("\n"),_T("\\N"));

	// Mode
	int mode = 0;
	if (Options.AsBool(_T("Syntax Highlight Enabled"))) mode = 1;

	// Syntax highlighted
	if (mode == 1) {
		// Define attributes
		wxTextAttr Normal(Options.AsColour(_T("Syntax Highlight Normal")));
		wxTextAttr Brackets(Options.AsColour(_T("Syntax Highlight Brackets")));
		wxTextAttr Slashes(Options.AsColour(_T("Syntax Highlight Slashes")));
		wxTextAttr Tags(Options.AsColour(_T("Syntax Highlight Tags")));
		wxTextAttr Error(Options.AsColour(_T("Syntax Highlight Error")));
		wxTextAttr *cur;

		// Define font size
		wxFont font = Normal.GetFont();
		wxString fontname = Options.AsText(_T("Font Face"));
		if (fontname != _T("")) font.SetFaceName(fontname);
		font.SetPointSize(Options.AsInt(_T("Font Size")));
		Normal.SetFont(font);
		Brackets.SetFont(font);
		Slashes.SetFont(font);
		Tags.SetFont(font);
		Error.SetFont(font);

		// Prepare
		cur = &Normal;
		long from=0,to=0;
		TextEdit->GetSelection(&from,&to);
		TextEdit->Clear();
		TextEdit->SetValue(text);
		TextEdit->SetStyle(0,TextEdit->GetLastPosition()+1,Tags);

		// Scan
		size_t len = text.Len();
		size_t start = 0;
		int depth = 0;
		wchar_t curchar;
		for (unsigned int i=0;i<len;i++) {
			curchar = text[i];
			if (curchar == _T('{')) {
				depth++;
				if (cur) TextEdit->SetStyle(start,i,*cur);
				start = i+1;
				cur = &Brackets;
				TextEdit->SetStyle(start-1,start,*cur);
				if (depth == 0) cur = &Normal;
				//else if (depth == 1) cur = &Tags;
				else if (depth == 1) cur = NULL;
				else {
					cur = &Error;
					break;
				}
				continue;
			}
			if (curchar == _T('}')) {
				depth--;
				if (cur) TextEdit->SetStyle(start,i,*cur);
				start = i+1;
				cur = &Brackets;
				TextEdit->SetStyle(start-1,start,*cur);
				if (depth == 0) cur = &Normal;
				//else if (depth == 1) cur = &Tags;
				else if (depth == 1) cur = NULL;
				else {
					cur = &Error;
					break;
				}
				continue;
			}
			if (depth > 0 && curchar == _T('\\')) {
				if (cur) TextEdit->SetStyle(start,i,*cur);
				start = i+1;
				TextEdit->SetStyle(start-1,start,Slashes);
				//cur = &Tags;
				cur = NULL;
				continue;
			}
		}
		if (cur) TextEdit->SetStyle(start,TextEdit->GetLastPosition()+1,*cur);
		TextEdit->SetSelection(from,to);
	}

	// 0x2600
	else if (mode == 2) {

	}

	// Plain
	else {
		// Prepare
		long from=0,to=0;
		TextEdit->GetSelection(&from,&to);
		TextEdit->Clear();

		// Make style
		wxTextAttr Normal(Options.AsColour(_T("Syntax Highlight Normal")));
		wxFont font = Normal.GetFont();
		wxString fontname = Options.AsText(_T("Font Face"));
		if (fontname != _T("")) font.SetFaceName(fontname);
		font.SetPointSize(Options.AsInt(_T("Font Size")));
		Normal.SetFont(font);

		// Set
		TextEdit->SetValue(text);
		TextEdit->SetStyle(0,text.Length(),Normal);
		TextEdit->SetSelection(from,to);
	}

	TextEdit->Thaw();
	textEditReady = true;
}


///////////////
// Event table
BEGIN_EVENT_TABLE(SubsEditBox, wxPanel)
	EVT_TEXT(EDIT_BOX, SubsEditBox::OnEditText)
	EVT_CHECKBOX(SYNTAX_BOX, SubsEditBox::OnSyntaxBox)
	EVT_RADIOBUTTON(RADIO_TIME_BY_FRAME, SubsEditBox::OnFrameRadio)
	EVT_RADIOBUTTON(RADIO_TIME_BY_TIME, SubsEditBox::OnTimeRadio)
	EVT_KEY_DOWN(SubsEditBox::OnKeyDown)
	EVT_COMBOBOX(STYLE_COMBOBOX, SubsEditBox::OnStyleChange)
	EVT_COMBOBOX(ACTOR_COMBOBOX, SubsEditBox::OnActorChange)
	EVT_TEXT_ENTER(ACTOR_COMBOBOX, SubsEditBox::OnActorChange)
	EVT_TEXT_ENTER(LAYER_BOX, SubsEditBox::OnLayerChange)
	EVT_TEXT_ENTER(STARTTIME_BOX, SubsEditBox::OnStartTimeChange)
	EVT_TEXT_ENTER(ENDTIME_BOX, SubsEditBox::OnEndTimeChange)
	EVT_TEXT_ENTER(DURATION_BOX, SubsEditBox::OnDurationChange)
	EVT_TEXT_ENTER(MARGINL_BOX, SubsEditBox::OnMarginLChange)
	EVT_TEXT_ENTER(MARGINR_BOX, SubsEditBox::OnMarginRChange)
	EVT_TEXT_ENTER(MARGINV_BOX, SubsEditBox::OnMarginVChange)
	EVT_TEXT_ENTER(EFFECT_BOX, SubsEditBox::OnEffectChange)
	EVT_CHECKBOX(COMMENT_CHECKBOX, SubsEditBox::OnCommentChange)

	EVT_MENU(EDIT_MENU_SPLIT_PRESERVE,SubsEditBox::OnSplitLinePreserve)
	EVT_MENU(EDIT_MENU_SPLIT_ESTIMATE,SubsEditBox::OnSplitLineEstimate)
	EVT_MENU(EDIT_MENU_CUT,SubsEditBox::OnCut)
	EVT_MENU(EDIT_MENU_COPY,SubsEditBox::OnCopy)
	EVT_MENU(EDIT_MENU_PASTE,SubsEditBox::OnPaste)
	EVT_MENU(EDIT_MENU_UNDO,SubsEditBox::OnUndo)
	EVT_MENU(EDIT_MENU_SELECT_ALL,SubsEditBox::OnSelectAll)

	EVT_BUTTON(BUTTON_COLOR1,SubsEditBox::OnButtonColor1)
	EVT_BUTTON(BUTTON_COLOR2,SubsEditBox::OnButtonColor2)
	EVT_BUTTON(BUTTON_COLOR3,SubsEditBox::OnButtonColor3)
	EVT_BUTTON(BUTTON_COLOR4,SubsEditBox::OnButtonColor4)
	EVT_BUTTON(BUTTON_FONT_NAME,SubsEditBox::OnButtonFontFace)
	EVT_BUTTON(BUTTON_BOLD,SubsEditBox::OnButtonBold)
	EVT_BUTTON(BUTTON_ITALICS,SubsEditBox::OnButtonItalics)
	EVT_BUTTON(BUTTON_UNDERLINE,SubsEditBox::OnButtonUnderline)
	EVT_BUTTON(BUTTON_STRIKEOUT,SubsEditBox::OnButtonStrikeout)
END_EVENT_TABLE()


/////////////////////
// Text edited event
void SubsEditBox::OnEditText(wxCommandEvent &event) {
	if (textEditReady) {
		SetText(TextEdit->GetValue());
		event.Skip();
	}
}


/////////////////////////////
// Syntax highlight checkbox
void SubsEditBox::OnSyntaxBox(wxCommandEvent &event) {
	SetText(TextEdit->GetValue());
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
		grid->SetByFrame(false);
	}
	event.Skip();
}


////////////
// Key down
void SubsEditBox::OnKeyDown(wxKeyEvent &event) {
	event.Skip();
}


//////////////////////////////////////////////////
// Sets state (enabled/disabled) for all controls
void SubsEditBox::SetControlsState (bool state) {
	if (state == controlState) return;
	controlState = state;

	// HACK: TextEdit workaround the stupid colour lock bug
	TextEdit->SetEditable(state);
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

	UpdateFrameTiming();

	// Clear values if it's false
	if (state==false) {
		TextEdit->SetValue(_T(""));
		StartTime->SetTime(0);
		EndTime->SetTime(0);
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
	if (VFR_Output.GetFrameRateType() != NONE) ByFrame->Enable(enabled);
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
	grid->ass->FlagAsModified();
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
	grid->ass->FlagAsModified();
	grid->CommitChanges();
	grid->EndBatch();
}


/////////////////
// Layer changed
void SubsEditBox::OnLayerChange(wxCommandEvent &event) {
	Layer->Commited();
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			long temp;
			Layer->GetValue().ToLong(&temp);
			cur->Layer = temp;
			cur->UpdateData();
		}
	}
	grid->ass->FlagAsModified();
	grid->CommitChanges();
	grid->EndBatch();
}


//////////////////////
// Start time changed
void SubsEditBox::OnStartTimeChange(wxCommandEvent &event) {
	if (StartTime->time > EndTime->time) StartTime->SetTime(EndTime->time.GetMS());
	bool join = Options.AsBool(_T("Link Time Boxes Commit")) && EndTime->HasBeenModified();
	StartTime->Update();
	if (join) EndTime->Update();
	CommitTimes(true,join,true);
}


////////////////////
// End time changed
void SubsEditBox::OnEndTimeChange(wxCommandEvent &event) {
	if (StartTime->time > EndTime->time) EndTime->SetTime(StartTime->time.GetMS());
	bool join = Options.AsBool(_T("Link Time Boxes Commit")) && StartTime->HasBeenModified();
	EndTime->Update();
	if (join) StartTime->Update();
	CommitTimes(join,true,false);
}


////////////////////
// Duration changed
void SubsEditBox::OnDurationChange(wxCommandEvent &event) {
	EndTime->SetTime(StartTime->time.GetMS() + Duration->time.GetMS());
	Duration->Update();
	CommitTimes(false,true,true);
}


///////////////////////
// Commit time changes
void SubsEditBox::CommitTimes(bool start,bool end,bool fromStart) {
	// Get selection
	if (!start && !end) return;
	grid->BeginBatch();
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

			// Update
			cur->UpdateData();
		}
	}

	// Commit
	grid->ass->FlagAsModified();
	grid->CommitChanges();
	grid->EndBatch();
	audio->SetDialogue(grid,grid->GetDialogue(sel[0]),sel[0]);
	video->UpdateSubsRelativeTime();
}


////////////////////
// Margin L changed
void SubsEditBox::OnMarginLChange(wxCommandEvent &event) {
	MarginL->Commited();
	grid->BeginBatch();
	wxArrayInt sel = grid->GetSelection();
	int n = sel.Count();
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->SetMarginString(MarginL->GetValue(),1);
			cur->UpdateData();
		}
	}
	MarginL->SetValue(cur->GetMarginString(1));
	grid->ass->FlagAsModified();
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
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->SetMarginString(MarginR->GetValue(),2);
			cur->UpdateData();
		}
	}
	MarginR->SetValue(cur->GetMarginString(2));
	grid->ass->FlagAsModified();
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
	AssDialogue *cur;
	for (int i=0;i<n;i++) {
		cur = grid->GetDialogue(sel[i]);
		if (cur) {
			cur->SetMarginString(MarginV->GetValue(),3);
			cur->UpdateData();
		}
	}
	MarginV->SetValue(cur->GetMarginString(3));
	grid->ass->FlagAsModified();
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
	grid->ass->FlagAsModified();
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
	grid->ass->FlagAsModified();
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
	control->KeyPress(event);
}


///////////////////////
// Actual text changed
void SubsEditBox::KeyPress(wxKeyEvent &event) {
	int key = event.GetKeyCode();

	if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
		if (enabled) {
			// Update line
			CommitText();

			// Next line if control is not held down
			bool updated = false;
			if (!event.m_controlDown) {
				AssDialogue *cur = grid->GetDialogue(linen);
				int nrows = grid->GetRows();
				int next = linen+1;
				if (next >= nrows) {
					AssDialogue *newline = new AssDialogue;
					newline->Start = cur->End;
					newline->End.SetMS(cur->End.GetMS()+5000);
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
				grid->ass->FlagAsModified();
				grid->CommitChanges();
			}
			return;
		}
	}

	event.Skip();
}


///////////////
// Commit text
void SubsEditBox::CommitText() {
	AssDialogue *cur = grid->GetDialogue(linen);

	// Update line
	if (cur) {
		cur->Text = TextEdit->GetValue();
		//cur->ParseASSTags();
		cur->UpdateData();
		grid->Refresh(false);
		audio->SetDialogue(grid,cur,linen);
	}
}


////////////////////////
// Edit box constructor
SubsTextEditCtrl::SubsTextEditCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
: wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
}


////////////////////////
// Edit box event table
BEGIN_EVENT_TABLE(SubsTextEditCtrl,wxTextCtrl)
	EVT_MOUSE_EVENTS(SubsTextEditCtrl::OnMouseEvent)
END_EVENT_TABLE()


///////////////////////////////
// Split line preserving times
void SubsEditBox::OnSplitLinePreserve (wxCommandEvent &event) {
	long from,to;
	TextEdit->GetSelection(&from, &to);
	grid->SplitLine(linen,from,0);
}


///////////////////////////////
// Split line estimating times
void SubsEditBox::OnSplitLineEstimate (wxCommandEvent &event) {
	long from,to;
	TextEdit->GetSelection(&from, &to);
	grid->SplitLine(linen,from,1);
}


///////
// Cut
void SubsEditBox::OnCut(wxCommandEvent &event) {
	TextEdit->Cut();
}


////////
// Copy
void SubsEditBox::OnCopy(wxCommandEvent &event) {
	TextEdit->Copy();
}


/////////
// Paste
void SubsEditBox::OnPaste(wxCommandEvent &event) {
	TextEdit->Paste();
}


////////
// Undo
void SubsEditBox::OnUndo(wxCommandEvent &event) {
	TextEdit->Undo();
}


//////////////
// Select All
void SubsEditBox::OnSelectAll(wxCommandEvent &event) {
	TextEdit->SetSelection(-1,-1);
}


///////////////
// Mouse event
void SubsTextEditCtrl::OnMouseEvent(wxMouseEvent &event) {
	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		if (control->linen >= 0) {
			// Popup
			wxMenu menu;
			menu.Append(EDIT_MENU_UNDO,_("&Undo"))->Enable(CanUndo());
			menu.AppendSeparator();
			menu.Append(EDIT_MENU_CUT,_("Cu&t"))->Enable(CanCut());
			menu.Append(EDIT_MENU_COPY,_("&Copy"))->Enable(CanCopy());
			menu.Append(EDIT_MENU_PASTE,_("&Paste"))->Enable(CanPaste());
			menu.AppendSeparator();
			menu.Append(EDIT_MENU_SELECT_ALL,_("Select &All"));
			menu.AppendSeparator();
			menu.Append(EDIT_MENU_SPLIT_PRESERVE,_("Split at cursor (preserve times)"));
			menu.Append(EDIT_MENU_SPLIT_ESTIMATE,_("Split at cursor (estimate times)"));
			PopupMenu(&menu);
			return;
		}
	}

	event.Skip();
}


//////////////////////////////////////
// Gets block number at text position
int SubsEditBox::BlockAtPos(int pos) {
	// Prepare
	int n=0;
	wxString text = TextEdit->GetValue();;
	int max = text.Length()-1;

	// Find block number at pos
	for (int i=0;i<=pos;i++) {
		if (i > 0 && text[i] == _T('{')) n++;
		if (text[i] == _T('}') && i != max && i != pos && i != pos -1 && (i+1 == max || text[i+1] != _T('{'))) n++;
	}

	return n;
}


////////////////
// Set override
void SubsEditBox::SetOverride (wxString tagname,wxString preValue,int forcePos) {
	// Selection
	long selstart, selend;
	if (forcePos != -1) {
		selstart = forcePos;
		selend = forcePos;
	}
	else TextEdit->GetSelection(&selstart,&selend);
	int len = TextEdit->GetValue().Length();
	selstart = MID(0,selstart,len);
	selend = MID(0,selend,len);

	// Current tag name
	wxString alttagname = tagname;
	if (tagname == _T("\\1c")) tagname = _T("\\c");

	// Get block at start
	size_t blockn = BlockAtPos(selstart);
	AssDialogue *line = new AssDialogue();
	line->Text = TextEdit->GetValue();
	line->ParseASSTags();
	AssDialogueBlock *block = line->Blocks.at(blockn);

	// Insert variables
	wxString insert;
	wxString insert2;
	int shift;

	// Default value
	wxColour startcolor;
	wxFont startfont;
	bool isColor = false;
	bool isFont = false;
	bool isPos = false;
	bool isFlag = false;
	bool state = false;
	AssStyle *style = AssFile::top->GetStyle(grid->GetDialogue(linen)->Style);
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
		startfont.SetPointSize(style->fontsize);
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
	else if (tagname == _T("\\pos")) {
		isPos = true;
	}
	bool hasEnd = isFlag;

	// Find current value of style
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	if (isFont || isColor || isFlag) {
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
					}
				}
			}
		}
	}

	// Toggle value
	if (isFlag) {
		state = !state;
		int stateval = 0;
		if (state) stateval = 1;

		// Generate insert string
		insert = tagname + wxString::Format(_T("%i"),stateval);
		insert2 = tagname + wxString::Format(_T("%i"),1-stateval);
	}

	// Choose color
	if (isColor) {
		// Pick from dialog
		//wxColour color = wxGetColourFromUser(this,startcolor);
		wxColour color = GetColorFromUser(((AegisubApp*)wxTheApp)->frame, startcolor);
		if (!color.Ok() || color == startcolor) return;

		// Generate insert string 
		AssColor asscolor(color);
		insert = tagname + asscolor.GetASSFormatted(false);
	}

	// Choose font
	if (isFont) {
		// Pick from dialog
		wxFont font = wxGetFontFromUser(this,startfont);
		if (!font.Ok()) return;

		// Generate insert string
		if (font.GetFaceName() != startfont.GetFaceName()) insert = _T("\\fn") + font.GetFaceName();
		if (font.GetPointSize() != startfont.GetPointSize()) insert += _T("\\fs") + wxString::Format(_T("%i"),font.GetPointSize());
		if (font.GetWeight() != startfont.GetWeight()) insert += _T("\\b") + wxString::Format(_T("%i"),font.GetWeight() == wxFONTWEIGHT_BOLD ? 1 : 0);
		if (font.GetStyle() != startfont.GetStyle()) insert += _T("\\i") + wxString::Format(_T("%i"),font.GetStyle() == wxFONTSTYLE_ITALIC ? 1 : 0);
		if (font.GetUnderlined() != startfont.GetUnderlined()) insert += _T("\\u") + wxString::Format(_T("%i"),font.GetUnderlined() ? 1 : 0);
		if (insert.IsEmpty()) return;
	}

	// Pos
	if (isPos) {
		insert = tagname + preValue;
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
		for (size_t i=0;i<override->Tags.size()-1;i++) {
			if (override->Tags.at(i)->Name == tagname) {
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
		SetText(line->Text);
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
			for (size_t i=0;i<override->Tags.size()-1;i++) {
				if (override->Tags.at(i)->Name == tagname) {
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
		TextEdit->SetSelection(origStart+shift,selend+shift);
	}

	// Commit changes and shift selection
	SetText(line->Text);
	line->ClearBlocks();
	TextEdit->SetSelection(selstart+shift,selend+shift);
	TextEdit->SetFocus();
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

