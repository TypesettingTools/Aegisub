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

#include "dialog_style_manager.h"

#ifndef AGI_PRE
#include <tr1/functional>

#include <wx/bmpbuttn.h>
#include <wx/clipbrd.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/tokenzr.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "dialog_selected_choices.h"
#include "dialog_style_editor.h"
#include "include/aegisub/context.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "persist_location.h"
#include "selection_controller.h"
#include "standard_paths.h"
#include "utils.h"

using std::tr1::placeholders::_1;

namespace {

wxBitmapButton *add_bitmap_button(wxWindow *parent, wxSizer *sizer, wxBitmap const& img, wxString const& tooltip) {
	wxBitmapButton *btn = new wxBitmapButton(parent, -1, img);
	btn->SetToolTip(tooltip);
	sizer->Add(btn, wxSizerFlags().Expand());
	return btn;
}

wxSizer *make_move_buttons(wxWindow *parent, wxButton **up, wxButton **down, wxButton **top, wxButton **bottom, wxButton **sort) {
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->AddStretchSpacer(1);

	*up     = add_bitmap_button(parent, sizer, GETIMAGE(arrow_up_24), _("Move style up"));
	*down   = add_bitmap_button(parent, sizer, GETIMAGE(arrow_down_24), _("Move style down"));
	*top    = add_bitmap_button(parent, sizer, GETIMAGE(arrow_up_stop_24), _("Move style to top"));
	*bottom = add_bitmap_button(parent, sizer, GETIMAGE(arrow_down_stop_24), _("Move style to bottom"));
	*sort   = add_bitmap_button(parent, sizer, GETIMAGE(arrow_sort_24), _("Sort styles alphabetically"));

	sizer->AddStretchSpacer(1);
	return sizer;
}

wxSizer *make_edit_buttons(wxWindow *parent, wxString move_label, wxButton **move, wxButton **nw, wxButton **edit, wxButton **copy, wxButton **del) {
	wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

	*move = new wxButton(parent, -1, move_label);
	*nw = new wxButton(parent, -1, _("&New"));
	*edit = new wxButton(parent, -1, _("&Edit"));
	*copy = new wxButton(parent, -1, _("&Copy"));
	*del = new wxButton(parent, -1, _("&Delete"));

	sizer->Add(*nw, wxSizerFlags(1).Expand().Border(wxRIGHT));
	sizer->Add(*edit, wxSizerFlags(1).Expand().Border(wxRIGHT));
	sizer->Add(*copy, wxSizerFlags(1).Expand().Border(wxRIGHT));
	sizer->Add(*del, wxSizerFlags(1).Expand());

	return sizer;
}

template<class Func>
wxString unique_name(Func name_checker, wxString const& source_name) {
	if (name_checker(source_name)) {
		wxString name = wxString::Format(_("%s - Copy"), source_name);
		for (int i = 2; name_checker(name); ++i)
			name = wxString::Format(_("%s - Copy (%d)"), source_name, i);
		return name;
	}
	return source_name;
}

wxString get_clipboard_text() {
	wxString text;
	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject rawdata;
			wxTheClipboard->GetData(rawdata);
			text = rawdata.GetText();
		}
		wxTheClipboard->Close();
	}
	return text;
}

template<class Func1, class Func2>
void add_styles(Func1 name_checker, Func2 style_adder) {
	wxStringTokenizer st(get_clipboard_text(), '\n');
	while (st.HasMoreTokens()) {
		try {
			AssStyle *s = new AssStyle(st.GetNextToken().Trim(true));
			s->name = unique_name(name_checker, s->name);
			style_adder(s);
		}
		catch (...) {
			wxMessageBox(_("Could not parse style"), _("Could not parse style"), wxOK | wxICON_EXCLAMATION);
		}
	}
}

int confirm_delete(int n, wxWindow *parent, wxString const& title) {
	wxString message = n == 1 ?
		_("Are you sure you want to delete this style?") :
		wxString::Format(_("Are you sure you want to delete these %d styles?"), n);
	return wxMessageBox(message, title, wxYES_NO | wxICON_EXCLAMATION, parent);
}

