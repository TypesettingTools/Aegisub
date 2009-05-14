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


///////////
// Headers
#include "config.h"

#include <wx/recguard.h>
#include "dialog_styling_assistant.h"
#include "subs_grid.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_dialogue.h"
#include "video_display.h"
#include "video_context.h"
#include "vfr.h"
#include "frame_main.h"
#include "audio_display.h"
#include "audio_box.h"
#include "hotkeys.h"
#include "utils.h"
#include "help_button.h"
#include "subs_edit_box.h"


///////////////
// Constructor
DialogStyling::DialogStyling (wxWindow *parent,SubtitlesGrid *_grid) :
wxDialog (parent, -1, _("Styling assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX)
{
	// Set icon
	SetIcon(BitmapToIcon(wxBITMAP(styling_toolbutton)));

	// Variables
	grid = _grid;
	audio = VideoContext::Get()->audio->box->audioDisplay;
	video = video->Get();
	needCommit = false;
	linen = -1;

	// Top sizer
	wxSizer *TopSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Current line"));
	CurLine = new wxTextCtrl(this,-1,_("Current line"),wxDefaultPosition,wxSize(300,60),wxTE_MULTILINE | wxTE_READONLY);
	TopSizer->Add(CurLine,1,wxEXPAND,0);

	// Left sizer
	Styles = new wxListBox(this,STYLE_LIST,wxDefaultPosition,wxSize(150,180),grid->ass->GetStyles());
	wxSizer *LeftSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Styles available"));
	LeftSizer->Add(Styles,1,wxEXPAND,0);

	// Right sizer
	wxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *RightTop = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Set style"));
	wxSizer *RightMiddle = new wxStaticBoxSizer(wxVERTICAL,this,_("Keys"));
	wxSizer *RightBottom = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Actions"));
	TypeBox = new StyleEditBox(this);
	RightTop->Add(TypeBox,1,wxEXPAND);

	// Shortcuts
	//wxStaticText *Keys = new wxStaticText(this,-1,_("Enter:\t\tAccept changes\nPage up:\tPrevious line\nPage down:\tNext line\nEnd:\t\tPlay sound\nClick on list:\tSet style\nCtrl+enter:\tAccept without going to next"));
	wxSizer *KeysInnerSizer = new wxGridSizer(2,0,5);
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Accept")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Accept changes")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Preview")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Preview changes")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Prev")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Previous line")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Next")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Next line")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Play Video")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Play Video")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Play Audio")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Play Audio")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Click on list:")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Select style")));

	// Right Middle
	PreviewCheck = new wxCheckBox(this,-1,_("Enable preview (slow)"));
	PreviewCheck->SetValue(true);
	RightMiddle->Add(KeysInnerSizer,0,wxEXPAND | wxBOTTOM,5);
	RightMiddle->Add(PreviewCheck,0,0,0);
	RightMiddle->AddStretchSpacer(1);

	// Rest of right sizer
	PlayVideoButton = new wxButton(this,BUTTON_PLAY_VIDEO,_("Play Video"));
	PlayAudioButton = new wxButton(this,BUTTON_PLAY_AUDIO,_("Play Audio"));
	RightBottom->AddStretchSpacer(1);
	RightBottom->Add(PlayAudioButton,0,wxLEFT | wxRIGHT | wxBOTTOM,5);
	RightBottom->Add(PlayVideoButton,0,wxBOTTOM | wxRIGHT,5);
	RightBottom->AddStretchSpacer(1);

	RightSizer->Add(RightTop,0,wxEXPAND | wxBOTTOM,5);
	RightSizer->Add(RightMiddle,0,wxEXPAND | wxBOTTOM,5);
	RightSizer->Add(RightBottom,0,wxEXPAND,5);

	// Bottom sizer
	wxSizer *BottomSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer->Add(LeftSizer,1,wxEXPAND | wxRIGHT,5);
	BottomSizer->Add(RightSizer,1,wxEXPAND,0);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,_T("Styling Assistant")));
	ButtonSizer->Realize();

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxEXPAND | wxALL,5);
	MainSizer->Add(BottomSizer,1,wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);

	// Position window
	if (lastx == -1 && lasty == -1) {
		CenterOnParent();
	} else {
		Move(lastx, lasty);
	}

	// h4x
	origColour = TypeBox->GetBackgroundColour();

	// Set selection
	wxArrayInt sel = grid->GetSelection();
	if (sel.Count() > 0) JumpToLine(sel[0]);
	else JumpToLine(0);
}


