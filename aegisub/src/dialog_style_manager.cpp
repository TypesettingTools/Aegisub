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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_style_manager.cpp
/// @brief Style Manager dialogue box and partial logic
/// @ingroup style_editor

#include "config.h"

#ifndef AGI_PRE
#include <wx/clipbrd.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/tokenzr.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "dialog_style_editor.h"
#include "dialog_style_manager.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "standard_paths.h"
#include "subs_grid.h"
#include "utils.h"

/// @brief Constructor 
/// @param parent 
/// @param _grid  
DialogStyleManager::DialogStyleManager (wxWindow *parent,SubtitlesGrid *_grid)
: wxDialog (parent,-1,_("Styles Manager"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE,_T("DialogStylesManager"))
{
	// Set icon
	SetIcon(BitmapToIcon(GETIMAGE(style_toolbutton_24)));

	// Vars
	grid = _grid;

	// Catalog
	wxSizer *CatalogBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Catalog of available storages"));
	CatalogList = new wxComboBox(this,LIST_CATALOG, _T(""), wxDefaultPosition, wxSize(-1,-1), 0, NULL, wxCB_READONLY | wxCB_READONLY, wxDefaultValidator, _T("Catalog List"));
	wxButton *CatalogNew = new wxButton(this, BUTTON_CATALOG_NEW, _("New"));
	wxButton *CatalogDelete = new wxButton(this, BUTTON_CATALOG_DELETE, _("Delete"));
	CatalogBox->Add(CatalogList,1,wxEXPAND | wxRIGHT | wxALIGN_RIGHT,5);
	CatalogBox->Add(CatalogNew,0,wxRIGHT,5);
	CatalogBox->Add(CatalogDelete,0,0,0);

	// Storage styles list
	StorageList = new wxListBox(this, LIST_STORAGE, wxDefaultPosition, wxSize(240,250), 0, NULL, wxLB_EXTENDED);
	wxSizer *StorageBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Storage"));
	wxSizer *StorageButtons = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *StorageButtonsLow = new wxBoxSizer(wxVERTICAL);
	wxSizer *StorageListSizer = new wxBoxSizer(wxHORIZONTAL);
	MoveToLocal = new wxButton(this, BUTTON_STORAGE_COPYTO, _("Copy to current script ->"));
	StorageNew = new wxButton(this, BUTTON_STORAGE_NEW, _("New"));
	StorageEdit = new wxButton(this, BUTTON_STORAGE_EDIT, _("Edit"));
	StorageCopy = new wxButton(this, BUTTON_STORAGE_COPY, _("Copy"));
	StorageDelete = new wxButton(this, BUTTON_STORAGE_DELETE, _("Delete"));
	StorageButtons->Add(StorageNew,1,wxEXPAND | wxRIGHT,5);
	StorageButtons->Add(StorageEdit,1,wxEXPAND | wxRIGHT,5);
	StorageButtons->Add(StorageCopy,1,wxEXPAND | wxRIGHT,5);
	StorageButtons->Add(StorageDelete,1,wxEXPAND | wxALL,0);
	StorageMoveUp = new wxBitmapButton(this, BUTTON_STORAGE_UP, GETIMAGE(arrow_up_24));
	StorageMoveDown = new wxBitmapButton(this, BUTTON_STORAGE_DOWN, GETIMAGE(arrow_down_24));
	StorageMoveTop = new wxBitmapButton(this, BUTTON_STORAGE_TOP, GETIMAGE(arrow_up_stop_24));
	StorageMoveBottom = new wxBitmapButton(this, BUTTON_STORAGE_BOTTOM, GETIMAGE(arrow_down_stop_24));
	StorageSort = new wxBitmapButton(this, BUTTON_STORAGE_SORT, GETIMAGE(arrow_sort_24));
	StorageMoveUp->SetToolTip(_("Move style up."));
	StorageMoveDown->SetToolTip(_("Move style down."));
	StorageMoveTop->SetToolTip(_("Move style to top."));
	StorageMoveBottom->SetToolTip(_("Move style to bottom."));
	StorageSort->SetToolTip(_("Sort styles alphabetically."));
	StorageButtonsLow->AddStretchSpacer(1);
	StorageButtonsLow->Add(StorageMoveTop,0,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageMoveUp,0,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageMoveDown,0,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageMoveBottom,0,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageSort,0,wxEXPAND | wxALL,0);
	StorageButtonsLow->AddStretchSpacer(1);
	StorageListSizer->Add(StorageList,1,wxEXPAND | wxRIGHT,0);
	StorageListSizer->Add(StorageButtonsLow,0,wxEXPAND | wxALL,0);
	StorageBox->Add(StorageListSizer,1,wxEXPAND | wxBOTTOM,5);
	StorageBox->Add(MoveToLocal,0,wxEXPAND | wxBOTTOM,5);
	StorageBox->Add(StorageButtons,0,wxEXPAND | wxBOTTOM,0);
	MoveToLocal->Disable();
	StorageEdit->Disable();
	StorageCopy->Disable();
	StorageDelete->Disable();

	// Local styles list
	CurrentList = new wxListBox(this, LIST_CURRENT, wxDefaultPosition, wxSize(240,250), 0, NULL, wxLB_EXTENDED);
	wxSizer *CurrentBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Current script"));
	wxSizer *CurrentButtons = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *CurrentButtonsLow = new wxBoxSizer(wxVERTICAL);
	wxSizer *CurrentListSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *MoveImportSizer = new wxBoxSizer(wxHORIZONTAL);
	MoveToStorage = new wxButton(this, BUTTON_CURRENT_COPYTO, _("<- Copy to storage"));
	MoveImportSizer->Add(MoveToStorage,1,wxEXPAND | wxRIGHT,5);
	MoveImportSizer->Add(new wxButton(this, BUTTON_CURRENT_IMPORT, _("Import from script...")),1,wxEXPAND,0);
	CurrentNew = new wxButton(this, BUTTON_CURRENT_NEW, _("New"));
	CurrentEdit = new wxButton(this, BUTTON_CURRENT_EDIT, _("Edit"));
	CurrentCopy = new wxButton(this, BUTTON_CURRENT_COPY, _("Copy"));
	CurrentDelete = new wxButton(this, BUTTON_CURRENT_DELETE, _("Delete"));
	CurrentButtons->Add(CurrentNew,1,wxEXPAND | wxRIGHT,5);
	CurrentButtons->Add(CurrentEdit,1,wxEXPAND | wxRIGHT,5);
	CurrentButtons->Add(CurrentCopy,1,wxEXPAND | wxRIGHT,5);
	CurrentButtons->Add(CurrentDelete,1,wxEXPAND | wxALL,0);
	CurrentMoveUp = new wxBitmapButton(this, BUTTON_CURRENT_UP, GETIMAGE(arrow_up_24));
	CurrentMoveDown = new wxBitmapButton(this, BUTTON_CURRENT_DOWN, GETIMAGE(arrow_down_24));
	CurrentMoveTop = new wxBitmapButton(this, BUTTON_CURRENT_TOP, GETIMAGE(arrow_up_stop_24));
	CurrentMoveBottom = new wxBitmapButton(this, BUTTON_CURRENT_BOTTOM, GETIMAGE(arrow_down_stop_24));
	CurrentSort = new wxBitmapButton(this, BUTTON_CURRENT_SORT, GETIMAGE(arrow_sort_24));
	CurrentMoveUp->SetToolTip(_("Move style up."));
	CurrentMoveDown->SetToolTip(_("Move style down."));
	CurrentMoveTop->SetToolTip(_("Move style to top."));
	CurrentMoveBottom->SetToolTip(_("Move style to bottom."));
	CurrentSort->SetToolTip(_("Sort styles alphabetically."));
	CurrentButtonsLow->AddStretchSpacer(1);
	CurrentButtonsLow->Add(CurrentMoveTop,0,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentMoveUp,0,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentMoveDown,0,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentMoveBottom,0,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentSort,0,wxEXPAND | wxALL,0);
	CurrentButtonsLow->AddStretchSpacer(1);
	CurrentListSizer->Add(CurrentList,1,wxEXPAND | wxRIGHT,0);
	CurrentListSizer->Add(CurrentButtonsLow,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(CurrentListSizer,1,wxEXPAND | wxBOTTOM,5);
	CurrentBox->Add(MoveImportSizer,0,wxEXPAND | wxBOTTOM,5);
	CurrentBox->Add(CurrentButtons,0,wxEXPAND | wxBOTTOM,0);
	MoveToStorage->Disable();
	CurrentEdit->Disable();
	CurrentCopy->Disable();
	CurrentDelete->Disable();

	// Buttons
	wxStdDialogButtonSizer *buttonSizer = new wxStdDialogButtonSizer();
	buttonSizer->SetCancelButton(new wxButton(this, wxID_CLOSE));
	buttonSizer->AddButton(new HelpButton(this,_T("Styles Manager")));
	buttonSizer->Realize();

	// General layout
	wxSizer *StylesSizer = new wxBoxSizer(wxHORIZONTAL);
	StylesSizer->Add(StorageBox,0,wxRIGHT | wxEXPAND,5);
	StylesSizer->Add(CurrentBox,0,wxLEFT | wxEXPAND,0);
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(CatalogBox,0,wxEXPAND | wxLEFT | wxRIGHT | wxTOP,5);
	MainSizer->Add(StylesSizer,1,wxEXPAND | wxALL,5);
	MainSizer->Add(buttonSizer,0,wxBOTTOM | wxEXPAND,5);

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
	LoadCurrentStyles(grid->ass);

	//Set key handlers for lists
	CatalogList->PushEventHandler(new DialogStyleManagerEvent(this));
	StorageList->PushEventHandler(new DialogStyleManagerEvent(this));
	CurrentList->PushEventHandler(new DialogStyleManagerEvent(this));

	// Select default item
	wxString selected_style;
	if (_grid) {
		AssDialogue *dia = _grid->GetDialogue(_grid->GetFirstSelRow());
		if(dia)	selected_style = dia->Style;
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
	UpdateMoveButtons();
}

/// @brief Destructor 
DialogStyleManager::~DialogStyleManager() {
	int sel = CatalogList->GetSelection();
	if (sel != wxNOT_FOUND) {
		grid->ass->SetScriptInfo(_T("Last Style Storage"),CatalogList->GetString(sel));
	}
	CatalogList->PopEventHandler(true);
	StorageList->PopEventHandler(true);
	CurrentList->PopEventHandler(true);
	Store.Clear();
}

/// @brief Load the catalog of stored styles
void DialogStyleManager::LoadCatalog () {
	CatalogList->Clear();

	// Create catalog if it doesn't exist
	wxString dirname = StandardPaths::DecodePath(_T("?user/catalog/"));
	if (!wxDirExists(dirname)) {
		if (!wxMkdir(dirname)) {
			throw _T("Failed creating directory for style catalogues");
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
	dirname = StandardPaths::DecodePath(_T("?user/catalog/*.sty"));

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
	wxString pickStyle = grid->ass->GetScriptInfo(_T("Last Style Storage"));
	if (pickStyle.IsEmpty()) pickStyle = _T("Default");
	int opt = CatalogList->FindString(pickStyle, false);
	if (opt != wxNOT_FOUND) {
		CatalogList->SetSelection(opt);
		wxCommandEvent dummy;
		OnChangeCatalog(dummy);
	}
}

/// @brief Load the list of styles from a subtitle file
void DialogStyleManager::LoadCurrentStyles (AssFile *subs) {
	using std::list;
	AssStyle *style;

	CurrentList->Clear();
	styleMap.clear();

	for (list<AssEntry*>::iterator cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		style = dynamic_cast<AssStyle*>(*cur);
		if (style && style->Valid) {
			CurrentList->Append(style->name);
			styleMap.push_back(style);
		}
	}
	UpdateMoveButtons();
}

/// @brief Load the list of styles from the currently active storage
void DialogStyleManager::LoadStorageStyles () {
	using std::list;

	StorageList->Clear();
	styleStorageMap.clear();

	for (list<AssStyle*>::iterator cur=Store.style.begin();cur!=Store.style.end();cur++) {
		AssStyle *style = *cur;
		if (style && style->Valid) {
			StorageList->Append(style->name);
			styleStorageMap.push_back(style);
		}
	}

	// Flag change
	wxCommandEvent dummy;
	OnStorageChange(dummy);
	UpdateMoveButtons();
}

/// @brief Enables or disables storage actions 
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
	UpdateMoveButtons();
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
	EVT_BUTTON(BUTTON_CURRENT_IMPORT, DialogStyleManager::OnCurrentImport)
	EVT_BUTTON(BUTTON_CURRENT_UP, DialogStyleManager::OnCurrentMoveUp)
	EVT_BUTTON(BUTTON_CURRENT_TOP, DialogStyleManager::OnCurrentMoveTop)
	EVT_BUTTON(BUTTON_CURRENT_DOWN, DialogStyleManager::OnCurrentMoveDown)
	EVT_BUTTON(BUTTON_CURRENT_BOTTOM, DialogStyleManager::OnCurrentMoveBottom)
	EVT_BUTTON(BUTTON_CURRENT_SORT, DialogStyleManager::OnCurrentSort)
	EVT_BUTTON(BUTTON_STORAGE_UP, DialogStyleManager::OnStorageMoveUp)
	EVT_BUTTON(BUTTON_STORAGE_TOP, DialogStyleManager::OnStorageMoveTop)
	EVT_BUTTON(BUTTON_STORAGE_DOWN, DialogStyleManager::OnStorageMoveDown)
	EVT_BUTTON(BUTTON_STORAGE_BOTTOM, DialogStyleManager::OnStorageMoveBottom)
	EVT_BUTTON(BUTTON_STORAGE_SORT, DialogStyleManager::OnStorageSort)
END_EVENT_TABLE()

//////////
// Events

/// @brief Close 
void DialogStyleManager::OnClose (wxCommandEvent &) {
	GetPosition(&lastx, &lasty);
	Close();
}

/// @brief Change catalog entry 
void DialogStyleManager::OnChangeCatalog (wxCommandEvent &) {
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
	UpdateMoveButtons();
}

/// @brief New catalog entry button
void DialogStyleManager::OnCatalogNew (wxCommandEvent &) {
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

		// Make sure that there is no storage with the same name (case insensitive search since Windows filenames are case insensitive)
		if (CatalogList->FindString(name, false) != wxNOT_FOUND) {
			wxMessageBox(_("A catalog with that name already exists."),_("Catalog name conflict"),wxICON_ERROR|wxOK);
			return;
		}

		// Warn about bad characters
		if (badchars_removed > 0) {
			wxMessageBox(wxString::Format(_("The specified catalog name contains one or more illegal characters. They have been replaced with underscores instead.\nThe catalog has been renamed to \"%s\"."), name.c_str()),_("Invalid characters"));
		}

		// Add to list of storages
		Store.Clear();
		StorageList->Clear();
		CatalogList->Append(name);
		CatalogList->SetStringSelection(name);
		StorageActions(true);

		// Save
		wxString dirname = StandardPaths::DecodePath(_T("?user/catalog/"));
		if (!wxDirExists(dirname)) {
			if (!wxMkdir(dirname)) {
				throw _T("Failed creating directory for style catalogues");
			}
		}
		Store.Save(name);
	}
	UpdateMoveButtons();
}

/// @brief Catalog delete 
void DialogStyleManager::OnCatalogDelete (wxCommandEvent &) {
	int sel = CatalogList->GetSelection();
	if (sel != wxNOT_FOUND) {
		wxString name = CatalogList->GetString(sel);
		wxString message = wxString::Format(_("Are you sure you want to delete the storage \"%s\" from the catalog?"), name.c_str());
		int option = wxMessageBox(message, _("Confirm delete"), wxYES_NO | wxICON_EXCLAMATION , this);
		if (option == wxYES) {
			wxRemoveFile(StandardPaths::DecodePath(_T("?user/catalog/") + name + _T(".sty")));
			CatalogList->Delete(sel);
			StorageList->Clear();
			StorageActions(false);
		}
	}
	UpdateMoveButtons();
}

/// @brief Edit style on storage 
void DialogStyleManager::OnStorageEdit (wxCommandEvent &) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);
	if (n == 1) {
		AssStyle *selStyle = styleStorageMap[selections[0]];
		DialogStyleEditor editor(this,selStyle,grid,false,&Store);
		if (editor.ShowModal()) {
			StorageList->SetString(selections[0],selStyle->name);
			Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		}
	}
	UpdateMoveButtons();
}

/// @brief Edit style on current script 
void DialogStyleManager::OnCurrentEdit (wxCommandEvent &) {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);
	if (n == 1) {
		AssStyle *selStyle = styleMap[selections[0]];
		DialogStyleEditor editor(this,selStyle,grid,true,&Store);
		if (editor.ShowModal()) {
			CurrentList->SetString(selections[0],selStyle->name);
		}
	}
	UpdateMoveButtons();
}

/// @brief Selection on current script changed 
void DialogStyleManager::OnCurrentChange (wxCommandEvent &) {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);

	CurrentEdit->Enable(n == 1);
	CurrentCopy->Enable(n == 1);
	CurrentDelete->Enable(n > 0);
	MoveToStorage->Enable(n > 0);
	UpdateMoveButtons();
}

