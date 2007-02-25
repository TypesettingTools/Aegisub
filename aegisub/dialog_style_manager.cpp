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
#include <wx/filedlg.h>
#include <wx/choicdlg.h>
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
	wxSizer *StorageButtonsLow = new wxBoxSizer(wxHORIZONTAL);
	MoveToLocal = new wxButton(this, BUTTON_STORAGE_COPYTO, _("Copy to current script ->"), wxDefaultPosition, wxSize(205,25));
	StorageNew = new wxButton(this, BUTTON_STORAGE_NEW, _("New"), wxDefaultPosition, wxSize(40,25));
	StorageEdit = new wxButton(this, BUTTON_STORAGE_EDIT, _("Edit"), wxDefaultPosition, wxSize(40,25));
	StorageCopy = new wxButton(this, BUTTON_STORAGE_COPY, _("Copy"), wxDefaultPosition, wxSize(40,25));
	StorageDelete = new wxButton(this, BUTTON_STORAGE_DELETE, _("Delete"), wxDefaultPosition, wxSize(40,25));
	StorageButtons->Add(StorageNew,1,wxEXPAND | wxALL,0);
	StorageButtons->Add(StorageEdit,1,wxEXPAND | wxALL,0);
	StorageButtons->Add(StorageCopy,1,wxEXPAND | wxALL,0);
	StorageButtons->Add(StorageDelete,1,wxEXPAND | wxALL,0);
	StorageMoveUp = new wxButton(this, BUTTON_STORAGE_UP, _("Up"), wxDefaultPosition, wxSize(40,25));
	StorageMoveDown = new wxButton(this, BUTTON_STORAGE_DOWN, _("Down"), wxDefaultPosition, wxSize(40,25));
	StorageMoveTop = new wxButton(this, BUTTON_STORAGE_TOP, _("Top"), wxDefaultPosition, wxSize(40,25));
	StorageMoveBottom = new wxButton(this, BUTTON_STORAGE_BOTTOM, _("Bottom"), wxDefaultPosition, wxSize(40,25));
	StorageSort = new wxButton(this, BUTTON_STORAGE_SORT, _("Sort"), wxDefaultPosition, wxSize(40,25));
	StorageButtonsLow->Add(StorageMoveTop,1,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageMoveUp,1,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageMoveDown,1,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageMoveBottom,1,wxEXPAND | wxALL,0);
	StorageButtonsLow->Add(StorageSort,1,wxEXPAND | wxALL,0);
	StorageBox->Add(StorageList,0,wxEXPAND | wxALL,0);
	StorageBox->Add(MoveToLocal,0,wxEXPAND | wxALL,0);
	StorageBox->Add(StorageButtons,0,wxEXPAND | wxALL,0);
	StorageBox->Add(StorageButtonsLow,0,wxEXPAND | wxALL,0);
	MoveToLocal->Disable();
	StorageEdit->Disable();
	StorageCopy->Disable();
	StorageDelete->Disable();

	// Local styles list
	CurrentList = new wxListBox(this, LIST_CURRENT, wxDefaultPosition, wxSize(205,250), 0, NULL, wxLB_EXTENDED);
	wxSizer *CurrentBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Current script"));
	wxSizer *CurrentButtons = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *CurrentButtonsLow = new wxBoxSizer(wxHORIZONTAL);
	MoveToStorage = new wxButton(this, BUTTON_CURRENT_COPYTO, _("<- Copy to storage"), wxDefaultPosition, wxSize(205,25));
	CurrentNew = new wxButton(this, BUTTON_CURRENT_NEW, _("New"), wxDefaultPosition, wxSize(40,25));
	CurrentEdit = new wxButton(this, BUTTON_CURRENT_EDIT, _("Edit"), wxDefaultPosition, wxSize(40,25));
	CurrentCopy = new wxButton(this, BUTTON_CURRENT_COPY, _("Copy"), wxDefaultPosition, wxSize(40,25));
	CurrentDelete = new wxButton(this, BUTTON_CURRENT_DELETE, _("Delete"), wxDefaultPosition, wxSize(40,25));
	CurrentButtons->Add(CurrentNew,1,wxEXPAND | wxALL,0);
	CurrentButtons->Add(CurrentEdit,1,wxEXPAND | wxALL,0);
	CurrentButtons->Add(CurrentCopy,1,wxEXPAND | wxALL,0);
	CurrentButtons->Add(CurrentDelete,1,wxEXPAND | wxALL,0);
	CurrentMoveUp = new wxButton(this, BUTTON_CURRENT_UP, _("Up"), wxDefaultPosition, wxSize(40,25));
	CurrentMoveDown = new wxButton(this, BUTTON_CURRENT_DOWN, _("Down"), wxDefaultPosition, wxSize(40,25));
	CurrentMoveTop = new wxButton(this, BUTTON_CURRENT_TOP, _("Top"), wxDefaultPosition, wxSize(40,25));
	CurrentMoveBottom = new wxButton(this, BUTTON_CURRENT_BOTTOM, _("Bottom"), wxDefaultPosition, wxSize(40,25));
	CurrentSort = new wxButton(this, BUTTON_CURRENT_SORT, _("Sort"), wxDefaultPosition, wxSize(40,25));
	CurrentButtonsLow->Add(CurrentMoveTop,1,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentMoveUp,1,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentMoveDown,1,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentMoveBottom,1,wxEXPAND | wxALL,0);
	CurrentButtonsLow->Add(CurrentSort,1,wxEXPAND | wxALL,0);
	CurrentBox->Add(CurrentList,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(MoveToStorage,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(CurrentButtons,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(CurrentButtonsLow,0,wxEXPAND | wxALL,0);
	CurrentBox->Add(new wxButton(this, BUTTON_CURRENT_IMPORT, _("Import from script...")),0,wxEXPAND);
	MoveToStorage->Disable();
	CurrentEdit->Disable();
	CurrentCopy->Disable();
	CurrentDelete->Disable();

	// General layout
	wxSizer *StylesSizer = new wxBoxSizer(wxHORIZONTAL);
	StylesSizer->Add(StorageBox,0,wxRIGHT | wxEXPAND,5);
	StylesSizer->Add(CurrentBox,0,wxLEFT | wxEXPAND,0);
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

	//Set key handlers for lists
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
	UpdateMoveButtons();
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
			int answer = wxMessageBox(wxString::Format(_T("There is already a style with the name \"%s\" on the current script. Proceed anyway?"),StorageList->GetString(selections[i]).c_str()),_T("Style name collision."),wxYES_NO);
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
	grid->ass->FlagAsModified(_("style copy"));
	grid->CommitChanges();
	UpdateMoveButtons();
}


/////////////////////
// Storage make copy
void DialogStyleManager::OnStorageCopy (wxCommandEvent &event) {
	wxArrayInt selections;
	StorageList->GetSelections(selections);
	AssStyle *temp = new AssStyle;

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
	UpdateMoveButtons();
}


/////////////////////
// Current make copy
void DialogStyleManager::OnCurrentCopy (wxCommandEvent &event) {
	wxArrayInt selections;
	CurrentList->GetSelections(selections);

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

	grid->ass->FlagAsModified(_("style copy"));
	grid->CommitChanges();
	UpdateMoveButtons();
}


//////////////////////
// Copy to clipboard
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
////////////////////////
// Paste from clipboard
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
		AssStyle *s = new AssStyle(st.GetNextToken().Trim(true));
		if (s->Valid) {
			AssFile::top->InsertStyle(s);
			LoadCurrentStyles(AssFile::top);
		}
		grid->ass->FlagAsModified(_("style paste"));
		grid->CommitChanges();
	}
}
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
		AssStyle *s = new AssStyle(st.GetNextToken().Trim(true));
		if (s->Valid) {
			Store.style.push_back(s);
			Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		}
		LoadStorageStyles();
		StorageList->SetStringSelection(s->name);
	}
}
///////////////
// Storage new
void DialogStyleManager::OnStorageNew (wxCommandEvent &event) {
	AssStyle *temp = new AssStyle;

	DialogStyleEditor editor(this,temp,grid);
	int modified = editor.ShowModal();
	if (modified) {
		Store.style.push_back(temp);
		Store.Save(CatalogList->GetString(CatalogList->GetSelection()));
		LoadStorageStyles();
	}
	else delete temp;
	UpdateMoveButtons();
}


