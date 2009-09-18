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

/// @file dialog_style_manager.h
/// @see dialog_style_manager.cpp
/// @ingroup style_editor
///


#pragma once


////////////
// Includes
#ifndef AGI_PRE
#include <vector>

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#endif

#include "ass_style_storage.h"


//////////////
// Prototypes
class AssFile;
class AssStyle;
class SubtitlesGrid;



/// DOCME
/// @class DialogStyleManager
/// @brief DOCME
///
/// DOCME
class DialogStyleManager : public wxDialog {
private:

	/// DOCME
	std::vector<AssStyle*> styleMap;

	/// DOCME
	std::vector<AssStyle*> styleStorageMap;


	/// DOCME
	SubtitlesGrid *grid;


	/// DOCME
	wxComboBox *CatalogList;

	/// DOCME
	wxListBox *StorageList;

	/// DOCME
	wxListBox *CurrentList;

	/// DOCME
	wxButton *MoveToLocal;

	/// DOCME
	wxButton *StorageNew;

	/// DOCME
	wxButton *StorageEdit;

	/// DOCME
	wxButton *StorageCopy;

	/// DOCME
	wxButton *StorageDelete;

	/// DOCME
	wxButton *StorageMoveUp;

	/// DOCME
	wxButton *StorageMoveDown;

	/// DOCME
	wxButton *StorageMoveTop;

	/// DOCME
	wxButton *StorageMoveBottom;

	/// DOCME
	wxButton *StorageSort;

	/// DOCME
	wxButton *MoveToStorage;

	/// DOCME
	wxButton *CurrentNew;

	/// DOCME
	wxButton *CurrentEdit;

	/// DOCME
	wxButton *CurrentCopy;

	/// DOCME
	wxButton *CurrentDelete;

	/// DOCME
	wxButton *CurrentMoveUp;

	/// DOCME
	wxButton *CurrentMoveDown;

	/// DOCME
	wxButton *CurrentMoveTop;

	/// DOCME
	wxButton *CurrentMoveBottom;

	/// DOCME
	wxButton *CurrentSort;


	/// DOCME
	AssStyleStorage Store;

	void StorageActions (bool state);
	void LoadCatalog ();
	void LoadCurrentStyles (AssFile *subs);
	void LoadStorageStyles ();
	void UpdateMoveButtons();
	void MoveStyles(bool storage,int type);


	/// DOCME

	/// DOCME
	static int lastx, lasty;

public:

	/// DOCME
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

	/// DOCME
	BUTTON_CATALOG_NEW = 1000,

	/// DOCME
	BUTTON_CATALOG_DELETE,

	/// DOCME
	BUTTON_STORAGE_COPYTO,

	/// DOCME
	BUTTON_STORAGE_NEW,

	/// DOCME
	BUTTON_STORAGE_EDIT,

	/// DOCME
	BUTTON_STORAGE_COPY,

	/// DOCME
	BUTTON_STORAGE_DELETE,

	/// DOCME
	BUTTON_STORAGE_UP,

	/// DOCME
	BUTTON_STORAGE_DOWN,

	/// DOCME
	BUTTON_STORAGE_TOP,

	/// DOCME
	BUTTON_STORAGE_BOTTOM,

	/// DOCME
	BUTTON_STORAGE_SORT,

	/// DOCME
	BUTTON_CURRENT_COPYTO,

	/// DOCME
	BUTTON_CURRENT_NEW,

	/// DOCME
	BUTTON_CURRENT_EDIT,

	/// DOCME
	BUTTON_CURRENT_COPY,

	/// DOCME
	BUTTON_CURRENT_DELETE,

	/// DOCME
	BUTTON_CURRENT_IMPORT,

	/// DOCME
	BUTTON_CURRENT_UP,

	/// DOCME
	BUTTON_CURRENT_DOWN,

	/// DOCME
	BUTTON_CURRENT_TOP,

	/// DOCME
	BUTTON_CURRENT_BOTTOM,

	/// DOCME
	BUTTON_CURRENT_SORT,

	/// DOCME
	LIST_CATALOG,

	/// DOCME
	LIST_STORAGE,

	/// DOCME
	LIST_CURRENT
};



/// DOCME
/// @class DialogStyleManagerEvent
/// @brief DOCME
///
/// DOCME
class DialogStyleManagerEvent : public wxEvtHandler {
private:

	/// DOCME
	DialogStyleManager *control;
	void OnKeyDown(wxKeyEvent &event);

public:
	DialogStyleManagerEvent(DialogStyleManager *control);
	DECLARE_EVENT_TABLE()
};
