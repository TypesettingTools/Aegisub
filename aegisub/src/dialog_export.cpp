// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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

/// @file dialog_export.cpp
/// @brief Export set-up dialogue box
/// @ingroup export
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <wx/tokenzr.h>
#endif

#include "dialog_export.h"

#include "ass_exporter.h"
#include "ass_file.h"
#include "include/aegisub/context.h"
#include "charset_conv.h"
#include "help_button.h"

DialogExport::DialogExport(agi::Context *c)
: wxDialog(c->parent, -1, _("Export"), wxDefaultPosition, wxSize(200, 100), wxCAPTION | wxCLOSE_BOX)
, c(c)
, exporter(new AssExporter(c))
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	wxArrayString filters = exporter->GetAllFilterNames();
	filter_list = new wxCheckListBox(this, -1, wxDefaultPosition, wxSize(200, 100), filters);
	filter_list->Bind(wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, &DialogExport::OnCheck, this);
	filter_list->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &DialogExport::OnChange, this);

	// Get selected filters
	wxString selected = c->ass->GetScriptInfo("Export filters");
	wxStringTokenizer token(selected, "|");
	while (token.HasMoreTokens()) {
		wxString cur = token.GetNextToken();
		if (!cur.empty()) {
			int idx = filters.Index(cur);
			if (idx != wxNOT_FOUND)
				filter_list->Check(idx);
		}
	}

	wxButton *btn_up = new wxButton(this, -1, _("Move &Up"), wxDefaultPosition, wxSize(90, -1));
	wxButton *btn_down = new wxButton(this, -1, _("Move &Down"), wxDefaultPosition, wxSize(90, -1));
	wxButton *btn_all = new wxButton(this, -1, _("Select &All"), wxDefaultPosition, wxSize(80, -1));
	wxButton *btn_none = new wxButton(this, -1, _("Select &None"), wxDefaultPosition, wxSize(80, -1));

	btn_up->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogExport::OnMoveUp, this);
	btn_down->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogExport::OnMoveDown, this);
	btn_all->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogExport::OnSelectAll, this);
	btn_none->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogExport::OnSelectNone, this);

	wxSizer *top_buttons = new wxBoxSizer(wxHORIZONTAL);
	top_buttons->Add(btn_up, wxSizerFlags(1).Expand());
	top_buttons->Add(btn_down, wxSizerFlags(1).Expand().Border(wxRIGHT));
	top_buttons->Add(btn_all, wxSizerFlags(1).Expand());
	top_buttons->Add(btn_none, wxSizerFlags(1).Expand());

	filter_description = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(200, 60), wxTE_MULTILINE | wxTE_READONLY);

	// Charset dropdown list
	wxStaticText *charset_list_label = new wxStaticText(this, -1, _("Text encoding:"));
	charset_list = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, agi::charset::GetEncodingsList<wxArrayString>());
	wxSizer *charset_list_sizer = new wxBoxSizer(wxHORIZONTAL);
	charset_list_sizer->Add(charset_list_label, wxSizerFlags().Center().Border(wxRIGHT));
	charset_list_sizer->Add(charset_list, wxSizerFlags(1).Expand());
	if (!charset_list->SetStringSelection(c->ass->GetScriptInfo("Export Encoding")))
		charset_list->SetStringSelection("Unicode (UTF-8)");

	wxSizer *top_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Filters"));
	top_sizer->Add(filter_list, wxSizerFlags(1).Expand());
	top_sizer->Add(top_buttons, wxSizerFlags(0).Expand());
	top_sizer->Add(filter_description, wxSizerFlags(0).Expand().Border(wxTOP));
	top_sizer->Add(charset_list_sizer, wxSizerFlags(0).Expand().Border(wxTOP));

	wxStdDialogButtonSizer *btn_sizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	btn_sizer->GetAffirmativeButton()->SetLabelText(_("Export..."));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogExport::OnProcess, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&HelpButton::OpenPage, "Export"), wxID_HELP);

	wxSizer *horz_sizer = new wxBoxSizer(wxHORIZONTAL);
	opt_sizer = new wxBoxSizer(wxVERTICAL);
	exporter->DrawSettings(this, opt_sizer);
	horz_sizer->Add(top_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxRIGHT));
	horz_sizer->Add(opt_sizer, wxSizerFlags(1).Expand().Border(wxALL & ~wxLEFT));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(horz_sizer, wxSizerFlags(1).Expand());
	main_sizer->Add(btn_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	SetSizerAndFit(main_sizer);
	RefreshOptions();
	CenterOnParent();
}

