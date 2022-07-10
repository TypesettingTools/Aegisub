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

#include <memory>
#include <string>
#include <vector>

#include <libaegisub/signal.h>

namespace agi {
class Thesaurus;
}

/// @class Thesaurus
/// @brief A wrapper around agi::Thesarus adding wx and Aegisub-specific stuff
class Thesaurus {
	/// The actual thesaurus implementation
	std::unique_ptr<agi::Thesaurus> impl;
	/// A cached list of languages available
	mutable std::vector<std::string> languages;

	/// Thesaurus language change slot
	agi::signal::Connection lang_listener;
	/// Thesaurus language change handler
	void OnLanguageChanged();

	/// Thesaurus path change slot
	agi::signal::Connection dict_path_listener;
	/// Thesaurus path change handler
	void OnPathChanged();

	bool* cancel_load = nullptr;

  public:
	/// A pair of a word and synonyms for that word
	typedef std::pair<std::string, std::vector<std::string>> Entry;

	Thesaurus();
	~Thesaurus();

	/// Get a list of synonyms for a word, grouped by possible meanings of the word
	/// @param word Word to get synonyms for
	std::vector<Entry> Lookup(std::string word);

	/// Get a list of language codes which thesauri are available for
	std::vector<std::string> GetLanguageList() const;
};
