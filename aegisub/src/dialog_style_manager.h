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


#ifndef DIALOG_STYLE_MANAGER_H
#define DIALOG_STYLE_MANAGER_H


////////////
// Includes
#include <wx/wxprec.h>
#include <wx/dialog.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <vector>
#include "ass_style_storage.h"


//////////////
// Prototypes
class AssFile;
class AssStyle;
class SubtitlesGrid;


/////////////////
// Manager Class
class DialogStyleManager : public wxDialog {
private:
	std::vector<AssStyle*> styleMap;
	std::vector<AssStyle*> styleStorageMap;

	SubtitlesGrid *grid;

	wxComboBox *CatalogList;
	wxListBox *StorageList;
	wxListBox *CurrentList;
	wxButton *MoveToLocal;
	wxButton *StorageNew;
	wxButton *StorageEdit;
	wxButton *StorageCopy;
	wxButton *StorageDelete;
	wxButton *StorageMoveUp;
	wxButton *StorageMoveDown;
	wxButton *StorageMoveTop;
	wxButton *StorageMoveBottom;
	wxButton *StorageSort;
	wxButton *MoveToStorage;
	wxButton *CurrentNew;
	wxButton *CurrentEdit;
	wxButton *CurrentCopy;
	wxButton *CurrentDelete;
	wxButton *CurrentMoveUp;
	wxButton *CurrentMoveDown;
	wxButton *CurrentMoveTop;
	wxButton *CurrentMoveBottom;
	wxButton *CurrentSort;

	AssStyleStorage Store;

	void StorageActions (bool state);
	void LoadCatalog ();
	void LoadCurrentStyles (AssFile *subs);
	void LoadStorageStyles ();
	void UpdateMoveButtons();
	void MoveStyles(bool storage,int type);

	static int lastx, lasty;

public:
	wxSizer *MainSizer;

	DialogStyleManager(wxWindow *parent,SubtitlesGrid *grid);
	~DialogStyleManager();

	void OnClose (wxCommandEvent &event);
	void OnChangeCatalog (wxCommandEvent &event);
	void OnCatalogNew (wxCommandEvent &event);
	void OnCatalogDelete (wxCommandEvent &event);
	void OnStorageEdit (wxCommandEvent &event);
	void OnCurrentEdit (wxCommandEvent &event);
	void OnCurrentMoveUp (wxCommandEvent &event);
	void OnCurrentMoveDown (wxCommandEvent &event);
	void OnCurrentMoveTop (wxCommandEvent &event);
	void OnCurrentMoveBottom (wxCommandEvent &event);
	void OnCurrentSort (wxCommandEvent &event);
	void OnStorageChange (wxCommandEvent &event);
	void OnCurrentChange (wxCommandEvent &event);
	void OnCopyToStorage (wxCommandEvent &event);
	void OnCopyToCurrent (wxCommandEvent &event);
	void OnStorageCopy (wxCommandEvent &event);
	void OnCurrentCopy (wxCommandEvent &event);
	void OnStorageNew (wxCommandEvent &event);
	void OnCurrentNew (wxCommandEvent &event);
	void OnStorageMoveUp (wxCommandEvent &event);
	void OnStorageMoveDown (wxCommandEvent &event);
	void OnStorageMoveTop (wxCommandEvent &event);
	void OnStorageMoveBottom (wxCommandEvent &event);
	void OnStorageSort (wxCommandEvent &event);
	void OnStorageDelete (wxCommandEvent &event);
	void OnCurrentDelete (wxCommandEvent &event);
	void OnCurrentImport (wxCommandEvent &event);
	void OnKeyDown (wxKeyEvent &event);
	void CopyToClipboard (wxListBox *list, std::vector<AssStyle*> v);
	void PasteToCurrent();
	void PasteToStorage();


	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	BUTTON_CATALOG_NEW = 1000,
	BUTTON_CATALOG_DELETE,
	BUTTON_STORAGE_COPYTO,
	BUTTON_STORAGE_NEW,
	BUTTON_STORAGE_EDIT,
	BUTTON_STORAGE_COPY,
	BUTTON_STORAGE_DELETE,
	BUTTON_STORAGE_UP,
	BUTTON_STORAGE_DOWN,
	BUTTON_STORAGE_TOP,
	BUTTON_STORAGE_BOTTOM,
	BUTTON_STORAGE_SORT,
	BUTTON_CURRENT_COPYTO,
	BUTTON_CURRENT_NEW,
	BUTTON_CURRENT_EDIT,
	BUTTON_CURRENT_COPY,
	BUTTON_CURRENT_DELETE,
	BUTTON_CURRENT_IMPORT,
	BUTTON_CURRENT_UP,
	BUTTON_CURRENT_DOWN,
	BUTTON_CURRENT_TOP,
	BUTTON_CURRENT_BOTTOM,
	BUTTON_CURRENT_SORT,
	LIST_CATALOG,
	LIST_STORAGE,
	LIST_CURRENT
};


/////////////////
// Event handler
class DialogStyleManagerEvent : public wxEvtHandler {
private:
	DialogStyleManager *control;
	void OnKeyDown(wxKeyEvent &event);

public:
	DialogStyleManagerEvent(DialogStyleManager *control);
	DECLARE_EVENT_TABLE()
};


#endif