/// @brief Selection on storage changed 
void DialogStyleManager::OnStorageChange (wxCommandEvent &) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);

	StorageEdit->Enable(n == 1);
	StorageCopy->Enable(n == 1);
	StorageDelete->Enable(n > 0);
	MoveToLocal->Enable(n > 0);
	UpdateMoveButtons();
}

/// @brief Copy to Storage 
void DialogStyleManager::OnCopyToStorage (wxCommandEvent &) {
	using std::list;
	// Check if there is actually a storage
	if (!StorageNew->IsEnabled()) return;

	list<wxString> copied;
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);
	for (int i = 0; i < n; i++) {
		wxString styleName = CurrentList->GetString(selections[i]);
		bool addStyle = true;
		
		for (list<AssStyle *>::iterator style = Store.style.begin(); style != Store.style.end(); ++style) {
			if ((*style)->name.CmpNoCase(styleName) == 0) {
				addStyle = false;
				if (wxYES == wxMessageBox(wxString::Format(_T("There is already a style with the name \"%s\" on the current storage. Proceed and overwrite anyway?"),styleName.c_str()), _T("Style name collision."), wxYES_NO)) {
					**style = *styleMap.at(selections[i]);
					copied.push_back(styleName);
				}
				break;
			}
		}
		if (addStyle) {
			AssStyle *temp = new AssStyle(*styleMap.at(selections[i]));
			Store.style.push_back(temp);
			copied.push_back(styleName);
		}
	}
	Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
	LoadStorageStyles();
	for (list<wxString>::iterator name = copied.begin(); name != copied.end(); ++name) {
		StorageList->SetStringSelection(*name, true);
	}
	wxCommandEvent dummy;
	OnStorageChange(dummy);
}

