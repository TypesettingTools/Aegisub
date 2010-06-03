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

/// @file hotkeys.cpp
/// @brief Keep hotkey configuration and handle hotkey presses
/// @ingroup main_ui
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <fstream>

#include <wx/accel.h>
#include <wx/filename.h>
#include <wx/log.h>
#endif

#include "hotkeys.h"
#include "text_file_reader.h"
#include "text_file_writer.h"


/// @brief Constructors  HotkeyType //////////////////////////////////
///
HotkeyType::HotkeyType() {
	flags = 0;
	keycode = 0;
}


/// @brief DOCME
/// @param text 
/// @param name 
///
HotkeyType::HotkeyType(wxString text,wxString name) {
	Parse(text);
	origName = name;
}



/// @brief Get string of hotkey 
/// @return 
///
wxString HotkeyType::GetText() const {
	wxString text;

	// Modifiers
	if (flags & wxACCEL_CTRL) text += _T("Ctrl-");
	if (flags & wxACCEL_ALT) text += _T("Alt-");
	if (flags & wxACCEL_SHIFT) text += _T("Shift-");

	// Key name
	text += GetKeyName(keycode);

	return text;
}



/// @brief Parse text into hotkey 
/// @param text 
///
void HotkeyType::Parse(wxString text) {
	// Reset
	flags = 0;
	keycode = 0;
	wxString work = text.Lower();

	// Parse modifiers
	while (true) {
		// Ctrl
		if (work.Left(5) == _T("ctrl-")) {
			flags |= wxACCEL_CTRL;
			work = work.Mid(5);
			continue;
		}

		// Alt
		if (work.Left(4) == _T("alt-")) {
			flags |= wxACCEL_ALT;
			work = work.Mid(4);
			continue;
		}

		// Shift
		if (work.Left(6) == _T("shift-")) {
			flags |= wxACCEL_SHIFT;
			work = work.Mid(6);
			continue;
		}

		break;
	}

	// Get key name
	FillMap();
	bool got = false;
	std::map<int,wxString>::iterator cur;
	for (cur = keyName.begin();cur != keyName.end();cur++) {
		if (cur->second.Lower() == work) {
			keycode = cur->first;
			got = true;
			break;
		}
	}

	// Didn't find, check if it's a raw code
	if (!got) {
		if (work.Left(1) == _T("[") && work.Right(1) == _T("]")) {
			work = work.Mid(1,work.Length()-2);
			if (work.IsNumber()) {
				long temp;
				work.ToLong(&temp);
				keycode = temp;
			}
		}
	}
}



/// DOCME
std::map<int,wxString> HotkeyType::keyName;

/// @brief DOCME
/// @param keycode 
/// @return 
///
wxString HotkeyType::GetKeyName(int keycode) {
	// Fill map
	FillMap();

	// Blank key
	if (keycode == 0) return _T("");

	// Get key name
	std::map<int,wxString>::iterator cur = keyName.find(keycode);
	if (cur != keyName.end()) return cur->second;
	else return wxString::Format(_T("[%i]"),keycode);
}



