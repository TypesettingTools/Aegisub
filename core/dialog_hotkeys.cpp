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
#include "dialog_hotkeys.h"
#include "frame_main.h"
#include "main.h"


///////////////
// Constructor
DialogHotkeys::DialogHotkeys(FrameMain *_parent)
: wxDialog(NULL, -1, _("Hotkeys"), wxDefaultPosition, wxSize(300,300), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER, _T("Hotkeys"))
{
	// Backup original shortcuts
	origKeys = Hotkeys.key;

	// Set variables
	modified = false;
	parent = _parent;

	// Description
	wxStaticText *text = new wxStaticText(this,-1,_("List of all hotkeys (shortcuts) available in Aegisub. Double click on any item to reassign it."));

	// List of shortcuts
	Shortcuts = new wxListView(this,Hotkey_List,wxDefaultPosition,wxSize(450,380),wxLC_REPORT | wxLC_SINGLE_SEL);
	Shortcuts->InsertColumn(0,_("Function"),wxLIST_FORMAT_LEFT,245);
	Shortcuts->InsertColumn(1,_("Key"),wxLIST_FORMAT_LEFT,180);

	// Populate list
	std::map<wxString,HotkeyType>::iterator cur;
	for (cur = Hotkeys.key.end();cur-- != Hotkeys.key.begin();) {
		wxListItem item;
		item.SetText(wxGetTranslation(cur->second.origName));
		item.SetData(&cur->second);
		int pos = Shortcuts->InsertItem(item);
		Shortcuts->SetItem(pos,1,cur->second.GetText());
	}

	// Button sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
#ifndef __APPLE__
	ButtonSizer->Add(new wxButton(this,wxID_OK),0,wxRIGHT|wxEXPAND,5);
	ButtonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT|wxEXPAND,0);
#else
	ButtonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT|wxEXPAND,5);
	ButtonSizer->Add(new wxButton(this,wxID_OK),0,wxRIGHT|wxEXPAND,0);
#endif

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(text,0,wxALL|wxEXPAND,5);
	MainSizer->Add(Shortcuts,1,wxLEFT|wxRIGHT|wxTOP|wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxALL|wxEXPAND,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogHotkeys,wxDialog)
	EVT_LIST_ITEM_ACTIVATED (Hotkey_List,DialogHotkeys::OnEditItem)
	EVT_BUTTON(wxID_OK,DialogHotkeys::OnOK)
	EVT_BUTTON(wxID_CANCEL,DialogHotkeys::OnCancel)
END_EVENT_TABLE()


//////////////
// OK pressed
void DialogHotkeys::OnOK(wxCommandEvent &event) {
	// Apply changes if modified
	if (modified) {
		// Save changes
		Hotkeys.modified = true;
		Hotkeys.Save();

		// Rebuild menu
		parent->InitMenu();

		// Rebuild accelerator table
		parent->SetAccelerators();
	}

	// Close
	EndModal(0);
}


//////////////////
// Cancel pressed
void DialogHotkeys::OnCancel(wxCommandEvent &event) {
	// Restore if it was modified
	if (modified) Hotkeys.key = origKeys;

	// Close
	EndModal(0);
}


/////////////////
// Edit a hotkey
void DialogHotkeys::OnEditItem(wxListEvent &event) {
	// Get key and store old
	HotkeyType *curKey = (HotkeyType *)event.GetData();
	int oldKeycode = curKey->keycode;
	int oldFlags = curKey->flags;

	// Open dialog
	DialogInputHotkey input(curKey,event.GetText());
	input.ShowModal();

	// Update stuff if it changed
	if (oldKeycode != curKey->keycode || oldFlags != curKey->flags) {
		Shortcuts->SetItem(event.GetIndex(),1,curKey->GetText());
		modified = true;
	}
}


/////////////////////
// Input constructor
DialogInputHotkey::DialogInputHotkey(HotkeyType *_key,wxString name)
: wxDialog(NULL, -1, _("Press Key"), wxDefaultPosition, wxSize(200,50), wxCAPTION | wxWANTS_CHARS , _T("Press key"))
{
	// Key
	key = _key;

	// Text
	wxStaticText *text = new wxStaticText(this,-1,_("Press key to bind to \"") + name + _("\" or esc to cancel."));

	// Key capturer
	capture = new CaptureKey(this);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(text,1,wxALL,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
}


////////////////////////
// Capturer constructor
CaptureKey::CaptureKey(DialogInputHotkey *_parent)
: wxTextCtrl(_parent,-1,_T(""),wxDefaultPosition,wxSize(0,0))
{
	parent = _parent;
	SetFocus();
}


/////////////////////
// Input event table
BEGIN_EVENT_TABLE(CaptureKey,wxTextCtrl)
	EVT_KEY_DOWN(CaptureKey::OnKeyDown)
	EVT_KILL_FOCUS(CaptureKey::OnLoseFocus)
END_EVENT_TABLE()


///////////////
// On key down
void CaptureKey::OnKeyDown(wxKeyEvent &event) {
	int keycode = event.GetKeyCode();

	if (keycode == WXK_ESCAPE) parent->EndModal(0);
	else if (keycode != WXK_SHIFT && keycode != WXK_CONTROL && keycode != WXK_ALT) {
		parent->key->keycode = keycode;
		int mod = 0;
		if (event.m_altDown) mod |= wxACCEL_ALT;
		if (event.m_controlDown) mod |= wxACCEL_CTRL;
		if (event.m_shiftDown) mod |= wxACCEL_SHIFT;
		parent->key->flags = mod;
		parent->EndModal(0);
	}
	else event.Skip();
}


//////////////
// Keep focus
void CaptureKey::OnLoseFocus(wxFocusEvent &event) {
	SetFocus();
}
