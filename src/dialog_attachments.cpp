// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file dialog_attachments.cpp
/// @brief Manage files attached to the subtitle file
/// @ingroup tools_ui
///

#include "config.h"

#include "dialog_attachments.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include "ass_attachment.h"
#include "ass_file.h"
#include "compat.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "utils.h"

DialogAttachments::DialogAttachments(wxWindow *parent, AssFile *ass)
: wxDialog(parent, -1, _("Attachment List"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
, ass(ass)
{
	SetIcon(GETICON(attach_button_16));

	listView = new wxListView(this, -1, wxDefaultPosition, wxSize(500, 200));
	UpdateList();

	auto attachFont = new wxButton(this, -1, _("Attach &Font"));
	auto attachGraphics = new wxButton(this, -1, _("Attach &Graphics"));
	extractButton = new wxButton(this, -1, _("E&xtract"));
	deleteButton = new wxButton(this, -1, _("&Delete"));
	extractButton->Enable(false);
	deleteButton->Enable(false);

	auto buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(attachFont, 1);
	buttonSizer->Add(attachGraphics, 1);
	buttonSizer->Add(extractButton, 1);
	buttonSizer->Add(deleteButton, 1);
	buttonSizer->Add(new HelpButton(this, "Attachment Manager"), 1, wxLEFT, 5);
	buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("&Close")), 1);

	auto mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(listView, 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 5);
	mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 5);
	SetSizerAndFit(mainSizer);
	CenterOnParent();

	attachFont->Bind(wxEVT_BUTTON, &DialogAttachments::OnAttachFont, this);
	attachGraphics->Bind(wxEVT_BUTTON, &DialogAttachments::OnAttachGraphics, this);
	extractButton->Bind(wxEVT_BUTTON, &DialogAttachments::OnExtract, this);
	deleteButton->Bind(wxEVT_BUTTON, &DialogAttachments::OnDelete, this);

	listView->Bind(wxEVT_LIST_ITEM_SELECTED, &DialogAttachments::OnListClick, this);
	listView->Bind(wxEVT_LIST_ITEM_DESELECTED, &DialogAttachments::OnListClick, this);
	listView->Bind(wxEVT_LIST_ITEM_FOCUSED, &DialogAttachments::OnListClick, this);
}

void DialogAttachments::UpdateList() {
	listView->ClearAll();

	listView->InsertColumn(0, _("Attachment name"), wxLIST_FORMAT_LEFT, 280);
	listView->InsertColumn(1, _("Size"), wxLIST_FORMAT_LEFT, 100);
	listView->InsertColumn(2, _("Group"), wxLIST_FORMAT_LEFT, 100);

	for (auto& attach : ass->Attachments) {
		int row = listView->GetItemCount();
		listView->InsertItem(row, to_wx(attach.GetFileName(true)));
		listView->SetItem(row, 1, PrettySize(attach.GetSize()));
		listView->SetItem(row, 2, to_wx(attach.GroupHeader()));
	}
}

void DialogAttachments::AttachFile(wxFileDialog &diag, wxString const& commit_msg) {
	if (diag.ShowModal() == wxID_CANCEL) return;

	wxArrayString paths;
	diag.GetPaths(paths);

	for (auto const& fn : paths)
		ass->InsertAttachment(agi::fs::path(fn));

	ass->Commit(commit_msg, AssFile::COMMIT_ATTACHMENT);

	UpdateList();
}

void DialogAttachments::OnAttachFont(wxCommandEvent &) {
	wxFileDialog diag(this,
		_("Choose file to be attached"),
		to_wx(OPT_GET("Path/Fonts Collector Destination")->GetString()), "", "Font Files (*.ttf)|*.ttf",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

	AttachFile(diag, _("attach font file"));
}

void DialogAttachments::OnAttachGraphics(wxCommandEvent &) {
	wxFileDialog diag(this,
		_("Choose file to be attached"),
		"", "",
		"Graphic Files (*.bmp, *.gif, *.jpg, *.ico, *.wmf)|*.bmp;*.gif;*.jpg;*.ico;*.wmf",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

	AttachFile(diag, _("attach graphics file"));
}

void DialogAttachments::OnExtract(wxCommandEvent &) {
	int i = listView->GetFirstSelected();
	if (i == -1) return;

	agi::fs::path path;
	bool fullPath = false;

	// Multiple or single?
	if (listView->GetNextSelected(i) != -1)
		path = wxDirSelector(_("Select the path to save the files to:"), to_wx(OPT_GET("Path/Fonts Collector Destination")->GetString())).c_str();
	else {
		path = SaveFileSelector(
			_("Select the path to save the file to:"),
			"Path/Fonts Collector Destination",
			ass->Attachments[i].GetFileName(),
			".ttf", "Font Files (*.ttf)|*.ttf",
			this);
		fullPath = true;
	}
	if (path.empty()) return;

	// Loop through items in list
	while (i != -1) {
		auto& attach = ass->Attachments[i];
		attach.Extract(fullPath ? path : path/attach.GetFileName());
		i = listView->GetNextSelected(i);
	}
}

void DialogAttachments::OnDelete(wxCommandEvent &) {
	size_t removed = 0;
	for (auto i = listView->GetFirstSelected(); i != -1; i = listView->GetNextSelected(i))
		ass->Attachments.erase(ass->Attachments.begin() + i - removed++);

	ass->Commit(_("remove attachment"), AssFile::COMMIT_ATTACHMENT);

	UpdateList();
	extractButton->Enable(false);
	deleteButton->Enable(false);
}

void DialogAttachments::OnListClick(wxListEvent &) {
	bool hasSel = listView->GetFirstSelected() != -1;
	extractButton->Enable(hasSel);
	deleteButton->Enable(hasSel);
}
