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


////////////
// Includes
#include <wx/filename.h>
#include "dialog_style_manager.h"
#include "dialog_style_editor.h"
#include "ass_style.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "main.h"
#include "options.h"
#include "subs_grid.h"


///////////////
// Constructor
DialogStyleManager::DialogStyleManager (wxWindow *parent,SubtitlesGrid *_grid)
: wxDialog (parent,-1,_("Styles Manager"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE,_T("DialogStylesManager"))
{
	// Vars
	grid = _grid;

	// Catalog
	wxSizer *CatalogBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Catalog of available storages"));
	CatalogList = new wxComboBox(this,LIST_CATALOG, _T(""), wxDefaultPosition, wxSize(180,20), 0, NULL, wxCB_READONLY | wxCB_READONLY, wxDefaultValidator, _T("Catalog List"));
	wxButton *CatalogNew = new wxButton(this, BUTTON_CATALOG_NEW, _("New"), wxDefaultPosition, wxSize(60,20));
	wxButton *CatalogDelete = new wxButton(this, BUTTON_CATALOG_DELETE, _("Delete"), wxDefaultPosition, wxSize(60,20));
	CatalogBox->Add(CatalogList,1,wxEXPAND | wxRIGHT | wxALIGN_RIGHT,5);
	CatalogBox->Add(CatalogNew,0,wxRIGHT,5);
	CatalogBox->Add(CatalogDelete,0,0,0);

	// Storage styles list
	StorageList = new wxListBox(this, LIST_STORAGE, wxDefaultPosition, wxSize(205,250), 0, NULL, wxLB_EXTENDED);
	wxSizer *StorageBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Storage"));
	wxSizer *StorageButtons = new wxBoxSizer(wxHORIZONTAL);
	MoveToLocal = new wxButton(this, BUTTON_STORAGE_COPYTO, _("Copy to current script ->"), wxDefaultPosition, wxSize(205,25));
	StorageNew = new wxButton(this, BUTTON_STORAGE_NEW, _("New"), wxDefaultPosition, wxSize(40,25));
	StorageEdit = new wxButton(this, BUTTON_STORAGE_EDIT, _("Edit"), wxDefaultPosition, wxSize(40,25));
	StorageCopy = new wxButton(this, BUTTON_STORAGE_COPY, _("Copy"), wxDefaultPosition, wxSize(40,25));
	StorageDelete = new wxButton(this, BUTTON_STORAGE_DELETE, _("Delete"), wxDefaultPosition, wxSize(40,25));
	StorageButtons->Add(StorageNew,1,wxEXPAND | wxALL,0);
	StorageButtons->Add(StorageEdit,1,wxEXPAND | wxALL,0);
	StorageButtons->Add(StorageCopy,1,wxEXPAND | wxALL,0);
	StorageButtons->Add(StorageDelete,1,wxEXPAND | wxALL,0);
	StorageBox->Add(StorageList,0,wxEXPAND | wxALL,0);
	StorageBox->Add(MoveToLocal,0,wxEXPAND | wxALL,0);
	StorageBox->Add(StorageButtons,0,wxEXPAND | wxALL,0);
	MoveToLocal->Disable();
	StorageEdit->Disable();
	StorageCopy->Disable();
	StorageDelete->Disable();

	// Local styles list
	CurrentList = new wxListBox(this, LIST_CURRENT, wxDefaultPosition, wxSize(205,250), 0, NULL, wxLB_EXTENDED);
	wxSizer *CurrentBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Current script"));
	wxSizer *CurrentButtons = new wxBoxSizer(wxHORIZONTAL);
	MoveToStorage = new wxButton(this, BUTTON_CURRENT_COPYTO, _("<- Copy to storage"), wxDefaultPosition, wxSize(205,25));
	CurrentNew = new wxButton(this, BUTTON_CURRENT_NEW, _("New"), wxDefaultPosition, wxSize(40,25));
	CurrentEdit = new wxButton(this, BUTTON_CURRENT_EDIT, _("Edit"), wxDefaultPosition, wxSize(40,25));
	CurrentCopy = new wxButton(this, BUTTON_CURRENT_COPY, _("Copy"), wxDefaultPosition, wxSize(40,25));
	CurrentDelete = new wxButton(this, BUTTON_CURRENT_DELETE, _("Delete"), wxDefaultPosition, wxSize(40,25));
	CurrentButtons->Add(CurrentNew,1,wxEXPAND | wxALL,0);
	CurrentButtons->Add(CurrentEdit,1,wxEXPAND | wxALL,0);
	CurrentButtons->Add(CurrentCopy,1,wxEXPAND | wxALL,0);
	CurrentButtons->Add(CurrentDelete,1,wxEXPAND | wxALL,0);
	CurrentBox->Add(CurrentList,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(MoveToStorage,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(CurrentButtons,0,wxEXPAND | wxALL,0);
	MoveToStorage->Disable();
	CurrentEdit->Disable();
	CurrentCopy->Disable();
	CurrentDelete->Disable();

	// General layout
	wxSizer *StylesSizer = new wxBoxSizer(wxHORIZONTAL);
	StylesSizer->Add(StorageBox,0,wxRIGHT,5);
	StylesSizer->Add(CurrentBox,0,wxLEFT,0);
	wxButton *CloseButton = new wxButton(this, wxID_CLOSE, _T(""), wxDefaultPosition, wxSize(100,25));
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(CatalogBox,0,wxEXPAND | wxLEFT | wxRIGHT | wxTOP,5);
	MainSizer->Add(StylesSizer,0,wxEXPAND | wxALL,5);
	MainSizer->Add(CloseButton,0,wxBOTTOM | wxALIGN_CENTER,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);

	// Position window
	if (lastx == -1 && lasty == -1) {
		CenterOnParent();
	} else {
		Move(lastx, lasty);
	}

	// Populate lists
	LoadCatalog();
	LoadCurrentStyles(AssFile::top);

	// Select default item
	wxString selected_style;
	if (_grid) {
		AssDialogue *dia = _grid->GetDialogue(_grid->GetFirstSelRow());
		selected_style = dia->Style;
	}

	if (StorageList->GetCount() && StorageList->SetStringSelection(selected_style)) {
		StorageEdit->Enable();
		StorageCopy->Enable();
		StorageDelete->Enable();
		MoveToLocal->Enable();
	}
	if (CurrentList->GetCount() && CurrentList->SetStringSelection(selected_style)) {
		CurrentEdit->Enable();
		CurrentCopy->Enable();
		CurrentDelete->Enable();
		MoveToStorage->Enable();
	}
}


//////////////
// Destructor
DialogStyleManager::~DialogStyleManager() {
	int sel = CatalogList->GetSelection();
	if (sel != wxNOT_FOUND) {
		AssFile::top->SetScriptInfo(_T("Last Style Storage"),CatalogList->GetString(sel));
	}
}


/////////////////////
// Loads the catalog
void DialogStyleManager::LoadCatalog () {
	// Clear
	CatalogList->Clear();

	// Create catalog if it doesn't exist
	wxString dirname = AegisubApp::folderName;
	dirname += _T("/catalog/");
	if (!wxDirExists(dirname)) {
		if (!wxMkdir(dirname)) {
			throw _T("Error creating directory!");
		}
		else {
			// Create default style
			Store.Clear();
			AssStyle *defstyle = new AssStyle;
			Store.style.push_back(defstyle);
			Store.Save(_T("Default"));
		}
	}

	// Get dir
	dirname = AegisubApp::folderName;
	dirname += _T("/catalog/*.sty");

	// Populate
	wxString curfile = wxFindFirstFile(dirname,wxFILE);
	wxString path,name,ext;
	while (!curfile.empty()) {
		wxFileName::SplitPath(curfile,&path,&name,&ext);
		CatalogList->Append(name);
		curfile = wxFindNextFile();
	}

	// Set to default if available
	StorageActions(false);
	wxString pickStyle = AssFile::top->GetScriptInfo(_T("Last Style Storage"));
	if (pickStyle.IsEmpty()) pickStyle = _T("Default");
	int opt = CatalogList->FindString(pickStyle);
	if (opt != wxNOT_FOUND) {
		CatalogList->SetSelection(opt);
		wxCommandEvent dummy;
		OnChangeCatalog(dummy);
	}
}


////////////////////
// Loads style list
void DialogStyleManager::LoadCurrentStyles (AssFile *subs) {
	using std::list;
	AssStyle *style;

	// Reset
	CurrentList->Clear();
	styleMap.clear();

	// Add styles
	for (list<AssEntry*>::iterator cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		style = AssEntry::GetAsStyle(*cur);
		if (style) {
			if (style->Valid) {
				CurrentList->Append(style->name);
				styleMap.push_back(style);
			}
		}
	}
}

void DialogStyleManager::LoadStorageStyles () {
	using std::list;
	AssStyle *style;

	// Reset
	StorageList->Clear();
	styleStorageMap.clear();

	// Add styles
	for (list<AssStyle*>::iterator cur=Store.style.begin();cur!=Store.style.end();cur++) {
		style = *cur;
		if (style) {
			if (style->Valid) {
				StorageList->Append(style->name);
				styleStorageMap.push_back(style);
			}
		}
	}

	// Flag change
	wxCommandEvent dummy;
	OnStorageChange(dummy);
}


///////////////////////////////////////
// Enables or disables storage actions
void DialogStyleManager::StorageActions (bool state) {
	StorageList->Enable(state);
	MoveToLocal->Enable(state);
	StorageNew->Enable(state);
	StorageCopy->Enable(state);
	StorageDelete->Enable(state);

	wxCommandEvent dummy;
	OnStorageChange(dummy);
	
	wxArrayInt selections;
	if (CurrentList->GetSelections(selections) != 0) MoveToStorage->Enable(state);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogStyleManager, wxDialog)
	EVT_BUTTON(wxID_CLOSE, DialogStyleManager::OnClose)
	EVT_BUTTON(BUTTON_CATALOG_NEW, DialogStyleManager::OnCatalogNew)
	EVT_BUTTON(BUTTON_CATALOG_DELETE, DialogStyleManager::OnCatalogDelete)
	EVT_COMBOBOX(LIST_CATALOG, DialogStyleManager::OnChangeCatalog)
	EVT_LISTBOX(LIST_CURRENT, DialogStyleManager::OnCurrentChange)
	EVT_LISTBOX(LIST_STORAGE, DialogStyleManager::OnStorageChange)
	EVT_LISTBOX_DCLICK(LIST_STORAGE, DialogStyleManager::OnStorageEdit)
	EVT_LISTBOX_DCLICK(LIST_CURRENT, DialogStyleManager::OnCurrentEdit)
	EVT_BUTTON(BUTTON_CURRENT_COPYTO, DialogStyleManager::OnCopyToStorage)
	EVT_BUTTON(BUTTON_STORAGE_COPYTO, DialogStyleManager::OnCopyToCurrent)
	EVT_BUTTON(BUTTON_CURRENT_EDIT, DialogStyleManager::OnCurrentEdit)
	EVT_BUTTON(BUTTON_STORAGE_EDIT, DialogStyleManager::OnStorageEdit)
	EVT_BUTTON(BUTTON_CURRENT_COPY, DialogStyleManager::OnCurrentCopy)
	EVT_BUTTON(BUTTON_STORAGE_COPY, DialogStyleManager::OnStorageCopy)
	EVT_BUTTON(BUTTON_CURRENT_NEW, DialogStyleManager::OnCurrentNew)
	EVT_BUTTON(BUTTON_STORAGE_NEW, DialogStyleManager::OnStorageNew)
	EVT_BUTTON(BUTTON_CURRENT_DELETE, DialogStyleManager::OnCurrentDelete)
	EVT_BUTTON(BUTTON_STORAGE_DELETE, DialogStyleManager::OnStorageDelete)
END_EVENT_TABLE()


//////////
// Events

/////////
// Close
void DialogStyleManager::OnClose (wxCommandEvent &event) {
	GetPosition(&lastx, &lasty);
	Close();
}


////////////////////////
// Change catalog entry
void DialogStyleManager::OnChangeCatalog (wxCommandEvent &event) {
	int sel = CatalogList->GetSelection();
	if (sel != wxNOT_FOUND) {
		StorageActions(true);
		Store.Load(CatalogList->GetString(sel));
		LoadStorageStyles();
	}
	else {
		StorageActions(false);
		Store.Clear();
		LoadStorageStyles();
	}
}


/////////////////////
// New catalog entry
void DialogStyleManager::OnCatalogNew (wxCommandEvent &event) {
	wxString name = wxGetTextFromUser(_("New storage name:"), _("New catalog entry"), _T(""), this);
	if (!name.empty()) {
		// Remove bad characters from the name
		wxString badchars = wxFileName::GetForbiddenChars();
		int badchars_removed = 0;
		for (size_t i = 0; i < name.Length(); ++i) {
			for (size_t j = 0; j < badchars.Length(); ++j) {
				if (name[i] == badchars[j]) {
					name[i] = _T('_');
					++badchars_removed;
				}
			}
		}
		if (badchars_removed > 0) {
			wxLogWarning(_("The specified catalog name contains one or more illegal characters. They have been replaced with underscores instead.\nThe catalog has been renamed to \"%s\"."), name.c_str());
		}

		Store.Clear();
		StorageList->Clear();
		CatalogList->Append(name);
		CatalogList->SetStringSelection(name);
		StorageActions(true);

		wxString dirname = AegisubApp::folderName;
		dirname += _T("/catalog/");
		if (!wxDirExists(dirname)) {
			if (!wxMkdir(dirname)) {
				throw _T("Error creating directory!");
			}
		}
		Store.Save(name);
	}
}


//////////////////
// Catalog delete
void DialogStyleManager::OnCatalogDelete (wxCommandEvent &event) {
	int sel = CatalogList->GetSelection();
	if (sel != wxNOT_FOUND) {
		wxString name = CatalogList->GetString(sel);
		wxString message = _("Are you sure you want to delete the storage \"");
		message += name;
		message += _("\" from the catalog?");
		int option = wxMessageBox(message, _("Confirm delete"), wxYES_NO | wxICON_EXCLAMATION , this);
		if (option == wxYES) {
			wxString filename = AegisubApp::folderName;
			filename += _T("/catalog/");
			filename += name;
			filename += _T(".sty");
			wxRemoveFile(filename);
			CatalogList->Delete(sel);
			StorageList->Clear();
			StorageActions(false);
		}
	}
}


/////////////////////////
// Edit style on storage
void DialogStyleManager::OnStorageEdit (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);
	AssStyle *temp;
	if (n == 1) {
		temp = styleStorageMap.at(selections[0]);
		DialogStyleEditor editor(this,temp,grid);
		int modified = editor.ShowModal();
		if (modified) {
			//LoadStorageStyles();
			StorageList->SetString(selections[0],temp->name);
			Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		}
	}
	else if (n > 1) {
	}	
}


////////////////////////////////
// Edit style on current script
void DialogStyleManager::OnCurrentEdit (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);
	AssStyle *temp;
	if (n == 1) {
		temp = styleMap.at(selections[0]);
		DialogStyleEditor editor(this,temp,grid);
		int modified = editor.ShowModal();
		if (modified) {
			CurrentList->SetString(selections[0],temp->name);
		}
	}
	else if (n > 1) {
	}
}


///////////////////////////////////////
// Selection on current script changed
void DialogStyleManager::OnCurrentChange (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);

	CurrentEdit->Enable(n == 1);
	CurrentCopy->Enable(n == 1);
	CurrentDelete->Enable(n > 0);
	MoveToStorage->Enable(n > 0);
}


