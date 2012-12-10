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
#include "utils.h"

#include <libaegisub/of_type_adaptor.h>

#include <algorithm>

#include <wx/intl.h>

using namespace std::placeholders;

FontCollector::FontCollector(FontCollectorStatusCallback status_callback, FontFileLister &lister)
: status_callback(status_callback)
, lister(lister)
, missing(0)
, missing_glyphs(0)
{
}

void FontCollector::ProcessDialogueLine(const AssDialogue *line, int index) {
	if (line->Comment) return;

	boost::ptr_vector<AssDialogueBlock> blocks(line->ParseTags());
	StyleInfo style = styles[line->Style];
	StyleInfo initial = style;

	bool overriden = false;

	for (auto& block : blocks) {
		if (AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride *>(&block)) {
			for (auto const& tag : ovr->Tags) {
				wxString name = tag.Name;

				if (name == "\\r") {
					style = styles[tag.Params[0].Get<wxString>(line->Style)];
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
			wxString text = txt->GetText();

			if (text.empty() || (text.size() >= 2 && text.StartsWith("{") && text.EndsWith("}")))
				continue;

			if (overriden)
				used_styles[style].lines.insert(index);
			std::set<wxUniChar>& chars = used_styles[style].chars;
			for (wxString::const_iterator it = text.begin(); it != text.end(); ++it) {
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
		// Do nothing with drawing blocks
	}
}

void FontCollector::ProcessChunk(std::pair<StyleInfo, UsageData> const& style) {
	if (style.second.chars.empty()) return;

	FontFileLister::CollectionResult res = lister.GetFontPaths(style.first.facename, style.first.bold, style.first.italic, style.second.chars);

	if (res.paths.empty()) {
		status_callback(wxString::Format(_("Could not find font '%s'\n"), style.first.facename), 2);
		PrintUsage(style.second);
		++missing;
	}
	else {
		for (size_t i = 0; i < res.paths.size(); ++i) {
			if (results.insert(res.paths[i]).second)
				status_callback(wxString::Format(_("Found '%s' at '%s'\n"), style.first.facename, res.paths[i]), 0);
		}

		if (res.missing.size()) {
			if (res.missing.size() > 50)
				status_callback(wxString::Format(_("'%s' is missing %d glyphs used.\n"), style.first.facename, (int)res.missing.size()), 2);
			else if (res.missing.size() > 0)
				status_callback(wxString::Format(_("'%s' is missing the following glyphs used: %s\n"), style.first.facename, res.missing), 2);
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

std::vector<wxString> FontCollector::GetFontPaths(const AssFile *file) {
	missing = 0;
	missing_glyphs = 0;

	status_callback(_("Parsing file\n"), 0);

	for (auto style : file->Line | agi::of_type<const AssStyle>()) {
		StyleInfo &info = styles[style->name];
		info.facename = style->font;
		info.bold     = style->bold;
		info.italic   = style->italic;
		used_styles[info].styles.insert(style->name);
	}

	int index = 0;
	for (auto diag : file->Line | agi::of_type<const AssDialogue>())
		ProcessDialogueLine(diag, ++index);

	status_callback(_("Searching for font files\n"), 0);
	for_each(used_styles.begin(), used_styles.end(), bind(&FontCollector::ProcessChunk, this, _1));
	status_callback(_("Done\n\n"), 0);

	std::vector<wxString> paths;
	paths.reserve(results.size());
	paths.insert(paths.end(), results.begin(), results.end());

	if (missing == 0)
		status_callback(_("All fonts found.\n"), 1);
	else
		status_callback(wxString::Format(_("%d fonts could not be found.\n"), missing), 2);
	if (missing_glyphs != 0)
		status_callback(wxString::Format(_("%d fonts were found, but were missing glyphs used in the script.\n"), missing_glyphs), 2);

	return paths;
}

bool FontCollector::StyleInfo::operator<(StyleInfo const& rgt) const {
#define CMP(field) \
	if (field < rgt.field) return true; \
	if (field > rgt.field) return false

	CMP(facename);
	CMP(bold);
	CMP(italic);

#undef CMP

	return false;
}
