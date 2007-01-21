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
#include <wx/wxprec.h>
#include <wx/settings.h>
#include "dialog_translation.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "subs_grid.h"
#include "video_display.h"
#include "subs_edit_box.h"
#include "options.h"
#include "audio_display.h"
#include "frame_main.h"
#include "hotkeys.h"


///////////////
// Constructor
DialogTranslation::DialogTranslation (wxWindow *parent,AssFile *_subs,SubtitlesGrid *_grid,int startrow,bool preview)
: wxDialog(parent, -1, _("Translation Assistant"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX, _T("TranslationAssistant"))
{
	// Set variables
	enablePreview = preview;
	main = parent;
	subs = _subs;
	grid = _grid;
	audio = VideoContext::Get()->audio;

	// Translation box
	wxSizer *TranslationSizer = new wxBoxSizer(wxVERTICAL);
	OrigText = new wxTextCtrl(this,TEXT_ORIGINAL,_T(""),wxDefaultPosition,wxSize(300,80),wxTE_MULTILINE | wxTE_RICH  | wxTE_READONLY);
	TransText = new wxTextCtrl(this,TEXT_TRANS,_T(""),wxDefaultPosition,wxSize(300,80),wxTE_MULTILINE | wxTE_RICH2);
	OrigText->SetEventHandler(new DialogTranslationEvent(this));
	TransText->SetEventHandler(new DialogTranslationEvent(this));
	TransText->SetFocus();
	wxSizer *OriginalTransSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Original"));
	wxSizer *TranslatedSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Translation"));
	LineCount = new wxStaticText(this,-1,_("Current line: ?"));
	OriginalTransSizer->Add(LineCount,0,wxBOTTOM,5);
	OriginalTransSizer->Add(OrigText,1,wxEXPAND,0);
	TranslatedSizer->Add(TransText,1,wxEXPAND,0);
	TranslationSizer->Add(OriginalTransSizer,1,wxEXPAND,0);
	TranslationSizer->Add(TranslatedSizer,1,wxTOP|wxEXPAND,5);

	// Hotkeys
	wxSizer *KeysSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Keys"));
	wxSizer *KeysInnerSizer = new wxGridSizer(2,0,5);
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Translation Assistant Accept")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Accept changes")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Translation Assistant Preview")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Preview changes")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Translation Assistant Prev")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Previous line")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Translation Assistant Next")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Next line")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Translation Assistant Insert Original")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Insert original")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,Hotkeys.GetText(_T("Translation Assistant Play")) + _T(": ")));
	KeysInnerSizer->Add(new wxStaticText(this,-1,_("Play Audio")));
	PreviewCheck = new wxCheckBox(this,PREVIEW_CHECK,_("Enable preview"));
	PreviewCheck->SetValue(preview);
	PreviewCheck->SetEventHandler(new DialogTranslationEvent(this));
	KeysSizer->Add(KeysInnerSizer,0,wxEXPAND,0);
	KeysSizer->Add(PreviewCheck,0,wxTOP,5);

	// Button sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
	wxButton *PlayButton = new wxButton(this,BUTTON_TRANS_PLAY,_("Play Audio"));
	PlayButton->Enable(audio->loaded);
	ButtonSizer->Add(PlayButton,0,wxRIGHT,0);
	ButtonSizer->Add(new wxButton(this,wxID_CLOSE),0,wxRIGHT,0);

	// General layout
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TranslationSizer,1,wxALL | wxEXPAND,5);
	MainSizer->Add(KeysSizer,0,wxLEFT | wxBOTTOM | wxRIGHT | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxALIGN_RIGHT | wxLEFT | wxBOTTOM | wxRIGHT,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);

	// Position window
	if (lastx == -1 && lasty == -1) {
		CenterOnParent();
	} else {
		Move(lastx, lasty);
	}

	// Set subs/grid
	JumpToLine(startrow,0);
	UpdatePreview();
}


