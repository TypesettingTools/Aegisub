// Copyright (c) 2005, Rodrigo Braz Monteiro
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


///////////
// Headers
#include "config.h"

#ifdef WIN32
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shobjidl.h>
// Typedefs for some functions we link dynamically because they only exist on Vista+
// and we need to check for their presence
typedef HRESULT (WINAPI *PTaskDialogIndirect)(const TASKDIALOGCONFIG *pTaskConfig, __out_opt int *pnButton, __out_opt int *pnRadioButton, __out_opt BOOL *pfVerificationFlagChecked);
typedef HRESULT (WINAPI *PSHCreateAssociationRegistration)(REFIID riid, void **ppv);
#pragma comment(lib,"shlwapi")
#endif


#include <wx/wxprec.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/config.h>
#include <wx/dialog.h>
#include <wx/checklst.h>
#include <wx/slider.h>
#include <vector>
#include <string>
#include <algorithm>

#include "dialog_associations.h"
#include "options.h"



#ifdef WIN32
class OldRegistrationChecker {
public:
	struct FileType {
		std::wstring ext;
		std::wstring progid;
		std::wstring typedesc;
		FileType(const std::wstring &_ext, const std::wstring &_progid, const std::wstring &_typedesc)
			: ext(_ext)
			, progid(_progid)
			, typedesc(_typedesc)
		{ }
	};

private:
	std::vector<FileType> types;

	HKEY hkey_user_classes;
	HKEY hkey_explorer_fileexts;

public:
	OldRegistrationChecker();
	~OldRegistrationChecker();

	size_t GetTypeCount() const { return types.size(); }
	const FileType& GetType(size_t index) const { return types.at(index); }

	bool HasType(const FileType &type) const;
	bool HasAllTypes() const;
	void TakeType(const FileType &type) const;
	void TakeAllTypes() const;
};


class DialogAssociations : public wxDialog {
private:
	wxCheckListBox *list;

	void OnOK(wxCommandEvent &event);

	struct FileTypeIndexMapping {
		wxString display_name;
		size_t reg_index;
		bool operator < (const FileTypeIndexMapping &other) { return display_name < other.display_name; }
	};
	std::vector<FileTypeIndexMapping> mapping;

	OldRegistrationChecker reg;

public:
	DialogAssociations(wxWindow *parent);
	~DialogAssociations();

	DECLARE_EVENT_TABLE()
};



OldRegistrationChecker::OldRegistrationChecker()
{
	// Read the registration data written for Vista+ systems into HKLM
	// We can just as well re-use the data the installer already writes!
	HKEY hkey_caps = 0;
	HKEY hkey_machine_classes = 0;

	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Aegisub\\Capabilities\\FileAssociations", 0, KEY_READ, &hkey_caps) == ERROR_SUCCESS &&
		RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes", 0, KEY_READ, &hkey_machine_classes) == ERROR_SUCCESS)
	{
		int enum_index = 0;
		const size_t alloc_value_size = 256;
		wchar_t *value_ext = new wchar_t[alloc_value_size];
		wchar_t *value_pid = new wchar_t[alloc_value_size];
		wchar_t *value_dsc = new wchar_t[alloc_value_size];

		LRESULT enumres = ERROR_SUCCESS;
		while (enumres == ERROR_SUCCESS)
		{
			DWORD value_ext_size = (DWORD)alloc_value_size;
			// Find next listed file extension
			enumres = RegEnumValueW(hkey_caps, enum_index, value_ext, &value_ext_size, 0, 0, 0, 0);

			if (enumres != ERROR_SUCCESS) break;

			DWORD value_pid_size = (DWORD)alloc_value_size*2;
			DWORD value_type = REG_NONE;
			// Grab the ProgID for that extension
			if (RegQueryValueExW(hkey_caps, value_ext, 0, &value_type, (LPBYTE)value_pid, &value_pid_size) == ERROR_SUCCESS &&
				value_type == REG_SZ)
			{
				DWORD value_dsc_size = (DWORD)alloc_value_size*2;
				// And find the human-readable name for that ProgID
				if (RegQueryValueW(hkey_machine_classes, value_pid, value_dsc, (PLONG)&value_dsc_size) == ERROR_SUCCESS)
				{
					// And dump it all into our table
					types.push_back(FileType(
						std::wstring(value_ext, value_ext_size),
						std::wstring(value_pid, value_pid_size/2),
						std::wstring(value_dsc, value_dsc_size/2)));
				}
			}

			enum_index += 1;
		}

		delete[] value_ext;
		delete[] value_pid;
		delete[] value_dsc;
	}

	if (hkey_caps)
		RegCloseKey(hkey_caps);
	if (hkey_machine_classes)
		RegCloseKey(hkey_machine_classes);

	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_READ|KEY_WRITE, &hkey_user_classes) != ERROR_SUCCESS)
		hkey_user_classes = 0;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts", 0, KEY_READ|KEY_WRITE, &hkey_explorer_fileexts) != ERROR_SUCCESS)
		hkey_explorer_fileexts = 0;
}


