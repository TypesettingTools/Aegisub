// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "config.h"

#include "dialog_autosave.h"

#include "compat.h"
#include "main.h"
#include "standard_paths.h"

#include <boost/range/adaptor/map.hpp>

#ifndef AGI_PRE
#include <map>

#include <wx/statbox.h>
#endif

DialogAutosave::DialogAutosave(wxWindow *parent)
: wxDialog(parent, -1, _("Open autosave file"), wxDefaultPosition, wxSize(800, 350))
, directory(StandardPaths::DecodePath(to_wx(OPT_GET("Path/Auto/Save")->GetString())))
{
	wxSizer *files_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Files"));
	file_list = new wxListBox(this, -1);
	file_list->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &DialogAutosave::OnSelectFile, this);
	files_box->Add(file_list, wxSizerFlags(1).Expand().Border());

	wxSizer *versions_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Versions"));
	version_list = new wxListBox(this, -1);
	version_list->Bind(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, [=](wxCommandEvent&) { EndModal(wxID_OK); });
	versions_box->Add(version_list, wxSizerFlags(1).Expand().Border());

	wxSizer *boxes_sizer = new wxBoxSizer(wxHORIZONTAL);
	boxes_sizer->Add(files_box, wxSizerFlags(1).Expand().Border());
	boxes_sizer->Add(versions_box, wxSizerFlags(1).Expand().Border());

	wxStdDialogButtonSizer *btn_sizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
	btn_sizer->GetAffirmativeButton()->SetLabelText(_("Open"));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(boxes_sizer, wxSizerFlags(1).Expand().Border());
	main_sizer->Add(btn_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	SetSizer(main_sizer);

	Populate();
	if (file_list->IsEmpty())
		btn_sizer->GetAffirmativeButton()->Disable();
}

void DialogAutosave::Populate() {
	wxDir dir;
	if (!dir.Open(directory)) return;

	wxString fn;
	if (!dir.GetFirst(&fn, "*.AUTOSAVE.ass", wxDIR_FILES))
		return;

	std::map<wxString, AutosaveFile> files_map;
	do {
		wxString date_str;
		wxString name = fn.Left(fn.size() - 13).BeforeLast('.', &date_str);
		if (!name) continue;

		wxDateTime date;
		if (!date.ParseFormat(date_str, "%Y-%m-%d-%H-%M-%S"))
			continue;

		auto it = files_map.find(name);
		if (it == files_map.end())
			it = files_map.emplace(name, name).first;
		it->second.versions.emplace_back(fn, date);
	} while (dir.GetNext(&fn));

	for (auto& file : files_map | boost::adaptors::map_values)
		files.emplace_back(std::move(file));

	for (auto& file : files) {
		sort(begin(file.versions), end(file.versions),
			[](Version const& a, Version const& b) { return a.date > b.date; });
	}

	sort(begin(files), end(files),
		[](AutosaveFile const& a, AutosaveFile const& b) { return a.versions[0].date > b.versions[0].date; });

	for (auto const& file : files)
		file_list->Append(file.name);
	file_list->SetSelection(0);

	wxCommandEvent evt;
	OnSelectFile(evt);
}

void DialogAutosave::OnSelectFile(wxCommandEvent&) {
	version_list->Clear();
	int sel_file = file_list->GetSelection();
	if (sel_file < 0) return;

	for (auto const& version : files[sel_file].versions)
		version_list->Append(version.date.Format());
	version_list->SetSelection(0);
}

wxString DialogAutosave::ChosenFile() const {
	int sel_file = file_list->GetSelection();
	if (sel_file < 0) return "";

	int sel_version = version_list->GetSelection();
	if (sel_version < 0) return "";

	return wxFileName(directory, files[sel_file].versions[sel_version].filename).GetFullPath();
}