int get_single_sel(wxListBox *lb) {
	wxArrayInt selections;
	int n = lb->GetSelections(selections);
	return n == 1 ? selections[0] : -1;
}

}

DialogStyleManager::DialogStyleManager(agi::Context *context)
: wxDialog(context->parent, -1, _("Styles Manager"))
, c(context)
, commit_connection(c->ass->AddCommitListener(&DialogStyleManager::LoadCurrentStyles, this))
{
	using std::tr1::bind;
	SetIcon(BitmapToIcon(GETIMAGE(style_toolbutton_24)));

	// Catalog
	wxSizer *CatalogBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Catalog of available storages"));
	CatalogList = new wxComboBox(this,-1, "", wxDefaultPosition, wxSize(-1,-1), 0, NULL, wxCB_READONLY);
	wxButton *CatalogNew = new wxButton(this, -1, _("New"));
	CatalogDelete = new wxButton(this, -1, _("Delete"));
	CatalogBox->Add(CatalogList,1,wxEXPAND | wxRIGHT | wxALIGN_RIGHT,5);
	CatalogBox->Add(CatalogNew,0,wxRIGHT,5);
	CatalogBox->Add(CatalogDelete,0,0,0);

	// Storage styles list
	wxSizer *StorageButtons = make_edit_buttons(this, _("Copy to &current script ->"), &MoveToLocal, &StorageNew, &StorageEdit, &StorageCopy, &StorageDelete);

	wxSizer *StorageListSizer = new wxBoxSizer(wxHORIZONTAL);
	StorageList = new wxListBox(this, -1, wxDefaultPosition, wxSize(240,250), 0, NULL, wxLB_EXTENDED);
	StorageListSizer->Add(StorageList,1,wxEXPAND | wxRIGHT,0);
	StorageListSizer->Add(make_move_buttons(this, &StorageMoveUp, &StorageMoveDown, &StorageMoveTop, &StorageMoveBottom, &StorageSort), wxSizerFlags().Expand());

	wxSizer *StorageBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Storage"));
	StorageBox->Add(StorageListSizer,1,wxEXPAND | wxBOTTOM,5);
	StorageBox->Add(MoveToLocal,0,wxEXPAND | wxBOTTOM,5);
	StorageBox->Add(StorageButtons,0,wxEXPAND | wxBOTTOM,0);

	// Local styles list
	wxButton *CurrentImport = new wxButton(this, -1, _("&Import from script..."));
	wxSizer *CurrentButtons = make_edit_buttons(this, _("<- Copy to &storage"), &MoveToStorage, &CurrentNew, &CurrentEdit, &CurrentCopy, &CurrentDelete);

	wxSizer *MoveImportSizer = new wxBoxSizer(wxHORIZONTAL);
	MoveImportSizer->Add(MoveToStorage,1,wxEXPAND | wxRIGHT,5);
	MoveImportSizer->Add(CurrentImport,1,wxEXPAND,0);

	wxSizer *CurrentListSizer = new wxBoxSizer(wxHORIZONTAL);
	CurrentList = new wxListBox(this, -1, wxDefaultPosition, wxSize(240,250), 0, NULL, wxLB_EXTENDED);
	CurrentListSizer->Add(CurrentList,1,wxEXPAND | wxRIGHT,0);
	CurrentListSizer->Add(make_move_buttons(this, &CurrentMoveUp, &CurrentMoveDown, &CurrentMoveTop, &CurrentMoveBottom, &CurrentSort), wxSizerFlags().Expand());

	wxSizer *CurrentBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Current script"));
	CurrentBox->Add(CurrentListSizer,1,wxEXPAND | wxBOTTOM,5);
	CurrentBox->Add(MoveImportSizer,0,wxEXPAND | wxBOTTOM,5);
	CurrentBox->Add(CurrentButtons,0,wxEXPAND | wxBOTTOM,0);

	// Buttons
	wxStdDialogButtonSizer *buttonSizer = CreateStdDialogButtonSizer(wxCANCEL | wxHELP);
	buttonSizer->GetCancelButton()->SetLabel(_("Close"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&HelpButton::OpenPage, "Styles Manager"), wxID_HELP);

	// General layout
	wxSizer *StylesSizer = new wxBoxSizer(wxHORIZONTAL);
	StylesSizer->Add(StorageBox,0,wxRIGHT | wxEXPAND,5);
	StylesSizer->Add(CurrentBox,0,wxLEFT | wxEXPAND,0);
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(CatalogBox,0,wxEXPAND | wxLEFT | wxRIGHT | wxTOP,5);
	MainSizer->Add(StylesSizer,1,wxEXPAND | wxALL,5);
	MainSizer->Add(buttonSizer,0,wxBOTTOM | wxEXPAND,5);

	SetSizerAndFit(MainSizer);

	// Position window
	persist.reset(new PersistLocation(this, "Tool/Style Manager"));

	// Populate lists
	LoadCatalog();
	LoadCurrentStyles(AssFile::COMMIT_STYLES | AssFile::COMMIT_DIAG_META);

	//Set key handlers for lists
	CatalogList->Bind(wxEVT_KEY_DOWN, &DialogStyleManager::OnKeyDown, this);
	StorageList->Bind(wxEVT_KEY_DOWN, &DialogStyleManager::OnKeyDown, this);
	CurrentList->Bind(wxEVT_KEY_DOWN, &DialogStyleManager::OnKeyDown, this);

	StorageMoveUp->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, true, 0));
	StorageMoveTop->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, true, 1));
	StorageMoveDown->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, true, 2));
	StorageMoveBottom->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, true, 3));
	StorageSort->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, true, 4));

	CurrentMoveUp->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, false, 0));
	CurrentMoveTop->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, false, 1));
	CurrentMoveDown->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, false, 2));
	CurrentMoveBottom->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, false, 3));
	CurrentSort->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::MoveStyles, this, false, 4));

	CatalogNew->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCatalogNew, this));
	CatalogDelete->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCatalogDelete, this));

	StorageNew->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnStorageNew, this));
	StorageEdit->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnStorageEdit, this));
	StorageCopy->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnStorageCopy, this));
	StorageDelete->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnStorageDelete, this));

	CurrentNew->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCurrentNew, this));
	CurrentEdit->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCurrentEdit, this));
	CurrentCopy->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCurrentCopy, this));
	CurrentDelete->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCurrentDelete, this));

	CurrentImport->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCurrentImport, this));

	MoveToLocal->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCopyToCurrent, this));
	MoveToStorage->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogStyleManager::OnCopyToStorage, this));

	CatalogList->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, bind(&DialogStyleManager::OnChangeCatalog, this));

	StorageList->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, bind(&DialogStyleManager::UpdateButtons, this));
	StorageList->Bind(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, bind(&DialogStyleManager::OnStorageEdit, this));

	CurrentList->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, bind(&DialogStyleManager::UpdateButtons, this));
	CurrentList->Bind(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, bind(&DialogStyleManager::OnCurrentEdit, this));

	c->selectionController->AddSelectionListener(this);
}