OldRegistrationChecker::~OldRegistrationChecker()
{
	if (hkey_user_classes)
		RegCloseKey(hkey_user_classes);
	if (hkey_explorer_fileexts)
		RegCloseKey(hkey_explorer_fileexts);
}


bool OldRegistrationChecker::HasAllTypes() const
{
	bool res = true;
	for (size_t i = 0; i < types.size(); ++i)
	{
		res &= HasType(types[i]);
	}
	return res;
}


void OldRegistrationChecker::TakeAllTypes() const
{
	for (size_t i = 0; i < types.size(); ++i)
	{
		TakeType(types[i]);
	}
}


bool OldRegistrationChecker::HasType(const FileType &type) const
{
	// Checks to perform:
	// 1. Check hkey_explorer_fileexts\<ext> for value "Progid":
	//    * If the value data is our <progid> then we have the type
	//    * If the value data is something else, we don't have the type
	//    * If the value doesn't exist, continue
	// 2. Check hkey_explorer_fileexts\<ext> for value "Application":
	//    * If the value data is "aegisub32.exe" then we have the type
	//    * If the value data is something else, we don't have the type
	//    * If the value doesn't exist, continue
	// 3. Check the default value of HKEY_CLASSES_ROOT\<ext>:
	//    * If it is our <progid> then we have the type
	//    * If it is something else, we don't have the type
	//    * If the value doesn't exist, we don't have the type

	const DWORD alloc_size = 260;
	wchar_t *data = new wchar_t[alloc_size];
	DWORD data_type, data_size;

	HKEY hkey_fileext;
	if (RegOpenKeyExW(hkey_explorer_fileexts, type.ext.c_str(), 0, KEY_READ, &hkey_fileext) == ERROR_SUCCESS)
	{
		// Step 1
		data_type = REG_NONE;
		data_size = alloc_size*sizeof(wchar_t);
		if (RegQueryValueExW(hkey_fileext, L"Progid", 0, &data_type, (LPBYTE)data, &data_size) == ERROR_SUCCESS &&
			data_type == REG_SZ)
		{
			std::wstring cur_progid(data, data_size/2);

			RegCloseKey(hkey_fileext);
			delete[] data;

			return type.progid.compare(cur_progid) == 0;
		}

		// Step 2
		data_type = REG_NONE;
		data_size = alloc_size*sizeof(wchar_t);
		if (RegQueryValueExW(hkey_fileext, L"Application", 0, &data_type, (LPBYTE)data, &data_size) == ERROR_SUCCESS &&
			data_type == REG_SZ)
		{
			std::wstring cur_app(data, data_size/2);

			RegCloseKey(hkey_fileext);
			delete[] data;

			return cur_app.compare(L"aegisub32.exe") == 0; // aegisub should't get registrations like this, but better safe than sorry...
		}

		RegCloseKey(hkey_fileext);
	}

	// Step 3
	bool res = false;
	data_size = alloc_size*sizeof(wchar_t);
	if (RegQueryValueW(HKEY_CLASSES_ROOT, type.ext.c_str(), data, (PLONG)&data_size) == ERROR_SUCCESS)
	{
		std::wstring cur_progid(data, data_size/2);
		res = type.progid.compare(cur_progid) == 0;
	}

	delete[] data;
	return res;
}