/// @brief Copy to Current 
void DialogStyleManager::OnCopyToCurrent (wxCommandEvent &) {
	using std::list;
	list<wxString> copied;
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);
	for (int i = 0; i < n; i++) {
		wxString styleName = StorageList->GetString(selections[i]);
		bool addStyle = true;
		
		for (std::vector<AssStyle *>::iterator style = styleMap.begin(); style != styleMap.end(); ++style) {
			if ((*style)->name.CmpNoCase(styleName) == 0) {
				addStyle = false;
				if (wxYES == wxMessageBox(wxString::Format(_T("There is already a style with the name \"%s\" on the current script. Proceed and overwrite anyway?"),styleName.c_str()), _T("Style name collision."), wxYES_NO)) {
					**style = *styleStorageMap.at(selections[i]);
					copied.push_back(styleName);
				}
				break;
			}
		}
		if (addStyle) {
			AssStyle *temp = new AssStyle(*styleStorageMap.at(selections[i]));
			grid->ass->InsertStyle(temp);
			copied.push_back(styleName);
		}
	}
	LoadCurrentStyles(grid->ass);
	for (list<wxString>::iterator name = copied.begin(); name != copied.end(); ++name) {
		CurrentList->SetStringSelection(*name, true);
	}
	grid->ass->Commit(_("style copy"));
	grid->CommitChanges();
	wxCommandEvent dummy;
	OnCurrentChange(dummy);
}

