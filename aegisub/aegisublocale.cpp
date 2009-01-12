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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include <wx/wxprec.h>
#include <wx/intl.h>
#include <locale.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/choicdlg.h>
#include "aegisublocale.h"
#include "standard_paths.h"
#include <wx/stdpaths.h>

///////////////
// Constructor
AegisubLocale::AegisubLocale () {
	locale = NULL;
	curCode = -1;
}

AegisubLocale::~AegisubLocale() {
	delete locale;
}

//////////////
// Initialize
void AegisubLocale::Init(int language) {
	if (language == -1) language = wxLANGUAGE_ENGLISH;
	if (locale) delete locale;
	curCode = language;
	locale = new wxLocale(language);

#ifdef __WINDOWS__
	locale->AddCatalogLookupPathPrefix(StandardPaths::DecodePath(_T("?data/locale/")));
	locale->AddCatalog(_T("aegisub"));
#else
	locale->AddCatalog(_T(GETTEXT_PACKAGE));
#endif

	locale->AddCatalog(_T("wxstd"));
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_CTYPE, "C");
}


///////////////////
// Pick a language
int AegisubLocale::PickLanguage() {
	// Get list
	wxArrayInt langs = GetAvailableLanguages();

	// Check if english is in it, else add it
	if (langs.Index(wxLANGUAGE_ENGLISH) == wxNOT_FOUND) {
		langs.Insert(wxLANGUAGE_ENGLISH,0);
	}

	// Check if user local language is available, if so, make it first
	int user = wxLocale::GetSystemLanguage();
	if (langs.Index(user) != wxNOT_FOUND) {
		langs.Remove(user);
		langs.Insert(user,0);
	}

	// Generate names
	wxArrayString langNames;
	for (size_t i=0;i<langs.Count();i++) {
		langNames.Add(wxLocale::GetLanguageName(langs[i]));
	}

	// Nothing to pick
	if (langs.Count() == 0) return -1;

	// Popup
	int picked = wxGetSingleChoiceIndex(_T("Please choose a language:"),_T("Language"),langNames,NULL,-1,-1,true,300,400);
	if (picked == -1) return -1;
	return langs[picked];
}


///////////////////////////////////
// Get list of available languages
wxArrayInt AegisubLocale::GetAvailableLanguages() {
	wxArrayInt final;

#ifdef __WINDOWS__
	wxString temp1;

	// Open directory
	wxString folder = StandardPaths::DecodePath(_T("?data/locale/"));
	wxDir dir;
	if (!dir.Exists(folder)) return final;
	if (!dir.Open(folder)) return final;

	// Enumerate folders
	for (bool cont = dir.GetFirst(&temp1,_T(""),wxDIR_DIRS);cont;cont = dir.GetNext(&temp1)) {
		// Check if .so exists inside folder
		wxFileName file(folder + temp1 + _T("/aegisub.mo"));
		if (file.FileExists()) {
			const wxLanguageInfo *lang = wxLocale::FindLanguageInfo(temp1);
			if (lang) {
				final.Add(lang->Language);
			}
		}
	}

#else

	wchar_t* langs[] = {
		_T("ca"),
		_T("da"),
		_T("de"),
		_T("es"),
		_T("fr_FR"),
		_T("hu"),
		_T("it"),
		_T("ja"),
		_T("ko"),
		_T("ru"),
		_T("pt_BR"),
		_T("zh_TW")
	};

	size_t len = sizeof(langs)/sizeof(wchar_t*);
	for (size_t i=0; i<len; i++) {
		const wxLanguageInfo *lang = wxLocale::FindLanguageInfo(langs[i]);

		// If the locale file doesn't exist then don't list it as an option. 
		wxString locDir = wxStandardPaths::Get().GetLocalizedResourcesDir(langs[i], wxStandardPathsBase::ResourceCat_Messages);
		wxFileName file(wxString::Format(_T("%s/%s.mo"), locDir.c_str(), _T(GETTEXT_PACKAGE)));
		if (lang && file.FileExists()) final.Add(lang->Language);
	}
#endif

	return final;
}
