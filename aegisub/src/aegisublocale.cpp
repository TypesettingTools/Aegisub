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

/// @file aegisublocale.cpp
/// @brief Enumerate available locales for picking translation on Windows
/// @ingroup utility
///

#include "config.h"

#ifndef AGI_PRE
#include <locale.h>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include "aegisublocale.h"
#include "standard_paths.h"

AegisubLocale::~AegisubLocale() {
}

int AegisubLocale::EnglishId() const {
	static const int english_ids[] = {
		wxLANGUAGE_ENGLISH,
		wxLANGUAGE_ENGLISH_US,
		wxLANGUAGE_ENGLISH_UK,
		wxLANGUAGE_ENGLISH_AUSTRALIA,
		wxLANGUAGE_ENGLISH_BELIZE,
		wxLANGUAGE_ENGLISH_BOTSWANA,
		wxLANGUAGE_ENGLISH_CANADA,
		wxLANGUAGE_ENGLISH_CARIBBEAN,
		wxLANGUAGE_ENGLISH_DENMARK,
		wxLANGUAGE_ENGLISH_EIRE,
		wxLANGUAGE_ENGLISH_JAMAICA,
		wxLANGUAGE_ENGLISH_NEW_ZEALAND,
		wxLANGUAGE_ENGLISH_PHILIPPINES,
		wxLANGUAGE_ENGLISH_SOUTH_AFRICA,
		wxLANGUAGE_ENGLISH_TRINIDAD,
		wxLANGUAGE_ENGLISH_ZIMBABWE,
		0
	};

	for (const int *id = english_ids; *id; ++id) {
		if (wxLocale::IsAvailable(*id)) {
			return *id;
		}
	}

	return -1;
}

void AegisubLocale::Init(int language) {
	if (language == -1)
		language = EnglishId();

	if (!wxLocale::IsAvailable(language))
		language = wxLANGUAGE_UNKNOWN;

	locale.reset(new wxLocale(language));

#ifdef __WINDOWS__
	locale->AddCatalogLookupPathPrefix(StandardPaths::DecodePath("?data/locale/"));
	locale->AddCatalog("aegisub");
#else
	locale->AddCatalog(AEGISUB_COMMAND);
#endif

	locale->AddCatalog("wxstd");
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_CTYPE, "C");
}

int AegisubLocale::PickLanguage() {
	wxArrayInt langs = GetAvailableLanguages();

	// Check if english is in it, else add it
	if (langs.Index(wxLANGUAGE_ENGLISH) == wxNOT_FOUND) {
		int id = EnglishId();
		if (id)
			langs.Insert(id, 0);
	}

	// Check if user local language is available, if so, make it first
	int user = wxLocale::GetSystemLanguage();
	if (langs.Index(user) != wxNOT_FOUND) {
		langs.Remove(user);
		langs.Insert(user, 0);
	}

	// Nothing to pick
	if (langs.empty()) return -1;

	// Only one language, so don't bother asking the user
	if (langs.size() == 1 && !locale)
		return langs[0];

	// Generate names
	wxArrayString langNames;
	for (size_t i = 0; i < langs.size(); ++i)
		langNames.Add(wxLocale::GetLanguageName(langs[i]));

	long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxOK | wxCENTRE;
	if (locale)
		style |= wxCANCEL;

	wxSingleChoiceDialog dialog(NULL, "Please choose a language:", "Language", langNames,
#if wxCHECK_VERSION(2, 9, 4)
			(void **)0,
#else
			0,
#endif
			style);
	if (dialog.ShowModal() == wxID_OK) {
		int picked = dialog.GetSelection();
		if (locale && langs[picked] == locale->GetLanguage())
			return -1;
		return langs[picked];
	}

	return -1;
}

wxArrayInt AegisubLocale::GetAvailableLanguages() {
	wxArrayInt final;

#ifdef __WINDOWS__
	// Open directory
	wxString folder = StandardPaths::DecodePath("?data/locale/");
	wxDir dir;
	if (!dir.Exists(folder)) return final;
	if (!dir.Open(folder)) return final;

	// Enumerate folders
	wxString temp1;
	for (bool cont = dir.GetFirst(&temp1, "", wxDIR_DIRS); cont; cont = dir.GetNext(&temp1)) {
		// Check if .so exists inside folder
		if (wxFileName::FileExists(folder + temp1 + "/aegisub.mo")) {
			const wxLanguageInfo *lang = wxLocale::FindLanguageInfo(temp1);
			if (lang) {
				final.Add(lang->Language);
			}
		}
	}
#else
	const char* langs[] = {
		"ca",
		"cs",
		"da",
		"de",
		"el",
		"es",
		"eu",
		"fa",
		"fi",
		"fr_FR",
		"hu",
		"id",
		"it",
		"ja",
		"ko",
		"pl",
		"pt_BR",
		"pt_PT",
		"ru",
		"sr_RS",
		"sr_RS@latin",
		"vi",
		"zh_CN",
		"zh_TW"
	};

	size_t len = sizeof(langs)/sizeof(char*);
	for (size_t i=0; i<len; i++) {
		const wxLanguageInfo *lang = wxLocale::FindLanguageInfo(langs[i]);

		// If the locale file doesn't exist then don't list it as an option.
		wxString locDir = wxStandardPaths::Get().GetLocalizedResourcesDir(langs[i], wxStandardPathsBase::ResourceCat_Messages);
		wxFileName file(wxString::Format("%s/%s.mo", locDir, AEGISUB_COMMAND));
		if (lang && file.FileExists()) final.Add(lang->Language);
	}
#endif

	return final;
}
