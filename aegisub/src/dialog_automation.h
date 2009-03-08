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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#pragma once

#ifndef DIALOG_AUTOMATION_H
#define DIALOG_AUTOMATION_H

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <vector>

namespace Automation4 { class ScriptManager; class Script; };

class DialogAutomation : public wxDialog {
private:
	struct ExtraScriptInfo {
		Automation4::Script *script;
		bool is_global;
	};
	std::vector<ExtraScriptInfo> script_info;

	Automation4::ScriptManager *local_manager;
	Automation4::AutoloadScriptManager *global_manager;

	wxListView *list;
	wxButton *add_button;
	wxButton *remove_button;
	wxButton *reload_button;
	wxButton *info_button;
	wxButton *reload_autoload_button;
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
	Automation_List_Box = 1000,
	Automation_Add_Script,
	Automation_Remove_Script,
	Automation_Reload_Script,
	Automation_Show_Info,
	Automation_Reload_Autoload
};

#endif
