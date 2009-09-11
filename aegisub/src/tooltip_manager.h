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

/// @file tooltip_manager.h
/// @see tooltip_manager.cpp
/// @ingroup custom_control
///




///////////
// Headers
#ifndef AGI_PRE
#include <list>

#include <wx/arrstr.h>
#include <wx/string.h>
#include <wx/window.h>
#endif


/// DOCME
/// @class ToolTipBinding
/// @brief DOCME
///
/// DOCME
class ToolTipBinding {
	friend class ToolTipManager;
private:

	/// DOCME
	wxWindow *window;

	/// DOCME
	wxString toolTip;

	/// DOCME
	wxArrayString hotkeys;

	void Update();
};



/// DOCME
/// @class ToolTipManager
/// @brief DOCME
///
/// DOCME
class ToolTipManager {
private:

	/// @brief DOCME
	///
	ToolTipManager() {};
	ToolTipManager(ToolTipManager const&);
	ToolTipManager& operator=(ToolTipManager const&);

	static ToolTipManager &GetInstance();


	/// DOCME
	std::list<ToolTipBinding> tips;

	void DoUpdate();
	void AddTips(wxWindow *window,wxString tooltip,wxArrayString hotkeys);

public:

	/// @brief DOCME
	///
	static void Update() { GetInstance().DoUpdate(); }

	/// @brief DOCME
	/// @param window  
	/// @param tooltip 
	/// @param hotkeys 
	///
	static void Bind(wxWindow *window,wxString tooltip,wxArrayString hotkeys) { GetInstance().AddTips(window,tooltip,hotkeys); }
	static void Bind(wxWindow *window,wxString tooltip,wxString hotkey=_T(""));
	static void Bind(wxWindow *window,wxString tooltip,wxString hotkey1,wxString hotkey2);
};