DialogExport::~DialogExport() {
	wxString infoList;
	for (size_t i = 0; i < filter_list->GetCount(); ++i) {
		if (filter_list->IsChecked(i))
			infoList += filter_list->GetString(i) + "|";
	}
	if (!infoList.empty()) infoList.RemoveLast();
	c->ass->SetScriptInfo("Export filters", infoList);
}

void DialogExport::OnProcess(wxCommandEvent &) {
	if (!TransferDataFromWindow()) return;

	wxString filename = wxFileSelector(_("Export subtitles file"), "", "", "", AssFile::GetWildcardList(2), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);
	if (filename.empty()) return;

	for (size_t i = 0; i < filter_list->GetCount(); ++i) {
		if (filter_list->IsChecked(i))
			exporter->AddFilter(filter_list->GetString(i));
	}

	try {
		wxBusyCursor busy;
		c->ass->SetScriptInfo("Export Encoding", charset_list->GetStringSelection());
		exporter->Export(filename, charset_list->GetStringSelection(), this);
	}
	catch (agi::UserCancelException const&) {
	}
	catch (const char *error) {
		wxMessageBox(error, "Error exporting subtitles", wxOK | wxICON_ERROR, this);
	}
	catch (wxString const& error) {
		wxMessageBox(error, "Error exporting subtitles", wxOK | wxICON_ERROR, this);
	}
	catch (agi::Exception const& err) {
		wxMessageBox(err.GetMessage(), "Error exporting subtitles", wxOK | wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox("Unknown error", "Error exporting subtitles", wxOK | wxICON_ERROR, this);
	}

	EndModal(0);
}

void DialogExport::OnCheck(wxCommandEvent &) {
	RefreshOptions();
}

void DialogExport::OnChange(wxCommandEvent &) {
	int n = filter_list->GetSelection();
	if (n != wxNOT_FOUND) {
		wxString name = filter_list->GetString(n);
		filter_description->SetValue(exporter->GetDescription(name));
	}
}

// Swap the items at idx and idx + 1
static void swap(wxCheckListBox *list, int idx, int sel_dir) {
	if (idx < 0 || idx + 1 == (int)list->GetCount()) return;

	list->Freeze();
	wxString tempname = list->GetString(idx);
	bool tempval = list->IsChecked(idx);
	list->SetString(idx, list->GetString(idx + 1));
	list->Check(idx, list->IsChecked(idx + 1));
	list->SetString(idx + 1, tempname);
	list->Check(idx + 1, tempval);
	list->SetSelection(idx + sel_dir);
	list->Thaw();
}

void DialogExport::OnMoveUp(wxCommandEvent &) {
	swap(filter_list, filter_list->GetSelection() - 1, 0);
}

void DialogExport::OnMoveDown(wxCommandEvent &) {
	swap(filter_list, filter_list->GetSelection(), 1);
}

void DialogExport::OnSelectAll(wxCommandEvent &) {
	SetAll(true);
}

void DialogExport::OnSelectNone(wxCommandEvent &) {
	SetAll(false);
}

void DialogExport::SetAll(bool new_value) {
	filter_list->Freeze();
	for (size_t i = 0; i < filter_list->GetCount(); ++i)
		filter_list->Check(i, new_value);
	filter_list->Thaw();

	RefreshOptions();
}

void DialogExport::RefreshOptions() {
	for (size_t i = 0; i < filter_list->GetCount(); ++i) {
		if (wxSizer *sizer = exporter->GetSettingsSizer(filter_list->GetString(i)))
			opt_sizer->Show(sizer, filter_list->IsChecked(i), true);
	}
	Layout();
	GetSizer()->Fit(this);
}
