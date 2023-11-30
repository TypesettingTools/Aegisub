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

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_style_storage.h"
#include "charset_detect.h"
#include "compat.h"
#include "dialog_manager.h"
#include "dialog_style_editor.h"
#include "dialogs.h"
#include "format.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "persist_location.h"
#include "selection_controller.h"
#include "subtitle_format.h"

#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/signal.h>
#include <libaegisub/split.h>
#include <libaegisub/vfr.h>

#include <algorithm>
#include <functional>
#include <future>
#include <memory>
#include <vector>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/combobox.h>
#include <wx/filename.h>
#include <wx/fontenum.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

namespace {
class DialogStyleManager final : public wxDialog {
	agi::Context *c; ///< Project context
	std::unique_ptr<PersistLocation> persist;

	agi::signal::Connection commit_connection;
	agi::signal::Connection active_line_connection;

	std::shared_future<wxArrayString> font_list;

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
	void LoadCurrentStyles(int commit_type);
	/// Enable/disable all of the buttons as appropriate
	void UpdateButtons();
	/// Move styles up or down
	/// @param storage Storage or current file styles
	/// @param type 0: up; 1: top; 2: down; 3: bottom; 4: sort
	void MoveStyles(bool storage, int type);

	/// Open the style editor for the given style on the script
	/// @param style Style to edit, or nullptr for new
	/// @param new_name Default new name for copies
	void ShowCurrentEditor(AssStyle *style, std::string const& new_name = "");

	/// Open the style editor for the given style in the storage
	/// @param style Style to edit, or nullptr for new
	/// @param new_name Default new name for copies
	void ShowStorageEditor(AssStyle *style, std::string const& new_name = "");

	/// Save the storage and update the view after a change
	void UpdateStorage();

	void OnChangeCatalog();
	void OnCatalogNew();
	void OnCatalogDelete();

	void OnCopyToCurrent();
	void OnCopyToStorage();

	void OnCurrentCopy();
	void OnCurrentDelete();
	void OnCurrentEdit();
	void OnCurrentImport();
	void OnCurrentNew();

	void OnStorageCopy();
	void OnStorageDelete();
	void OnStorageEdit();
	void OnStorageNew();

	void OnKeyDown(wxKeyEvent &event);
	void PasteToCurrent();
	void PasteToStorage();

	template<class T>
	void CopyToClipboard(wxListBox *list, T const& v);

