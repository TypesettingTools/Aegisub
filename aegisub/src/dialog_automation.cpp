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

#ifdef WITH_AUTOMATION

#ifndef AGI_PRE
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#endif

#include "auto4_base.h"
#include "dialog_automation.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "options.h"
#include "utils.h"


/// @brief DOCME
/// @param parent         
/// @param _local_manager 
///
DialogAutomation::DialogAutomation(wxWindow *parent, Automation4::ScriptManager *_local_manager)
: wxDialog(parent, -1, _("Automation Manager"), wxDefaultPosition, wxDefaultSize)
{
	// Set icon
	SetIcon(BitmapToIcon(GETIMAGE(automation_toolbutton_24)));

	local_manager = _local_manager;
	global_manager = wxGetApp().global_scripts;

	// create main controls
	list = new wxListView(this, Automation_List_Box, wxDefaultPosition, wxSize(600, 175), wxLC_REPORT|wxLC_SINGLE_SEL);
	add_button = new wxButton(this, Automation_Add_Script, _("&Add"));
	remove_button = new wxButton(this, Automation_Remove_Script, _("&Remove"));
	reload_button = new wxButton(this, Automation_Reload_Script, _("Re&load"));
	info_button = new wxButton(this, Automation_Show_Info, _("Show &Info"));
	reload_autoload_button = new wxButton(this, Automation_Reload_Autoload, _("Re&scan Autoload Dir"));
	close_button = new wxButton(this, wxID_CANCEL, _("&Close"));

	// add headers to list view
	list->InsertColumn(0, _T(""), wxLIST_FORMAT_CENTER, 20);
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
	button_box->Add(new HelpButton(this,_T("Automation Manager")), 0);
	button_box->Add(close_button, 0);
	button_box->AddStretchSpacer(2);

	// main layout
	wxSizer *main_box = new wxBoxSizer(wxVERTICAL);
	main_box->Add(list, 1, wxEXPAND|wxALL, 5);
	main_box->Add(button_box, 0, wxEXPAND|(wxALL&~wxTOP), 5);
	main_box->SetSizeHints(this);
	SetSizer(main_box);
	Center();

	// why doesn't this work... the button gets the "default" decoration but doesn't answer to Enter
	// ("esc" does work)
	SetDefaultItem(close_button);
	SetAffirmativeId(wxID_CANCEL);
	close_button->SetDefault();

	RebuildList();
	UpdateDisplay();
}



/// @brief DOCME
///
void DialogAutomation::RebuildList()
{
	script_info.clear();
	list->DeleteAllItems();

	// fill the list view
	const std::vector<Automation4::Script*> &local_scripts = local_manager->GetScripts();
	for (std::vector<Automation4::Script*>::const_iterator i = local_scripts.begin(); i != local_scripts.end(); ++i) {
		ExtraScriptInfo ei;
		ei.script = *i;
		ei.is_global = false;
		AddScript(ei);
	}
	const std::vector<Automation4::Script*> &global_scripts = global_manager->GetScripts();
	for (std::vector<Automation4::Script*>::const_iterator i = global_scripts.begin(); i != global_scripts.end(); ++i) {
		ExtraScriptInfo ei;
		ei.script = *i;
		ei.is_global = true;
		AddScript(ei);
	}

}



/// @brief DOCME
/// @param ei 
///
void DialogAutomation::AddScript(ExtraScriptInfo &ei)
{
	script_info.push_back(ei);

	wxListItem itm;
	if (ei.is_global) {
		itm.SetText(_T("G"));
	} else {
		itm.SetText(_T("L"));
	}
	itm.SetData((int)script_info.size()-1);
	itm.SetId(list->GetItemCount());
	int i = list->InsertItem(itm);
	list->SetItem(i, 1, ei.script->GetName());
	list->SetItem(i, 2, wxFileName(ei.script->GetFilename()).GetFullName());
	list->SetItem(i, 3, ei.script->GetDescription());
	if (ei.script->GetLoadedState() == false) {
		list->SetItemBackgroundColour(i, wxColour(255,128,128));
	}
}



/// @brief DOCME
///
void DialogAutomation::UpdateDisplay()
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	bool selected = i >= 0;
	bool global = true;
	if (selected) {
		const ExtraScriptInfo &ei = script_info[list->GetItemData(i)];
		global = ei.is_global;
	}
	add_button->Enable(true);
	remove_button->Enable(selected && !global);
	reload_button->Enable(selected);
	info_button->Enable(true);
	reload_autoload_button->Enable(true);
	close_button->Enable(true);
}


BEGIN_EVENT_TABLE(DialogAutomation, wxDialog)
	EVT_BUTTON(Automation_Add_Script,DialogAutomation::OnAdd)
	EVT_BUTTON(Automation_Remove_Script,DialogAutomation::OnRemove)
	EVT_BUTTON(Automation_Reload_Script,DialogAutomation::OnReload)
	EVT_BUTTON(Automation_Show_Info,DialogAutomation::OnInfo)
	EVT_BUTTON(Automation_Reload_Autoload,DialogAutomation::OnReloadAutoload)
	EVT_LIST_ITEM_SELECTED(Automation_List_Box,DialogAutomation::OnSelectionChange)
	EVT_LIST_ITEM_DESELECTED(Automation_List_Box,DialogAutomation::OnSelectionChange)
END_EVENT_TABLE()



