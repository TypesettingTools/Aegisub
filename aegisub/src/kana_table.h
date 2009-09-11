// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file kana_table.h
/// @see kana_table.cpp
/// @ingroup kara_timing_copy
///


///////////
// Headers
#ifndef AGI_PRE
#include <list>

#include <wx/string.h>
#endif


/// @class KanaEntry
/// @brief Base class for Kana + Romaji tuples.
class KanaEntry {
public:

	/// Hiragana
	wxString hiragana;

	/// Katakana
	wxString katakana;

	/// Hepburn romaji.
	wxString hepburn;


	/// @brief Constructor
	KanaEntry() {}

	KanaEntry(const wxString &hira, const wxString &kata, const wxString &hep) {
		hiragana = hira;
		katakana = kata;
		hepburn = hep;
	}
};



/// @class KanaTable
/// @brief Table of Hiragana, Katakana and Hepburn romaji tuples.
///
class KanaTable {
private:
	void Insert(const wchar_t *hira, const wchar_t *kata, const wchar_t *hep);

public:

	/// Memory list.
	std::list<KanaEntry> entries;
	KanaTable();
	~KanaTable();
};