///////////////
// Current new
void DialogStyleManager::OnCurrentNew (wxCommandEvent &event) {
	AssStyle *temp = new AssStyle;

	DialogStyleEditor editor(this,temp,grid);
	int modified = editor.ShowModal();
	if (modified) {
		AssFile::top->InsertStyle(temp);
		LoadCurrentStyles(AssFile::top);
	}
	else delete temp;
	UpdateMoveButtons();
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
		StorageCopy->Enable(false);
		StorageDelete->Enable(false);
	}
	UpdateMoveButtons();
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
		CurrentCopy->Enable(false);
		CurrentDelete->Enable(false);

		grid->ass->FlagAsModified(_("style delete"));
		grid->CommitChanges();
	}
	UpdateMoveButtons();
}


/////////////////////////////////////
// Import styles from another script
void DialogStyleManager::OnCurrentImport(wxCommandEvent &event) {
	// Get file name
	wxString path = Options.AsText(_T("Last open subtitles path"));	
	wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),AssFile::GetWildcardList(0),wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (!filename.IsEmpty()) {
		// Save path
		wxFileName filepath(filename);
		Options.SetText(_T("Last open subtitles path"), filepath.GetPath());
		Options.Save();

		try {
			// Load file
			AssFile temp;
			temp.Load(filename,_T(""),false);

			// Get styles
			wxArrayString styles = temp.GetStyles();
			if (styles.Count() == 0 || (styles.Count() == 1 && styles[0] == _T("Default"))) {
				wxMessageBox(_("There selected file has no available styles."),_("Error Importing Styles"),wxOK);
				return;
			}

			// Get selection
			wxArrayInt selections;
			int res = wxGetMultipleChoices(selections,_("Choose styles to import:"),_("Import Styles"),styles);
			if (res == -1 || selections.Count() == 0) return;
			bool modified = false;

			// Loop through selection
			for (unsigned int i=0;i<selections.Count();i++) {
				// Check if there is already a style with that name
				int test = CurrentList->FindString(styles[selections[i]]);
				if (test != wxNOT_FOUND) {
					int answer = wxMessageBox(wxString::Format(_T("There is already a style with the name \"%s\" on the current script. Proceed anyway?"),styles[selections[i]].c_str()),_T("Style name collision."),wxYES_NO);
					if (answer != wxYES) continue;
				}

				// Copy
				modified = true;
				AssStyle *tempStyle = new AssStyle;
				*tempStyle = *temp.GetStyle(styles[selections[i]]);
				AssFile::top->InsertStyle(tempStyle);
			}

			// Update
			if (modified) {
				LoadCurrentStyles(grid->ass);
				grid->ass->FlagAsModified(_("style import"));
				grid->CommitChanges();
			}
		}
		catch (...) {
		}
	}
}