DialogStyleManager::~DialogStyleManager() {
	c->selectionController->RemoveSelectionListener(this);
}

void DialogStyleManager::LoadCurrentStyles(int commit_type) {
	if (commit_type & AssFile::COMMIT_STYLES || commit_type == AssFile::COMMIT_NEW) {
		CurrentList->Clear();
		styleMap.clear();

		for (entryIter cur = c->ass->Line.begin(); cur != c->ass->Line.end(); ++cur) {
			if (AssStyle *style = dynamic_cast<AssStyle*>(*cur)) {
				CurrentList->Append(style->name);
				styleMap.push_back(style);
			}
		}
	}

	if (commit_type & AssFile::COMMIT_DIAG_META) {
		AssDialogue *dia = c->selectionController->GetActiveLine();
		CurrentList->DeselectAll();
		if (dia && commit_type != AssFile::COMMIT_NEW)
			CurrentList->SetStringSelection(dia->Style);
		else
			CurrentList->SetSelection(0);
	}

	UpdateButtons();
}

void DialogStyleManager::OnActiveLineChanged(AssDialogue *new_line) {
	if (new_line) {
		CurrentList->DeselectAll();
		CurrentList->SetStringSelection(new_line->Style);
		UpdateButtons();
	}
}

void DialogStyleManager::UpdateStorage() {
	Store.Save();

	StorageList->Clear();
	for (AssStyleStorage::iterator cur = Store.begin(); cur != Store.end(); ++cur)
		StorageList->Append((*cur)->name);

	UpdateButtons();
}

