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
#include <wx/recguard.h>
#include "dialog_styling_assistant.h"
#include "subs_grid.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_dialogue.h"
#include "video_display.h"
#include "vfr.h"
#include "frame_main.h"
#include "audio_display.h"
#include "audio_box.h"
#include "hotkeys.h"


///////////////
// Constructor
DialogStyling::DialogStyling (wxWindow *parent,SubtitlesGrid *_grid) :
wxDialog (parent, -1, _("Styling assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxRESIZE_BORDER)
{
	// Variables
	grid = _grid;
	audio = VideoContext::Get()->audio->box->audioDisplay;
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
	wxSizer *RightBottom = new wxStaticBoxSizer(wxVERTICAL,this,_("Keys"));
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
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Styling Assistant Play")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Play Audio")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Click on list:")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Select style")));

	// Rest of right sizer
	PreviewCheck = new wxCheckBox(this,-1,_("Enable preview (slow)"));
	PreviewCheck->SetValue(true);
	RightBottom->Add(KeysInnerSizer,0,wxEXPAND | wxBOTTOM,5);
	RightBottom->Add(PreviewCheck,0,0,0);
	RightBottom->AddStretchSpacer(1);
	RightSizer->Add(RightTop,0,wxEXPAND | wxBOTTOM,5);
	RightSizer->Add(RightBottom,1,wxEXPAND | wxBOTTOM,0);

	// Bottom sizer
	wxSizer *BottomSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer->Add(LeftSizer,1,wxEXPAND | wxRIGHT,5);
	BottomSizer->Add(RightSizer,1,wxEXPAND,0);

	// Button sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->Add(new wxButton(this,wxID_HELP),0,wxRIGHT,0);
	ButtonSizer->AddStretchSpacer(1);
	wxButton *PlayButton = new wxButton(this,BUTTON_PLAY,_("Play Audio"));
	PlayButton->Enable(audio->loaded);
	ButtonSizer->Add(PlayButton,0,wxRIGHT,5);
	ButtonSizer->Add(new wxButton(this,wxID_OK),0,wxRIGHT,0);

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
		grid->ass->FlagAsModified();
		grid->CommitChanges();
	}
}


////////////////
// Jump to line
void DialogStyling::JumpToLine(int n) {
	// Check stuff
	if (linen == n) return;
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
	if (TypeBox->GetValue().IsEmpty()) TypeBox->SetValue(Styles->GetString(0));
	TypeBox->SetSelection(0,TypeBox->GetValue().Length());

	// Update grid
	grid->SelectRow(linen,false);
	grid->MakeCellVisible(linen,0);

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
		grid->ass->FlagAsModified();
		grid->CommitChanges();
	}
	else needCommit = true;

	// Next line
	if (jump) JumpToLine(linen+1);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogStyling,wxDialog)
	EVT_BUTTON(wxID_HELP, DialogStyling::OnHelpButton)
	EVT_BUTTON(BUTTON_PLAY, DialogStyling::OnPlayButton)
	//EVT_TEXT_ENTER(ENTER_STYLE_BOX, DialogStyling::OnStyleBoxEnter)
	EVT_TEXT(ENTER_STYLE_BOX, DialogStyling::OnStyleBoxModified)
	EVT_LISTBOX(STYLE_LIST, DialogStyling::OnListClicked)
	EVT_KEY_DOWN(DialogStyling::OnKeyDown)
END_EVENT_TABLE()


///////////////
// Key pressed
void DialogStyling::OnKeyDown(wxKeyEvent &event) {
	int keycode = event.GetKeyCode();

	// Previous line
	if (keycode == WXK_PRIOR) {
		JumpToLine(linen-1);
	}

	// Next line
	if (keycode == WXK_NEXT) {
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


///////////////
// Help button
void DialogStyling::OnHelpButton(wxCommandEvent &event) {
	FrameMain::OpenHelp(_T("stylingassistant2.htm"));
}


///////////////
// Play button
void DialogStyling::OnPlayButton(wxCommandEvent &event) {
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
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_controlDown,event.m_altDown,event.m_shiftDown);

	// Backspace
	if (event.GetKeyCode() == WXK_BACK && !event.m_controlDown && !event.m_altDown && !event.m_shiftDown) {
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

	// Play audio
	if (Hotkeys.IsPressed(_T("Styling Assistant Play"))) {
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