/// @brief Storage make copy 
void DialogStyleManager::OnStorageCopy (wxCommandEvent &) {
	wxArrayInt selections;
	StorageList->GetSelections(selections);
	if (selections.size() == 0) return;
	AssStyle *temp = new AssStyle(*(styleStorageMap.at(selections[0])));

	wxString newName = _("Copy of ");
	newName += temp->name;
	temp->name = newName;

	DialogStyleEditor editor(this,temp,grid,false,&Store,true);
	int modified = editor.ShowModal();
	if (modified) {
		Store.style.push_back(temp);
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		LoadStorageStyles();
		StorageList->SetStringSelection(temp->name); // the copy/delete/copy-to-local buttons stay disabled after this?
	}
	else delete temp;
	UpdateMoveButtons();
}

/// @brief Current make copy 
void DialogStyleManager::OnCurrentCopy (wxCommandEvent &) {
	wxArrayInt selections;
	CurrentList->GetSelections(selections);
	if (selections.size() == 0) return;

	AssStyle *temp = new AssStyle(styleMap.at(selections[0])->GetEntryData());
	wxString newName = _("Copy of ");
	newName += temp->name;
	temp->name = newName;

	DialogStyleEditor editor(this,temp,grid,true,&Store,true);
	int modified = editor.ShowModal();
	if (modified) {
		grid->ass->InsertStyle(temp);
		LoadCurrentStyles(grid->ass);
		CurrentList->SetStringSelection(temp->name); // but even without this, the copy/delete/copy-to-storage buttons stay enabled?
	}
	else delete temp;

	grid->ass->Commit(_("style copy"));
	grid->CommitChanges();
	UpdateMoveButtons();
}

