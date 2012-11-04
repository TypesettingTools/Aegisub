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

/// @file thesaurus.cpp
/// @brief Thesaurus implementation
/// @ingroup thesaurus
///

#include "config.h"

#include "thesaurus.h"

#ifndef AGI_PRE
#include <wx/dir.h>
#include <wx/filename.h>
#endif

#include <libaegisub/log.h>
#include <libaegisub/thesaurus.h>
#include <libaegisub/util.h>

#include "compat.h"
#include "main.h"
#include "standard_paths.h"

Thesaurus::Thesaurus()
: lang_listener(OPT_SUB("Tool/Thesaurus/Language", &Thesaurus::OnLanguageChanged, this))
, dict_path_listener(OPT_SUB("Path/Dictionary", &Thesaurus::OnPathChanged, this))
{
	OnLanguageChanged();
}

Thesaurus::~Thesaurus() {
	// Explicit empty destructor needed for scoped_ptr with incomplete types
}

void Thesaurus::Lookup(std::string word, std::vector<Entry> *result) {
	if (!impl.get()) return;
	agi::util::str_lower(word);
	impl->Lookup(word, result);
}

std::vector<std::string> Thesaurus::GetLanguageList() const {
	if (!languages.empty()) return languages;

	wxArrayString idx, dat;

	// Get list of dictionaries
	wxString path = StandardPaths::DecodePath("?data/dictionaries/");
	if (wxFileName::DirExists(path)) {
		wxDir::GetAllFiles(path, &idx, "th_*.idx", wxDIR_FILES);
		wxDir::GetAllFiles(path, &dat, "th_*.dat", wxDIR_FILES);
	}
	path = StandardPaths::DecodePath(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()) + "/");
	if (wxFileName::DirExists(path)) {
		wxDir::GetAllFiles(path, &idx, "th_*.idx", wxDIR_FILES);
		wxDir::GetAllFiles(path, &dat, "th_*.dat", wxDIR_FILES);
	}
	if (idx.empty() || dat.empty()) return languages;

	idx.Sort();
	dat.Sort();

	// Drop extensions and the th_ prefix
	for (auto& fn : idx) fn = fn.Mid(3, fn.size() - 7);
	for (auto& fn : dat) fn = fn.Mid(3, fn.size() - 7);

	// Verify that each idx has a dat
	for (size_t i = 0, j = 0; i < idx.size() && j < dat.size(); ) {
		int cmp = idx[i].Cmp(dat[j]);
		if (cmp < 0) ++i;
		else if (cmp > 0) ++j;
		else {
			// Don't insert a language twice if it's in both the user dir and
			// the app's dir
			std::string name = from_wx(wxFileName(dat[j]).GetName().Mid(3));
			if (languages.empty() || name != languages.back())
				languages.push_back(name);
			++i;
			++j;
		}
	}
	return languages;
}

void Thesaurus::OnLanguageChanged() {
	impl.reset();

	std::string language = OPT_GET("Tool/Thesaurus/Language")->GetString();
	if (language.empty()) return;

	wxString path = StandardPaths::DecodePath(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()) + "/");

	// Get index and data paths
	wxString idxpath = wxString::Format("%s/th_%s.idx", path, language);
	wxString datpath = wxString::Format("%s/th_%s.dat", path, language);

	// If they aren't in the user dictionary path, check the application directory
	if (!wxFileExists(idxpath) || !wxFileExists(datpath)) {
		path = StandardPaths::DecodePath("?data/dictionaries/");
		idxpath = wxString::Format("%s/th_%s.idx", path, language);
		datpath = wxString::Format("%s/th_%s.dat", path, language);

		if (!wxFileExists(idxpath) || !wxFileExists(datpath)) return;
	}

	LOG_I("thesaurus/file") << "Using thesaurus: " << datpath.c_str();

	impl.reset(new agi::Thesaurus(STD_STR(datpath), STD_STR(idxpath)));
}

void Thesaurus::OnPathChanged() {
	languages.clear();
}