////////////////////////////////
// Selection on storage changed
void DialogStyleManager::OnStorageChange (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);

	StorageEdit->Enable(n == 1);
	StorageCopy->Enable(n == 1);
	StorageDelete->Enable(n > 0);
	MoveToLocal->Enable(n > 0);
}


///////////////////
// Copy to Storage
void DialogStyleManager::OnCopyToStorage (wxCommandEvent &event) {
	// Check if there is actually a storage
	if (!StorageNew->IsEnabled()) {
		return;
	}

	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);
	AssStyle *temp;
	for (int i=0;i<n;i++) {
		int test = StorageList->FindString(CurrentList->GetString(selections[i]));
		if (test == wxNOT_FOUND) {
			temp = new AssStyle;
			*temp = *styleMap.at(selections[i]);
			Store.style.push_back(temp);
		}
		else {
			// Bug user?
		}
	}
	Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
	LoadStorageStyles();
}


///////////////////
// Copy to Current
void DialogStyleManager::OnCopyToCurrent (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);
	AssStyle *temp;
	for (int i=0;i<n;i++) {
		// Check if there is already a style with that name
		int test = CurrentList->FindString(StorageList->GetString(selections[i]));
		bool proceed = test==-1;
		if (!proceed) {
			int answer = wxMessageBox(_T("There is already a style with that name on the current script. Proceed anyway?"),_T("Style name collision."),wxYES_NO);
			if (answer == wxYES) proceed = true;
		}

		// Copy
		if (proceed) {
			temp = new AssStyle;
			*temp = *styleStorageMap.at(selections[i]);
			AssFile::top->InsertStyle(temp);
		}

		// Return
		else return;
	}
	LoadCurrentStyles(AssFile::top);
	grid->ass->FlagAsModified();
	grid->CommitChanges();
}