/// @brief Copy to clipboard 
/// @param list 
/// @param v    
void DialogStyleManager::CopyToClipboard (wxListBox *list, std::vector<AssStyle*> v) {
	wxString data = _T("");
	AssStyle *s;
	wxArrayInt selections;
	list->GetSelections(selections);

	for(int unsigned i=0;i<selections.size();i++) {
		if (i!=0) data += _T("\r\n");
		s = v.at(selections[i]);
		s->UpdateData();
		data += s->GetEntryData();
	}

	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(data));
		wxTheClipboard->Close();
	}
}

/// @brief Paste from clipboard 
void DialogStyleManager::PasteToCurrent() {
	wxString data = _T("");

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject rawdata;
			wxTheClipboard->GetData(rawdata);
			data = rawdata.GetText();
		}
		wxTheClipboard->Close();
	}

	wxStringTokenizer st(data,_T('\n'));
	while (st.HasMoreTokens()) {
		AssStyle *s = NULL;
		try {
			s = new AssStyle(st.GetNextToken().Trim(true));
			if (s->Valid) {
				while (grid->ass->GetStyle(s->name) != NULL)
					s->name = _T("Copy of ") + s->name;

				s->UpdateData();
				grid->ass->InsertStyle(s);
				LoadCurrentStyles(grid->ass);

				grid->ass->Commit(_("style paste"));
				grid->CommitChanges();
			}
			else
				wxMessageBox(_("Could not parse style"), _("Could not parse style"), wxOK | wxICON_EXCLAMATION , this);
		}
		catch (...) {
			delete s;
			wxMessageBox(_("Could not parse style"), _("Could not parse style"), wxOK | wxICON_EXCLAMATION , this);
		}

	}
}

