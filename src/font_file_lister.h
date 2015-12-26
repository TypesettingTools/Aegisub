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

#include <libaegisub/fs_fwd.h>
#include <libaegisub/scoped_ptr.h>

#include <boost/filesystem/path.hpp>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include <wx/string.h>

class AssDialogue;
class AssFile;

typedef std::function<void (wxString, int)> FontCollectorStatusCallback;

struct CollectionResult {
	/// Characters which could not be found in any font files
	wxString missing;
	/// Paths to the file(s) containing the requested font
	std::vector<agi::fs::path> paths;
	bool fake_bold = false;
	bool fake_italic = false;
};

#ifdef _WIN32
class GdiFontFileLister {
	std::unordered_multimap<uint32_t, agi::fs::path> index;
	agi::scoped_holder<HDC> dc;
	std::string buffer;

	bool ProcessLogFont(LOGFONTW const& expected, LOGFONTW const& actual, std::vector<int> const& characters);

public:
	/// Constructor
	/// @param cb Callback for status logging
	GdiFontFileLister(FontCollectorStatusCallback &cb);

	/// @brief Get the path to the font with the given styles
	/// @param facename Name of font face
	/// @param bold ASS font weight
	/// @param italic Italic?
	/// @param characters Characters in this style
	/// @return Path to the matching font file(s), or empty if not found
	CollectionResult GetFontPaths(std::string const& facename, int bold, bool italic, std::vector<int> const& characters);
};

using FontFileLister = GdiFontFileLister;
#else
typedef struct _FcConfig FcConfig;
typedef struct _FcFontSet FcFontSet;

/// @class FontConfigFontFileLister
/// @brief fontconfig powered font lister
class FontConfigFontFileLister {
	agi::scoped_holder<FcConfig*> config;

	/// @brief Case-insensitive match ASS/SSA font family against full name. (also known as "name for humans")
	/// @param family font fullname
	/// @param bold weight attribute
	/// @param italic italic attribute
	/// @return font set
	FcFontSet *MatchFullname(const char *family, int weight, int slant);
public:
	/// Constructor
	/// @param cb Callback for status logging
	FontConfigFontFileLister(FontCollectorStatusCallback &cb);

	/// @brief Get the path to the font with the given styles
	/// @param facename Name of font face
	/// @param bold ASS font weight
	/// @param italic Italic?
	/// @param characters Characters in this style
	/// @return Path to the matching font file(s), or empty if not found
	CollectionResult GetFontPaths(std::string const& facename, int bold, bool italic, std::vector<int> const& characters);
};

using FontFileLister = FontConfigFontFileLister;
#endif

/// @class FontCollector
/// @brief Class which collects the paths to all fonts used in a script
class FontCollector {
	/// All data needed to find the font file used to render text
	struct StyleInfo {
		std::string facename;
		int bold;
		bool italic;
		bool operator<(StyleInfo const& rgt) const;
	};

	/// Data about where each style is used
	struct UsageData {
		std::vector<int> chars;          ///< Characters used in this style which glyphs will be needed for
		std::vector<int> lines;          ///< Lines on which this style is used via overrides
		std::vector<std::string> styles; ///< ASS styles which use this style
	};

	/// Message callback provider by caller
	FontCollectorStatusCallback status_callback;

	FontFileLister lister;

	/// The set of all glyphs used in the file
	std::map<StyleInfo, UsageData> used_styles;
	/// Style name -> ASS style definition
	std::map<std::string, StyleInfo> styles;
	/// Paths to found required font files
	std::vector<agi::fs::path> results;
	/// Number of fonts which could not be found
	int missing = 0;
	/// Number of fonts which were found, but did not contain all used glyphs
	int missing_glyphs = 0;

	/// Gather all of the unique styles with text on a line
	void ProcessDialogueLine(const AssDialogue *line, int index);

	/// Get the font for a single style
	void ProcessChunk(std::pair<StyleInfo, UsageData> const& style);

	/// Print the lines and styles on which a missing font is used
	void PrintUsage(UsageData const& data);

public:
	/// Constructor
	/// @param status_callback Function to pass status updates to
	/// @param lister The actual font file lister
	FontCollector(FontCollectorStatusCallback status_callback);

	/// @brief Get a list of the locations of all font files used in the file
	/// @param file Lines in the subtitle file to check
	/// @param status Callback function for messages
	/// @return List of paths to fonts
	std::vector<agi::fs::path> GetFontPaths(const AssFile *file);
};
