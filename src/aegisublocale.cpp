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

/// @file aegisublocale.cpp
/// @brief Enumerate available locales for picking translation on Windows
/// @ingroup utility
///

#include "aegisublocale.h"

#include "options.h"

#include <libaegisub/path.h>

#include <algorithm>
#include <boost/locale.hpp>
#include <clocale>
#include <functional>

#include <wx/intl.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

#ifndef AEGISUB_CATALOG
#define AEGISUB_CATALOG "aegisub"
#endif

wxTranslations *AegisubLocale::GetTranslations() {
	wxTranslations *translations = wxTranslations::Get();
	if (!translations) {
		wxTranslations::Set(translations = new wxTranslations);
		wxFileTranslationsLoader::AddCatalogLookupPathPrefix(config::path->Decode("?data/locale/").wstring());
	}
	return translations;
}

void AegisubLocale::Init(wxString const& language) {
	wxTranslations *translations = GetTranslations();
	translations->SetLanguage(language);
	translations->AddCatalog(AEGISUB_CATALOG);
	translations->AddStdCatalog();

	setlocale(LC_NUMERIC, "C");
	setlocale(LC_CTYPE, "C");
	active_language = language;
}

wxString AegisubLocale::PickLanguage() {
	if (!active_language) {
		wxString os_ui_language = GetTranslations()->GetBestTranslation(AEGISUB_CATALOG);
		if (!os_ui_language.empty())
			return os_ui_language;
	}

	wxArrayString langs = GetTranslations()->GetAvailableTranslations(AEGISUB_CATALOG);
	langs.insert(langs.begin(), "en_US");

	// Check if user local language is available, if so, make it first
	const wxLanguageInfo *info = wxLocale::GetLanguageInfo(wxLocale::GetSystemLanguage());
	if (info) {
		auto it = std::find(langs.begin(), langs.end(), info->CanonicalName);
		if (it != langs.end())
			std::rotate(langs.begin(), it, it + 1);
	}

	// Nothing to pick
	if (langs.empty()) return "";

	// Only one language, so don't bother asking the user
	if (langs.size() == 1 && !active_language)
		return langs[0];

	// Generate names
	wxArrayString langNames;
	for (auto const& lang : langs) {
		const wxLanguageInfo *info = wxLocale::FindLanguageInfo(lang);
		if (info)
			langNames.push_back(wxLocale::GetLanguageName(info->Language));
		else
			langNames.push_back(lang);
	}

	long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxOK | wxCENTRE;
	if (!active_language.empty())
		style |= wxCANCEL;

	wxSingleChoiceDialog dialog(nullptr, "Please choose a language:", "Language", langNames,
			(void **)nullptr,
			style);
	if (dialog.ShowModal() == wxID_OK) {
		int picked = dialog.GetSelection();
		if (langs[picked] != active_language)
			return langs[picked];
	}

	return "";
}