/// @brief DOCME
/// @param evt 
///
void DialogAutomation::OnAdd(wxCommandEvent &evt)
{
	// build filename filter list
	wxString fnfilter, catchall;
	const std::vector<Automation4::ScriptFactory*> &factories = Automation4::ScriptFactory::GetFactories();
	for (int i = 0; i < (int)factories.size(); i++) {
		const Automation4::ScriptFactory *fact = factories[i];
		if (fact->GetEngineName().IsEmpty() || fact->GetFilenamePattern().IsEmpty())
			continue;
		fnfilter = wxString::Format(_T("%s%s scripts (%s)|%s|"), fnfilter.c_str(), fact->GetEngineName().c_str(), fact->GetFilenamePattern().c_str(), fact->GetFilenamePattern().c_str());
		catchall << fact->GetFilenamePattern() << _T(";");
	}
#ifdef __WINDOWS__
	fnfilter += _T("All files|*.*");
#else
	fnfilter += _T("All files|*");
#endif
	if (!catchall.IsEmpty()) {
		catchall.RemoveLast();
	}
	if (factories.size() > 1) {
		fnfilter = _T("All supported scripts|") + catchall + _T("|") + fnfilter;
	}

	wxString fname = wxFileSelector(_("Add Automation script"), Options.AsText(_T("Last open automation path")), wxEmptyString, wxEmptyString, fnfilter, wxFD_OPEN|wxFD_FILE_MUST_EXIST, this);

	if (!fname.IsEmpty()) {

		wxFileName fnpath(fname);
		Options.SetText(_T("Last open automation path"), fnpath.GetPath());

		// TODO: make sure each script is only loaded once. check in both local and global managers!!
		// it doesn't break for macros, but will for export filters, and maybe for file formats,
		// and makes for confusion in the UI anyhow

		try {
			ExtraScriptInfo ei;
			ei.script = Automation4::ScriptFactory::CreateFromFile(fname, false);
			local_manager->Add(ei.script);
			ei.is_global = false;
			AddScript(ei);
		}
		catch (const wchar_t *e) {
			wxLogError(e);
		}
		catch (...) {
			wxLogError(_T("Unknown error loading script"));
		}
	}
}


/// @brief DOCME
/// @param evt 
/// @return 
///
void DialogAutomation::OnRemove(wxCommandEvent &evt)
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


/// @brief DOCME
/// @param evt 
/// @return 
///
void DialogAutomation::OnReload(wxCommandEvent &evt)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (i < 0) return;
	const ExtraScriptInfo &ei = script_info[list->GetItemData(i)];

	try {
		ei.script->Reload();
	}
	catch (const wchar_t *e) {
		wxMessageBox(e, _T("Error reloading Automation script"), wxOK|wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("An unknown error occurred reloading Automation script."), _T("Error reloading Automation script"), wxOK|wxICON_ERROR, this);
	}

	list->SetItem(i, 1, ei.script->GetName());
	list->SetItem(i, 2, wxFileName(ei.script->GetFilename()).GetFullName());
	list->SetItem(i, 3, ei.script->GetDescription());
	if (ei.script->GetLoadedState() == false) {
		list->SetItemBackgroundColour(i, wxColour(255,128,128));
	}
}


/// @brief DOCME
/// @param evt 
///
void DialogAutomation::OnInfo(wxCommandEvent &evt)
{
	int i = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	ExtraScriptInfo *ei = 0;
	if (i >= 0)
		ei = &script_info[list->GetItemData(i)];

	wxString info = wxString::Format(
		_("Total scripts loaded: %d\nGlobal scripts loaded: %d\nLocal scripts loaded: %d\n\n"),
		local_manager->GetScripts().size() + global_manager->GetScripts().size(),
		global_manager->GetScripts().size(),
		local_manager->GetScripts().size());

	info += _("Scripting engines installed:\n");
	const std::vector<Automation4::ScriptFactory*> &factories = Automation4::ScriptFactory::GetFactories();
	for (std::vector<Automation4::ScriptFactory*>::const_iterator i = factories.begin(); i != factories.end(); ++i) {
		info += wxString::Format(_T("- %s (%s)\n"), (*i)->GetEngineName().c_str(), (*i)->GetFilenamePattern().c_str());
	}

	if (ei) {
		info += wxString::Format(_("\nScript info:\nName: %s\nDescription: %s\nAuthor: %s\nVersion: %s\nFull path: %s\nState: %s\n\nFeatures provided by script:\n"),
			ei->script->GetName().c_str(),
			ei->script->GetDescription().c_str(),
			ei->script->GetAuthor().c_str(),
			ei->script->GetVersion().c_str(),
			ei->script->GetFilename().c_str(),
			ei->script->GetLoadedState() ? _("Correctly loaded") : _("Failed to load"));
		for (std::vector<Automation4::Feature*>::iterator f = ei->script->GetFeatures().begin(); f != ei->script->GetFeatures().end(); ++f) {
			switch ((*f)->GetClass()) {
				case Automation4::SCRIPTFEATURE_MACRO:
					info += _("    Macro: "); break;
				case Automation4::SCRIPTFEATURE_FILTER:
					info += _("    Export filter: "); break;
				case Automation4::SCRIPTFEATURE_SUBFORMAT:
					info += _("    Subtitle format handler: "); break;
				default:
					info += _T("    Unknown class: "); break;
			}
			info += (*f)->GetName() + _T("\n");
		}
	}

	wxMessageBox(info, _("Automation Script Info"));
}


/// @brief DOCME
/// @param evt 
///
void DialogAutomation::OnReloadAutoload(wxCommandEvent &evt)
{
	global_manager->Reload();
	RebuildList();
	UpdateDisplay();
}


/// @brief DOCME
/// @param evt 
///
void DialogAutomation::OnSelectionChange(wxListEvent &evt)
{
	UpdateDisplay();
}

#endif // WITH_AUTOMATION


