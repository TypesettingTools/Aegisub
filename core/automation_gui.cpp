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

#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textfile.h>
#include <wx/utils.h>
#include "automation_gui.h"
#include "options.h"


DialogAutomationManager::DialogAutomationManager(wxWindow *parent, SubtitlesGrid *grid)
: wxDialog(parent, -1, _("Automation Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("AutoMan"))
{
	subgrid = grid;

	// create main controls
	script_list = new wxListView(this, AUTOMAN_SCRIPTLIST, wxDefaultPosition, wxSize(600, 175), wxLC_REPORT|wxLC_SINGLE_SEL);
	create_button = new wxButton(this, AUTOMAN_CREATE, _("&Create..."));
	add_button = new wxButton(this, AUTOMAN_ADD, _("&Add..."));
	remove_button = new wxButton(this, AUTOMAN_REMOVE, _("&Remove"));
	test_button = new wxButton(this, AUTOMAN_APPLY, _("&Apply now"));
	edit_button = new wxButton(this, AUTOMAN_EDIT, _("&Edit..."));
	reload_button = new wxButton(this, AUTOMAN_RELOAD, _("Rel&oad"));
	close_button = new wxButton(this, wxID_CLOSE, _("C&lose"));

	// add headers to list view
	script_list->InsertColumn(0, _("Script name"), wxLIST_FORMAT_LEFT, 140);
	script_list->InsertColumn(1, _("File"), wxLIST_FORMAT_LEFT, 100);
	//script_list->InsertColumn(2, _("Flags"), wxLIST_FORMAT_LEFT, 45);
	script_list->InsertColumn(2, _("Description"), wxLIST_FORMAT_LEFT, 350);

	// button layout
	wxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
	button_box->AddStretchSpacer(2);
	button_box->Add(create_button, 0);
	button_box->Add(add_button, 0);
	button_box->Add(remove_button, 0);
	button_box->AddSpacer(10);
	button_box->Add(test_button, 0);
	button_box->AddSpacer(10);
	button_box->Add(edit_button, 0);
	button_box->Add(reload_button, 0);
	button_box->AddSpacer(10);
	button_box->Add(close_button, 0);
	button_box->AddStretchSpacer(2);

	// main layout
	wxSizer *main_box = new wxBoxSizer(wxVERTICAL);
	main_box->Add(script_list, 1, wxEXPAND|wxALL, 5);
	main_box->Add(button_box, 0, wxEXPAND|wxALL&~wxTOP, 5);
	main_box->SetSizeHints(this);
	SetSizer(main_box);
	Center();

	// fill the list view
	std::list<AssAutomationFilter*>::const_iterator f = AssAutomationFilter::GetFilterList().begin();
	for (;f != AssAutomationFilter::GetFilterList().end(); ++f) {
		AddScriptToList(*f);
	}

	UpdateDisplay();
}


DialogAutomationManager::~DialogAutomationManager()
{
}


BEGIN_EVENT_TABLE(DialogAutomationManager,wxDialog)
	EVT_BUTTON(AUTOMAN_CREATE,DialogAutomationManager::OnCreate)
	EVT_BUTTON(AUTOMAN_ADD,DialogAutomationManager::OnAdd)
	EVT_BUTTON(AUTOMAN_REMOVE,DialogAutomationManager::OnRemove)
	EVT_BUTTON(AUTOMAN_APPLY,DialogAutomationManager::OnApply)
	EVT_BUTTON(AUTOMAN_EDIT,DialogAutomationManager::OnEdit)
	EVT_BUTTON(AUTOMAN_RELOAD,DialogAutomationManager::OnReload)
	EVT_BUTTON(wxID_CLOSE,DialogAutomationManager::OnClose)
	EVT_LIST_ITEM_SELECTED(AUTOMAN_SCRIPTLIST,DialogAutomationManager::OnSelectionChange)
	EVT_LIST_ITEM_DESELECTED(AUTOMAN_SCRIPTLIST,DialogAutomationManager::OnSelectionChange)
END_EVENT_TABLE()


void DialogAutomationManager::OnCreate(wxCommandEvent &event)
{
	wxString path = Options.AsText(_T("Last open automation path"));	
	wxString sfnames = wxFileSelector(_("Create Automation script"), path, _T("*.lua"), _T("lua"), _T("Automation Lua scripts (*.lua)|*.lua|All files (*.*)|*.*"), wxSAVE|wxOVERWRITE_PROMPT, this);
	if (sfnames.empty()) return;
	Options.SetText(_T("Last open automation path"), sfnames);
	
	wxFileName sfname(sfnames);

	if (sfname.FileExists()) {
		if (!wxRemoveFile(sfnames)) {
			wxMessageBox(_T("The old file by this name could not be deleted."), _T("Error creating Automation script"), wxOK|wxICON_ERROR, this);
			return;
		}
	}

	wxTextFile file;
	file.Create(sfnames);
	file.AddLine(_T("-- Aegisub Automation script"));
	file.AddLine(_T(""));
	file.AddLine(_T("-- You should change these two lines"));
	file.AddLine(wxString::Format(_T("name = \"%s\""), sfname.GetName().c_str()));
	file.AddLine(_T("description = \"New Automation script\""));
	file.AddLine(_T(""));
	file.AddLine(_T("-- Enter the configuration settings here, if needed. Refer to the manual for details"));
	file.AddLine(_T("configuration = {}"));
	file.AddLine(_T(""));
	file.AddLine(_T("-- You should NOT change this line!"));
	file.AddLine(_T("version, kind = 3, 'basic_ass'"));
	file.AddLine(_T(""));
	file.AddLine(_T("-- You should write your script in this function (don't change its name!)"));
	file.AddLine(_T("function process_lines(meta, styles, lines, config)"));
	file.AddLine(_T("\t-- For now, just return the subtitles as-is, no changes"));
	file.AddLine(_T("\treturn lines"));
	file.AddLine(_T("end"));
	file.AddLine(_T(""));
	file.Write(wxTextFileType_None, wxConvUTF8);
	file.Close();

	AutomationScriptFile *sfile = AutomationScriptFile::CreateFromFile(sfnames);
	AutomationScript *script = new AutomationScript(sfile);
	AssAutomationFilter *filter = new AssAutomationFilter(script);
	AddScriptToList(filter);
	delete sfile;

	EditScript(script);
}


void DialogAutomationManager::OnAdd(wxCommandEvent &event)
{
	wxString path = Options.AsText(_T("Last open automation path"));	
	wxString sfnames = wxFileSelector(_("Load Automation script"), path, _T("*.lua"), _T("lua"), _T("Automation Lua scripts (*.lua)|*.lua|All files (*.*)|*.*"), wxOPEN|wxFILE_MUST_EXIST, this);
	if (sfnames.empty()) return;
	Options.SetText(_T("Last open automation path"), sfnames);

	try {
		AutomationScriptFile *sfile = AutomationScriptFile::CreateFromFile(sfnames);
		AutomationScript *script = new AutomationScript(sfile);
		AssAutomationFilter *filter = new AssAutomationFilter(script);
		wxString script_settings = subgrid->ass->GetScriptInfo(wxString::Format(_T("Automation Settings %s"), wxFileName(sfnames).GetFullName().c_str()));
		script->configuration.unserialize(script_settings);
		AddScriptToList(filter);
		delete sfile;
	}
	catch (AutomationError &err) {
		wxMessageBox(wxString::Format(_T("Error loading Automation script '%s':\r\n\r\n%s"), sfnames.c_str(), err.message.c_str()), _T("Error loading Automation script"), wxOK | wxICON_ERROR, this);
	}
	catch (wxString &err) {
		wxMessageBox(wxString::Format(_T("Error loading Automation script %s:\r\n\r\n%s"), sfnames.c_str(), err.c_str()), _T("Error loading Automation script"), wxOK|wxICON_ERROR, this);
	}
	catch (const wchar_t *err) {
		wxMessageBox(wxString::Format(_T("Error loading Automation script %s:\r\n\r\n%s"), sfnames.c_str(), err), _T("Error loading Automation script"), wxOK|wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error loading Automation script."), _T("Error loading Automation script"), wxOK | wxICON_ERROR, this);
	}
}


void DialogAutomationManager::OnRemove(wxCommandEvent &event)
{
	// assume only one item can be selected at a time...
	// removing multiple scripts might be disasterous (for the user) anyway
	// TODO: ask for confirmation if script supports configuration
	int selid = script_list->GetFirstSelected();
	AssAutomationFilter *filter = (AssAutomationFilter*)script_list->GetItemData(selid);
	script_list->DeleteItem(selid);
	AutomationScript *script = filter->GetScript();
	delete filter;
	delete script;
	UpdateDisplay();
}


void DialogAutomationManager::OnApply(wxCommandEvent &event)
{
	int selid = script_list->GetFirstSelected();
	AssAutomationFilter *filter = (AssAutomationFilter*)script_list->GetItemData(selid);
	// Attempt to make a config window, if needed
	{
		wxDialog *dlg = new wxDialog(this, -1, filter->GetScript()->name, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("AutomationScriptConfigDlg"));
		try {
			wxWindow *config_frame = filter->GetConfigDialogWindow(dlg);
			if (!config_frame) {
				delete dlg;
				goto skip_config;
			}
			wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
			main_sizer->Add(config_frame, 0, wxALL, 7);
			wxButton *ok = new wxButton(dlg, wxID_OK);
			wxButton *cancel = new wxButton(dlg, wxID_CANCEL);
			wxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
			button_sizer->Add(ok, 0, wxALL, 7);
			button_sizer->Add(cancel, 0, wxALL&~wxLEFT, 7);
			main_sizer->Add(button_sizer, wxALIGN_CENTER);
			dlg->SetSizer(main_sizer);
			main_sizer->SetSizeHints(dlg);
			dlg->Layout();
			switch (dlg->ShowModal()) {
				case wxID_OK:
					filter->LoadSettings(false);
					delete dlg;
					break;
				case wxID_CANCEL:
					delete dlg;
					return;
				default:
					wxLogWarning(_T("Config dialog returned an unexpected value. This shouldn't happen. Please report to a dev."));
					delete dlg;
					return;
			}
		}
		catch (...) {
			delete dlg;
			wxLogError(_T("Error while working on Automation script config dialog. This shouldn't happen. Please report to a dev."));
			return;
		}
	}
skip_config:
	// Now apply script
	filter->ProcessSubs(subgrid->ass);
	subgrid->LoadFromAss();
	script_list->Select(selid, false);
	UpdateDisplay();
}


void DialogAutomationManager::OnEdit(wxCommandEvent &event)
{
	int selid = script_list->GetFirstSelected();
	AssAutomationFilter *filter = (AssAutomationFilter*)script_list->GetItemData(selid);
	AutomationScript *script = filter->GetScript();
	EditScript(script);
}


void DialogAutomationManager::OnReload(wxCommandEvent &event)
{
	int selid = script_list->GetFirstSelected();
	AssAutomationFilter *filter = (AssAutomationFilter*)script_list->GetItemData(selid);
	AutomationScript *script = filter->GetScript();
	wxFileName sfname(script->filename);
	if (sfname.FileExists()) {
		AutomationScript *newscript;
		try {
			AutomationScriptFile *sfile = AutomationScriptFile::CreateFromFile(sfname.GetFullPath());
			newscript = new AutomationScript(sfile);
			delete sfile;
		}
		catch (AutomationError &err) {
			wxMessageBox(wxString::Format(_T("Error reloading Automation script '%s'.\r\nThe old version has been retained.\r\n\r\n%s"), sfname.GetFullPath().c_str(), err.message.c_str()), _T("Error reloading Automation script"), wxOK | wxICON_ERROR, this);
			return;
		}
		newscript->configuration = script->configuration;
		delete filter;
		delete script;
		AssAutomationFilter *newfilter = new AssAutomationFilter(newscript);
		script_list->DeleteItem(selid);
		AddScriptToList(newfilter);
		script_list->Select(0);
	} else {
		wxMessageBox(_T("The script file could not be found on disk. If you want to remove the script, please use the Remove button."), _T("Error reloading Automation script"), wxOK|wxICON_EXCLAMATION, this);
	}
}


void DialogAutomationManager::OnClose(wxCommandEvent &event)
{
	EndModal(0);
}


void DialogAutomationManager::OnSelectionChange(wxListEvent &event)
{
	UpdateDisplay();
}


void DialogAutomationManager::UpdateDisplay()
{
	bool script_selected = script_list->GetSelectedItemCount() > 0;
	// enable/disable buttons
	create_button->Enable(true);
	add_button->Enable(true);
	remove_button->Enable(script_selected);
	test_button->Enable(script_selected);
	edit_button->Enable(script_selected);
	reload_button->Enable(script_selected);
	close_button->Enable(true);
}


void DialogAutomationManager::AddScriptToList(AssAutomationFilter *filter)
{
	wxFileName fn(filter->GetScript()->filename);
	wxListItem item;
	item.SetText(filter->GetScript()->name);
	item.SetData(filter);
	int i = script_list->InsertItem(item);
	script_list->SetItem(i, 1, fn.GetFullName());
	//script_list->SetItem(i, 2, _T("")); // Flags - unused
	script_list->SetItem(i, 2, filter->GetScript()->description);
}

void DialogAutomationManager::EditScript(AutomationScript *script)
{
	if (!script) {
		wxMessageBox(_T("DialogAutomationManager::EditScript() called without a valid script object. Sloppy programming? You can probably blame jfs."), _T("Blame Canada!"), wxOK|wxICON_ERROR);
		return;
	}

	wxFileName sfname(script->filename);
	if (!sfname.FileExists()) {
		wxMessageBox(_T("The script file \"%s\" does not exist, and thus cannot be edited."), _T("Automation warning"), wxOK|wxICON_WARNING);
		return;
	}

	wxString editor;
	if (!Options.IsDefined(_T("automation script editor")) || wxGetKeyState(WXK_SHIFT)) {
		wxMessageBox(_T("You have not selected a script editor yet. Please select your script editor in the next window. It's recommended to use an editor with syntax highlighting for Lua scripts."), _T("Aegisub"), wxOK|wxICON_INFORMATION);
#ifdef __WIN32__
		editor = wxFileSelector(_T("Select script editor"), _T(""), _T("C:\\Windows\\Notepad.exe"), _T("exe"), _T("Execatuables (*.exe)|*.exe|All files (*.*)|*.*"), wxOPEN|wxFILE_MUST_EXIST);
#else
		char *env_editor = getenv("EDITOR");
		wxString editor(env_editor ? env_editor : "/usr/bin/gvim", wxConvLocal);
		editor = wxFileSelector(_T("Select script editor"), _T(""), editor, _T(""), _T("All files (*)|*"), wxOPEN|wxFILE_MUST_EXIST);
#endif
		if (editor.empty()) return;
		Options.SetText(_T("automation script editor"), editor);
		Options.Save();
	} else {
		editor = Options.AsText(_T("automation script editor"));
	}

	wxWCharBuffer editorbuf = editor.c_str(), sfnamebuf = sfname.GetFullPath().c_str();
	wchar_t **cmdline = new wchar_t*[3];
	cmdline[0] = editorbuf.data();
	cmdline[1] = sfnamebuf.data();
	cmdline[2] = 0;
	long res = wxExecute(cmdline);
	delete cmdline;

	if (!res) {
		wxMessageBox(_T("Some error occurred trying to launch the external editor. Sorry!"), _T("Automation Error"), wxOK|wxICON_ERROR);
	}
}

