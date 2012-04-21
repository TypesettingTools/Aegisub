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

#include "spellchecker_hunspell.h"

#ifndef AGI_PRE
#include <algorithm>
#include <iterator>
#include <set>

#include <wx/dir.h>
#include <wx/filename.h>
#endif

#include <hunspell/hunspell.hxx>

#include <libaegisub/io.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/log.h>

#include "charset_conv.h"
#include "compat.h"
#include "main.h"
#include "standard_paths.h"

HunspellSpellChecker::HunspellSpellChecker()
: lang_listener(OPT_SUB("Tool/Spell Checker/Language", &HunspellSpellChecker::OnLanguageChanged, this))
, dict_path_listener(OPT_SUB("Path/Dictionary", &HunspellSpellChecker::OnPathChanged, this))
{
	OnLanguageChanged();
}

HunspellSpellChecker::~HunspellSpellChecker() {
}

bool HunspellSpellChecker::CanAddWord(wxString word) {
	if (!hunspell) return false;
	try {
		conv->Convert(STD_STR(word));
		return true;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

void HunspellSpellChecker::AddWord(wxString word) {
	if (!hunspell) return;

	std::string sword = STD_STR(word);

	// Add it to the in-memory dictionary
	hunspell->add(conv->Convert(sword).c_str());

	std::set<std::string> words;

	// Ensure that the path exists
	wxFileName fn(userDicPath);
	if (!fn.DirExists()) {
		wxFileName::Mkdir(fn.GetPath());
	}
	// Read the old contents of the user's dictionary
	else {
		try {
			agi::scoped_ptr<std::istream> stream(agi::io::Open(STD_STR(userDicPath)));
			remove_copy_if(
				++agi::line_iterator<std::string>(*stream),
				agi::line_iterator<std::string>(),
				inserter(words, words.end()),
				mem_fun_ref(&std::string::empty));
		}
		catch (agi::FileNotFoundError&) {
			LOG_I("dictionary/hunspell/add") << "User dictionary not found; creating it";
		}
	}

	// Add the word
	words.insert(sword);

	// Write the new dictionary
	{
		agi::io::Save writer(STD_STR(userDicPath));
		writer.Get() << words.size() << "\n";
		copy(words.begin(), words.end(), std::ostream_iterator<std::string>(writer.Get(), "\n"));
	}

	// Announce a language change so that any other spellcheckers pick up the
	// new word
	lang_listener.Block();
	OPT_SET("Tool/Spell Checker/Language")->SetString(OPT_GET("Tool/Spell Checker/Language")->GetString());
	lang_listener.Unblock();
}

bool HunspellSpellChecker::CheckWord(wxString word) {
	if (!hunspell) return true;
	try {
		return hunspell->spell(conv->Convert(STD_STR(word)).c_str()) == 1;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

wxArrayString HunspellSpellChecker::GetSuggestions(wxString word) {
	wxArrayString suggestions;
	if (!hunspell) return suggestions;

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
	if (!languages.empty()) return languages;

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
	for (size_t i = 0, j = 0; i < dic.size() && j < aff.size(); ) {
		int cmp = dic[i].Cmp(aff[j]);
		if (cmp < 0) ++i;
		else if (cmp > 0) ++j;
		else {
			// Don't insert a language twice if it's in both the user dir and
			// the app's dir
			wxString name = wxFileName(aff[j]).GetName();
			if (languages.empty() || name != languages.back())
				languages.push_back(name);
			++i;
			++j;
		}
	}
	return languages;
}

void HunspellSpellChecker::OnLanguageChanged() {
	hunspell.reset();

	std::string language = OPT_GET("Tool/Spell Checker/Language")->GetString();
	if (language.empty()) return;

	wxString custDicRoot = StandardPaths::DecodePath(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()));
	wxString dataDicRoot = StandardPaths::DecodePath("?data/dictionaries");

	// If the user has a dic/aff pair in their dictionary path for this language
	// use that; otherwise use the one from Aegisub's install dir, adding words
	// from the dic in the user's dictionary path if it exists
	wxString affPath = wxString::Format("%s/%s.aff", custDicRoot, language);
	wxString dicPath = wxString::Format("%s/%s.dic", custDicRoot, language);
	userDicPath = wxString::Format("%s/user_%s.dic", StandardPaths::DecodePath("?user/dictionaries"), language);
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
	if (!hunspell) return;

	conv.reset(new agi::charset::IconvWrapper("utf-8", hunspell->get_dic_encoding()));
	rconv.reset(new agi::charset::IconvWrapper(hunspell->get_dic_encoding(), "utf-8"));

	try {
		agi::scoped_ptr<std::istream> stream(agi::io::Open(STD_STR(userDicPath)));
		agi::line_iterator<std::string> userDic(*stream);
		agi::line_iterator<std::string> end;
		++userDic; // skip entry count line
		for (; userDic != end; ++userDic) {
			if (userDic->empty()) continue;
			try {
				hunspell->add(conv->Convert(*userDic).c_str());
			}
			catch (agi::charset::ConvError const&) {
				// Normally this shouldn't happen, but some versions of Aegisub
				// wrote words in the wrong charset
			}
		}
	}
	catch (agi::Exception const&) {
		// File doesn't exist or we don't have permission to read it
	}
}

void HunspellSpellChecker::OnPathChanged() {
	languages.clear();
}

#endif // WITH_HUNSPELL
