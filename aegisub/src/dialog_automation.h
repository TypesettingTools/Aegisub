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

/// @file dialog_automation.h
/// @see dialog_automation.cpp
/// @ingroup secondary_ui
///

#include <vector>

#include <wx/dialog.h>

#include <libaegisub/signal.h>

namespace Automation4 {
	class ScriptManager;
	class AutoloadScriptManager;
	class Script;
}

namespace agi { struct Context; }

class wxButton;
class wxListEvent;
class wxListView;

class DialogAutomation : public wxDialog {
	agi::Context *context;

	/// Struct to attach a flag for global/local to scripts
	struct ExtraScriptInfo {
		Automation4::Script *script;
		bool is_global;
	};

	/// Currently loaded scripts
	std::vector<ExtraScriptInfo> script_info;

	/// File-local script manager
	Automation4::ScriptManager *local_manager;

	/// Listener for external changes to the local scripts
	agi::signal::Connection local_scripts_changed;

	/// Global script manager
	Automation4::ScriptManager *global_manager;

	/// Listener for external changes to the global scripts
	agi::signal::Connection global_scripts_changed;


	/// List of loaded scripts
	wxListView *list;

	/// Unload a local script
	wxButton *remove_button;

	/// Reload a script
	wxButton *reload_button;

	void RebuildList();
	void AddScript(Automation4::Script *script, bool is_global);
	void SetScriptInfo(int i, Automation4::Script *script);
	void UpdateDisplay();

	void OnAdd(wxCommandEvent &);
	void OnRemove(wxCommandEvent &);
	void OnReload(wxCommandEvent &);

	void OnInfo(wxCommandEvent &);
	void OnReloadAutoload(wxCommandEvent &);

public:
	DialogAutomation(agi::Context *context);
};
