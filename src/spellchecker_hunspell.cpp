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

#ifdef WITH_HUNSPELL
#include "spellchecker_hunspell.h"

#include "options.h"

#include <libaegisub/charset_conv.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/make_unique.h>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <hunspell/hunspell.hxx>

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
		auto stream = agi::io::Open(userDicPath);
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
		free(results[i]);
	}

	free(results);

	return suggestions;
}

static std::vector<std::string> langs(const char *filter) {
	std::vector<std::string> paths;
	auto data_path = config::path->Decode("?data/dictionaries/");
	auto user_path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString());

	agi::fs::DirectoryIterator(data_path, filter).GetAll(paths);
	agi::fs::DirectoryIterator(user_path, filter).GetAll(paths);

	// Drop extensions
	for (auto& fn : paths) fn.resize(fn.size() - 4);

	boost::sort(paths);
	paths.erase(unique(begin(paths), end(paths)), end(paths));

	return paths;
}

std::vector<std::string> HunspellSpellChecker::GetLanguageList() {
	if (languages.empty())
		boost::set_intersection(langs("*.dic"), langs("*.aff"), back_inserter(languages));
	return languages;
}

static bool check_path(agi::fs::path const& path, std::string const& language, agi::fs::path& aff, agi::fs::path& dic) {
	aff = path/str(boost::format("%s.aff") % language);
	dic = path/str(boost::format("%s.dic") % language);
	return agi::fs::FileExists(aff) && agi::fs::FileExists(dic);
}

void HunspellSpellChecker::OnLanguageChanged() {
	hunspell.reset();

	auto language = OPT_GET("Tool/Spell Checker/Language")->GetString();
	if (language.empty()) return;

	agi::fs::path aff, dic;
	auto path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString() + "/");
	if (!check_path(path, language, aff, dic)) {
		path = config::path->Decode("?data/dictionaries/");
		if (!check_path(path, language, aff, dic))
			return;
	}

	LOG_I("dictionary/file") << dic;

	hunspell = agi::make_unique<Hunspell>(agi::fs::ShortName(aff).c_str(), agi::fs::ShortName(dic).c_str());
	if (!hunspell) return;

	conv = agi::make_unique<agi::charset::IconvWrapper>("utf-8", hunspell->get_dic_encoding());
	rconv = agi::make_unique<agi::charset::IconvWrapper>(hunspell->get_dic_encoding(), "utf-8");

	userDicPath = config::path->Decode("?user/dictionaries")/str(boost::format("user_%s.dic") % language);
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