/// @brief Fill map 
/// @return 
///
void HotkeyType::FillMap() {
	if (keyName.empty()) {
		keyName[WXK_BACK] = _T("Backspace");
		keyName[WXK_SPACE] = _T("Space");
		keyName[WXK_RETURN] = _T("Enter");
		keyName[WXK_TAB] = _T("Tab");
		keyName[WXK_PAUSE] = _T("Pause");

		keyName[WXK_LEFT] = _T("Left");
		keyName[WXK_RIGHT] = _T("Right");
		keyName[WXK_UP] = _T("Up");
		keyName[WXK_DOWN] = _T("Down");

		keyName[WXK_INSERT] = _T("Insert");
		keyName[WXK_DELETE] = _T("Delete");
		keyName[WXK_HOME] = _T("Home");
		keyName[WXK_END] = _T("End");
		keyName[WXK_PAGEUP] = _T("PgUp");
		keyName[WXK_PAGEDOWN] = _T("PgDn");
		
		keyName[WXK_NUMPAD0] = _T("KP_0");
		keyName[WXK_NUMPAD1] = _T("KP_1");
		keyName[WXK_NUMPAD2] = _T("KP_2");
		keyName[WXK_NUMPAD3] = _T("KP_3");
		keyName[WXK_NUMPAD4] = _T("KP_4");
		keyName[WXK_NUMPAD5] = _T("KP_5");
		keyName[WXK_NUMPAD6] = _T("KP_6");
		keyName[WXK_NUMPAD7] = _T("KP_7");
		keyName[WXK_NUMPAD8] = _T("KP_8");
		keyName[WXK_NUMPAD9] = _T("KP_9");
		keyName[WXK_NUMPAD_ADD] = _T("KP_Add");
		keyName[WXK_NUMPAD_SUBTRACT] = _T("KP_Subtract");
		keyName[WXK_NUMPAD_SUBTRACT] = _T("KP_Subtract");
		keyName[WXK_NUMPAD_MULTIPLY] = _T("KP_Multiply");
		keyName[WXK_NUMPAD_DIVIDE] = _T("KP_Divide");
		keyName[WXK_NUMPAD_DECIMAL] = _T("KP_Decimal");
		keyName[WXK_NUMPAD_ENTER] = _T("KP_Enter");

		keyName[WXK_F1] = _T("F1");
		keyName[WXK_F2] = _T("F2");
		keyName[WXK_F3] = _T("F3");
		keyName[WXK_F4] = _T("F4");
		keyName[WXK_F5] = _T("F5");
		keyName[WXK_F6] = _T("F6");
		keyName[WXK_F7] = _T("F7");
		keyName[WXK_F8] = _T("F8");
		keyName[WXK_F9] = _T("F9");
		keyName[WXK_F10] = _T("F10");
		keyName[WXK_F11] = _T("F11");
		keyName[WXK_F12] = _T("F12");

		for (char i='!';i<='`';i++) {
			keyName[i] = wchar_t(i);
		}
	}
}



/// DOCME
HotkeyManager Hotkeys;



/// @brief Constructor 
///
HotkeyManager::HotkeyManager() {
	modified = false;
}



/// @brief Destructor 
///
HotkeyManager::~HotkeyManager() {
	key.clear();
}



/// @brief Save 
/// @return 
///
void HotkeyManager::Save() {
	// Check if it's actually modified
	if (!modified) return;

	// Open file
	using namespace std;
	TextFileWriter file(filename,_T("UTF-8"));
	file.WriteLineToFile(_T("[Hotkeys]"));

	// Put variables in it
	for (map<wxString,HotkeyType>::iterator cur=key.begin();cur!=key.end();cur++) {
		file.WriteLineToFile((*cur).first + _T("=") + (*cur).second.GetText());
	}

	// Close
	modified = false;
}



/// @brief Load 
/// @return 
///
void HotkeyManager::Load() {
	// Load defaults
	LoadDefaults();

	// Check if file exists (create if it doesn't)
	wxFileName path(filename);
	if (!path.FileExists()) {
		modified = true;
		Save();
		return;
	}

	// Open file
	using namespace std;
	TextFileReader file(filename);
	wxString header;
	try {
		if (!file.IsBinary())
			header = file.ReadLineFromFile();
	}
	catch (wxString e) {
		header = _T("");
	}
	if (header != _T("[Hotkeys]")) {
		wxFileName backupfn(filename);
		backupfn.SetFullName(_T("hotkeys.bak"));
		wxCopyFile(filename, backupfn.GetFullPath());
		modified = true;
		Save();
		wxLogWarning(_T("Hotkeys file corrupted, defaults restored.\nA backup of the corrupted file was made."));
		return;
	}

	// Get variables
	wxString curLine;
	map<wxString,HotkeyType>::iterator cur;
	while (file.HasMoreLines()) {
		// Parse line
		try {
			curLine = file.ReadLineFromFile();
		}
		catch (wxString e) {
			wxFileName backupfn(filename);
			backupfn.SetFullName(_T("hotkeys.bak"));
			wxCopyFile(filename, backupfn.GetFullPath());
			modified = true;
			Save();
			wxLogWarning(_T("Hotkeys file corrupted, defaults restored.\nA backup of the corrupted file was made."));
			return;
		}
		if (curLine.IsEmpty()) continue;
		size_t pos = curLine.Find(_T("="));
		if (pos == wxString::npos) continue;
		wxString func = curLine.Left(pos);
		wxString value = curLine.Mid(pos+1);

		// Find it
		cur = key.find(func);
		if (cur != key.end()) {
			(*cur).second.Parse(value);
		}

		// Commented out so it discards anything that isn't listed:
		//else SetHotkey(func,value);
	}

	// Close
	Save();
}



