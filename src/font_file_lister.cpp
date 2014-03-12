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

/// @file font_file_lister.cpp
/// @brief Base-class for font collector implementations
/// @ingroup font_collector
///

#include "config.h"

#include "font_file_lister.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "utils.h"

#include <algorithm>
#include <tuple>
#include <unicode/uchar.h>
#include <wx/intl.h>

using namespace std::placeholders;

namespace {
	wxString format_missing(wxString const& str) {
		wxString printable;
		wxString unprintable;
		for (wxUniChar c : str) {
			if (!u_isUWhiteSpace(c.GetValue()))
				printable += c;
			else {
				unprintable += wxString::Format("\n - U+%04X ", c.GetValue());
				UErrorCode ec;
				char buf[1024];
				auto len = u_charName(c.GetValue(), U_EXTENDED_CHAR_NAME, buf, sizeof buf, &ec);
				if (len != 0 && U_SUCCESS(ec))
					unprintable += to_wx(buf);
				if (c.GetValue() == 0xA0)
					unprintable += " (\\h)";
			}
		}

		return printable + unprintable;
	}
}

FontCollector::FontCollector(FontCollectorStatusCallback status_callback, FontFileLister &lister)
: status_callback(std::move(status_callback))
, lister(lister)
{
}

void FontCollector::ProcessDialogueLine(const AssDialogue *line, int index) {
	if (line->Comment) return;

	auto style_it = styles.find(line->Style);
	if (style_it == end(styles)) {
		status_callback(wxString::Format(_("Style '%s' does not exist\n"), to_wx(line->Style)), 2);
		++missing;
		return;
	}

	boost::ptr_vector<AssDialogueBlock> blocks(line->ParseTags());
	StyleInfo style = style_it->second;
	StyleInfo initial = style;

	bool overriden = false;

	for (auto& block : blocks) {
		if (AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride *>(&block)) {
			for (auto const& tag : ovr->Tags) {
				std::string const& name = tag.Name;

				if (name == "\\r") {
					style = styles[tag.Params[0].Get(line->Style.get())];
					overriden = false;
				}
				else if (name == "\\b") {
					style.bold = tag.Params[0].Get(initial.bold);
					overriden = true;
				}
				else if (name == "\\i") {
					style.italic = tag.Params[0].Get(initial.italic);
					overriden = true;
				}
				else if (name == "\\fn") {
					style.facename = tag.Params[0].Get(initial.facename);
					overriden = true;
				}
			}
		}
		else if (AssDialogueBlockPlain *txt = dynamic_cast<AssDialogueBlockPlain *>(&block)) {
			wxString text(to_wx(txt->GetText()));

			if (text.empty())
				continue;

			if (overriden)
				used_styles[style].lines.insert(index);
			std::set<wxUniChar>& chars = used_styles[style].chars;
			for (auto it = text.begin(); it != text.end(); ++it) {
				wxUniChar cur = *it;
				if (cur == L'\\' && it + 1 != text.end()) {
					wxUniChar next = *++it;
					if (next == 'N' || next == 'n')
						continue;
					if (next == 'h')
						cur = 0xA0;
					else
						--it;
				}
				chars.insert(cur);
			}
		}
		// Do nothing with drawing and comment blocks
	}
}

void FontCollector::ProcessChunk(std::pair<StyleInfo, UsageData> const& style) {
	if (style.second.chars.empty()) return;

	FontFileLister::CollectionResult res = lister.GetFontPaths(style.first.facename, style.first.bold, style.first.italic, style.second.chars);

	if (res.paths.empty()) {
		status_callback(wxString::Format(_("Could not find font '%s'\n"), to_wx(style.first.facename)), 2);
		PrintUsage(style.second);
		++missing;
	}
	else {
		for (auto& elem : res.paths) {
			if (results.insert(elem).second)
				status_callback(wxString::Format(_("Found '%s' at '%s'\n"), to_wx(style.first.facename), elem.make_preferred().wstring()), 0);
		}

		if (res.missing.size()) {
			if (res.missing.size() > 50)
				status_callback(wxString::Format(_("'%s' is missing %d glyphs used.\n"), to_wx(style.first.facename), (int)res.missing.size()), 2);
			else if (res.missing.size() > 0)
				status_callback(wxString::Format(_("'%s' is missing the following glyphs used: %s\n"), to_wx(style.first.facename), format_missing(res.missing)), 2);
			PrintUsage(style.second);
			++missing_glyphs;
		}
	}
}

void FontCollector::PrintUsage(UsageData const& data) {
	if (data.styles.size()) {
		status_callback(wxString::Format(_("Used in styles:\n")), 2);
		for (auto const& style : data.styles)
			status_callback(wxString::Format("  - %s\n", style), 2);
	}

	if (data.lines.size()) {
		status_callback(wxString::Format(_("Used on lines:")), 2);
		for (int line : data.lines)
			status_callback(wxString::Format(" %d", line), 2);
		status_callback("\n", 2);
	}
	status_callback("\n", 2);
}

std::vector<agi::fs::path> FontCollector::GetFontPaths(const AssFile *file) {
	missing = 0;
	missing_glyphs = 0;

	status_callback(_("Parsing file\n"), 0);

	for (auto const& style : file->Styles) {
		StyleInfo &info = styles[style.name];
		info.facename = style.font;
		info.bold     = style.bold;
		info.italic   = style.italic;
		used_styles[info].styles.insert(style.name);
	}

	int index = 0;
	for (auto const& diag : file->Events)
		ProcessDialogueLine(&diag, ++index);

	status_callback(_("Searching for font files\n"), 0);
	for_each(used_styles.begin(), used_styles.end(), bind(&FontCollector::ProcessChunk, this, _1));
	status_callback(_("Done\n\n"), 0);

	std::vector<agi::fs::path> paths;
	paths.reserve(results.size());
	paths.insert(paths.end(), results.begin(), results.end());

	if (missing == 0)
		status_callback(_("All fonts found.\n"), 1);
	else
		status_callback(wxString::Format(wxPLURAL("One font could not be found\n", "%d fonts could not be found.\n", missing), missing), 2);
	if (missing_glyphs != 0)
		status_callback(wxString::Format(wxPLURAL(
				"One font was found, but was missing glyphs used in the script.\n",
				"%d fonts were found, but were missing glyphs used in the script.\n",
				missing_glyphs),
			missing_glyphs), 2);

	return paths;
}

bool FontCollector::StyleInfo::operator<(StyleInfo const& rgt) const {
	return std::tie(facename, bold, italic) < std::tie(rgt.facename, rgt.bold, rgt.italic);
}