//////////////
// Destructor
DialogStyling::~DialogStyling () {
	GetPosition(&lastx, &lasty);
	if (needCommit) {
		grid->ass->FlagAsModified(_("style changes"));
		grid->CommitChanges();
	}
}


////////////////
// Jump to line
void DialogStyling::JumpToLine(int n) {
	// Check stuff
	if (n == -1) return;

	// Get line
	AssDialogue *nextLine = grid->GetDialogue(n);
	if (!nextLine) return;
	line = nextLine;

	// Set number
	linen = n;

	// Set text
	CurLine->SetValue(line->Text);

	// Set focus
	TypeBox->SetFocus();
	bool matched = false;
	for (size_t i = 0; i < Styles->GetCount(); i++) {
		if (TypeBox->GetValue().IsSameAs(Styles->GetString(i),true)) {
			matched = true;
			break;
		}
	}
	if (!matched || TypeBox->GetValue().IsEmpty()) TypeBox->SetValue(Styles->GetString(0));
	TypeBox->SetSelection(0,TypeBox->GetValue().Length());

	// Update grid
	grid->SelectRow(linen,false);
	grid->MakeCellVisible(linen,0);
	grid->editBox->SetToLine(linen);

	// Update display
	if (PreviewCheck->IsChecked()) VideoContext::Get()->JumpToFrame(VFR_Output.GetFrameAtTime(line->Start.GetMS(),true));
}


/////////////////////////////
// Set style of current line
void DialogStyling::SetStyle (wxString curName, bool jump) {
	// Get line
	AssDialogue *line = grid->GetDialogue(linen);

	// Update line
	line->Style = curName;
	line->UpdateData();

	// Update grid/subs
	grid->Refresh(false);
	if (PreviewCheck->IsChecked()) {
		grid->ass->FlagAsModified(_("styling assistant"));
		grid->CommitChanges();
	}
	else needCommit = true;

	// Next line
	if (jump) JumpToLine(linen+1);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogStyling,wxDialog)
	EVT_ACTIVATE(DialogStyling::OnActivate)
	EVT_BUTTON(BUTTON_PLAY_VIDEO, DialogStyling::OnPlayVideoButton)
	EVT_BUTTON(BUTTON_PLAY_AUDIO, DialogStyling::OnPlayAudioButton)
	//EVT_TEXT_ENTER(ENTER_STYLE_BOX, DialogStyling::OnStyleBoxEnter)
	EVT_TEXT(ENTER_STYLE_BOX, DialogStyling::OnStyleBoxModified)
	EVT_LISTBOX(STYLE_LIST, DialogStyling::OnListClicked)
	EVT_KEY_DOWN(DialogStyling::OnKeyDown)
END_EVENT_TABLE()


///////////////////////////
// Dialog was De/Activated
void DialogStyling::OnActivate(wxActivateEvent &event) {
	// Dialog lost focus
	if (!event.GetActive()) {
		if (!PreviewCheck->IsChecked()) {
			grid->ass->FlagAsModified(_("styling assistant"));
			grid->CommitChanges();
		}
		return;
	}
	// Enable/disable play video/audio buttons
	PlayVideoButton->Enable(video->IsLoaded());
	PlayAudioButton->Enable(audio->loaded);
	// Update grid
	if (grid->ass != AssFile::top)
		grid->LoadFromAss(AssFile::top,false,true);
	// Fix style list
	Styles->Set(grid->ass->GetStyles());
	// Fix line selection
	linen = grid->GetFirstSelRow();
	JumpToLine(linen);
}


///////////////
// Key pressed
void DialogStyling::OnKeyDown(wxKeyEvent &event) {
	int keycode = event.GetKeyCode();

	// Previous line
	if (keycode == WXK_PAGEUP) {
		JumpToLine(linen-1);
	}

	// Next line
	if (keycode == WXK_PAGEDOWN) {
		JumpToLine(linen+1);
	}

	event.Skip();
}