/////////////////////
// Storage make copy
void DialogStyleManager::OnStorageCopy (wxCommandEvent &event) {
	wxArrayInt selections;
	AssStyle *temp = new AssStyle;

	int n = StorageList->GetSelections(selections);
	*temp = *(styleStorageMap.at(selections[0]));
	wxString newName = _("Copy of ");
	newName += temp->name;
	temp->name = newName;

	DialogStyleEditor editor(this,temp,grid);
	int modified = editor.ShowModal();
	if (modified) {
		Store.style.push_back(temp);
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		LoadStorageStyles();
		StorageList->SetStringSelection(temp->name); // the copy/delete/copy-to-local buttons stay disabled after this?
	}
	else delete temp;
}


/////////////////////
// Current make copy
void DialogStyleManager::OnCurrentCopy (wxCommandEvent &event) {
	wxArrayInt selections;

	int n = CurrentList->GetSelections(selections);
	AssStyle *temp = new AssStyle(styleMap.at(selections[0])->GetEntryData());
	wxString newName = _("Copy of ");
	newName += temp->name;
	temp->name = newName;

	DialogStyleEditor editor(this,temp,grid);
	int modified = editor.ShowModal();
	if (modified) {
		AssFile::top->InsertStyle(temp);
		LoadCurrentStyles(AssFile::top);
		CurrentList->SetStringSelection(temp->name); // but even without this, the copy/delete/copy-to-storage buttons stay enabled?
	}
	else delete temp;

	grid->ass->FlagAsModified();
	grid->CommitChanges();
}


