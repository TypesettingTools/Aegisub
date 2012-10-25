// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

void Thesaurus::Lookup(wxString const& word, std::vector<Entry> *result) {
	if (!impl.get()) return;
	impl->Lookup(STD_STR(word.Lower()), result);
}

wxArrayString Thesaurus::GetLanguageList() const {
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
	for (size_t i = 0; i < idx.size(); ++i) idx[i] = idx[i].Mid(3, idx[i].size() - 7);
	for (size_t i = 0; i < dat.size(); ++i) dat[i] = dat[i].Mid(3, dat[i].size() - 7);

	// Verify that each idx has a dat
	for (size_t i = 0, j = 0; i < idx.size() && j < dat.size(); ) {
		int cmp = idx[i].Cmp(dat[j]);
		if (cmp < 0) ++i;
		else if (cmp > 0) ++j;
		else {
			// Don't insert a language twice if it's in both the user dir and
			// the app's dir
			wxString name = wxFileName(dat[j]).GetName().Mid(3);
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
