// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file spellchecker_hunspell.cpp
/// @brief Hunspell-based spell checker implementation
/// @ingroup spelling
///

#include "config.h"

#ifdef WITH_HUNSPELL

#ifndef AGI_PRE
#include <algorithm>
#include <iterator>
#include <list>

#include <wx/dir.h>
#include <wx/filename.h>
#endif

#include <libaegisub/io.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/log.h>

#include "charset_conv.h"
#include "compat.h"
#include "main.h"
#include "spellchecker_hunspell.h"
#include "standard_paths.h"

HunspellSpellChecker::HunspellSpellChecker() {
	SetLanguage(lagi_wxString(OPT_GET("Tool/Spell Checker/Language")->GetString()));
}

HunspellSpellChecker::~HunspellSpellChecker() {
}

bool HunspellSpellChecker::CanAddWord(wxString word) {
	if (!hunspell.get()) return false;
	try {
		conv->Convert(STD_STR(word));
		return true;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

void HunspellSpellChecker::AddWord(wxString word) {
	if (!hunspell.get()) return;

	std::string sword = STD_STR(word);

	// Add it to the in-memory dictionary
#ifdef WITH_OLD_HUNSPELL
	hunspell->put_word(conv->Convert(sword).c_str());
#else
	hunspell->add(conv->Convert(sword).c_str());
#endif

	std::list<std::string> words;

	// Ensure that the path exists
	wxFileName fn(userDicPath);
	if (!fn.DirExists()) {
		wxFileName::Mkdir(fn.GetPath());
	}
	// Read the old contents of the user's dictionary
	else {
		std::auto_ptr<std::istream> stream(agi::io::Open(STD_STR(userDicPath)));
		std::remove_copy_if(
			++agi::line_iterator<std::string>(*stream.get()),
			agi::line_iterator<std::string>(),
			std::back_inserter(words),
			std::mem_fun_ref(&std::string::empty));
	}

	// Add the word
	words.push_back(sword);
	words.sort();

	// Write the new dictionary
	try {
		agi::io::Save writer(STD_STR(userDicPath));
		writer.Get() << words.size() << "\n";
		std::copy(words.begin(), words.end(), std::ostream_iterator<std::string>(writer.Get(), "\n"));
	}
	catch (const agi::Exception&) {
		// Failed to open file
	}
}

bool HunspellSpellChecker::CheckWord(wxString word) {
	if (!hunspell.get()) return true;
	try {
		return hunspell->spell(conv->Convert(STD_STR(word)).c_str()) == 1;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

wxArrayString HunspellSpellChecker::GetSuggestions(wxString word) {
	wxArrayString suggestions;
	if (!hunspell.get()) return suggestions;

	// Grab raw from Hunspell
	char **results;
	int n = hunspell->suggest(&results,conv->Convert(STD_STR(word)).c_str());

	suggestions.reserve(n);
	// Convert each
	for (int i = 0; i < n; ++i) {
		try {
			suggestions.Add(lagi_wxString(rconv->Convert(results[i])));
		}
		catch (agi::charset::ConvError const&) {
			// Shouldn't ever actually happen...
		}
		delete results[i];
	}

	delete results;

	return suggestions;
}

wxArrayString HunspellSpellChecker::GetLanguageList() {
	wxArrayString dic, aff;

	// Get list of dictionaries
	wxString path = StandardPaths::DecodePath("?data/dictionaries/");
	if (wxFileName::DirExists(path)) {
		wxDir::GetAllFiles(path, &dic, "*.dic", wxDIR_FILES);
		wxDir::GetAllFiles(path, &aff, "*.aff", wxDIR_FILES);
	}
	path = StandardPaths::DecodePath(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()) + "/");
	if (wxFileName::DirExists(path)) {
		wxDir::GetAllFiles(path, &dic, "*.dic", wxDIR_FILES);
		wxDir::GetAllFiles(path, &aff, "*.aff", wxDIR_FILES);
	}
	if (aff.empty()) return wxArrayString();

	dic.Sort();
	aff.Sort();

	// Drop extensions
	for (size_t i = 0; i < dic.size(); ++i) dic[i].resize(dic[i].size() - 4);
	for (size_t i = 0; i < aff.size(); ++i) aff[i].resize(aff[i].size() - 4);

	// Verify that each aff has a dic
	wxArrayString ret;
	for (size_t i = 0, j = 0; i < dic.size() && j < aff.size(); ) {
		int cmp = dic[i].Cmp(aff[j]);
		if (cmp < 0) ++i;
		else if (cmp > 0) ++j;
		else {
			// Don't insert a language twice if it's in both the user dir and
			// the app's dir
			wxString name = wxFileName(aff[j]).GetName();
			if (ret.empty() || name != ret.back())
				ret.push_back(name);
			++i;
			++j;
		}
	}
	return ret;
}

void HunspellSpellChecker::SetLanguage(wxString language) {
	if (language.empty()) return;

	wxString userDicRoot = StandardPaths::DecodePath(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()));
	wxString dataDicRoot = StandardPaths::DecodePath("?data/dictionaries");

	// If the user has a dic/aff pair in their dictionary path for this language
	// use that; otherwise use the one from Aegisub's install dir, adding words
	// from the dic in the user's dictionary path if it exists
	wxString affPath = wxString::Format("%s/%s.aff", userDicRoot, language);
	wxString dicPath = wxString::Format("%s/%s.dic", userDicRoot, language);
	userDicPath = wxString::Format("%s/user_%s.dic", userDicRoot, language);
	if (!wxFileExists(affPath) || !wxFileExists(dicPath)) {
		affPath = wxString::Format("%s/%s.aff", dataDicRoot, language);
		dicPath = wxString::Format("%s/%s.dic", dataDicRoot, language);
	}

	LOG_I("dictionary/file") << dicPath;

	if (!wxFileExists(affPath) || !wxFileExists(dicPath)) {
		LOG_D("dictionary/file") << "Dictionary not found";
		return;
	}

	hunspell.reset(new Hunspell(affPath.mb_str(csConvLocal), dicPath.mb_str(csConvLocal)));
	if (!hunspell.get()) return;

	conv.reset(new agi::charset::IconvWrapper("utf-8", hunspell->get_dic_encoding()));
	rconv.reset(new agi::charset::IconvWrapper(hunspell->get_dic_encoding(), "utf-8"));

	if (userDicPath == dicPath || !wxFileExists(userDicPath)) return;

	try {
		std::auto_ptr<std::istream> stream(agi::io::Open(STD_STR(userDicPath)));
		agi::line_iterator<std::string> userDic(*stream.get());
		agi::line_iterator<std::string> end;
		++userDic; // skip entry count line
		for (; userDic != end; ++userDic) {
			if ((*userDic).empty()) continue;
			try {
#ifdef WITH_OLD_HUNSPELL
				hunspell->put_word(conv->Convert(*userDic).c_str());
#else
				hunspell->add(conv->Convert(*userDic).c_str());
#endif
			}
			catch (agi::charset::ConvError const&) {
				// Normally this shouldn't happen, but some versions of Aegisub
				// wrote words in the wrong charset
			}
		}
	}
	catch (agi::Exception const&) {
		// File ceased to exist between when we checked and when we tried to
		// open it or we don't have permission to read it for whatever reason
	}
}

#endif // WITH_HUNSPELL
