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


#ifndef DIALOG_HOTKEYS_H
#define DIALOG_HOTKEYS_H


////////////
// Includes
#include <wx/wxprec.h>
#include <wx/listctrl.h>
#include <map>
#include "hotkeys.h"


//////////////
// Prototypes
class FrameMain;
class DialogInputHotkey;


/////////////////////
// List dialog class
class DialogHotkeys : public wxDialog {
private:
	std::map<wxString,HotkeyType> origKeys;
	wxListView *Shortcuts;
	bool modified;
	FrameMain *parent;

	void OnEditItem(wxListEvent &event);
	void OnOK(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);

public:
	DialogHotkeys(FrameMain *parent);

	DECLARE_EVENT_TABLE()
};


/////////////////////
// Capture key class
class CaptureKey : public wxTextCtrl {
private:
	DialogInputHotkey *parent;
	void OnKeyDown(wxKeyEvent &event);
	void OnLoseFocus(wxFocusEvent &event);

public:
	CaptureKey(DialogInputHotkey *parent);

	DECLARE_EVENT_TABLE()
};


//////////////////////
// Input dialog class
class DialogInputHotkey : public wxDialog {
	friend class CaptureKey;

private:
	CaptureKey *capture;
	HotkeyType *key;

public:
	DialogInputHotkey(HotkeyType *key,wxString name);
};


///////
// IDs
enum {
	Hotkey_List = 2500
};


#endif
