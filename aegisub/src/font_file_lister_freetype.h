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
//
// $Id$

/// @file font_file_lister_freetype.h
/// @see font_file_lister_freetype.cpp
/// @ingroup font_collector
///

#ifdef WITH_FREETYPE2

#include "font_file_lister.h"

#ifndef AGI_PRE
#include <map>
#include <set>

#include <wx/string.h>
#endif

/// @class FreetypeFontFileLister
/// @brief Freetype2-based font collector
class FreetypeFontFileLister : public FontFileLister {
	/// Map of face name -> possible containing files
	std::map<wxString, std::set<wxString> > font_files;

	/// Set of files which were loaded from the cache
	std::set<wxString> indexed_files;

	/// Add a font to the list of mappings
	/// @param filename Full path to font file
	/// @param facename Font name
	void AddFont(wxString const& filename, wxString facename);

	/// Add a font to the list of mappings
	/// @param filename Full path to font file
	/// @param family Family name
	/// @param style Style name
	void AddFont(wxString const& filename, wxString const& family, wxString const& style);

public:
	/// Constructor
	/// @param cb Callback for status logging
	FreetypeFontFileLister(FontCollectorStatusCallback cb);

	CollectionResult GetFontPaths(wxString const& facename, int, bool, std::set<wxUniChar> const&);
};

#endif