/// @brief Set all hotkeys to the default values
///
void HotkeyManager::LoadDefaults() {
	modified = true;


	/// @note () is used here instead of _T(). This is done so the strings can be extracted.
	///       However, since this function is called before locale is set, it won't ever be translated.
	///       Keep this in mind: THESE CANNOT BE TRANSLATED HERE!
	///       As a safeguard, _() is undefined here
	#undef _

	/// DOCME
	#define _(a) _T(a)

	SetHotkey(_("New subtitles"),_T("Ctrl-N"));
	SetHotkey(_("Open subtitles"),_T("Ctrl-O"));
	SetHotkey(_("Save subtitles"),_T("Ctrl-S"));
#ifdef __APPLE__
	SetHotkey(_("Exit"),_T("Ctrl-Q"));
	SetHotkey(_("Help"),_T("Ctrl-?"));
	SetHotkey(_("Options"),_T("Ctrl-,"));
#else
	SetHotkey(_("Exit"),_T("Alt-F4"));
	SetHotkey(_("Help"),_T("F1"));
	SetHotkey(_("Options"),_T("Alt-O"));
#endif

	SetHotkey(_("Edit Box Commit"),_T("Ctrl-Enter"));
	SetHotkey(_("Undo"),_T("Ctrl-Z"));
#ifdef __APPLE__
	SetHotkey(_("Redo"),_T("Ctrl-Shift-Z"));
#else
	SetHotkey(_("Redo"),_T("Ctrl-Y"));
#endif
	SetHotkey(_("Shift Times"),_T("Ctrl-I"));
	SetHotkey(_("Find"),_T("Ctrl-F"));
#ifdef __APPLE__
	SetHotkey(_("Find Next"),_T("Ctrl-G"));
	SetHotkey(_("Replace"),_T("Ctrl-Shift-F")); // non-standard?
#else
	SetHotkey(_("Find Next"),_T("F3"));
	SetHotkey(_("Replace"),_T("Ctrl-H"));
#endif
	SetHotkey(_("Select Lines"),_T(""));
	SetHotkey(_("Copy"),_T("Ctrl-C"));
	SetHotkey(_("Cut"),_T("Ctrl-X"));
	SetHotkey(_("Paste"),_T("Ctrl-V"));
	SetHotkey(_("Paste Over"),_T("Ctrl-Shift-V"));

#ifdef __APPLE__
	SetHotkey(_("Video Jump"),_T("Ctrl-J"));
#else
	SetHotkey(_("Video Jump"),_T("Ctrl-G"));
#endif
	SetHotkey(_("Jump Video to Start"),_T("Ctrl-1"));
	SetHotkey(_("Jump Video to End"),_T("Ctrl-2"));
	SetHotkey(_("Set Start to Video"),_T("Ctrl-3"));
	SetHotkey(_("Set End to Video"),_T("Ctrl-4"));
	SetHotkey(_("Snap to Scene"),_T("Ctrl-5"));
	SetHotkey(_("Shift by Current Time"),_T("Ctrl-6"));
	SetHotkey(_("Zoom 50%"),_T(""));
	SetHotkey(_("Zoom 100%"),_T(""));
	SetHotkey(_("Zoom 200%"),_T(""));

	SetHotkey(_("Video global prev frame"),_T("Ctrl-KP_4"));
	SetHotkey(_("Video global next frame"),_T("Ctrl-KP_6"));
	SetHotkey(_("Video global focus seek"),_T("Ctrl-Space"));
	SetHotkey(_("Video global play"),_T("Ctrl-P"));
	SetHotkey(_("Grid global prev line"),_T("Ctrl-KP_8"));
	SetHotkey(_("Grid global next line"),_T("Ctrl-KP_2"));
	SetHotkey(_("Save Subtitles Alt"),_T("F2"));
	SetHotkey(_("Video global zoom in"),_T("Ctrl-KP_Add"));
	SetHotkey(_("Video global zoom out"),_T("Ctrl-KP_Subtract"));

	SetHotkey(_("Grid move row down"),_T("Alt-Down"));
	SetHotkey(_("Grid move row up"),_T("Alt-Up"));
#ifdef __APPLE__
	SetHotkey(_("Grid delete rows"),_T("Ctrl-Backspace"));
#else
	SetHotkey(_("Grid delete rows"),_T("Ctrl-Delete"));
#endif
	SetHotkey(_("Grid duplicate rows"),_T(""));
	SetHotkey(_("Grid duplicate and shift one frame"),_T("Ctrl-D"));

	SetHotkey(_("Audio Commit Alt"),_T("G"));
	SetHotkey(_("Audio Commit"),_T("Enter"));
	SetHotkey(_("Audio Commit (Stay)"),_T("F8"));
	SetHotkey(_("Audio Prev Line"),_T("Left"));
	SetHotkey(_("Audio Prev Line Alt"),_T("Z"));
	SetHotkey(_("Audio Next Line"),_T("Right"));
	SetHotkey(_("Audio Next Line Alt"),_T("X"));
	SetHotkey(_("Audio Play"),_T("Space"));
	SetHotkey(_("Audio Play Alt"),_T("S"));
	SetHotkey(_("Audio Play or Stop"),_T("B"));
	SetHotkey(_("Audio Stop"),_T("H"));
	SetHotkey(_("Audio Karaoke Increase Len"),_T("KP_Add"));
	SetHotkey(_("Audio Karaoke Decrease Len"),_T("KP_Subtract"));
	SetHotkey(_("Audio Karaoke Increase Len Shift"),_T("Shift-KP_Add"));
	SetHotkey(_("Audio Karaoke Decrease Len Shift"),_T("Shift-KP_Subtract"));
	SetHotkey(_("Audio Scroll Left"),_T("A"));
	SetHotkey(_("Audio Scroll Right"),_T("F"));
	SetHotkey(_("Audio Play First 500ms"),_T("E"));
	SetHotkey(_("Audio Play Last 500ms"),_T("D"));
	SetHotkey(_("Audio Play 500ms Before"),_T("Q"));
	SetHotkey(_("Audio Play 500ms After"),_T("W"));
	SetHotkey(_("Audio Play To End"),_T("T"));
	SetHotkey(_("Audio Play Original Line"),_T("R"));
	SetHotkey(_("Audio Add Lead In"),_T("C"));
	SetHotkey(_("Audio Add Lead Out"),_T("V"));

	SetHotkey(_("Audio Medusa Toggle"),_T("Ctrl-KP_Multiply"));
	SetHotkey(_("Audio Medusa Play"),_T("KP_5"));
	SetHotkey(_("Audio Medusa Stop"),_T("KP_8"));
	SetHotkey(_("Audio Medusa Shift Start Back"),_T("KP_4"));
	SetHotkey(_("Audio Medusa Shift Start Forward"),_T("KP_6"));
	SetHotkey(_("Audio Medusa Shift End Back"),_T("KP_7"));
	SetHotkey(_("Audio Medusa Shift End Forward"),_T("KP_9"));
	SetHotkey(_("Audio Medusa Play Before"),_T("KP_1"));
	SetHotkey(_("Audio Medusa Play After"),_T("KP_3"));
	SetHotkey(_("Audio Medusa Next"),_T("KP_2"));
	SetHotkey(_("Audio Medusa Previous"),_T("KP_0"));
	SetHotkey(_("Audio Medusa Enter"),_T("KP_Enter"));

	SetHotkey(_("Translation Assistant Play Audio"),_T("End"));
	SetHotkey(_("Translation Assistant Play Video"),_T("Home"));
	SetHotkey(_("Translation Assistant Next"),_T("PgDn"));
	SetHotkey(_("Translation Assistant Prev"),_T("PgUp"));
	SetHotkey(_("Translation Assistant Accept"),_T("Enter"));
	SetHotkey(_("Translation Assistant Preview"),_T("F8"));
	SetHotkey(_("Translation Assistant Insert Original"),_T("Insert"));

	SetHotkey(_("Styling Assistant Play Audio"),_T("End"));
	SetHotkey(_("Styling Assistant Play Video"),_T("Home"));
	SetHotkey(_("Styling Assistant Next"),_T("PgDn"));
	SetHotkey(_("Styling Assistant Prev"),_T("PgUp"));
	SetHotkey(_("Styling Assistant Accept"),_T("Enter"));
	SetHotkey(_("Styling Assistant Preview"),_T("F8"));

	SetHotkey(_("Visual Tool Default"), _T("A"));
	SetHotkey(_("Visual Tool Drag"), _T("S"));
	SetHotkey(_("Visual Tool Rotate Z"), _T("D"));
	SetHotkey(_("Visual Tool Rotate X/Y"), _T("F"));
	SetHotkey(_("Visual Tool Scale"), _T("G"));
	SetHotkey(_("Visual Tool Rectangular Clip"), _T("H"));
	SetHotkey(_("Visual Tool Vector Clip"), _T("J"));
}