/// @brief DOCME
void DialogStyleManager::PasteToStorage() {
	wxString data = _T("");

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject rawdata;
			wxTheClipboard->GetData(rawdata);
			data = rawdata.GetText();
		}
		wxTheClipboard->Close();
	}

	wxStringTokenizer st(data,_T('\n'));
	while (st.HasMoreTokens()) {
		AssStyle *s = NULL;
		try {
			s = new AssStyle(st.GetNextToken().Trim(true));
			if (s->Valid) {
				while (Store.GetStyle(s->name) != NULL)
					s->name = _T("Copy of ") + s->name;

				s->UpdateData();
				Store.style.push_back(s);
				Store.Save(CatalogList->GetString(CatalogList->GetSelection()));

				LoadStorageStyles();
				StorageList->SetStringSelection(s->name);
			}
			else
				wxMessageBox(_("Could not parse style"), _("Could not parse style"), wxOK | wxICON_EXCLAMATION , this);
		}
		catch(...) {
			delete s;
			wxMessageBox(_("Could not parse style"), _("Could not parse style"), wxOK | wxICON_EXCLAMATION , this);
		}
	}
}

/// @brief Storage new 
void DialogStyleManager::OnStorageNew (wxCommandEvent &) {
	AssStyle *temp = new AssStyle;

	DialogStyleEditor editor(this,temp,grid,false,&Store,true);
	int modified = editor.ShowModal();
	if (modified) {
		Store.style.push_back(temp);
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		LoadStorageStyles();
	}
	else delete temp;
	UpdateMoveButtons();
}

/// @brief Current new 
void DialogStyleManager::OnCurrentNew (wxCommandEvent &) {
	AssStyle *temp = new AssStyle;

	DialogStyleEditor editor(this,temp,grid,true,&Store,true);
	int modified = editor.ShowModal();
	if (modified) {
		grid->ass->InsertStyle(temp);
		LoadCurrentStyles(grid->ass);
	}
	else delete temp;
	UpdateMoveButtons();
}

/// @brief Storage delete 
void DialogStyleManager::OnStorageDelete (wxCommandEvent &) {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);

	wxString message;
	if (n!=1) {
		message = _("Are you sure you want to delete these ");
		message += wxString::Format(_T("%i"),n);
		message += _(" styles?");
	}
	else message = _("Are you sure you want to delete this style?");
	int option = wxMessageBox(message, _("Confirm delete from storage"), wxYES_NO | wxICON_EXCLAMATION , this);

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
		StorageEdit->Enable(false);
		StorageCopy->Enable(false);
		StorageDelete->Enable(false);
	}
	UpdateMoveButtons();
}

/// @brief Current delete 
void DialogStyleManager::OnCurrentDelete (wxCommandEvent &) {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);

	wxString message;
	if (n!=1) {
		message = _("Are you sure you want to delete these ");
		message += wxString::Format(_T("%i"),n);
		message += _(" styles?");
	}
	else message = _("Are you sure you want to delete this style?");
	int option = wxMessageBox(message, _("Confirm delete from current"), wxYES_NO | wxICON_EXCLAMATION , this);

	if (option == wxYES) {
		AssStyle *temp;
		for (int i=0;i<n;i++) {
			temp = styleMap.at(selections[i]);
			grid->ass->Line.remove(temp);
			delete temp;
		}
		LoadCurrentStyles(grid->ass);

		// Set buttons
		MoveToStorage->Enable(false);
		CurrentEdit->Enable(false);
		CurrentCopy->Enable(false);
		CurrentDelete->Enable(false);

		grid->ass->Commit(_("style delete"));
		grid->CommitChanges();
	}
	UpdateMoveButtons();
}

/// @brief Import styles from another script 
void DialogStyleManager::OnCurrentImport(wxCommandEvent &) {
	// Get file name
	wxString path = lagi_wxString(OPT_GET("Path/Last/Subtitles")->GetString());	
	wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),AssFile::GetWildcardList(0),wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (!filename.IsEmpty()) {
		// Save path
		wxFileName filepath(filename);
		OPT_SET("Path/Last/Subtitles")->SetString(STD_STR(filepath.GetPath()));

		try {
			// Load file
			AssFile temp;
			temp.Load(filename,_T(""),false);

			// Get styles
			wxArrayString styles = temp.GetStyles();
			if (styles.Count() == 0 || (styles.Count() == 1 && styles[0] == _T("Default"))) {
				wxMessageBox(_("The selected file has no available styles."),_("Error Importing Styles"),wxOK);
				return;
			}

			// Get selection
			wxArrayInt selections;
			int res = wxGetSelectedChoices(selections,_("Choose styles to import:"),_("Import Styles"),styles);
			if (res == -1 || selections.Count() == 0) return;
			bool modified = false;

			// Loop through selection
			for (unsigned int i=0;i<selections.Count();i++) {
				// Check if there is already a style with that name
				int test = CurrentList->FindString(styles[selections[i]], false);
				if (test != wxNOT_FOUND) {
					int answer = wxMessageBox(wxString::Format(_T("There is already a style with the name \"%s\" on the current script. Overwrite?"),styles[selections[i]].c_str()),_T("Style name collision."),wxYES_NO);
					if (answer == wxYES) {
						// Overwrite
						modified = true;
						// The GetString->FindString mess is a silly workaround for the fact that to vsfilter
						// (and the duplicate check a few lines above), style names aren't case sensitive, but to the
						// rest of Aegisub they are.
						*(grid->ass->GetStyle(CurrentList->GetString(CurrentList->FindString(styles[selections[i]], false)))) = *temp.GetStyle(styles[selections[i]]);
					}
					continue;
				}

				// Copy
				modified = true;
				AssStyle *tempStyle = new AssStyle;
				*tempStyle = *temp.GetStyle(styles[selections[i]]);
				grid->ass->InsertStyle(tempStyle);
			}

			// Update
			if (modified) {
				LoadCurrentStyles(grid->ass);
				grid->ass->Commit(_("style import"));
				grid->CommitChanges();
			}
		}
		catch (...) {
		}
	}
}

