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

#include "dialog_autosave.h"

#include "compat.h"
#include "libresrc/libresrc.h"
#include "options.h"

#include <libaegisub/path.h>

#include <boost/range/adaptor/map.hpp>

#include <map>

#include <wx/button.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

DialogAutosave::DialogAutosave(wxWindow *parent)
: wxDialog(parent, -1, _("Open autosave file"), wxDefaultPosition, wxSize(800, 350), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	SetIcon(GETICON(open_toolbutton_16));

	wxSizer *files_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Files"));
	file_list = new wxListBox(this, -1);
	file_list->Bind(wxEVT_LISTBOX, &DialogAutosave::OnSelectFile, this);
	files_box->Add(file_list, wxSizerFlags(1).Expand().Border());

	wxSizer *versions_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Versions"));
	version_list = new wxListBox(this, -1);
	version_list->Bind(wxEVT_LISTBOX_DCLICK, [=](wxCommandEvent&) { EndModal(wxID_OK); });
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

	std::map<wxString, AutosaveFile> files_map;
	Populate(files_map, OPT_GET("Path/Auto/Save")->GetString(), ".AUTOSAVE.ass", "%s");
	Populate(files_map, OPT_GET("Path/Auto/Backup")->GetString(), ".ORIGINAL.ass", _("%s [ORIGINAL BACKUP]"));
	Populate(files_map, "?user/recovered", ".ass", _("%s [RECOVERED]"));

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

	if (file_list->IsEmpty())
		btn_sizer->GetAffirmativeButton()->Disable();
	else {
		file_list->SetSelection(0);
		wxCommandEvent evt;
		OnSelectFile(evt);
	}
}

void DialogAutosave::Populate(std::map<wxString, AutosaveFile> &files_map, std::string const& path, wxString const& filter, wxString const& name_fmt) {
	wxString directory(config::path->Decode(path).wstring());

	wxDir dir;
	if (!dir.Open(directory)) return;

	wxString fn;
	if (!dir.GetFirst(&fn, "*" + filter, wxDIR_FILES))
		return;

	do {
		wxDateTime date;

		wxString date_str;
		wxString name = fn.Left(fn.size() - filter.size()).BeforeLast('.', &date_str);
		if (!name)
			name = date_str;
		else {
			if (!date.ParseFormat(date_str, "%Y-%m-%d-%H-%M-%S"))
				name += "." + date_str;
		}
		if (!date.IsValid())
			date = wxFileName(directory, fn).GetModificationTime();

		auto it = files_map.find(name);
		if (it == files_map.end())
			it = files_map.insert(std::make_pair(name, AutosaveFile{name})).first;
		it->second.versions.push_back(Version{wxFileName(directory, fn).GetFullPath(), date, wxString::Format(name_fmt, date.Format())});
	} while (dir.GetNext(&fn));
}

void DialogAutosave::OnSelectFile(wxCommandEvent&) {
	version_list->Clear();
	int sel_file = file_list->GetSelection();
	if (sel_file < 0) return;

	for (auto const& version : files[sel_file].versions)
		version_list->Append(version.display);
	version_list->SetSelection(0);
}

std::string DialogAutosave::ChosenFile() const {
	int sel_file = file_list->GetSelection();
	if (sel_file < 0) return "";

	int sel_version = version_list->GetSelection();
	if (sel_version < 0) return "";

	return from_wx(files[sel_file].versions[sel_version].filename);
}