/// @brief Set hotkey 
/// @param function 
/// @param hotkey   
///
void HotkeyManager::SetHotkey(wxString function,HotkeyType hotkey) {
	key[function.Lower()] = hotkey;
	modified = true;
}


/// @brief DOCME
/// @param function 
/// @param hotkey   
///
void HotkeyManager::SetHotkey(wxString function,wxString hotkey) {
	key[function.Lower()] = HotkeyType(hotkey,function);
	modified = true;
}



/// @brief Set file 
/// @param file 
///
void HotkeyManager::SetFile(wxString file) {
	filename = file;
}



/// @brief Get hotkey as text 
/// @param function 
/// @return 
///
const wxString HotkeyManager::GetText(wxString function) const {
	std::map<wxString,HotkeyType>::const_iterator cur = key.find(function.Lower());
	if (cur != key.end()) {
		return cur->second.GetText();
	}
	else throw _T("Hotkey not defined");
}



/// @brief Get hotkey as accelerator entry 
/// @param function 
/// @param id       
/// @return 
///
wxAcceleratorEntry HotkeyManager::GetAccelerator(wxString function,int id) const {
	std::map<wxString,HotkeyType>::const_iterator cur = key.find(function.Lower());
	if (cur != key.end()) {
		const HotkeyType *hotkey = &(*cur).second;
		wxAcceleratorEntry entry;
		entry.Set(hotkey->flags,hotkey->keycode,id);
		return entry;
	}
	else throw _T("Hotkey not defined");
}