/// @brief Enable or disable the move buttons
void DialogStyleManager::UpdateMoveButtons() {
	// Get storage selection
	wxArrayInt sels;
	int n = StorageList->GetSelections(sels);
	int firstStor = -1;
	int lastStor = -1;
	if (n) {
		firstStor = sels[0];
		lastStor = sels[n-1];
	}
	int itemsStor = StorageList->GetCount();

	// Check if selection is continuous
	bool contStor = true;
	for (int i = 1; i < n; ++i) {
		if (sels[i] != sels[i-1]+1) {
			contStor = false;
			break;
		}
	}

	// Get current selection
	n = CurrentList->GetSelections(sels);
	int firstCurr = -1;
	int lastCurr = -1;
	if (n) {
		firstCurr = sels[0];
		lastCurr = sels[n-1];
	}
	int itemsCurr = CurrentList->GetCount();

	// Check if selection is continuous
	bool contCurr = true;
	for (int i = 1; i < n; ++i) {
		if (sels[i] != sels[i-1]+1) {
			contCurr = false;
			break;
		}
	}

	// Set values
	StorageMoveUp->Enable(contStor && firstStor > 0);
	StorageMoveTop->Enable(contStor && firstStor > 0);
	StorageMoveDown->Enable(contStor && lastStor != -1 && lastStor < itemsStor-1);
	StorageMoveBottom->Enable(contStor && lastStor != -1 && lastStor < itemsStor-1);
	StorageSort->Enable(itemsStor > 1);
	CurrentMoveUp->Enable(contCurr && firstCurr > 0);
	CurrentMoveTop->Enable(contCurr && firstCurr > 0);
	CurrentMoveDown->Enable(contCurr && lastCurr != -1 && lastCurr < itemsCurr-1);
	CurrentMoveBottom->Enable(contCurr && lastCurr != -1 && lastCurr < itemsCurr-1);
	CurrentSort->Enable(itemsCurr > 1);
}

void DialogStyleManager::OnStorageMoveUp (wxCommandEvent &) { MoveStyles(true,0); }
void DialogStyleManager::OnStorageMoveTop (wxCommandEvent &) { MoveStyles(true,1); }
void DialogStyleManager::OnStorageMoveDown (wxCommandEvent &) { MoveStyles(true,2); }
void DialogStyleManager::OnStorageMoveBottom (wxCommandEvent &) { MoveStyles(true,3); }
void DialogStyleManager::OnStorageSort (wxCommandEvent &) { MoveStyles(true,4); }
void DialogStyleManager::OnCurrentMoveUp (wxCommandEvent &) { MoveStyles(false,0); }
void DialogStyleManager::OnCurrentMoveTop (wxCommandEvent &) { MoveStyles(false,1); }
void DialogStyleManager::OnCurrentMoveDown (wxCommandEvent &) { MoveStyles(false,2); }
void DialogStyleManager::OnCurrentMoveBottom (wxCommandEvent &) { MoveStyles(false,3); }
void DialogStyleManager::OnCurrentSort (wxCommandEvent &) { MoveStyles(false,4); }

