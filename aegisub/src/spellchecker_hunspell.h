// Copyright (c) 2008, Rodrigo Braz Monteiro
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

/// @file spellchecker_hunspell.h
/// @see spellchecker_hunspell.cpp
/// @ingroup spelling
///

#ifdef WITH_HUNSPELL

#include <memory>
#include <hunspell/hunspell.hxx>

#include "include/aegisub/spellchecker.h"
namespace agi {
	namespace charset {
		class IconvWrapper;
	}
}

/// @class HunspellSpellChecker
/// @brief Hunspell spell checker
///
class HunspellSpellChecker : public SpellChecker {
	/// Hunspell instance
	std::auto_ptr<Hunspell> hunspell;

	/// Conversions between the dictionary charset and utf-8
	std::auto_ptr<agi::charset::IconvWrapper> conv;
	std::auto_ptr<agi::charset::IconvWrapper> rconv;

	/// Languages which we have dictionaries for
	wxArrayString languages;

	/// Path to user-local dictionary.
	wxString userDicPath;

public:
	HunspellSpellChecker();
	~HunspellSpellChecker();

	/// @brief Add word to dictionary
	/// @param word Word to add.
	void AddWord(wxString word);

	/// @brief Can add to dictionary?
	/// @param word Word to check.
	/// @return Whether word can be added or not.
	bool CanAddWord(wxString word);

	/// @brief Check if the word is valid.
	/// @param word Word to check
	/// @return Whether word is valid or not.
	bool CheckWord(wxString word);

	/// @brief Get suggestions for word.
	/// @param word Word to get suggestions for
	/// @return List of suggestions
	wxArrayString GetSuggestions(wxString word);

	/// @brief Get a list of languages which dictionaries are present for
	wxArrayString GetLanguageList();
	/// @brief Set the spellchecker's language
	/// @param language Language code
	void SetLanguage(wxString language);
};

#endif
