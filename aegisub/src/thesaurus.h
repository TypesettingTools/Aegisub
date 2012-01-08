// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file thesaurus.h
/// @see thesaurus.cpp
/// @ingroup thesaurus
///

#ifndef AGI_PRE
#include <vector>

#include <wx/arrstr.h>
#include <wx/string.h>
#endif

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>

namespace agi { class Thesaurus; }

/// @class Thesaurus
/// @brief A wrapper around agi::Thesarus adding wx and Aegisub-specific stuff
class Thesaurus {
	/// The actual thesarus implementation
	agi::scoped_ptr<agi::Thesaurus> impl;
	/// A cached list of languages available
	mutable wxArrayString languages;

	/// Thesaurus language change slot
	agi::signal::Connection lang_listener;
	/// Thesaurus language change handler
	void OnLanguageChanged();

	/// Thesaurus path change slot
	agi::signal::Connection dict_path_listener;
	/// Thesaurus path change handler
	void OnPathChanged();
public:
	/// A pair of a word and synonyms for that word
	typedef std::pair<std::string, std::vector<std::string> > Entry;

	Thesaurus();
	~Thesaurus();

	/// Get a list of synonyms for a word, grouped by possible meanings of the word
	/// @param word Word to get synonyms for
	/// @param[out] result Output list
	void Lookup(wxString const& word, std::vector<Entry> *result);

	/// Get a list of language codes which thesauri are available for
	wxArrayString GetLanguageList() const;
};
