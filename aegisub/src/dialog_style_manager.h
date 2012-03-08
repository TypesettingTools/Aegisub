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
	agi::Context *c; ///< Project context
	agi::scoped_ptr<PersistLocation> persist;

	/// Styles in the current subtitle file
	std::vector<AssStyle*> styleMap;

	/// Style storage manager
	AssStyleStorage Store;

	wxComboBox *CatalogList;
	wxListBox *StorageList;
	wxListBox *CurrentList;

	wxButton *CatalogDelete;

	wxButton *MoveToLocal;
	wxButton *MoveToStorage;

	wxButton *StorageNew;
	wxButton *StorageEdit;
	wxButton *StorageCopy;
	wxButton *StorageDelete;
	wxButton *StorageMoveUp;
	wxButton *StorageMoveDown;
	wxButton *StorageMoveTop;
	wxButton *StorageMoveBottom;
	wxButton *StorageSort;

	wxButton *CurrentNew;
	wxButton *CurrentEdit;
	wxButton *CurrentCopy;
	wxButton *CurrentDelete;
	wxButton *CurrentMoveUp;
	wxButton *CurrentMoveDown;
	wxButton *CurrentMoveTop;
	wxButton *CurrentMoveBottom;
	wxButton *CurrentSort;

	/// Load the list of available storages
	void LoadCatalog();
	/// Load the style list from the subtitles file
	void LoadCurrentStyles(AssFile *subs);
	/// Enable/disable all of the buttons as appropriate
	void UpdateButtons();
	/// Move styles up or down
	/// @param storage Storage or current file styles
	/// @param type 0: up; 1: top; 2: down; 3: bottom; 4: sort
	void MoveStyles(bool storage, int type);

	/// Save the storage and update the view after a change
	void UpdateStorage();

	void OnChangeCatalog();
	void OnCatalogNew();
	void OnCatalogDelete();
	void OnStorageEdit();
	void OnCurrentEdit();
	void OnCopyToStorage();
	void OnCopyToCurrent();
	void OnStorageCopy();
	void OnCurrentCopy();
	void OnStorageNew();
	void OnCurrentNew();
	void OnStorageDelete();
	void OnCurrentDelete();
	void OnCurrentImport();
	void OnKeyDown(wxKeyEvent &event);
	void PasteToCurrent();
	void PasteToStorage();

	template<class T>
	void CopyToClipboard(wxListBox *list, T const& v);

public:
	DialogStyleManager(agi::Context *context);
	~DialogStyleManager();
};