	void OnActiveLineChanged(AssDialogue *new_line);

public:
	DialogStyleManager(agi::Context *context);
};

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
std::string unique_name(Func name_checker, std::string const& source_name) {
	if (name_checker(source_name)) {
		std::string name = agi::format(_("%s - Copy"), source_name);
		for (int i = 2; name_checker(name); ++i)
			name = agi::format(_("%s - Copy (%d)"), source_name, i);
		return name;
	}
	return source_name;
}

template<class Func1, class Func2>
void add_styles(Func1 name_checker, Func2 style_adder) {
	auto cb = GetClipboard();
	int failed_to_parse = 0;
	for (auto tok : agi::Split(cb, '\n')) {
		tok = agi::Trim(tok);
		if (tok.empty()) continue;
		try {
			AssStyle *s = new AssStyle(tok);
			s->name = unique_name(name_checker, s->name);
			style_adder(s);
		}
		catch (...) {
			++failed_to_parse;
		}
	}
	if (failed_to_parse)
		wxMessageBox(_("Could not parse style"), _("Could not parse style"), wxOK | wxICON_EXCLAMATION);
}

int confirm_delete(int n, wxWindow *parent, wxString const& title) {
	return wxMessageBox(
		fmt_plural(n, "Are you sure you want to delete this style?", "Are you sure you want to delete these %d styles?", n),
		title, wxYES_NO | wxICON_EXCLAMATION, parent);
}

int get_single_sel(wxListBox *lb) {
	wxArrayInt selections;
	int n = lb->GetSelections(selections);
	return n == 1 ? selections[0] : -1;
}

DialogStyleManager::DialogStyleManager(agi::Context *context)
: wxDialog(context->parent, -1, _("Styles Manager"))
, c(context)
, commit_connection(c->ass->AddCommitListener(&DialogStyleManager::LoadCurrentStyles, this))
, active_line_connection(c->selectionController->AddActiveLineListener(&DialogStyleManager::OnActiveLineChanged, this))
, font_list(std::async(std::launch::async, []() -> wxArrayString {
	wxArrayString fontList = wxFontEnumerator::GetFacenames();
	fontList.Sort();
	return fontList;
}))
{
	using std::bind;
	SetIcon(GETICON(style_toolbutton_16));

	// Catalog
	wxSizer *CatalogBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Catalog of available storages"));
	CatalogList = new wxComboBox(this,-1, "", wxDefaultPosition, wxSize(-1,-1), 0, nullptr, wxCB_READONLY);
	wxButton *CatalogNew = new wxButton(this, -1, _("New"));
	CatalogDelete = new wxButton(this, -1, _("Delete"));
	CatalogBox->Add(CatalogList,1,wxEXPAND | wxRIGHT,5);
	CatalogBox->Add(CatalogNew,0,wxRIGHT,5);
	CatalogBox->Add(CatalogDelete,0,0,0);

	// Storage styles list
	wxSizer *StorageButtons = make_edit_buttons(this, _("Copy to &current script ->"), &MoveToLocal, &StorageNew, &StorageEdit, &StorageCopy, &StorageDelete);

	wxSizer *StorageListSizer = new wxBoxSizer(wxHORIZONTAL);
	StorageList = new wxListBox(this, -1, wxDefaultPosition, wxSize(240,250), 0, nullptr, wxLB_EXTENDED);
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
	CurrentList = new wxListBox(this, -1, wxDefaultPosition, wxSize(240,250), 0, nullptr, wxLB_EXTENDED);
	CurrentListSizer->Add(CurrentList,1,wxEXPAND | wxRIGHT,0);
	CurrentListSizer->Add(make_move_buttons(this, &CurrentMoveUp, &CurrentMoveDown, &CurrentMoveTop, &CurrentMoveBottom, &CurrentSort), wxSizerFlags().Expand());

	wxSizer *CurrentBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Current script"));
	CurrentBox->Add(CurrentListSizer,1,wxEXPAND | wxBOTTOM,5);
	CurrentBox->Add(MoveImportSizer,0,wxEXPAND | wxBOTTOM,5);
	CurrentBox->Add(CurrentButtons,0,wxEXPAND | wxBOTTOM,0);

	// Buttons
	wxStdDialogButtonSizer *buttonSizer = CreateStdDialogButtonSizer(wxCANCEL | wxHELP);
	buttonSizer->GetCancelButton()->SetLabel(_("Close"));
	Bind(wxEVT_BUTTON, bind(&HelpButton::OpenPage, "Styles Manager"), wxID_HELP);

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
	persist = std::make_unique<PersistLocation>(this, "Tool/Style Manager");

	// Populate lists
	LoadCatalog();
	LoadCurrentStyles(AssFile::COMMIT_STYLES | AssFile::COMMIT_DIAG_META);

	//Set key handlers for lists
	CatalogList->Bind(wxEVT_KEY_DOWN, &DialogStyleManager::OnKeyDown, this);
	StorageList->Bind(wxEVT_KEY_DOWN, &DialogStyleManager::OnKeyDown, this);
	CurrentList->Bind(wxEVT_KEY_DOWN, &DialogStyleManager::OnKeyDown, this);

	StorageMoveUp->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(true, 0); });
	StorageMoveTop->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(true, 1); });
	StorageMoveDown->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(true, 2); });
	StorageMoveBottom->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(true, 3); });
	StorageSort->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(true, 4); });

	CurrentMoveUp->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(false, 0); });
	CurrentMoveTop->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(false, 1); });
	CurrentMoveDown->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(false, 2); });
	CurrentMoveBottom->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(false, 3); });
	CurrentSort->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { MoveStyles(false, 4); });

	CatalogNew->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCatalogNew(); });
	CatalogDelete->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCatalogDelete(); });

	StorageNew->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnStorageNew(); });
	StorageEdit->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnStorageEdit(); });
	StorageCopy->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnStorageCopy(); });
	StorageDelete->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnStorageDelete(); });

	CurrentNew->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCurrentNew(); });
	CurrentEdit->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCurrentEdit(); });
	CurrentCopy->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCurrentCopy(); });
	CurrentDelete->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCurrentDelete(); });

	CurrentImport->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCurrentImport(); });

	MoveToLocal->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCopyToCurrent(); });
	MoveToStorage->Bind(wxEVT_BUTTON, [=, this](wxCommandEvent&) { OnCopyToStorage(); });

	CatalogList->Bind(wxEVT_COMBOBOX, [=, this](wxCommandEvent&) { OnChangeCatalog(); });

	StorageList->Bind(wxEVT_LISTBOX, [=, this](wxCommandEvent&) { UpdateButtons(); });
	StorageList->Bind(wxEVT_LISTBOX_DCLICK, [=, this](wxCommandEvent&) { OnStorageEdit(); });

	CurrentList->Bind(wxEVT_LISTBOX, [=, this](wxCommandEvent&) { UpdateButtons(); });
	CurrentList->Bind(wxEVT_LISTBOX_DCLICK, [=, this](wxCommandEvent&) { OnCurrentEdit(); });
}