void OldRegistrationChecker::TakeType(const FileType &type) const
{
	// Making sure we have the type:
	// 1. From hkey_explorer_fileexts\<ext> delete any "Application" value
	// 2. From hkey_explorer_fileexts\<ext> delete any "Progid" value
	// 3. Read HKCR\<ext> default value:
	//    * If it is our <progid>, we are done
	//    * If there is no value, continue to step 4
	//    * If the value is something else, write a value with that as name to
	//      hkey_user_classes\<ext>\Progids with blank data and continue
	// 4. Set default value of hkey_user_classes\<ext> to our <progid>
	// First two removes any customisation the user has done in Explorer, last two
	// override anything set in HKLM\SOFTWARE\Classes\<ext>.

	HKEY hkey_fileext;
	if (RegOpenKeyExW(hkey_explorer_fileexts, type.ext.c_str(), 0, KEY_READ|KEY_WRITE, &hkey_fileext) == ERROR_SUCCESS)
	{
		// Step 1 and 2
		RegDeleteValueW(hkey_fileext, L"Application");
		RegDeleteValueW(hkey_fileext, L"Progid");
		RegCloseKey(hkey_fileext);
	}

	// Step 3
	DWORD old_progid_size = 260*2;
	wchar_t *old_progid = new wchar_t[old_progid_size];
	if (RegQueryValueW(HKEY_CLASSES_ROOT, type.ext.c_str(), old_progid, (PLONG)&old_progid_size) == ERROR_SUCCESS)
	{
		std::wstring old_progid_str(old_progid, old_progid_size/2);
		// Check if existing progid matches ours
		if (type.progid.compare(old_progid_str) == 0)
		{
			// It does, we're done
			delete[] old_progid;
			return;
		}
		else if (old_progid_str.size() != 0)
		{
			// Not our progid and not blank, make sure it's present for Open With
			HKEY hkey_user_ext = 0;
			HKEY hkey_progids = 0;
			if (RegOpenKeyExW(hkey_user_classes, type.ext.c_str(), 0, KEY_WRITE, &hkey_user_ext) == ERROR_SUCCESS &&
				RegOpenKeyExW(hkey_user_ext, L"OpenWithProgids", 0, KEY_WRITE, &hkey_progids) == ERROR_SUCCESS)
			{
				RegSetValueExW(hkey_progids, old_progid_str.c_str(), 0, REG_NONE, 0, 0);
			}

			if (hkey_progids)
				RegCloseKey(hkey_progids);
			if (hkey_user_ext)
				RegCloseKey(hkey_user_ext);
		}
	}
	delete[] old_progid;

	// Step 4
	RegSetValueW(hkey_user_classes, type.ext.c_str(), REG_SZ, type.progid.c_str(), (type.progid.size()+1)*sizeof(wchar_t));
}


DialogAssociations::DialogAssociations (wxWindow *parent)
: wxDialog (parent,-1,_("Associate file types"),wxDefaultPosition,wxDefaultSize)
, reg()
{
	size_t reg_max_index = reg.GetTypeCount();
	for (size_t i = 0; i < reg_max_index; ++i)
	{
		const OldRegistrationChecker::FileType &type = reg.GetType(i);
		if (reg.HasType(type))
			continue;

		FileTypeIndexMapping entry;
		entry.display_name = wxString::Format(_T("%s (%s)"), type.ext.c_str(), type.typedesc.c_str());
		entry.reg_index = i;
		mapping.push_back(entry);
	}
	std::sort(mapping.begin(), mapping.end());

	wxArrayString list_items;
	for (size_t i = 0; i < mapping.size(); ++i)
	{
		list_items.Add(mapping[i].display_name);
	}
	list = new wxCheckListBox(this, -1, wxDefaultPosition, wxSize(-1, 120), list_items);

	wxStaticText *helptext;

	if (mapping.size() > 0)
		helptext = new wxStaticText(this, -1, _("Aegisub can take over the following file types.\n\nIf you want Aegisub to no longer be associated with a file type, you must tell another program to take over the file type."));
	else
		helptext = new wxStaticText(this, -1, _("Aegisub is already associated with all supported file types.\n\nIf you want Aegisub to no longer be associated with a file type, you must tell another program to take over the file type."));
	helptext->Wrap(340);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(helptext, 0, wxALL, 12);
	sizer->Add(list, 1, wxEXPAND|wxALL&(~wxTOP), 12);
	if (mapping.size() > 0)
	{
		sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL&(~wxTOP), 12);
	}
	else
	{
		sizer->Hide(list);
		sizer->Add(CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL&(~wxTOP), 12);
	}

	SetSizerAndFit(sizer);
	CentreOnParent();
}


DialogAssociations::~DialogAssociations()
{
	// Do nothing
}


BEGIN_EVENT_TABLE(DialogAssociations, wxDialog)
	EVT_BUTTON(wxID_OK,DialogAssociations::OnOK)
END_EVENT_TABLE()


void DialogAssociations::OnOK(wxCommandEvent &event)
{
	for (size_t i = 0; i < mapping.size(); ++i)
	{
		if (list->IsChecked(i))
		{
			const OldRegistrationChecker::FileType &type = reg.GetType(mapping[i].reg_index);
			reg.TakeType(type);
		}
	}

	event.Skip();
}
#endif



