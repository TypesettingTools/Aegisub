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

#include "options.h"

#include <libaegisub/charset_conv.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>

#include <hunspell/hunspell.hxx>

#include <boost/format.hpp>

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

bool HunspellSpellChecker::CanRemoveWord(std::string const& word) {
	return !!customWords.count(word);
}

void HunspellSpellChecker::AddWord(std::string const& word) {
	if (!hunspell) return;

	// Add it to the in-memory dictionary
	hunspell->add(conv->Convert(word).c_str());

	// Add the word
	if (customWords.insert(word).second)
		WriteUserDictionary();
}

void HunspellSpellChecker::RemoveWord(std::string const& word) {
	if (!hunspell) return;

	// Remove it from the in-memory dictionary
	hunspell->remove(conv->Convert(word).c_str());

	auto word_iter = customWords.find(word);
	if (word_iter != customWords.end()) {
		customWords.erase(word_iter);

		WriteUserDictionary();
	}
}

void HunspellSpellChecker::ReadUserDictionary() {
	customWords.clear();

	// Read the old contents of the user's dictionary
	try {
		std::unique_ptr<std::istream> stream(agi::io::Open(userDicPath));
		copy_if(
			++agi::line_iterator<std::string>(*stream), agi::line_iterator<std::string>(),
			inserter(customWords, customWords.end()),
			[](std::string const& str) { return !str.empty(); });
	}
	catch (agi::fs::FileNotFound const&) {
		// Not an error; user dictionary just doesn't exist
	}
}

void HunspellSpellChecker::WriteUserDictionary() {
	// Ensure that the path exists
	agi::fs::CreateDirectory(userDicPath.parent_path());

	// Write the new dictionary
	{
		agi::io::Save writer(userDicPath);
		writer.Get() << customWords.size() << "\n";
		copy(customWords.begin(), customWords.end(), std::ostream_iterator<std::string>(writer.Get(), "\n"));
	}

	// Announce a language change so that any other spellcheckers reload the
	// current dictionary to get the addition/removal
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

	std::vector<std::string> dic, aff;

	// Get list of dictionaries
	auto path = config::path->Decode("?data/dictionaries/");
	agi::fs::DirectoryIterator(path, "*.dic").GetAll(dic);
	agi::fs::DirectoryIterator(path, "*.aff").GetAll(aff);

	path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString());
	agi::fs::DirectoryIterator(path, "*.dic").GetAll(dic);
	agi::fs::DirectoryIterator(path, "*.aff").GetAll(aff);

	if (dic.empty() || aff.empty()) return languages;

	sort(begin(dic), end(dic));
	sort(begin(aff), end(aff));

	// Drop extensions
	for (size_t i = 0; i < dic.size(); ++i) dic[i].resize(dic[i].size() - 4);
	for (size_t i = 0; i < aff.size(); ++i) aff[i].resize(aff[i].size() - 4);

	// Verify that each aff has a dic
	for (size_t i = 0, j = 0; i < dic.size() && j < aff.size(); ) {
		int cmp = dic[i].compare(aff[j]);
		if (cmp < 0) ++i;
		else if (cmp > 0) ++j;
		else {
			// Don't insert a language twice if it's in both the user dir and
			// the app's dir
			if (languages.empty() || aff[j] != languages.back())
				languages.push_back(aff[j]);
			++i;
			++j;
		}
	}
	return languages;
}

void HunspellSpellChecker::OnLanguageChanged() {
	hunspell.reset();

	auto language = OPT_GET("Tool/Spell Checker/Language")->GetString();
	if (language.empty()) return;

	auto custDicRoot = config::path->Decode(OPT_GET("Path/Dictionary")->GetString());
	auto dataDicRoot = config::path->Decode("?data/dictionaries");

	// If the user has a dic/aff pair in their dictionary path for this language
	// use that; otherwise use the one from Aegisub's install dir, adding words
	// from the dic in the user's dictionary path if it exists
	auto affPath = custDicRoot/(language + ".aff");
	auto dicPath = custDicRoot/(language + ".dic");
	userDicPath = config::path->Decode("?user/dictionaries")/str(boost::format("user_%s.dic") % language);
	if (!agi::fs::FileExists(affPath) || !agi::fs::FileExists(dicPath)) {
		affPath = dataDicRoot/(language + ".aff");
		dicPath = dataDicRoot/(language + ".dic");
	}

	LOG_I("dictionary/file") << dicPath;

	if (!agi::fs::FileExists(affPath) || !agi::fs::FileExists(dicPath)) {
		LOG_D("dictionary/file") << "Dictionary not found";
		return;
	}

	hunspell.reset(new Hunspell(agi::fs::ShortName(affPath).c_str(), agi::fs::ShortName(dicPath).c_str()));
	if (!hunspell) return;

	conv.reset(new agi::charset::IconvWrapper("utf-8", hunspell->get_dic_encoding()));
	rconv.reset(new agi::charset::IconvWrapper(hunspell->get_dic_encoding(), "utf-8"));

	ReadUserDictionary();

	for (auto const& word : customWords) {
		try {
			hunspell->add(conv->Convert(word).c_str());
		}
		catch (agi::charset::ConvError const&) {
			// Normally this shouldn't happen, but some versions of Aegisub
			// wrote words in the wrong charset
		}
	}
}

void HunspellSpellChecker::OnPathChanged() {
	languages.clear();
}

#endif // WITH_HUNSPELL