///////////////
// Storage new
void DialogStyleManager::OnStorageNew (wxCommandEvent &event) {
	wxArrayInt selections;
	AssStyle *temp = new AssStyle;

	DialogStyleEditor editor(this,temp,grid);
	int modified = editor.ShowModal();
	if (modified) {
		Store.style.push_back(temp);
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		LoadStorageStyles();
	}
	else delete temp;
}


///////////////
// Current new
void DialogStyleManager::OnCurrentNew (wxCommandEvent &event) {
	wxArrayInt selections;
	AssStyle *temp = new AssStyle;

	DialogStyleEditor editor(this,temp,grid);
	int modified = editor.ShowModal();
	if (modified) {
		AssFile::top->InsertStyle(temp);
		LoadCurrentStyles(AssFile::top);
	}
	else delete temp;
}


//////////////////
// Storage delete
void DialogStyleManager::OnStorageDelete (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);

	wxString message;
	if (n!=1) {
		message = _("Are you sure you want to delete these ");
		message += wxString::Format(_T("%i"),n);
		message += _(" styles?");
	}
	else message = _("Are you sure you want to delete this style?");
	int option = wxMessageBox(message, _("Confirm delete"), wxYES_NO | wxICON_EXCLAMATION , this);

	if (option == wxYES) {
		AssStyle *temp;
		for (int i=0;i<n;i++) {
			temp = styleStorageMap.at(selections[i]);
			Store.style.remove(temp);
			delete temp;
		}
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		LoadStorageStyles();
		
		// Set buttons
		MoveToLocal->Enable(false);
		StorageCopy->Enable(false);
		StorageDelete->Enable(false);
	}
}


//////////////////
// Current delete
void DialogStyleManager::OnCurrentDelete (wxCommandEvent &event) {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);

	wxString message;
	if (n!=1) {
		message = _("Are you sure you want to delete these ");
		message += wxString::Format(_T("%i"),n);
		message += _(" styles?");
	}
	else message = _("Are you sure you want to delete this style?");
	int option = wxMessageBox(message, _("Confirm delete"), wxYES_NO | wxICON_EXCLAMATION , this);

	if (option == wxYES) {
		AssStyle *temp;
		for (int i=0;i<n;i++) {
			temp = styleMap.at(selections[i]);
			AssFile::top->Line.remove(temp);
			delete temp;
		}
		LoadCurrentStyles(AssFile::top);

		// Set buttons
		MoveToStorage->Enable(false);
		CurrentCopy->Enable(false);
		CurrentDelete->Enable(false);

		grid->ass->FlagAsModified();
		grid->CommitChanges();
	}
}


int DialogStyleManager::lastx = -1;
int DialogStyleManager::lasty = -1;