/// @brief Set last key pressed 
/// @param keypress 
/// @param ctrl     
/// @param alt      
/// @param shift    
///
void HotkeyManager::SetPressed(int keypress,bool ctrl,bool alt,bool shift) {
	lastKey = keypress;
    lastMod = 0;
	if (ctrl) lastMod |= wxACCEL_CTRL;
	if (alt) lastMod |= wxACCEL_ALT;
	if (shift) lastMod |= wxACCEL_SHIFT;
}



/// @brief Is pressed? 
/// @param function 
/// @return 
///
bool HotkeyManager::IsPressed(wxString function) const {
	std::map<wxString,HotkeyType>::const_iterator cur = key.find(function.Lower());
	if (cur != key.end()) {
		const HotkeyType *hotkey = &(*cur).second;
		return (hotkey->keycode == lastKey && hotkey->flags == lastMod);
	}
	else throw _T("Hotkey not defined");
}



/// @brief Search for a hotkey 
/// @param keycode 
/// @param mod     
///
HotkeyType *HotkeyManager::Find(int keycode,int mod) {
	for (std::map<wxString,HotkeyType>::iterator cur = key.begin();cur != key.end();cur++) {
		if (cur->second.keycode == keycode && cur->second.flags == mod) {
			return &(cur->second);
		}
	}

	return NULL;
}
