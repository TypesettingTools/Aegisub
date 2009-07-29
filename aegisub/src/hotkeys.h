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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file hotkeys.h
/// @see hotkeys.cpp
/// @ingroup main_ui
///


#ifndef HOTKEY_H

/// DOCME
#define HOTKEY_H


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/string.h>
#include <wx/accel.h>
#include <map>


//////////////
// Prototypes
class DialogOptions;



/// DOCME
/// @class HotkeyType
/// @brief DOCME
///
/// DOCME
class HotkeyType {
public:

	/// DOCME
	int flags;

	/// DOCME
	int keycode;

	/// DOCME
	wxString origName;

	HotkeyType();
	HotkeyType(wxString text,wxString name);
	
	void Parse(wxString text);
	wxString GetText();


	/// DOCME
	static std::map<int,wxString> keyName;
	static wxString GetKeyName(int keycode);
	static void FillMap();
};



/// DOCME
/// @class HotkeyManager
/// @brief DOCME
///
/// DOCME
class HotkeyManager {
	friend class DialogOptions;
private:

	/// DOCME
	bool modified;

	/// DOCME
	wxString filename;


	/// DOCME
	int lastKey;

	/// DOCME
	int lastMod;

public:

	/// DOCME
	std::map<wxString,HotkeyType> key;

	HotkeyManager();
	~HotkeyManager();

	void SetFile(wxString file);
	void SetHotkey(wxString function,HotkeyType hotkey);
	void SetHotkey(wxString function,wxString hotkey);
	void Save();
	void Load();
	void LoadDefaults();
	HotkeyType *Find(int keycode,int mod);

	wxString GetText(wxString function);
	wxAcceleratorEntry GetAccelerator(wxString function,int id);
	bool IsPressed(wxString function);
	void SetPressed(int key,bool ctrl=false,bool alt=false,bool shift=false);
};


///////////////////
// Global instance
extern HotkeyManager Hotkeys;


#endif


