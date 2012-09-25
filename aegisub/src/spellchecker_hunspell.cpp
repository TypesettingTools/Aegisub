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

bool HunspellSpellChecker::CanAddWord(std::string const& word) {
	if (!hunspell) return false;
	try {
		conv->Convert(word);
		return true;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

void HunspellSpellChecker::AddWord(std::string const& word) {
	if (!hunspell) return;

	// Add it to the in-memory dictionary
	hunspell->add(conv->Convert(word).c_str());

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
				[](std::string const& str) { return str.empty(); });
		}
		catch (agi::FileNotFoundError&) {
			LOG_I("dictionary/hunspell/add") << "User dictionary not found; creating it";
		}
	}

	// Add the word
	words.insert(word);

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

bool HunspellSpellChecker::CheckWord(std::string const& word) {
	if (!hunspell) return true;
	try {
		return hunspell->spell(conv->Convert(word).c_str()) == 1;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

std::vector<std::string> HunspellSpellChecker::GetSuggestions(std::string const& word) {
	std::vector<std::string> suggestions;
	if (!hunspell) return suggestions;

	char **results;
	int n = hunspell->suggest(&results, conv->Convert(word).c_str());

	suggestions.reserve(n);
	// Convert suggestions to UTF-8
	for (int i = 0; i < n; ++i) {
		try {
			suggestions.push_back(rconv->Convert(results[i]));
		}
		catch (agi::charset::ConvError const&) {
			// Shouldn't ever actually happen...
		}
		delete results[i];
	}

	delete results;

	return suggestions;
}

std::vector<std::string> HunspellSpellChecker::GetLanguageList() {
	if (!languages.empty()) return languages;

	wxArrayString dic, aff;

	// Get list of dictionaries
	wxString path = StandardPaths::DecodePath("?data/dictionaries/");
	if (wxFileName::DirExists(path)) {
		wxDir::GetAllFiles(path, &dic, "*.dic", wxDIR_FILES);
		wxDir::GetAllFiles(path, &aff, "*.aff", wxDIR_FILES);
	}
	path = StandardPaths::DecodePath(to_wx(OPT_GET("Path/Dictionary")->GetString()) + "/");
	if (wxFileName::DirExists(path)) {
		wxDir::GetAllFiles(path, &dic, "*.dic", wxDIR_FILES);
		wxDir::GetAllFiles(path, &aff, "*.aff", wxDIR_FILES);
	}
	if (aff.empty()) return std::vector<std::string>();

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
			std::string name = from_wx(wxFileName(aff[j]).GetName());
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

	wxString custDicRoot = StandardPaths::DecodePath(to_wx(OPT_GET("Path/Dictionary")->GetString()));
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