//////////////////////////
// Jumps to line at block
bool DialogTranslation::JumpToLine(int n,int block) {
	using std::vector;
	AssDialogue *nextLine;
	try {
		nextLine = grid->GetDialogue(n);
	} catch (...) {
		return false;
	}
	if (!nextLine) return false;
	current = nextLine;

	// Count blocks
	int nblocks = 0;
	current->ParseASSTags();
	size_t size_blocks = current->Blocks.size();
	for (size_t i=0;i<size_blocks;i++) {
		if (current->Blocks.at(i)->type == BLOCK_PLAIN) nblocks++;
	}

	// Wrap around
	if (block == 0xFFFF) block = nblocks-1;
	if (block < 0) {
		block = 0xFFFF;
		n--;
		return JumpToLine(n,block);
	}
	if (block >= nblocks) {
		block = 0;
		n++;
		return JumpToLine(n,block);
	}

	// Set current
	curline = n;
	current = grid->GetDialogue(n);
	curblock = block;
	LineCount->SetLabel(wxString::Format(_("Current line: %i/%i"),curline+1,grid->GetRows()));

	// Update grid
	grid->BeginBatch();
	grid->SelectRow(curline);
	grid->MakeCellVisible(curline,0);
	grid->editBox->SetToLine(curline);
	grid->EndBatch();

	// Set styles
	wxTextAttr Normal(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	wxTextAttr Highlight(wxColour(0,0,255));
	wxFont font = Normal.GetFont();
	font.SetPointSize(Options.AsInt(_T("Edit Font Size")));
	Normal.SetFont(font);
	Highlight.SetFont(font);

	// Adds blocks
	OrigText->Clear();
	AssDialogueBlock *curBlock;
	bool found = false;
	int pos=-1;
	for (vector<AssDialogueBlock*>::iterator cur=current->Blocks.begin();cur!=current->Blocks.end();cur++) {
		curBlock = *cur;
		if (curBlock->type == BLOCK_PLAIN) {
			pos++;
			if (pos == block) {
				OrigText->SetDefaultStyle(Highlight);
				found = true;
			}
			else OrigText->SetDefaultStyle(Normal);
		}
		else OrigText->SetDefaultStyle(Normal);

		if (curBlock->type == BLOCK_OVERRIDE) OrigText->AppendText(_T("{"));
		OrigText->AppendText(curBlock->text);
		if (curBlock->type == BLOCK_OVERRIDE) OrigText->AppendText(_T("}"));
	}
	OrigText->SetDefaultStyle(Normal);
	TransText->SetDefaultStyle(Normal);
	current->ClearBlocks();

	return true;
}


/////////////////////////
// Updates video preview
void DialogTranslation::UpdatePreview () {
	if (enablePreview) {
		try {
			if (VideoContext::Get()->IsLoaded()) {
				AssDialogue *cur = grid->GetDialogue(curline);
				VideoContext::Get()->JumpToTime(cur->Start.GetMS());
			}
		}
		catch (...) {
			return;
		}
	}
}


//////////////
// Play audio
void DialogTranslation::Play() {
	if (audio->loaded) {
		audio->Play(current->Start.GetMS(),current->End.GetMS());
	}
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogTranslation, wxDialog)
	EVT_BUTTON(wxID_CLOSE,DialogTranslation::OnClose)
	EVT_BUTTON(BUTTON_TRANS_PLAY,DialogTranslation::OnPlayButton)
END_EVENT_TABLE()


/////////////////
// Event handler

// Constructor
DialogTranslationEvent::DialogTranslationEvent(DialogTranslation *ctrl) {
	control = ctrl;
}

// Event table
BEGIN_EVENT_TABLE(DialogTranslationEvent, wxEvtHandler)
	EVT_KEY_DOWN(DialogTranslationEvent::OnTransBoxKey)
	EVT_CHECKBOX(PREVIEW_CHECK, DialogTranslationEvent::OnPreviewCheck)
END_EVENT_TABLE()

// Redirects
void DialogTranslationEvent::OnPreviewCheck(wxCommandEvent &event) { control->enablePreview = event.IsChecked(); }
void DialogTranslationEvent::OnTransBoxKey(wxKeyEvent &event) { control->OnTransBoxKey(event); }


/////////////////////
// Key pressed event
void DialogTranslation::OnTransBoxKey(wxKeyEvent &event) {
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_controlDown,event.m_altDown,event.m_shiftDown);

	// Previous
	if (Hotkeys.IsPressed(_T("Translation Assistant Prev"))) {
		bool ok = JumpToLine(curline,curblock-1);
		if (ok) {
			TransText->Clear();
			TransText->SetFocus();
		}

		UpdatePreview();
		return;
	}

	// Next
	if (Hotkeys.IsPressed(_T("Translation Assistant Next")) || (Hotkeys.IsPressed(_T("Translation Assistant Accept")) && TransText->GetValue().IsEmpty())) {
		bool ok = JumpToLine(curline,curblock+1);
		if (ok) {
			TransText->Clear();
			TransText->SetFocus();
		}

		UpdatePreview();
		return;
	}

	// Accept (enter)
	if (Hotkeys.IsPressed(_T("Translation Assistant Accept")) || Hotkeys.IsPressed(_T("Translation Assistant Preview"))) {
		// Store
		AssDialogue *cur = grid->GetDialogue(curline);
		cur->ParseASSTags();
		int nblock = -1;
		for (unsigned int i=0;i<cur->Blocks.size();i++) {
			if (cur->Blocks.at(i)->type == BLOCK_PLAIN) nblock++;
			if (nblock == curblock) {
				cur->Blocks.at(i)->text = TransText->GetValue();
				break;
			}
		}
		
		// Update line
		cur->UpdateText();
		cur->UpdateData();
		cur->ClearBlocks();
		subs->FlagAsModified();
		grid->CommitChanges();
		((FrameMain*)main)->UpdateTitle();
		UpdatePreview();

		// Next
		if (Hotkeys.IsPressed(_T("Translation Assistant Accept"))) {
			bool ok = JumpToLine(curline,curblock+1);
			TransText->Clear();
			TransText->SetFocus();
		}
		else bool ok = JumpToLine(curline,curblock);
		return;
	}

	// Insert original text (insert)
	if (Hotkeys.IsPressed(_T("Translation Assistant Insert Original"))) {
		using std::vector;
		AssDialogueBlock *curBlock;
		int pos = -1;
		current->ParseASSTags();
		for (vector<AssDialogueBlock*>::iterator cur=current->Blocks.begin();cur!=current->Blocks.end();cur++) {
			curBlock = *cur;
			if (curBlock->type == BLOCK_PLAIN) {
				pos++;
				if (pos == curblock) {
					TransText->WriteText(curBlock->text);
				}
			}
		}
		current->ClearBlocks();
		return;
	}

	// Play audio
	if (Hotkeys.IsPressed(_T("Translation Assistant Play"))) {
		Play();
		return;
	}

	// Skip anything else
	event.Skip();
}


///////////////
// Play button
void DialogTranslation::OnPlayButton(wxCommandEvent &event) {
	Play();
	TransText->SetFocus();
}


/////////
// Close
void DialogTranslation::OnClose (wxCommandEvent &event) {
	GetPosition(&lastx, &lasty);
	Close();
}


////////////
// Minimize
void DialogTranslation::OnMinimize (wxIconizeEvent &event) {
	//Iconize(true);
	if (main) ((wxFrame*)main)->Iconize(true);
	event.Skip();
}


int DialogTranslation::lastx = -1;
int DialogTranslation::lasty = -1;
