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

/// @file font_file_lister.h
/// @see font_file_lister.cpp
/// @ingroup font_collector
///

#pragma once

#ifndef AGI_PRE
#include <list>
#include <map>
#include <tr1/functional>
#include <set>
#include <vector>

#include <wx/string.h>
#endif

class AssEntry;
class AssDialogue;

typedef std::tr1::function<void (wxString, int)> FontCollectorStatusCallback;

/// @class FontFileLister
/// @brief Font lister interface
class FontFileLister {
public:
	struct CollectionResult {
		/// Characters which could not be found in any font files
		wxString missing;
		/// Paths to the file(s) containing the requested font
		std::vector<wxString> paths;
	};

	/// @brief Get the path to the font with the given styles
	/// @param facename Name of font face
	/// @param bold ASS font weight
	/// @param italic Italic?
	/// @param characters Characters in this style
	/// @return Path to the matching font file(s), or empty if not found
	virtual CollectionResult GetFontPaths(wxString const& facename, int bold, bool italic, std::set<wxUniChar> const& characters) = 0;
};

/// @class FontCollector
/// @brief Class which collects the paths to all fonts used in a script
class FontCollector {
	/// All data needed to find the font file used to render text
	struct StyleInfo {
		wxString facename;
		int bold;
		bool italic;
		bool operator<(StyleInfo const& rgt) const;
	};

	/// Data about where each style is used
	struct UsageData {
		std::set<wxUniChar> chars; ///< Characters used in this style which glyphs will be needed for
		std::set<int> lines;       ///< Lines on which this style is used via overrides
		std::set<wxString> styles; ///< ASS styles which use this style
	};

	/// Message callback provider by caller
	FontCollectorStatusCallback status_callback;
	/// The actual lister to use to get font paths
	FontFileLister &lister;

	/// The set of all glyphs used in the file
	std::map<StyleInfo, UsageData> used_styles;
	/// Style name -> ASS style definition
	std::map<wxString, StyleInfo> styles;
	/// Paths to found required font files
	std::set<wxString> results;
	/// Number of fonts which could not be found
	int missing;
	/// Number of fonts which were found, but did not contain all used glyphs
	int missing_glyphs;

	/// Gather all of the unique styles with text on a line
	void ProcessDialogueLine(AssDialogue *line, int index);

	/// Get the font for a single style
	void ProcessChunk(std::pair<StyleInfo, UsageData> const& style);

	/// Print the lines and styles on which a missing font is used
	void PrintUsage(UsageData const& data);

public:
	/// Constructor
	/// @param status_callback Function to pass status updates to
	/// @param lister The actual font file lister
	FontCollector(FontCollectorStatusCallback status_callback, FontFileLister &lister);

	/// @brief Get a list of the locations of all font files used in the file
	/// @param file Lines in the subtitle file to check
	/// @param status Callback function for messages
	/// @return List of paths to fonts
	std::vector<wxString> GetFontPaths(std::list<AssEntry*> const& file);
};
