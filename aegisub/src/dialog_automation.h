// Copyright (c) 2005, Niels Martin Hansen
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

/// @file dialog_automation.h
/// @see dialog_automation.cpp
/// @ingroup secondary_ui
///


#ifndef DIALOG_AUTOMATION_H

/// DOCME
#define DIALOG_AUTOMATION_H

#ifndef AGI_PRE
#include <vector>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#endif


/// DOCME
namespace Automation4 { class ScriptManager; class Script; };


/// DOCME
/// @class DialogAutomation
/// @brief DOCME
///
/// DOCME
class DialogAutomation : public wxDialog {
private:

	/// DOCME
	struct ExtraScriptInfo {

		/// DOCME
		Automation4::Script *script;

		/// DOCME
		bool is_global;
	};

	/// DOCME
	std::vector<ExtraScriptInfo> script_info;


	/// DOCME
	Automation4::ScriptManager *local_manager;

	/// DOCME
	Automation4::AutoloadScriptManager *global_manager;


	/// DOCME
	wxListView *list;

	/// DOCME
	wxButton *add_button;

	/// DOCME
	wxButton *remove_button;

	/// DOCME
	wxButton *reload_button;

	/// DOCME
	wxButton *info_button;

	/// DOCME
	wxButton *reload_autoload_button;

	/// DOCME
	wxButton *close_button;

	void RebuildList();
	void AddScript(ExtraScriptInfo &ei);
	void UpdateDisplay();

	void OnAdd(wxCommandEvent &evt);
	void OnRemove(wxCommandEvent &evt);
	void OnReload(wxCommandEvent &evt);
	void OnInfo(wxCommandEvent &evt);
	void OnReloadAutoload(wxCommandEvent &evt);
	void OnSelectionChange(wxListEvent &evt);

public:
	DialogAutomation(wxWindow *parent, Automation4::ScriptManager *_local_manager);

	DECLARE_EVENT_TABLE()
};

enum {

	/// DOCME
	Automation_List_Box = 1000,

	/// DOCME
	Automation_Add_Script,

	/// DOCME
	Automation_Remove_Script,

	/// DOCME
	Automation_Reload_Script,

	/// DOCME
	Automation_Show_Info,

	/// DOCME
	Automation_Reload_Autoload
};

#endif