////////////////////
// Edit box changed
void DialogStyling::OnStyleBoxModified (wxCommandEvent &event) {
	// Recursion guard
	static wxRecursionGuardFlag s_flag;
	wxRecursionGuard guard(s_flag);
	if (guard.IsInside()) return;

	// Find partial style name
	long from,to;
	TypeBox->GetSelection(&from,&to);
	wxString partial = TypeBox->GetValue().Left(from).Lower();
	int len = partial.Length();

	// Find first style that matches partial name
	bool match = false;
	int n = Styles->GetCount();
	wxString curname;
	for (int i=0;i<n;i++) {
		curname = Styles->GetString(i);
		if (curname.Left(len).Lower() == partial) {
			match = true;
			break;
		}
	}

	// Found
	if (match) {
		TypeBox->SetValue(curname);
		TypeBox->SetSelection(from,curname.Length());
		TypeBox->SetBackgroundColour(origColour);
		TypeBox->Refresh();
	}

	// Not found
	else {
		TypeBox->SetBackgroundColour(wxColour(255,108,108));
		TypeBox->Refresh();
	}
}


/////////////////
// Enter pressed
void DialogStyling::OnStyleBoxEnter (wxCommandEvent &event) {

}


//////////////////////
// Style list clicked
void DialogStyling::OnListClicked(wxCommandEvent &event) {
	int n = event.GetInt();
	SetStyle(Styles->GetString(n));
	Styles->SetSelection(wxNOT_FOUND);
	TypeBox->SetFocus();
}

/////////////////////
// Play video button
void DialogStyling::OnPlayVideoButton(wxCommandEvent &event) {
	video->PlayLine();
	TypeBox->SetFocus();
}

/////////////////////
// Play audio button
void DialogStyling::OnPlayAudioButton(wxCommandEvent &event) {
	audio->Play(line->Start.GetMS(),line->End.GetMS());
	TypeBox->SetFocus();
}


//////////////////////////////
// Style edit box constructor
StyleEditBox::StyleEditBox(DialogStyling *parent)
: wxTextCtrl(parent,ENTER_STYLE_BOX,_T(""),wxDefaultPosition,wxSize(180,-1),wxTE_PROCESS_ENTER)
{
	diag = parent;
}


////////////////////////////
// Event table for edit box
BEGIN_EVENT_TABLE(StyleEditBox,wxTextCtrl)
	EVT_KEY_DOWN(StyleEditBox::OnKeyDown)
END_EVENT_TABLE()


///////////////
// Key pressed
void StyleEditBox::OnKeyDown(wxKeyEvent &event) {
	//int keycode = event.GetKeyCode();
#ifdef __APPLE__
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_metaDown,event.m_altDown,event.m_shiftDown);
#else
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_controlDown,event.m_altDown,event.m_shiftDown);
#endif

	// Backspace
#ifdef __APPLE__
	if (event.GetKeyCode() == WXK_BACK && !event.m_metaDown && !event.m_altDown && !event.m_shiftDown) {
#else
	if (event.GetKeyCode() == WXK_BACK && !event.m_controlDown && !event.m_altDown && !event.m_shiftDown) {
#endif
		long from,to;
		GetSelection(&from,&to);
		if (from > 0) SetSelection(from-1,to);
	}

	// Previous line
	if (Hotkeys.IsPressed(_T("Styling Assistant Prev"))) {
		diag->JumpToLine(diag->linen-1);
		return;
	}

	// Next line
	if (Hotkeys.IsPressed(_T("Styling Assistant Next"))) {
		diag->JumpToLine(diag->linen+1);
		return;
	}

	// Play video
	if (Hotkeys.IsPressed(_T("Styling Assistant Play Video"))) {
		if (diag->video->IsLoaded()) {
			diag->video->PlayLine();
		}
		return;
	}

	// Play audio
	if (Hotkeys.IsPressed(_T("Styling Assistant Play Audio"))) {
		if (diag->audio->loaded) {
			diag->audio->Play(diag->line->Start.GetMS(),diag->line->End.GetMS());
		}
		return;
	}

	// Enter key
	if (Hotkeys.IsPressed(_T("Styling Assistant Accept")) || Hotkeys.IsPressed(_T("Styling Assistant Preview"))) {
		// Make sure that entered style is valid
		int n = diag->Styles->GetCount();
		wxString curName;
		bool match = false;
		for (int i=0;i<n;i++) {
			curName = diag->Styles->GetString(i);
			if (curName == GetValue()) {
				match = true;
				break;
			}
		}

		// Set style name
		if (match) {
			diag->SetStyle(curName,Hotkeys.IsPressed(_T("Styling Assistant Accept")));
		}
		return;
	}

	event.Skip();
}


int DialogStyling::lastx = -1;
int DialogStyling::lasty = -1;