void DialogStyleManager::LoadCurrentStyles(int commit_type) {
	if (commit_type & AssFile::COMMIT_STYLES || commit_type == AssFile::COMMIT_NEW) {
		CurrentList->Clear();
		styleMap.clear();

		for (auto& style : c->ass->Styles) {
			CurrentList->Append(to_wx(style.name));
			styleMap.push_back(&style);
		}
	}

	if (commit_type & AssFile::COMMIT_DIAG_META) {
		AssDialogue *dia = c->selectionController->GetActiveLine();
		CurrentList->DeselectAll();
		if (dia && commit_type != AssFile::COMMIT_NEW)
			CurrentList->SetStringSelection(to_wx(dia->Style));
		else
			CurrentList->SetSelection(0);
	}

	UpdateButtons();
}

void DialogStyleManager::OnActiveLineChanged(AssDialogue *new_line) {
	if (new_line) {
		CurrentList->DeselectAll();
		CurrentList->SetStringSelection(to_wx(new_line->Style));
		UpdateButtons();
	}
}

void DialogStyleManager::UpdateStorage() {
	Store.Save();

	StorageList->Clear();
	StorageList->Append(to_wx(Store.GetNames()));

	UpdateButtons();
}

void DialogStyleManager::OnChangeCatalog() {
	std::string catalog(from_wx(CatalogList->GetStringSelection()));
	c->ass->Properties.style_storage = catalog;
	Store.LoadCatalog(catalog);
	UpdateStorage();
}

void DialogStyleManager::LoadCatalog() {
	CatalogList->Clear();

	// Get saved style catalogs
	auto catalogs = AssStyleStorage::GetCatalogs();
	for (auto const& c : catalogs)
		CatalogList->Append(to_wx(c));

	// Create a default storage if there are none
	if (CatalogList->IsListEmpty()) {
		Store.LoadCatalog("Default");
		Store.push_back(std::make_unique<AssStyle>());
		Store.Save();
		CatalogList->Append("Default");
	}

	// Set to default if available
	std::string pickStyle = c->ass->Properties.style_storage;
	if (pickStyle.empty())
		pickStyle = "Default";

	int opt = CatalogList->FindString(to_wx(pickStyle), false);
	CatalogList->SetSelection(opt == wxNOT_FOUND ? 0 : opt);

	OnChangeCatalog();
}

