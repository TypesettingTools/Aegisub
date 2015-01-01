// Copyright (c) 2006, Niels Martin Hansen
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

#include "auto4_base.h"
#include "compat.h"
#include "command/command.h"
#include "dialog_manager.h"
#include "format.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"

#include <libaegisub/signal.h>

#include <algorithm>
#include <boost/range/algorithm/transform.hpp>
#include <vector>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filedlg.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

namespace {
/// Struct to attach a flag for global/local to scripts
struct ExtraScriptInfo {
	Automation4::Script *script;
	bool is_global;
};

class DialogAutomation final : public wxDialog {
	agi::Context *context;

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

DialogAutomation::DialogAutomation(agi::Context *c)
: wxDialog(c->parent, -1, _("Automation Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
, context(c)
, local_manager(c->local_scripts.get())
, local_scripts_changed(local_manager->AddScriptChangeListener(&DialogAutomation::RebuildList, this))
, global_manager(config::global_scripts)
, global_scripts_changed(global_manager->AddScriptChangeListener(&DialogAutomation::RebuildList, this))
{
	SetIcon(GETICON(automation_toolbutton_16));

	// create main controls
	list = new wxListView(this, -1, wxDefaultPosition, wxSize(600, 175), wxLC_REPORT|wxLC_SINGLE_SEL);
	wxButton *add_button = new wxButton(this, -1, _("&Add"));
	remove_button = new wxButton(this, -1, _("&Remove"));
	reload_button = new wxButton(this, -1, _("Re&load"));
	wxButton *info_button = new wxButton(this, -1, _("Show &Info"));
	wxButton *reload_autoload_button = new wxButton(this, -1, _("Re&scan Autoload Dir"));
	wxButton *close_button = new wxButton(this, wxID_CANCEL, _("&Close"));

	list->Bind(wxEVT_LIST_ITEM_SELECTED, std::bind(&DialogAutomation::UpdateDisplay, this));
	list->Bind(wxEVT_LIST_ITEM_DESELECTED, std::bind(&DialogAutomation::UpdateDisplay, this));
	add_button->Bind(wxEVT_BUTTON, &DialogAutomation::OnAdd, this);
	remove_button->Bind(wxEVT_BUTTON, &DialogAutomation::OnRemove, this);
	reload_button->Bind(wxEVT_BUTTON, &DialogAutomation::OnReload, this);
	info_button->Bind(wxEVT_BUTTON, &DialogAutomation::OnInfo, this);
	reload_autoload_button->Bind(wxEVT_BUTTON, &DialogAutomation::OnReloadAutoload, this);

	// add headers to list view
	list->InsertColumn(0, "", wxLIST_FORMAT_CENTER, 20);
	list->InsertColumn(1, _("Name"), wxLIST_FORMAT_LEFT, 140);
	list->InsertColumn(2, _("Filename"), wxLIST_FORMAT_LEFT, 90);
	list->InsertColumn(3, _("Description"), wxLIST_FORMAT_LEFT, 330);

	// button layout
	wxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
	button_box->AddStretchSpacer(2);
	button_box->Add(add_button, 0);
	button_box->Add(remove_button, 0);
	button_box->AddSpacer(10);
	button_box->Add(reload_button, 0);
	button_box->Add(info_button, 0);
	button_box->AddSpacer(10);
	button_box->Add(reload_autoload_button, 0);
	button_box->AddSpacer(10);
	button_box->Add(new HelpButton(this,"Automation Manager"), 0);
	button_box->Add(close_button, 0);
	button_box->AddStretchSpacer(2);

	// main layout
	wxSizer *main_box = new wxBoxSizer(wxVERTICAL);
	main_box->Add(list, wxSizerFlags(1).Expand().Border());
	main_box->Add(button_box, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	SetSizerAndFit(main_box);
	Center();

	// why doesn't this work... the button gets the "default" decoration but doesn't answer to Enter
	// ("esc" does work)
	SetDefaultItem(close_button);
	SetAffirmativeId(wxID_CANCEL);
	close_button->SetDefault();

	RebuildList();
}

void DialogAutomation::RebuildList()
{
	script_info.clear();
	list->DeleteAllItems();

	for (auto& script : local_manager->GetScripts()) AddScript(script.get(), false);
	for (auto& script : global_manager->GetScripts()) AddScript(script.get(), true);

	UpdateDisplay();
}

void DialogAutomation::SetScriptInfo(int i, Automation4::Script *script)
{
	list->SetItem(i, 1, to_wx(script->GetName()));
	list->SetItem(i, 2, script->GetPrettyFilename().wstring());
	list->SetItem(i, 3, to_wx(script->GetDescription()));
	if (!script->GetLoadedState())
		list->SetItemBackgroundColour(i, wxColour(255,128,128));
	else
		list->SetItemBackgroundColour(i, list->GetBackgroundColour());
}

void DialogAutomation::AddScript(Automation4::Script *script, bool is_global)
{
	ExtraScriptInfo ei = { script, is_global };
	script_info.push_back(ei);

	wxListItem itm;
	itm.SetText(is_global ? "G" : "L");
	itm.SetData((int)script_info.size()-1);
	itm.SetId(list->GetItemCount());
	SetScriptInfo(list->InsertItem(itm), script);
}

void DialogAutomation::UpdateDisplay()
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	bool selected = i >= 0;
	bool local = selected && !script_info[list->GetItemData(i)].is_global;
	remove_button->Enable(local);
	reload_button->Enable(selected);
}

template<class Container>
static bool has_file(Container const& c, agi::fs::path const& fn)
{
	return any_of(c.begin(), c.end(),
		[&](std::unique_ptr<Automation4::Script> const& s) { return fn == s->GetFilename(); });
}

void DialogAutomation::OnAdd(wxCommandEvent &)
{
	wxFileDialog diag(this,
		_("Add Automation script"),
		to_wx(OPT_GET("Path/Last/Automation")->GetString()),
		"",
		to_wx(Automation4::ScriptFactory::GetWildcardStr()),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

	if (diag.ShowModal() == wxID_CANCEL) return;

	wxArrayString fnames;
	diag.GetPaths(fnames);

	for (auto const& fname : fnames) {
		agi::fs::path fnpath(fname.wx_str());
		OPT_SET("Path/Last/Automation")->SetString(fnpath.parent_path().string());

		if (has_file(local_manager->GetScripts(), fnpath) || has_file(global_manager->GetScripts(), fnpath)) {
			wxLogError("Script '%s' is already loaded", fname);
			continue;
		}

		local_manager->Add(Automation4::ScriptFactory::CreateFromFile(fnpath, true));
	}
}

void DialogAutomation::OnRemove(wxCommandEvent &)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;
	const ExtraScriptInfo &ei = script_info[list->GetItemData(i)];
	if (ei.is_global) return;

	local_manager->Remove(ei.script);
}

void DialogAutomation::OnReload(wxCommandEvent &)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;

	ExtraScriptInfo const& ei = script_info[list->GetItemData(i)];
	if (ei.is_global)
		global_manager->Reload(ei.script);
	else
		local_manager->Reload(ei.script);
}

void DialogAutomation::OnInfo(wxCommandEvent &)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	ExtraScriptInfo *ei = i >= 0 ? &script_info[list->GetItemData(i)] : nullptr;

	wxArrayString info;
	std::back_insert_iterator<wxArrayString> append_info(info);

	info.push_back(fmt_tl(
		"Total scripts loaded: %d\nGlobal scripts loaded: %d\nLocal scripts loaded: %d\n",
		local_manager->GetScripts().size() + global_manager->GetScripts().size(),
		global_manager->GetScripts().size(),
		local_manager->GetScripts().size()));

	info.push_back(_("Scripting engines installed:"));
	boost::transform(Automation4::ScriptFactory::GetFactories(), append_info,
		[](std::unique_ptr<Automation4::ScriptFactory> const& f) {
			return fmt_wx("- %s (%s)", f->GetEngineName(), f->GetFilenamePattern());
		});

	if (ei) {
		info.push_back(fmt_tl("\nScript info:\nName: %s\nDescription: %s\nAuthor: %s\nVersion: %s\nFull path: %s\nState: %s\n\nFeatures provided by script:",
			ei->script->GetName(),
			ei->script->GetDescription(),
			ei->script->GetAuthor(),
			ei->script->GetVersion(),
			ei->script->GetFilename().wstring(),
			ei->script->GetLoadedState() ? _("Correctly loaded") : _("Failed to load")));

		boost::transform(ei->script->GetMacros(), append_info, [=](const cmd::Command *f) {
			return fmt_tl("    Macro: %s (%s)", f->StrDisplay(context), f->name());
		});
		boost::transform(ei->script->GetFilters(), append_info, [](const Automation4::ExportFilter* f) {
			return fmt_tl("    Export filter: %s", f->GetName());
		});
	}

	wxMessageBox(wxJoin(info, '\n', 0), _("Automation Script Info"));
}

void DialogAutomation::OnReloadAutoload(wxCommandEvent &)
{
	global_manager->Reload();
}
}

void ShowAutomationDialog(agi::Context *c) {
	c->dialog->Show<DialogAutomation>(c);
}