void ShowAssociationsDialog(wxWindow *parent)
{
#ifdef WIN32
	HRESULT res;
	IApplicationAssociationRegistrationUI *aarui;
	res = CoCreateInstance(
		CLSID_ApplicationAssociationRegistrationUI, 0,
		CLSCTX_INPROC_SERVER,
		IID_IApplicationAssociationRegistrationUI, (LPVOID*)&aarui);

	if (SUCCEEDED(res) && aarui != 0)
	{
		// We can call the Vista-style UI for registrations
		aarui->LaunchAdvancedAssociationUI(L"Aegisub");
		aarui->Release();
	}
	else
	{
		// Use the old way of a custom dialogue
		DialogAssociations dlg(parent);
		dlg.ShowModal();
	}
#endif
}


void FixOldAssociations();

void CheckFileAssociations(wxWindow *parent)
{
	// Portable configurations don't use associations
	if (Options.AsBool(_T("Local Config"))) return;
	// The user has opted out for the check
	if (!Options.AsBool(_T("Show Associations"))) return;

	// Clean up any old associations the user has lying around
	FixOldAssociations();

#ifdef WIN32
	HMODULE shell32 = LoadLibraryW(L"shell32.dll");
	HMODULE comctl32 = LoadLibraryW(L"comctl32.dll");
	if (!(shell32 && comctl32)) return; // this is serious failure, don't bother the user

	PSHCreateAssociationRegistration SHCreateAssociationRegistration =
		(PSHCreateAssociationRegistration)GetProcAddress(shell32, "SHCreateAssociationRegistration");
	PTaskDialogIndirect TaskDialogIndirect =
		(PTaskDialogIndirect)GetProcAddress(comctl32, "TaskDialogIndirect");

	const wchar_t *dialog_title = _("Aegisub");
	const wchar_t *dialog_bigtext = _("Make Aegisub default editor for subtitles?");
	const wchar_t *dialog_maintext = _("Aegisub is not your default editor for subtitle files. Do you want to make Aegisub your default editor for subtitle files?");
	const wchar_t *dialog_checkboxtext = _("Always perform this check when Aegisub starts");

	if (SHCreateAssociationRegistration && TaskDialogIndirect)
	{
		// We're on Vista or later, do new-style type registration
		HRESULT res;
		IApplicationAssociationRegistration *car;
		res = SHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, (LPVOID*)&car);

		if (SUCCEEDED(res))
		{
			BOOL is_default = FALSE;
			res = car->QueryAppIsDefaultAll(AL_EFFECTIVE, L"Aegisub", &is_default);

			if (SUCCEEDED(res) && !is_default)
			{
				TASKDIALOGCONFIG tdc = {sizeof(TASKDIALOGCONFIG)};
				tdc.hwndParent = (HWND)(parent ? parent->GetHandle() : 0);
				tdc.pszWindowTitle = dialog_title;
				tdc.pszMainInstruction = dialog_bigtext;
				tdc.pszContent = dialog_maintext;
				tdc.dwFlags = TDF_VERIFICATION_FLAG_CHECKED;
				tdc.pszVerificationText = dialog_checkboxtext;
				tdc.dwCommonButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;

				int btn = 0;
				BOOL show_again = TRUE;
				res = TaskDialogIndirect(&tdc, &btn, 0, &show_again);

				if (SUCCEEDED(res))
				{
					if (btn == IDYES)
					{
						car->SetAppAsDefaultAll(L"Aegisub");
					}

					Options.SetBool(_T("Show Associations"), show_again==TRUE);
				}
			}

			car->Release();
		}
	}
	else
	{
		// We're on XP or something similar old, do classic type registration
		OldRegistrationChecker checker;
		if (!checker.HasAllTypes())
		{
			wxDialog dlg(parent, -1, dialog_title, wxDefaultPosition, wxDefaultSize, wxCAPTION);
			wxStaticText *maintext = new wxStaticText(&dlg, -1, dialog_maintext);
			maintext->Wrap(360);
			wxCheckBox *check = new wxCheckBox(&dlg, -1, dialog_checkboxtext);
			check->SetValue(true);
			wxStdDialogButtonSizer *buttons = new wxStdDialogButtonSizer();
			buttons->AddButton(new wxButton(&dlg, wxID_OK, _("&Yes")));
			buttons->AddButton(new wxButton(&dlg, wxID_CANCEL, _("&No")));
			buttons->Realize();
			wxBoxSizer *dlgsizer = new wxBoxSizer(wxVERTICAL);
			dlgsizer->Add(maintext, 0, wxALL, 12);
			dlgsizer->Add(check, 0, wxEXPAND|wxALL&(~wxTOP), 12);
			dlgsizer->Add(buttons, 0, wxEXPAND|wxALL&(~wxTOP), 12);
			dlg.SetSizerAndFit(dlgsizer);
			dlg.CentreOnParent();

			int modalres = dlg.ShowModal();
			if (modalres == wxID_OK)
			{
				checker.TakeAllTypes();
			}

			Options.SetBool(_T("Show Associations"), check->GetValue());
		}
	}

	FreeLibrary(comctl32);
	FreeLibrary(shell32);

