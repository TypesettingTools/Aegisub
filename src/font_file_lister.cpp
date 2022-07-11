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

#include "font_file_lister.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "format.h"

#include <libaegisub/format_flyweight.h>
#include <libaegisub/format_path.h>

#include <algorithm>
#include <tuple>
#include <unicode/uchar.h>
#include <wx/intl.h>

namespace {
wxString format_missing(wxString const& str) {
	wxString printable;
	wxString unprintable;
	for (wxUniChar c : str) {
		if (!u_isUWhiteSpace(c.GetValue()))
			printable += c;
		else {
			unprintable += fmt_wx("\n - U+%04X ", c.GetValue());
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

FontCollector::FontCollector(FontCollectorStatusCallback status_callback)
: status_callback(std::move(status_callback))
, lister(this->status_callback)
{
}

void FontCollector::ProcessDialogueLine(const AssDialogue *line, int index) {
	if (line->Comment) return;

	auto style_it = styles.find(line->Style);
	if (style_it == end(styles)) {
		status_callback(fmt_tl("Style '%s' does not exist\n", line->Style), 2);
		++missing;
		return;
	}

	StyleInfo style = style_it->second;
	StyleInfo initial = style;

	bool overriden = false;

	for (auto& block : line->ParseTags()) {
		switch (block->GetType()) {
		case AssBlockType::OVERRIDE:
			for (auto const& tag : static_cast<AssDialogueBlockOverride&>(*block).Tags) {
				if (tag.Name == "\\r") {
					style = styles[tag.Params[0].Get(line->Style.get())];
					overriden = false;
				}
				else if (tag.Name == "\\b") {
					style.bold = tag.Params[0].Get(initial.bold);
					overriden = true;
				}
				else if (tag.Name == "\\i") {
					style.italic = tag.Params[0].Get(initial.italic);
					overriden = true;
				}
				else if (tag.Name == "\\fn") {
					style.facename = tag.Params[0].Get(initial.facename);
					overriden = true;
				}
			}
			break;
		case AssBlockType::PLAIN: {
			auto text = block->GetText();

			if (text.empty())
				continue;

			auto& usage = used_styles[style];

			if (overriden) {
				auto& lines = usage.lines;
				if (lines.empty() || lines.back() != index)
					lines.push_back(index);
			}

			auto& chars = usage.chars;
			auto size = static_cast<int>(text.size());
			for (int i = 0; i < size; ) {
				if (text[i] == '\\' && i + 1 < size) {
					char next = text[++i];
					if (next == 'N' || next == 'n') {
						++i;
						continue;
					}
					if (next == 'h') {
						++i;
						chars.push_back(0xA0);
						continue;
					}

					chars.push_back('\\');
					continue;
				}

				UChar32 c;
				U8_NEXT(&text[0], i, size, c);
				chars.push_back(c);
			}

			sort(begin(chars), end(chars));
			chars.erase(unique(chars.begin(), chars.end()), chars.end());
			break;
		}
		case AssBlockType::DRAWING:
		case AssBlockType::COMMENT:
			break;
		}
	}
}

void FontCollector::ProcessChunk(std::pair<StyleInfo, UsageData> const& style) {
	if (style.second.chars.empty()) return;

	auto res = lister.GetFontPaths(style.first.facename, style.first.bold, style.first.italic, style.second.chars);

	if (res.paths.empty()) {
		status_callback(fmt_tl("Could not find font '%s'\n", style.first.facename), 2);
		PrintUsage(style.second);
		++missing;
	}
	else {
		for (auto& elem : res.paths) {
			elem.make_preferred();
			if (std::find(begin(results), end(results), elem) == end(results)) {
				status_callback(fmt_tl("Found '%s' at '%s'\n", style.first.facename, elem), 0);
				results.push_back(elem);
			}
		}

		if (res.fake_bold)
			status_callback(fmt_tl("'%s' does not have a bold variant.\n", style.first.facename), 3);
		if (res.fake_italic)
			status_callback(fmt_tl("'%s' does not have an italic variant.\n", style.first.facename), 3);

		if (res.missing.size()) {
			if (res.missing.size() > 50)
				status_callback(fmt_tl("'%s' is missing %d glyphs used.\n", style.first.facename, res.missing.size()), 2);
			else if (res.missing.size() > 0)
				status_callback(fmt_tl("'%s' is missing the following glyphs used: %s\n", style.first.facename, format_missing(res.missing)), 2);
			PrintUsage(style.second);
			++missing_glyphs;
		}
		else if (res.fake_bold || res.fake_italic)
			PrintUsage(style.second);
	}
}

void FontCollector::PrintUsage(UsageData const& data) {
	if (data.styles.size()) {
		status_callback(_("Used in styles:\n"), 2);
		for (auto const& style : data.styles)
			status_callback(fmt_wx("  - %s\n", style), 2);
	}

	if (data.lines.size()) {
		status_callback(_("Used on lines:"), 2);
		for (int line : data.lines)
			status_callback(fmt_wx(" %d", line), 2);
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
		used_styles[info].styles.push_back(style.name);
	}

	int index = 0;
	for (auto const& diag : file->Events)
		ProcessDialogueLine(&diag, ++index);

	status_callback(_("Searching for font files\n"), 0);
	for (auto const& style : used_styles) ProcessChunk(style);
	status_callback(_("Done\n\n"), 0);

	std::vector<agi::fs::path> paths;
	paths.reserve(results.size());
	paths.insert(paths.end(), results.begin(), results.end());

	if (missing == 0)
		status_callback(_("All fonts found.\n"), 1);
	else
		status_callback(fmt_plural(missing, "One font could not be found\n", "%d fonts could not be found.\n", missing), 2);
	if (missing_glyphs != 0)
		status_callback(fmt_plural(missing_glyphs,
			"One font was found, but was missing glyphs used in the script.\n",
			"%d fonts were found, but were missing glyphs used in the script.\n",
			missing_glyphs), 2);

	return paths;
}

bool FontCollector::StyleInfo::operator<(StyleInfo const& rgt) const {
	return std::tie(facename, bold, italic) < std::tie(rgt.facename, rgt.bold, rgt.italic);
}
