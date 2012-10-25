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
#include "ass_override.h"
#include "ass_style.h"


#ifndef AGI_PRE
#include <algorithm>

#include <wx/intl.h>
#endif

using namespace std::tr1::placeholders;

FontCollector::FontCollector(FontCollectorStatusCallback status_callback, FontFileLister &lister)
: status_callback(status_callback)
, lister(lister)
, missing(0)
, missing_glyphs(0)
{
}

void FontCollector::ProcessDialogueLine(AssDialogue *line, int index) {
	if (line->Comment) return;

	line->ParseAssTags();
	StyleInfo style = styles[line->Style];
	StyleInfo initial = style;

	bool overriden = false;

	for (size_t i = 0; i < line->Blocks.size(); ++i) {
		if (AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride *>(line->Blocks[i])) {
			for (size_t j = 0; j < ovr->Tags.size(); ++j) {
				AssOverrideTag *tag = ovr->Tags[j];
				wxString name = tag->Name;

				if (name == "\\r") {
					style = styles[tag->Params[0]->Get(line->Style)];
					overriden = false;
				}
				else if (name == "\\b") {
					style.bold = tag->Params[0]->Get(initial.bold);
					overriden = true;
				}
				else if (name == "\\i") {
					style.italic = tag->Params[0]->Get(initial.italic);
					overriden = true;
				}
				else if (name == "\\fn") {
					style.facename = tag->Params[0]->Get(initial.facename);
					overriden = true;
				}
			}
		}
		else if (AssDialogueBlockPlain *txt = dynamic_cast<AssDialogueBlockPlain *>(line->Blocks[i])) {
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
	line->ClearBlocks();
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
		for (std::set<wxString>::const_iterator it = data.styles.begin(); it != data.styles.end(); ++it)
			status_callback(wxString::Format("  - %s\n", *it), 2);
	}

	if (data.lines.size()) {
		status_callback(wxString::Format(_("Used on lines:")), 2);
		for (std::set<int>::const_iterator it = data.lines.begin(); it != data.lines.end(); ++it)
			status_callback(wxString::Format(" %d", *it), 2);
		status_callback("\n", 2);
	}
	status_callback("\n", 2);
}

std::vector<wxString> FontCollector::GetFontPaths(std::list<AssEntry*> const& file) {
	missing = 0;
	missing_glyphs = 0;

	status_callback(_("Parsing file\n"), 0);

	int index = 0;
	for (std::list<AssEntry*>::const_iterator cur = file.begin(); cur != file.end(); ++cur) {
		if (AssStyle *style = dynamic_cast<AssStyle*>(*cur)) {
			StyleInfo &info = styles[style->name];
			info.facename = style->font;
			info.bold     = style->bold;
			info.italic   = style->italic;
			used_styles[info].styles.insert(style->name);
		}
		else if (AssDialogue *diag = dynamic_cast<AssDialogue*>(*cur))
			ProcessDialogueLine(diag, ++index);
	}

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