#else
	// Do nothing
#endif
}


// Check whether any old (2.1.7 and earlier) associations are in place, and migrate
// them to the new associations
void FixOldAssociations()
{
#ifdef WIN32
	// 1. Check if new style associations (progids) are set up, stop if they are not
	// 2. Check if old style associations ("Aegisub" progid) are set up, stop if not
	// 3. Check progids for these extensions, for each that is "Aegisub" change to:
	//    * .ass  -> Aegisub.ASSA.1
	//    * .ssa  -> Aegisub.SSA.1
	//    * .srt  -> Aegisub.SRT.1
	//    * .sub  -> Aegisub.Subtitle.1
	//    * .ttxt -> Aegisub.TTXT.1

	// The new progids we use
	const wchar_t *new_progids[] = {
		L"Aegisub.ASSA.1",
		L"Aegisub.SSA.1",
		L"Aegisub.SRT.1",
		L"Aegisub.Subtitle.1",
		L"Aegisub.TTXT.1",
		0
	};
	// File extensions from old associations in the order of the new progids above
	// so we can match one to the other
	const wchar_t *extensions[] = {
		L".ass",
		L".ssa",
		L".srt",
		L".sub",
		L".ttxt",
		0
	};

	// 1. Check for new progids
	HKEY hkey_machine_classes;
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes", 0, KEY_READ, &hkey_machine_classes) != ERROR_SUCCESS)
		// Can't access machine classes
		return;
	for (size_t i = 0; new_progids[i] != 0; ++i)
	{
		HKEY temp;
		if (RegOpenKeyExW(hkey_machine_classes, new_progids[i], 0, KEY_READ, &temp) == ERROR_SUCCESS)
		{
			RegCloseKey(temp);
		}
		else
		{
			// Missing a new progid, can't change associations
			RegCloseKey(hkey_machine_classes);
			return;
		}
	}
	RegCloseKey(hkey_machine_classes);

	// Check for old progid and delete it.
	// Make sure to explicitly check HKCU classes
	HKEY hkey_aegisub_progid;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\Aegisub", 0, KEY_READ|KEY_WRITE, &hkey_aegisub_progid) == ERROR_SUCCESS)
	{
		// Exists, remove subkeys
		RegCloseKey(hkey_aegisub_progid);
		// Let's use a Shell API function for this, because RegDeleteTree() was only introduced
		// in Windows 6.0, but this Shell API has been around since IE 4. Makes perfect sense!
		if (SHDeleteKeyW(HKEY_CURRENT_USER, L"Software\\Classes\\Aegisub") != ERROR_SUCCESS)
			// argh!
			return;
	}
	else
	{
		// Missing old progid, can't have old associations
		return;
	}

	// Reset associations to new progids
	HKEY hkey_user_classes;
	if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes", &hkey_user_classes) != ERROR_SUCCESS)
		return;
	for (size_t i = 0; new_progids[i] != 0; ++i)
	{
		const wchar_t *new_progid = new_progids[i];
		const wchar_t *extension = extensions[i];
		wchar_t old_progid[260] = {0};
		DWORD old_progid_size = sizeof(old_progid);
		DWORD old_progid_type = REG_NONE;
		if (SHGetValueW(HKEY_CLASSES_ROOT, extension, 0, &old_progid_type, old_progid, &old_progid_size) == ERROR_SUCCESS &&
			old_progid_type == REG_SZ &&
			wcscmp(L"Aegisub", old_progid) == 0) // safe even if old_progid doesn't have 0-termination, because L"Aegisub" has termination and the buffer is longer
		{
			SHSetValueW(hkey_user_classes, extension, 0, REG_SZ, (LPCVOID)new_progid, (wcslen(new_progid)+1)*sizeof(wchar_t));
		}
	}
	RegCloseKey(hkey_user_classes);

#else
	// Do nothing
#endif
}
