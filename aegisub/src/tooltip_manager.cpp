// Copyright (c) 2007, Rodrigo Braz Monteiro, Niels Martin Hansen
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

/// @file tooltip_manager.cpp
/// @brief Generate tooltips for controls by combining a base text and any hotkeys found for the function
/// @ingroup custom_control
///


///////////
// Headers
#include "config.h"

#include "tooltip_manager.h"
#include "hotkeys.h"


///////////////////
// Update all tips
void ToolTipManager::DoUpdate() {
	for (std::list<ToolTipBinding>::iterator cur=tips.begin();cur!=tips.end();cur++) {
		(*cur).Update();
	}
}


/////////////
// Add a tip
void ToolTipManager::AddTips(wxWindow *window,wxString tooltip,wxArrayString hotkeys) {
	ToolTipBinding tip;
	tip.hotkeys = hotkeys;
	tip.window = window;
	tip.toolTip = tooltip;
	tip.Update();
	tips.push_back(tip);
}


//////////////////////////
// Single hotkey overload
void ToolTipManager::Bind(wxWindow *window,wxString tooltip,wxString hotkey) {
	wxArrayString hotkeys;
	if (!hotkey.IsEmpty()) hotkeys.Add(hotkey);
	Bind(window,tooltip,hotkeys);
}


////////////////////////
// Two hotkeys overload
void ToolTipManager::Bind(wxWindow *window,wxString tooltip,wxString hotkey1,wxString hotkey2) {
	wxArrayString hotkeys;
	hotkeys.Add(hotkey1);
	hotkeys.Add(hotkey2);
	Bind(window,tooltip,hotkeys);
}


///////////////////
// Static instance
ToolTipManager &ToolTipManager::GetInstance() {
	static ToolTipManager instance;
	return instance;
}


////////////////
// Update a tip
void ToolTipBinding::Update() {
	wxString finalTip = toolTip;
	wxArrayString hotkeysLeft = hotkeys;
	while (hotkeysLeft.Count()) {
		finalTip.Replace(_T("%KEY%"),Hotkeys.GetText(hotkeysLeft[0]),false);
		hotkeysLeft.RemoveAt(0);
	}
	window->SetToolTip(finalTip);
}

