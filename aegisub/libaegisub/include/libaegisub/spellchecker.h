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

#pragma once

#include <string>
#include <vector>

namespace agi {
class SpellChecker {
public:
	virtual ~SpellChecker() { }

	/// Add word to the dictionary
	/// @param word Word to add
	virtual void AddWord(std::string const& word)=0;

	/// Can the word be added to the current dictionary?
	/// @param word Word to check
	/// @return Whether or not word can be added
	virtual bool CanAddWord(std::string const& word)=0;

	/// Check if the given word is spelled correctly
	/// @param word Word to check
	/// @return Whether or not the word is valid
	virtual bool CheckWord(std::string const& word)=0;

	/// Get possible corrections for a misspelled word
	/// @param word Word to get suggestions for
	/// @return List of suggestions, if any
	virtual std::vector<std::string> GetSuggestions(std::string const& word)=0;

	/// Get a list of languages which dictionaries are present for
	virtual std::vector<std::string> GetLanguageList()=0;
};

}
