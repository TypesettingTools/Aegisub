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
//
// $Id$

/// @file dialog_automation.cpp
/// @brief Manage loaded Automation scripts
/// @ingroup secondary_ui
///


#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#endif

#include "dialog_automation.h"

#include "auto4_base.h"
#include "compat.h"
#include "command/command.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "subtitle_format.h"
#include "utils.h"

using std::tr1::placeholders::_1;

DialogAutomation::DialogAutomation(agi::Context *c)
: wxDialog(c->parent, -1, _("Automation Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
, context(c)
, local_manager(c->local_scripts)
, global_manager(wxGetApp().global_scripts)
{
	SetIcon(BitmapToIcon(GETIMAGE(automation_toolbutton_24)));

	// create main controls
	list = new wxListView(this, -1, wxDefaultPosition, wxSize(600, 175), wxLC_REPORT|wxLC_SINGLE_SEL);
	add_button = new wxButton(this, -1, _("&Add"));
	remove_button = new wxButton(this, -1, _("&Remove"));
	reload_button = new wxButton(this, -1, _("Re&load"));
	info_button = new wxButton(this, -1, _("Show &Info"));
	reload_autoload_button = new wxButton(this, -1, _("Re&scan Autoload Dir"));
	close_button = new wxButton(this, wxID_CANCEL, _("&Close"));

	list->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &DialogAutomation::OnSelectionChange, this);
	list->Bind(wxEVT_COMMAND_LIST_ITEM_DESELECTED, &DialogAutomation::OnSelectionChange, this);
	add_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAutomation::OnAdd, this);
	remove_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAutomation::OnRemove, this);
	reload_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAutomation::OnReload, this);
	info_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAutomation::OnInfo, this);
	reload_autoload_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAutomation::OnReloadAutoload, this);

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
	UpdateDisplay();
}

template<class Container, class Pred>
static void for_each(Container const& c, Pred p)
{
	std::for_each(c.begin(), c.end(), p);
}

void DialogAutomation::RebuildList()
{
	script_info.clear();
	list->DeleteAllItems();

	for_each(local_manager->GetScripts(), bind(&DialogAutomation::AddScript, this, _1, false));
	for_each(global_manager->GetScripts(), bind(&DialogAutomation::AddScript, this, _1, true));
}

void DialogAutomation::SetScriptInfo(int i, Automation4::Script *script)
{
	list->SetItem(i, 1, script->GetName());
	list->SetItem(i, 2, wxFileName(script->GetFilename()).GetFullName());
	list->SetItem(i, 3, script->GetDescription());
	if (!script->GetLoadedState())
		list->SetItemBackgroundColour(i, wxColour(255,128,128));
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
	add_button->Enable(true);
	remove_button->Enable(local);
	reload_button->Enable(selected);
	info_button->Enable(true);
	reload_autoload_button->Enable(true);
	close_button->Enable(true);
}

template<class Container>
static bool has_file(Container const& c, wxFileName const& fn)
{
	return find_if(c.begin(), c.end(),
		bind(&wxFileName::SameAs, fn, bind(&Automation4::Script::GetFilename, _1), wxPATH_NATIVE)) != c.end();
}

void DialogAutomation::OnAdd(wxCommandEvent &)
{
	wxString fname = wxFileSelector(
		_("Add Automation script"),
		lagi_wxString(OPT_GET("Path/Last/Automation")->GetString()),
		"", "",
		Automation4::ScriptFactory::GetWildcardStr(),
		wxFD_OPEN | wxFD_FILE_MUST_EXIST,
		this);

	if (fname.empty()) return;

	wxFileName fnpath(fname);
	OPT_SET("Path/Last/Automation")->SetString(STD_STR(fnpath.GetPath()));

	if (has_file(local_manager->GetScripts(), fnpath) || has_file(global_manager->GetScripts(), fnpath)) {
		wxLogError("Script '%s' is already loaded", fname);
		return;
	}

	Automation4::Script *script = Automation4::ScriptFactory::CreateFromFile(fname, true);
	local_manager->Add(script);
	AddScript(script, false);
}

void DialogAutomation::OnRemove(wxCommandEvent &)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;
	const ExtraScriptInfo &ei = script_info[list->GetItemData(i)];
	if (ei.is_global) return;
	list->DeleteItem(i);
	local_manager->Remove(ei.script);
	// don't bother doing anything in script_info, it's relatively short-lived, and having any indexes change would break stuff
	list->Select(i);
}

void DialogAutomation::OnReload(wxCommandEvent &)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;

	Automation4::Script *script = script_info[list->GetItemData(i)].script;
	script->Reload();

	SetScriptInfo(i, script);

}

void DialogAutomation::OnInfo(wxCommandEvent &)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	ExtraScriptInfo *ei = i >= 0 ? &script_info[list->GetItemData(i)] : 0;

	wxString info = wxString::Format(
		_("Total scripts loaded: %d\nGlobal scripts loaded: %d\nLocal scripts loaded: %d\n\n"),
		local_manager->GetScripts().size() + global_manager->GetScripts().size(),
		global_manager->GetScripts().size(),
		local_manager->GetScripts().size());

	info += _("Scripting engines installed:\n");
	const std::vector<Automation4::ScriptFactory*> &factories = Automation4::ScriptFactory::GetFactories();
	for (std::vector<Automation4::ScriptFactory*>::const_iterator c = factories.begin(); c != factories.end(); ++c)
		info += wxString::Format("- %s (%s)\n", (*c)->GetEngineName(), (*c)->GetFilenamePattern());

	if (ei) {
		info += wxString::Format(_("\nScript info:\nName: %s\nDescription: %s\nAuthor: %s\nVersion: %s\nFull path: %s\nState: %s\n\nFeatures provided by script:\n"),
			ei->script->GetName(),
			ei->script->GetDescription(),
			ei->script->GetAuthor(),
			ei->script->GetVersion(),
			ei->script->GetFilename(),
			ei->script->GetLoadedState() ? _("Correctly loaded") : _("Failed to load"));

		std::vector<cmd::Command*> macros = ei->script->GetMacros();
		for (std::vector<cmd::Command*>::const_iterator f = macros.begin(); f != macros.end(); ++f)
			info += wxString::Format(_("    Macro: %s (%s)\n"),  (*f)->StrDisplay(context), (*f)->name());

		std::vector<Automation4::ExportFilter*> filters = ei->script->GetFilters();
		for (std::vector<Automation4::ExportFilter*>::const_iterator f = filters.begin(); f != filters.end(); ++f)
			info += _("    Export filter: ") + (*f)->GetName() + "\n";

		std::vector<SubtitleFormat*> formats = ei->script->GetFormats();
		for (std::vector<SubtitleFormat*>::const_iterator f = formats.begin(); f != formats.end(); ++f)
			info += _("    Subtitle format handler: ") + (*f)->GetName() + "\n";
	}

	wxMessageBox(info, _("Automation Script Info"));
}

void DialogAutomation::OnReloadAutoload(wxCommandEvent &)
{
	global_manager->Reload();
	RebuildList();
	UpdateDisplay();
}

void DialogAutomation::OnSelectionChange(wxListEvent &)
{
	UpdateDisplay();
}
