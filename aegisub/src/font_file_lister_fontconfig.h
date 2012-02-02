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

/// @file font_file_lister_fontconfig.h
/// @see font_file_lister_fontconfig.cpp
/// @ingroup font_collector
///

#ifdef WITH_FONTCONFIG

#include "font_file_lister.h"

typedef struct _FcConfig FcConfig;
typedef struct _FcFontSet FcFontSet;

/// @class FontConfigFontFileLister
/// @brief fontconfig powered font lister
class FontConfigFontFileLister : public FontFileLister {
	template<typename T> class scoped {
		T data;
		void (*destructor)(T);
	public:
		scoped(T data, void (*destructor)(T)) : data(data), destructor(destructor) { }
		~scoped() { if (data) destructor(data); }
		operator T() { return data; }
		T operator->() { return data; }
	};

	scoped<FcConfig*> config;

	/// @brief Case-insensitive match ASS/SSA font family against full name. (also known as "name for humans")
	/// @param family font fullname
	/// @param bold weight attribute
	/// @param italic italic attribute
	/// @return font set
	FcFontSet *MatchFullname(const char *family, int weight, int slant);
public:
	/// Constructor
	/// @param cb Callback for status logging
	FontConfigFontFileLister(FontCollectorStatusCallback cb);

	CollectionResult GetFontPaths(wxString const& facename, int bold, bool italic, std::set<wxUniChar> const& characters);
};

#endif
