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

/// @file font_file_lister.cpp
/// @brief Base-class for font collector implementations
/// @ingroup font_collector
///

#include "config.h"

#include "ass_dialogue.h"
#include "ass_override.h"
#include "ass_style.h"

#include "font_file_lister_fontconfig.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/intl.h>
#endif

using namespace std::tr1::placeholders;

FontCollector::FontCollector(FontCollectorStatusCallback status_callback, FontFileLister &lister)
: status_callback(status_callback)
, lister(lister)
{
}

void FontCollector::ProcessDialogueLine(AssDialogue *line) {
	if (line->Comment) return;

	line->ParseASSTags();
	StyleInfo style = styles[line->Style];
	StyleInfo initial = style;

	for (size_t i = 0; i < line->Blocks.size(); ++i) {
		if (AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride *>(line->Blocks[i])) {
			for (size_t j = 0; j < ovr->Tags.size(); ++j) {
				AssOverrideTag *tag = ovr->Tags[j];
				wxString name = tag->Name;

				if (name == "\\r")
					style = styles[tag->Params[0]->Get(line->Style)];
				if (name == "\\b")
					style.bold = tag->Params[0]->Get(initial.bold);
				else if (name == "\\i")
					style.italic = tag->Params[0]->Get(initial.italic);
				else if (name == "\\fn")
					style.facename = tag->Params[0]->Get(initial.facename);
				else
					continue;
			}
		}
		else if (AssDialogueBlockPlain *txt = dynamic_cast<AssDialogueBlockPlain *>(line->Blocks[i])) {
			wxString text = txt->GetText();
			if (text.size()) {
				std::set<wxUniChar>& chars = used_styles[style];
				for (size_t i = 0; i < text.size(); ++i)
					chars.insert(text[i]);
			}
		}
		// Do nothing with drawing blocks
	}
	line->ClearBlocks();
}

void FontCollector::ProcessChunk(std::pair<StyleInfo, std::set<wxUniChar> > const& style) {
	std::vector<wxString> paths = lister.GetFontPaths(style.first.facename, style.first.bold, style.first.italic, style.second);

	if (paths.empty()) {
		status_callback(wxString::Format("Could not find font '%s'\n", style.first.facename), 2);
		++missing;
	}
	else {
		for (size_t i = 0; i < paths.size(); ++i) {
			if (results.insert(paths[i]).second)
				status_callback(wxString::Format("Found '%s' at '%s'\n", style.first.facename, paths[i]), 0);
		}
	}
}

std::vector<wxString> FontCollector::GetFontPaths(std::list<AssEntry*> const& file) {
	missing = 0;

	status_callback(_("Parsing file\n"), 0);
	for (std::list<AssEntry*>::const_iterator cur = file.begin(); cur != file.end(); ++cur) {
		if (AssStyle *style = dynamic_cast<AssStyle*>(*cur)) {
			StyleInfo &info = styles[style->name];
			info.facename = style->font;
			info.bold     = style->bold;
			info.italic   = style->italic;
		}
		else if (AssDialogue *diag = dynamic_cast<AssDialogue*>(*cur))
			ProcessDialogueLine(diag);
	}

	if (used_styles.empty()) {
		status_callback(_("No non-empty dialogue lines found"), 2);
		return std::vector<wxString>();
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
