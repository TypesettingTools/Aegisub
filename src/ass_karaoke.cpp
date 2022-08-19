// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "ass_karaoke.h"

#include "ass_dialogue.h"

#include <libaegisub/ass/karaoke.h>
#include <libaegisub/format.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace agi::ass;

std::vector<KaraokeSyllable> ParseKaraokeSyllables(const AssDialogue *line) {
	std::vector<KaraokeSyllable> syls;
	if (!line) return syls;

	KaraokeSyllable syl;
	syl.start_time = line->Start;
	syl.duration = 0;
	syl.tag_type = "\\k";

	for (auto& block : line->ParseTags()) {
		std::string text = block->GetText();

		switch (block->GetType()) {
		case AssBlockType::PLAIN:
			syl.text += text;
			break;
		case AssBlockType::COMMENT:
		// drawings aren't override tags but they shouldn't show up in the
		// stripped text so pretend they are
		case AssBlockType::DRAWING:
			syl.ovr_tags[syl.text.size()] += text;
			break;
		case AssBlockType::OVERRIDE:
			auto ovr = static_cast<AssDialogueBlockOverride*>(block.get());
			bool in_tag = false;
			for (auto& tag : ovr->Tags) {
				if (tag.IsValid() && boost::istarts_with(tag.Name, "\\k")) {
					if (in_tag) {
						syl.ovr_tags[syl.text.size()] += "}";
						in_tag = false;
					}

					// Dealing with both \K and \kf is mildly annoying so just
					// convert them both to \kf
					if (tag.Name == "\\K") tag.Name = "\\kf";

					// Don't bother including zero duration zero length syls
					if (syl.duration > 0 || !syl.text.empty()) {
						syls.push_back(syl);
						syl.text.clear();
						syl.ovr_tags.clear();
					}

					syl.tag_type = tag.Name;
					syl.start_time += syl.duration;
					syl.duration = tag.Params[0].Get(0) * 10;
				}
				else {
					std::string& otext = syl.ovr_tags[syl.text.size()];
					// Merge adjacent override tags
					boost::trim_right_if(text, [](char c) { return c == '}'; });
					if (!in_tag)
						otext += "{";

					in_tag = true;
					otext += tag;
				}
			}

			if (in_tag)
				syl.ovr_tags[syl.text.size()] += "}";
			break;
		}
	}

	syls.push_back(syl);
	return syls;
}

void SetKaraokeLine(Karaoke& karaoke, const AssDialogue *line, bool auto_split, bool normalize) {
	karaoke.SetLine(ParseKaraokeSyllables(line), normalize, auto_split);
}