void DialogStyleManager::OnChangeCatalog() {
	c->ass->SetScriptInfo("Last Style Storage", CatalogList->GetStringSelection());
	Store.Load(CatalogList->GetStringSelection());
	UpdateStorage();
}

void DialogStyleManager::LoadCatalog() {
	CatalogList->Clear();

	// Get saved style catalogs
	wxString dirname = StandardPaths::DecodePath("?user/catalog/*.sty");
	for (wxString curfile = wxFindFirstFile(dirname, wxFILE); !curfile.empty(); curfile = wxFindNextFile())
		CatalogList->Append(wxFileName(curfile).GetName());

	// Create a default storage if there are none
	if (CatalogList->IsListEmpty())
		CatalogList->Append("Default");

	// Set to default if available
	wxString pickStyle = c->ass->GetScriptInfo("Last Style Storage");
	if (pickStyle.empty())
		pickStyle = "Default";

	int opt = CatalogList->FindString(pickStyle, false);
	if (opt != wxNOT_FOUND)
		CatalogList->SetSelection(opt);
	else
		CatalogList->SetSelection(0);

	OnChangeCatalog();
}

void DialogStyleManager::OnCatalogNew() {
	wxString name = wxGetTextFromUser(_("New storage name:"), _("New catalog entry"), "", this);
	if (!name) return;

	// Remove bad characters from the name
	wxString badchars = wxFileName::GetForbiddenChars();
	int badchars_removed = 0;
	for (size_t i = 0; i < name.size(); ++i) {
		if (badchars.find(name[i]) != badchars.npos) {
			name[i] = '_';
			++badchars_removed;
		}
	}

	// Make sure that there is no storage with the same name (case insensitive search since Windows filenames are case insensitive)
	if (CatalogList->FindString(name, false) != wxNOT_FOUND) {
		wxMessageBox(_("A catalog with that name already exists."), _("Catalog name conflict"), wxICON_ERROR|wxOK);
		return;
	}

	// Warn about bad characters
	if (badchars_removed) {
		wxMessageBox(
			wxString::Format(_("The specified catalog name contains one or more illegal characters. They have been replaced with underscores instead.\nThe catalog has been renamed to \"%s\"."), name),
			_("Invalid characters"));
	}

	// Add to list of storages
	CatalogList->Append(name);
	CatalogList->SetStringSelection(name);
	OnChangeCatalog();
}

void DialogStyleManager::OnCatalogDelete() {
	if (CatalogList->GetCount() == 1) return;

	wxString name = CatalogList->GetStringSelection();
	wxString message = wxString::Format(_("Are you sure you want to delete the storage \"%s\" from the catalog?"), name);
	int option = wxMessageBox(message, _("Confirm delete"), wxYES_NO | wxICON_EXCLAMATION , this);
	if (option == wxYES) {
		wxRemoveFile(StandardPaths::DecodePath("?user/catalog/" + name + ".sty"));
		CatalogList->Delete(CatalogList->GetSelection());
		CatalogList->SetSelection(0);
		OnChangeCatalog();
	}
}

void DialogStyleManager::OnCopyToStorage() {
	std::list<wxString> copied;
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);
	for (int i = 0; i < n; i++) {
		wxString styleName = CurrentList->GetString(selections[i]);

		if (AssStyle *style = Store.GetStyle(styleName)) {
			if (wxYES == wxMessageBox(wxString::Format(_("There is already a style with the name \"%s\" in the current storage. Proceed and overwrite anyway?"),styleName), _("Style name collision."), wxYES_NO)) {
				*style = *styleMap.at(selections[i]);
				copied.push_back(styleName);
			}
		}
		else {
			Store.push_back(new AssStyle(*styleMap.at(selections[i])));
			copied.push_back(styleName);
		}
	}

	UpdateStorage();
	for (std::list<wxString>::iterator name = copied.begin(); name != copied.end(); ++name)
		StorageList->SetStringSelection(*name, true);

	UpdateButtons();
}

