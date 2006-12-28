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

#include <wx/window.h>
#include <wx/listctrl.h>
#include "subs_grid.h"
#include "automation_filter.h"


class DialogAutomationManager : public wxDialog {
private:
	wxListView *script_list;
	wxButton *create_button;
	wxButton *add_button;
	wxButton *remove_button;
	wxButton *test_button;
	wxButton *edit_button;
	wxButton *reload_button;
	wxButton *close_button;

	SubtitlesGrid *subgrid;

	void OnCreate(wxCommandEvent &event);
	void OnAdd(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);
	void OnApply(wxCommandEvent &event);
	void OnEdit(wxCommandEvent &event);
	void OnReload(wxCommandEvent &event);
	void OnClose(wxCommandEvent &event);
	void OnSelectionChange(wxListEvent &event);

	void UpdateDisplay();
	void AddScriptToList(AssAutomationFilter *filter);

public:
	DialogAutomationManager(wxWindow *parent, SubtitlesGrid *grid);
	~DialogAutomationManager();

	static void EditScript(AutomationScript *script);

	DECLARE_EVENT_TABLE()
};

enum {
	AUTOMAN_SCRIPTLIST = 3200,
	AUTOMAN_CREATE,
	AUTOMAN_ADD,
	AUTOMAN_REMOVE,
	AUTOMAN_APPLY,
	AUTOMAN_EDIT,
	AUTOMAN_RELOAD,
	AUTOMAN_CLOSE
};
