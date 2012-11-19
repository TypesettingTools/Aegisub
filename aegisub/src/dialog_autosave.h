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

#ifndef AGI_PRE
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <wx/dialog.h>
#include <wx/string.h>
#endif

class wxListBox;

class DialogAutosave : public wxDialog {
	struct Version {
		wxString filename;
		wxDateTime date;
		wxString display;
		Version(wxString const& filename, wxDateTime const& date, wxString const& display)
		: filename(filename), date(date), display(display) { }
	};

	struct AutosaveFile {
		wxString name;
		std::vector<Version> versions;
		AutosaveFile(wxString const& name) : name(name) { }
	};

	std::vector<AutosaveFile> files;

	wxListBox *file_list;
	wxListBox *version_list;

	void Populate(std::map<wxString, AutosaveFile> &files_map, std::string const& path, wxString const& filter, wxString const& name_fmt);
	void OnSelectFile(wxCommandEvent&);

public:
	DialogAutosave(wxWindow *parent);
	wxString ChosenFile() const;
};