///////////////////////
// Update move buttons
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
	if (n) {
		int last = sels[0];
		for (int i=1;i<n;i++) {
			if (sels[i] != last+1) {
				contStor = false;
				break;
			}
			last = sels[i];
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
	if (n) {
		int last = sels[0];
		for (int i=1;i<n;i++) {
			if (sels[i] != last+1) {
				contCurr = false;
				break;
			}
			last = sels[i];
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


///////////////
// Move events
void DialogStyleManager::OnStorageMoveUp (wxCommandEvent &event) { MoveStyles(true,0); }
void DialogStyleManager::OnStorageMoveTop (wxCommandEvent &event) { MoveStyles(true,1); }
void DialogStyleManager::OnStorageMoveDown (wxCommandEvent &event) { MoveStyles(true,2); }
void DialogStyleManager::OnStorageMoveBottom (wxCommandEvent &event) { MoveStyles(true,3); }
void DialogStyleManager::OnStorageSort (wxCommandEvent &event) { MoveStyles(true,4); }
void DialogStyleManager::OnCurrentMoveUp (wxCommandEvent &event) { MoveStyles(false,0); }
void DialogStyleManager::OnCurrentMoveTop (wxCommandEvent &event) { MoveStyles(false,1); }
void DialogStyleManager::OnCurrentMoveDown (wxCommandEvent &event) { MoveStyles(false,2); }
void DialogStyleManager::OnCurrentMoveBottom (wxCommandEvent &event) { MoveStyles(false,3); }
void DialogStyleManager::OnCurrentSort (wxCommandEvent &event) { MoveStyles(false,4); }

/////////////////
// Move function
void DialogStyleManager::MoveStyles(bool storage, int type) {
	// Variables
	AssFile *subs = AssFile::top;
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
		wxArrayString stylNames;
		for (int i=0;i<nStyles;i++) stylNames.Add(srcStyls->at(i)->name.Lower());
		stylNames.Sort();
		AssStyle *curStyl;

		// Find each and copy it
		for (int i=0;i<nStyles;i++) {
			for (int j=0;j<nStyles;j++) {
				curStyl = srcStyls->at(j);
				if (curStyl->name.Lower() == stylNames[i]) {
					styls.push_back(curStyl);
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
			AssStyle *style = AssEntry::GetAsStyle(*cur);
			if (style) {
				subs->Line.insert(cur,styls[curn]);
				subs->Line.erase(cur);
				curn++;
			}
		}

		// Flag as modified
		grid->ass->FlagAsModified(_("style move"));
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


//////////////////
// Keydown event
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
			if (event.ControlDown()) {
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
			if (event.ControlDown()) {
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
//////////////////
// I have no clue
int DialogStyleManager::lastx = -1;
int DialogStyleManager::lasty = -1;


/////////////////////////////////
// DialogStyleManagerEvent stuff
DialogStyleManagerEvent::DialogStyleManagerEvent(DialogStyleManager *ctrl) {
	control = ctrl;
}
BEGIN_EVENT_TABLE(DialogStyleManagerEvent, wxEvtHandler)
	EVT_KEY_DOWN(DialogStyleManagerEvent::OnKeyDown)
END_EVENT_TABLE()
void DialogStyleManagerEvent::OnKeyDown(wxKeyEvent &event) {
	control->OnKeyDown(event); //we need to access controls, so rather than make the controls public...
}


