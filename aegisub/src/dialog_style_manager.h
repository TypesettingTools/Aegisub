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

#ifndef AGI_PRE
#include <vector>

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#endif

#include <libaegisub/scoped_ptr.h>
#include "ass_style_storage.h"

namespace agi { struct Context; }
class AssFile;
class AssStyle;
class PersistLocation;

/// DOCME
/// @class DialogStyleManager
/// @brief DOCME
///
/// DOCME
class DialogStyleManager : public wxDialog {
	agi::Context *c;
	agi::scoped_ptr<PersistLocation> persist;

	/// DOCME
	std::vector<AssStyle*> styleMap;

	/// DOCME
	std::vector<AssStyle*> styleStorageMap;

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


	/// DOCME
	AssStyleStorage Store;

	void StorageActions (bool state);
	void LoadCatalog ();
	void LoadCurrentStyles (AssFile *subs);
	void LoadStorageStyles ();
	void UpdateMoveButtons();
	void MoveStyles(bool storage,int type);

	/// DOCME
	wxSizer *MainSizer;

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

public:
	DialogStyleManager(agi::Context *context);
	~DialogStyleManager();

	DECLARE_EVENT_TABLE()
};