/// @brief Move function 
/// @param storage 
/// @param type    
void DialogStyleManager::MoveStyles(bool storage, int type) {
	// Variables
	AssFile *subs = grid->ass;
	wxListBox *list;
	if (storage) list = StorageList;
	else list = CurrentList;

	// Get selection
	wxArrayInt sels;
	int n = list->GetSelections(sels);
	int first = -1;;
	int last = -1;
	if (n) {
		first = sels[0];
		last = sels[n-1];
	}

	// Get total style count
	int nStyles = list->GetCount();

	// Get styles
	std::vector<AssStyle*> styls;
	std::vector<AssStyle*> *srcStyls;
	if (storage) srcStyls = &styleStorageMap;
	else srcStyls = &styleMap;

	// Move up
	if (type == 0) {
		for (int i=0;i<first-1;i++) styls.push_back(srcStyls->at(i));
		for (int i=first;i<=last;i++) styls.push_back(srcStyls->at(i));
		styls.push_back(srcStyls->at(first-1));
		for (int i=last+1;i<nStyles;i++) styls.push_back(srcStyls->at(i));
		first--;
		last--;
	}

	// Move to top
	if (type == 1) {
		for (int i=first;i<=last;i++) styls.push_back(srcStyls->at(i));
		for (int i=0;i<first;i++) styls.push_back(srcStyls->at(i));
		for (int i=last+1;i<nStyles;i++) styls.push_back(srcStyls->at(i));
		last = last-first;
		first = 0;
	}

	// Move down
	if (type == 2) {
		for (int i=0;i<first;i++) styls.push_back(srcStyls->at(i));
		styls.push_back(srcStyls->at(last+1));
		for (int i=first;i<=last;i++) styls.push_back(srcStyls->at(i));
		for (int i=last+2;i<nStyles;i++) styls.push_back(srcStyls->at(i));
		first++;
		last++;
	}

	// Move to bottom
	if (type == 3) {
		for (int i=0;i<first;i++) styls.push_back(srcStyls->at(i));
		for (int i=last+1;i<nStyles;i++) styls.push_back(srcStyls->at(i));
		for (int i=first;i<=last;i++) styls.push_back(srcStyls->at(i));
		first = nStyles-(last-first+1);
		last = nStyles-1;
	}

	// Sort
	if (type == 4) {
		// Get confirmation
		if (storage) {
			int res = wxMessageBox(_("Are you sure? This cannot be undone!"),_("Sort styles"),wxYES_NO);
			if (res == wxNO) return;
		}

		// Get sorted list
		wxArrayString styleNames;
		for (int i=0; i < nStyles; i++) styleNames.Add(srcStyls->at(i)->name);
		styleNames.Sort();

		std::list<AssStyle *> styles(srcStyls->begin(), srcStyls->end());
		for (int i = 0; i < nStyles; i++) {
			for (std::list<AssStyle *>::iterator style = styles.begin(); style != styles.end(); style++) {
				if ((*style)->name == styleNames[i]) {
					styls.push_back(*style);
					styles.erase(style);
					break;
				}
			}
		}

		// Zero selection
		first = 0;
		last = 0;
	}

	// Storage
	if (storage) {
		// Rewrite storage
		Store.style.clear();
		for (unsigned int i=0;i<styls.size();i++) Store.style.push_back(styls[i]);
		
		// Save storage
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
	}

	// Current
	else {
		// Replace styles
		entryIter next;
		int curn = 0;
		for (entryIter cur=subs->Line.begin();cur!=subs->Line.end();cur = next) {
			next = cur;
			next++;
			AssStyle *style = dynamic_cast<AssStyle*>(*cur);
			if (style) {
				subs->Line.insert(cur,styls[curn]);
				subs->Line.erase(cur);
				curn++;
			}
		}

		// Flag as modified
		grid->ass->Commit(_("style move"));
		grid->CommitChanges();
	}

	// Update
	for (int i=0;i<nStyles;i++) {
		list->SetString(i,styls[i]->name);
		if (i < first || i > last) list->Deselect(i);
		else list->Select(i);
	}

	// Set map
	*srcStyls = styls;

	// Update buttons
	UpdateMoveButtons();
}

/// @brief Keydown event 
void DialogStyleManager::OnKeyDown(wxKeyEvent &event) {
	wxCommandEvent evt;
	switch(event.GetKeyCode()) {
		case WXK_ESCAPE :
			OnClose(evt);
			break;

		case WXK_DELETE :
			if (wxWindow::FindFocus()==StorageList) {
				OnStorageDelete(evt);
			}
			else if (wxWindow::FindFocus()==CurrentList) {
				OnCurrentDelete(evt);
			}
			break;

		case 'C' :
		case 'c' :
#ifdef __APPLE__
			if (event.CmdDown()) {
#else
			if (event.ControlDown()) {
#endif
				if (wxWindow::FindFocus()==CurrentList) {
					CopyToClipboard(CurrentList,styleMap);
				}
				else if (wxWindow::FindFocus()==StorageList) {
					CopyToClipboard(StorageList,styleStorageMap);
				}
			}
			break;

		case 'V' :
		case 'v' :
#ifdef __APPLE__
			if (event.CmdDown()) {
#else
			if (event.ControlDown()) {
#endif
				if (wxWindow::FindFocus()==CurrentList) {
					PasteToCurrent();
				}
				else if (wxWindow::FindFocus()==StorageList) {
					PasteToStorage();
				}
			}

			break;
	}
}

/// DOCME
int DialogStyleManager::lastx = -1;

/// DOCME
int DialogStyleManager::lasty = -1;

/// @brief DialogStyleManagerEvent stuff 
/// @param ctrl 
DialogStyleManagerEvent::DialogStyleManagerEvent(DialogStyleManager *ctrl) {
	control = ctrl;
}
BEGIN_EVENT_TABLE(DialogStyleManagerEvent, wxEvtHandler)
	EVT_KEY_DOWN(DialogStyleManagerEvent::OnKeyDown)
END_EVENT_TABLE()

void DialogStyleManagerEvent::OnKeyDown(wxKeyEvent &event) {
	control->OnKeyDown(event); //we need to access controls, so rather than make the controls public...
}