void DialogStyleManager::OnCopyToCurrent() {
	std::list<wxString> copied;
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);
	for (int i = 0; i < n; i++) {
		wxString styleName = StorageList->GetString(selections[i]);
		bool addStyle = true;

		for (std::vector<AssStyle *>::iterator style = styleMap.begin(); style != styleMap.end(); ++style) {
			if ((*style)->name.CmpNoCase(styleName) == 0) {
				addStyle = false;
				if (wxYES == wxMessageBox(wxString::Format(_("There is already a style with the name \"%s\" in the current script. Proceed and overwrite anyway?"), styleName), _("Style name collision"), wxYES_NO)) {
					**style = *Store[selections[i]];
					copied.push_back(styleName);
				}
				break;
			}
		}
		if (addStyle) {
			c->ass->InsertStyle(new AssStyle(*Store[selections[i]]));
			copied.push_back(styleName);
		}
	}

	c->ass->Commit(_("style copy"), AssFile::COMMIT_STYLES);

	CurrentList->DeselectAll();
	for (std::list<wxString>::iterator name = copied.begin(); name != copied.end(); ++name)
		CurrentList->SetStringSelection(*name, true);
	UpdateButtons();
}

template<class T>
void DialogStyleManager::CopyToClipboard(wxListBox *list, T const& v) {
	wxString data;
	wxArrayInt selections;
	list->GetSelections(selections);

	for(size_t i = 0; i < selections.size(); ++i) {
		if (i) data += "\r\n";
		AssStyle *s = v[selections[i]];
		s->UpdateData();
		data += s->GetEntryData();
	}

	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(data));
		wxTheClipboard->Close();
	}
}

void DialogStyleManager::PasteToCurrent() {
	add_styles(
		bind(&AssFile::GetStyle, c->ass, _1),
		bind(&AssFile::InsertStyle, c->ass, _1));

	c->ass->Commit(_("style paste"), AssFile::COMMIT_STYLES);
}

void DialogStyleManager::PasteToStorage() {
	add_styles(
		bind(&AssStyleStorage::GetStyle, &Store, _1),
		bind(&AssStyleStorage::push_back, &Store, _1));

	UpdateStorage();
	StorageList->SetStringSelection(Store.back()->name);
	UpdateButtons();
}

void DialogStyleManager::ShowStorageEditor(AssStyle *style, wxString const& new_name) {
	DialogStyleEditor editor(this, style, c, &Store, new_name);
	if (editor.ShowModal()) {
		UpdateStorage();
		StorageList->SetStringSelection(editor.GetStyleName());
		UpdateButtons();
	}
}

void DialogStyleManager::OnStorageNew() {
	ShowStorageEditor(0);
}

void DialogStyleManager::OnStorageEdit() {
	int sel = get_single_sel(StorageList);
	if (sel == -1) return;
	ShowStorageEditor(Store[sel]);
}

void DialogStyleManager::OnStorageCopy() {
	int sel = get_single_sel(StorageList);
	if (sel == -1) return;

	ShowStorageEditor(Store[sel],
		unique_name(bind(&AssStyleStorage::GetStyle, &Store, _1), Store[sel]->name));
}

void DialogStyleManager::OnStorageDelete() {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);

	if (confirm_delete(n, this, _("Confirm delete from storage")) == wxYES) {
		for (int i = 0; i < n; i++)
			Store.Delete(selections[i] - i);
		UpdateStorage();
	}
}

void DialogStyleManager::ShowCurrentEditor(AssStyle *style, wxString const& new_name) {
	DialogStyleEditor editor(this, style, c, 0, new_name);
	if (editor.ShowModal()) {
		CurrentList->DeselectAll();
		CurrentList->SetStringSelection(editor.GetStyleName());
		UpdateButtons();
	}
}

void DialogStyleManager::OnCurrentNew() {
	ShowCurrentEditor(0);
}

void DialogStyleManager::OnCurrentEdit() {
	int sel = get_single_sel(CurrentList);
	if (sel == -1) return;
	ShowCurrentEditor(styleMap[sel]);
}

void DialogStyleManager::OnCurrentCopy() {
	int sel = get_single_sel(CurrentList);
	if (sel == -1) return;

	ShowCurrentEditor(styleMap[sel],
		unique_name(bind(&AssFile::GetStyle, c->ass, _1), styleMap[sel]->name));
}