void DialogStyleManager::OnCatalogNew() {
	wxString name = wxGetTextFromUser(_("New storage name:"), _("New catalog entry"), "", this);
	if (!name) return;

	// Remove bad characters from the name
	wxString badchars = wxFileName::GetForbiddenChars(wxPATH_DOS);
	int badchars_removed = 0;
	for (wxUniCharRef chr : name) {
		if (badchars.find(chr) != badchars.npos) {
			chr = '_';
			++badchars_removed;
		}
	}

	// Make sure that there is no storage with the same name (case insensitive search since Windows filenames are case insensitive)
	if (CatalogList->FindString(name, false) != wxNOT_FOUND) {
		wxMessageBox(_("A catalog with that name already exists."), _("Catalog name conflict"), wxOK | wxICON_ERROR | wxCENTER);
		return;
	}

	// Warn about bad characters
	if (badchars_removed) {
		wxMessageBox(
			fmt_tl("The specified catalog name contains one or more illegal characters. They have been replaced with underscores instead.\nThe catalog has been renamed to \"%s\".", name),
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
	wxString message = fmt_tl("Are you sure you want to delete the storage \"%s\" from the catalog?", name);
	int option = wxMessageBox(message, _("Confirm delete"), wxYES_NO | wxICON_EXCLAMATION , this);
	if (option == wxYES) {
		agi::fs::Remove(config::path->Decode("?user/catalog/" + from_wx(name) + ".sty"));
		CatalogList->Delete(CatalogList->GetSelection());
		CatalogList->SetSelection(0);
		OnChangeCatalog();
	}
}

void DialogStyleManager::OnCopyToStorage() {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);
	wxArrayString copied;
	copied.reserve(n);
	for (int i = 0; i < n; i++) {
		wxString styleName = CurrentList->GetString(selections[i]);

		if (AssStyle *style = Store.GetStyle(from_wx(styleName))) {
			if (wxYES == wxMessageBox(fmt_tl("There is already a style with the name \"%s\" in the current storage. Overwrite?", styleName), _("Style name collision"), wxYES_NO)) {
				*style = *styleMap.at(selections[i]);
				copied.push_back(styleName);
			}
		}
		else {
			Store.push_back(std::make_unique<AssStyle>(*styleMap.at(selections[i])));
			copied.push_back(styleName);
		}
	}

	UpdateStorage();
	for (auto const& style_name : copied)
		StorageList->SetStringSelection(style_name, true);

	UpdateButtons();
}

void DialogStyleManager::OnCopyToCurrent() {
	wxArrayInt selections;
	int n = StorageList->GetSelections(selections);
	wxArrayString copied;
	copied.reserve(n);
	for (int i = 0; i < n; i++) {
		wxString styleName = StorageList->GetString(selections[i]);

		if (AssStyle *style = c->ass->GetStyle(from_wx(styleName))) {
			if (wxYES == wxMessageBox(fmt_tl("There is already a style with the name \"%s\" in the current script. Overwrite?", styleName), _("Style name collision"), wxYES_NO)) {
				*style = *Store[selections[i]];
				copied.push_back(styleName);
			}
		}
		else {
			c->ass->Styles.push_back(*new AssStyle(*Store[selections[i]]));
			copied.push_back(styleName);
		}
	}

	c->ass->Commit(_("style copy"), AssFile::COMMIT_STYLES);

	CurrentList->DeselectAll();
	for (auto const& style_name : copied)
		CurrentList->SetStringSelection(style_name, true);
	UpdateButtons();
}

template<class T>
void DialogStyleManager::CopyToClipboard(wxListBox *list, T const& v) {
	wxArrayInt selections;
	list->GetSelections(selections);

	std::string data;
	for(size_t i = 0; i < selections.size(); ++i) {
		if (i) data += "\r\n";
		AssStyle *s = v[selections[i]];
		s->UpdateData();
		data += s->GetEntryData();
	}

	SetClipboard(data);
}

void DialogStyleManager::PasteToCurrent() {
	add_styles(
		[=, this](std::string const& str) { return c->ass->GetStyle(str); },
		[=, this](AssStyle *s) { c->ass->Styles.push_back(*s); });

	c->ass->Commit(_("style paste"), AssFile::COMMIT_STYLES);
}

void DialogStyleManager::PasteToStorage() {
	add_styles(
		[=, this](std::string const& str) { return Store.GetStyle(str); },
		[=, this](AssStyle *s) { Store.push_back(std::unique_ptr<AssStyle>(s)); });

	UpdateStorage();
	StorageList->SetStringSelection(to_wx(Store.back()->name));
	UpdateButtons();
}

void DialogStyleManager::ShowStorageEditor(AssStyle *style, std::string const& new_name) {
	DialogStyleEditor editor(this, style, c, &Store, new_name, font_list.get());
	if (editor.ShowModal()) {
		UpdateStorage();
		StorageList->SetStringSelection(to_wx(editor.GetStyleName()));
		UpdateButtons();
	}
}

void DialogStyleManager::OnStorageNew() {
	ShowStorageEditor(nullptr);
}

void DialogStyleManager::OnStorageEdit() {
	int sel = get_single_sel(StorageList);
	if (sel == -1) return;
	ShowStorageEditor(Store[sel]);
}

void DialogStyleManager::OnStorageCopy() {
	int sel = get_single_sel(StorageList);
	if (sel == -1) return;

	ShowStorageEditor(Store[sel], unique_name(
		[=, this](std::string const& str) { return Store.GetStyle(str); }, Store[sel]->name));
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

void DialogStyleManager::ShowCurrentEditor(AssStyle *style, std::string const& new_name) {
	DialogStyleEditor editor(this, style, c, nullptr, new_name, font_list.get());
	if (editor.ShowModal()) {
		CurrentList->DeselectAll();
		CurrentList->SetStringSelection(to_wx(editor.GetStyleName()));
		UpdateButtons();
	}
}

void DialogStyleManager::OnCurrentNew() {
	ShowCurrentEditor(nullptr);
}

void DialogStyleManager::OnCurrentEdit() {
	int sel = get_single_sel(CurrentList);
	if (sel == -1) return;
	ShowCurrentEditor(styleMap[sel]);
}

void DialogStyleManager::OnCurrentCopy() {
	int sel = get_single_sel(CurrentList);
	if (sel == -1) return;

	ShowCurrentEditor(styleMap[sel], unique_name(
		[=, this](std::string const& str) { return c->ass->GetStyle(str); },
		styleMap[sel]->name));
}

void DialogStyleManager::OnCurrentDelete() {
	wxArrayInt selections;
	int n = CurrentList->GetSelections(selections);

	if (confirm_delete(n, this, _("Confirm delete from current")) == wxYES) {
		for (int i = 0; i < n; i++) {
			delete styleMap.at(selections[i]);
		}
		c->ass->Commit(_("style delete"), AssFile::COMMIT_STYLES);
	}
}

void DialogStyleManager::OnCurrentImport() {
	auto filename = OpenFileSelector(_("Open subtitles file"), "Path/Last/Subtitles", "", "", SubtitleFormat::GetWildcards(0), this);
	if (filename.empty()) return;

	std::string charset;
	try {
		charset = CharSetDetect::GetEncoding(filename);
	}
	catch (agi::UserCancelException const&) {
		return;
	}

	AssFile temp;
	try {
		auto reader = SubtitleFormat::GetReader(filename, charset.c_str());
		if (!reader) {
			wxMessageBox("Unsupported subtitle format", "Error", wxOK | wxICON_ERROR | wxCENTER, this);
			return;
		}
		reader->ReadFile(&temp, filename, 0, charset.c_str());
	}
	catch (agi::Exception const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error", wxOK | wxICON_ERROR | wxCENTER, this);
	}
	catch (...) {
		wxMessageBox("Unknown error", "Error", wxOK | wxICON_ERROR | wxCENTER, this);
		return;
	}

	// Get styles
	auto styles = temp.GetStyles();
	if (styles.empty()) {
		wxMessageBox(_("The selected file has no available styles."), _("Error Importing Styles"));
		return;
	}

	// Get selection
	wxArrayInt selections;
	int res = GetSelectedChoices(this, selections, _("Choose styles to import:"), _("Import Styles"), to_wx(styles));
	if (res == -1 || selections.empty()) return;
	bool modified = false;

	// Loop through selection
	for (auto const& sel : selections) {
		// Check if there is already a style with that name
		if (AssStyle *existing = c->ass->GetStyle(styles[sel])) {
			int answer = wxMessageBox(
				fmt_tl("There is already a style with the name \"%s\" in the current script. Overwrite?", styles[sel]),
				_("Style name collision"),
				wxYES_NO);
			if (answer == wxYES) {
				modified = true;
				*existing = *temp.GetStyle(styles[sel]);
			}
			continue;
		}

		// Copy
		modified = true;
		c->ass->Styles.push_back(*new AssStyle(*temp.GetStyle(styles[sel])));
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

struct cmp_name {
	template<typename T>
	bool operator()(T const& lft, T const& rgt) const { return lft->name < rgt->name; }
};

template<class Cont>
static void do_move(Cont& styls, int type, int& first, int& last, bool storage) {
	auto begin = styls.begin();

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
			int res = wxMessageBox(_("Are you sure? This cannot be undone!"), _("Sort styles"), wxYES_NO | wxCENTER);
			if (res == wxNO) return;
		}

		sort(styls.begin(), styls.end(), cmp_name());

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
		size_t curn = 0;
		for (auto it = c->ass->Styles.begin(); it != c->ass->Styles.end(); ++it) {
			auto new_style_at_pos = c->ass->Styles.iterator_to(*styleMap[curn]);
			EntryList<AssStyle>::node_algorithms::swap_nodes(it.pointed_node(), new_style_at_pos.pointed_node());
			if (++curn == styleMap.size()) break;
			it = new_style_at_pos;
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
}

void ShowStyleManagerDialog(agi::Context *c) {
	c->dialog->Show<DialogStyleManager>(c);
}