void DialogStyleManager::OnCurrentDelete() {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);

	if (confirm_delete(n, this, _("Confirm delete from current")) == wxYES) {
		for (int i=0;i<n;i++) {
			AssStyle *temp = styleMap.at(selections[i]);
			c->ass->Line.remove(temp);
			delete temp;
		}
		c->ass->Commit(_("style delete"), AssFile::COMMIT_STYLES);
	}
}

void DialogStyleManager::OnCurrentImport() {
	// Get file name
	wxString path = lagi_wxString(OPT_GET("Path/Last/Subtitles")->GetString());
	wxString filename = wxFileSelector(_("Open subtitles file"),path,"","",AssFile::GetWildcardList(0),wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (!filename) return;

	OPT_SET("Path/Last/Subtitles")->SetString(STD_STR(wxFileName(filename).GetPath()));

	AssFile temp;
	try {
		temp.Load(filename, "", false);
	}
	catch (...) {
		return;
	}

	// Get styles
	wxArrayString styles = temp.GetStyles();
	if (styles.empty()) {
		wxMessageBox(_("The selected file has no available styles."), _("Error Importing Styles"));
		return;
	}

	// Get selection
	wxArrayInt selections;
	int res = GetSelectedChoices(this, selections, _("Choose styles to import:"), _("Import Styles"), styles);
	if (res == -1 || selections.empty()) return;
	bool modified = false;

	// Loop through selection
	for (size_t i = 0; i < selections.size(); ++i) {
		// Check if there is already a style with that name
		int test = CurrentList->FindString(styles[selections[i]], false);
		if (test != wxNOT_FOUND) {
			int answer = wxMessageBox(
				wxString::Format(_("There is already a style with the name \"%s\" on the current script. Overwrite?"), styles[selections[i]]),
				_("Style name collision"),
				wxYES_NO);
			if (answer == wxYES) {
				// Overwrite
				modified = true;
				// The result of GetString is used rather than the name
				// itself to deal with that AssFile::GetStyle is
				// case-sensitive, but style names are case insensitive
				*c->ass->GetStyle(CurrentList->GetString(test)) = *temp.GetStyle(styles[selections[i]]);
			}
			continue;
		}

		// Copy
		modified = true;
		AssStyle *tempStyle = new AssStyle;
		*tempStyle = *temp.GetStyle(styles[selections[i]]);
		c->ass->InsertStyle(tempStyle);
	}

	// Update
	if (modified)
		c->ass->Commit(_("style import"), AssFile::COMMIT_STYLES);
}

void DialogStyleManager::UpdateButtons() {
	CatalogDelete->Enable(CatalogList->GetCount() > 1);

	// Get storage selection
	wxArrayInt sels;
	int n = StorageList->GetSelections(sels);

	StorageEdit->Enable(n == 1);
	StorageCopy->Enable(n == 1);
	StorageDelete->Enable(n > 0);
	MoveToLocal->Enable(n > 0);

	int firstStor = -1;
	int lastStor = -1;
	if (n) {
		firstStor = sels[0];
		lastStor = sels[n-1];
	}

	// Check if selection is continuous
	bool contStor = true;
	for (int i = 1; i < n; ++i) {
		if (sels[i] != sels[i-1]+1) {
			contStor = false;
			break;
		}
	}

	int itemsStor = StorageList->GetCount();
	StorageMoveUp->Enable(contStor && firstStor > 0);
	StorageMoveTop->Enable(contStor && firstStor > 0);
	StorageMoveDown->Enable(contStor && lastStor != -1 && lastStor < itemsStor-1);
	StorageMoveBottom->Enable(contStor && lastStor != -1 && lastStor < itemsStor-1);
	StorageSort->Enable(itemsStor > 1);


	// Get current selection
	n = CurrentList->GetSelections(sels);

	CurrentEdit->Enable(n == 1);
	CurrentCopy->Enable(n == 1);
	CurrentDelete->Enable(n > 0);
	MoveToStorage->Enable(n > 0);

	int firstCurr = -1;
	int lastCurr = -1;
	if (n) {
		firstCurr = sels[0];
		lastCurr = sels[n-1];
	}

	// Check if selection is continuous
	bool contCurr = true;
	for (int i = 1; i < n; ++i) {
		if (sels[i] != sels[i-1]+1) {
			contCurr = false;
			break;
		}
	}

	int itemsCurr = CurrentList->GetCount();
	CurrentMoveUp->Enable(contCurr && firstCurr > 0);
	CurrentMoveTop->Enable(contCurr && firstCurr > 0);
	CurrentMoveDown->Enable(contCurr && lastCurr != -1 && lastCurr < itemsCurr-1);
	CurrentMoveBottom->Enable(contCurr && lastCurr != -1 && lastCurr < itemsCurr-1);
	CurrentSort->Enable(itemsCurr > 1);
}

static bool cmp_style_name(const AssStyle *lft, const AssStyle *rgt) {
	return lft->name < rgt->name;
}

template<class Cont>
static void do_move(Cont& styls, int type, int& first, int& last, bool storage) {
	typename Cont::iterator begin = styls.begin();

	// Move up
	if (type == 0) {
		if (first == 0) return;
		rotate(begin + first - 1, begin + first, begin + last + 1);
		first--;
		last--;
	}
	// Move to top
	else if (type == 1) {
		rotate(begin, begin + first, begin + last + 1);
		last = last - first;
		first = 0;
	}
	// Move down
	else if (type == 2) {
		if (last + 1 == (int)styls.size()) return;
		rotate(begin + first, begin + last + 1, begin + last + 2);
		first++;
		last++;
	}
	// Move to bottom
	else if (type == 3) {
		rotate(begin + first, begin + last + 1, styls.end());
		first = styls.size() - (last - first + 1);
		last = styls.size() - 1;
	}
	// Sort
	else if (type == 4) {
		// Get confirmation
		if (storage) {
			int res = wxMessageBox(_("Are you sure? This cannot be undone!"), _("Sort styles"), wxYES_NO);
			if (res == wxNO) return;
		}

		sort(styls.begin(), styls.end(), cmp_style_name);

		first = 0;
		last = 0;
	}
}

void DialogStyleManager::MoveStyles(bool storage, int type) {
	wxListBox *list = storage ? StorageList : CurrentList;

	// Get selection
	wxArrayInt sels;
	int n = list->GetSelections(sels);
	if (n == 0 && type != 4) return;

	int first = 0, last = 0;
	if (n) {
		first = sels.front();
		last = sels.back();
	}

	if (storage) {
		do_move(Store, type, first, last, true);
		UpdateStorage();
	}
	else {
		do_move(styleMap, type, first, last, false);

		// Replace styles
		entryIter next;
		int curn = 0;
		for (entryIter cur = c->ass->Line.begin(); cur != c->ass->Line.end(); cur = next) {
			next = cur;
			next++;
			if (dynamic_cast<AssStyle*>(*cur)) {
				c->ass->Line.insert(cur, styleMap[curn]);
				c->ass->Line.erase(cur);
				curn++;
			}
		}

		c->ass->Commit(_("style move"), AssFile::COMMIT_STYLES);
	}

	for (int i = 0 ; i < (int)list->GetCount(); ++i) {
		if (i < first || i > last)
			list->Deselect(i);
		else
			list->Select(i);
	}

	UpdateButtons();
}

void DialogStyleManager::OnKeyDown(wxKeyEvent &event) {
	wxWindow *focus = wxWindow::FindFocus();

	switch(event.GetKeyCode()) {
		case WXK_DELETE :
			if (focus == StorageList)
				OnStorageDelete();
			else if (focus == CurrentList)
				OnCurrentDelete();
			break;

		case 'C' :
		case 'c' :
			if (event.CmdDown()) {
				if (focus == StorageList)
					CopyToClipboard(StorageList, Store);
				else if (focus == CurrentList)
					CopyToClipboard(CurrentList, styleMap);
			}
			break;

		case 'V' :
		case 'v' :
			if (event.CmdDown()) {
				if (focus == StorageList)
					PasteToStorage();
				else if (focus == CurrentList)
					PasteToCurrent();
			}
			break;
		default:
			event.Skip();
			break;
	}
}
