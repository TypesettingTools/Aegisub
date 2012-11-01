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

#include "../config.h"

#include "libaegisub/calltip_provider.h"

#include "libaegisub/ass/dialogue_parser.h"

#include <boost/range.hpp>

namespace {
	struct proto_lit {
		const char *name;
		bool has_parens;
		const char *args;
	};

	proto_lit calltip_protos[] = {
		{ "move", true, "X1\0Y1\0X2\0Y2\0" },
		{ "move", true, "X1\0Y1\0X2\0Y2\0Start Time\0End Time\0" },
		{ "fn", false, "Font Name\0" },
		{ "bord", false, "Width\0" },
		{ "xbord", false, "Width\0" },
		{ "ybord", false, "Width\0" },
		{ "shad", false, "Depth\0" },
		{ "xshad", false, "Depth\0" },
		{ "yshad", false, "Depth\0" },
		{ "be", false, "Strength\0" },
		{ "blur", false, "Strength\0" },
		{ "fscx", false, "Scale\0" },
		{ "fscy", false, "Scale\0" },
		{ "fsp", false, "Spacing\0" },
		{ "fs", false, "Font Size\0" },
		{ "fe", false, "Encoding\0" },
		{ "frx", false, "Angle\0" },
		{ "fry", false, "Angle\0" },
		{ "frz", false, "Angle\0" },
		{ "fr", false, "Angle\0" },
		{ "pbo", false, "Offset\0" },
		{ "clip", true, "Command\0" },
		{ "clip", true, "Scale\0Command\0" },
		{ "clip", true, "X1\0Y1\0X2\0Y2\0" },
		{ "iclip", true, "Command\0" },
		{ "iclip", true, "Scale\0Command\0" },
		{ "iclip", true, "X1\0Y1\0X2\0Y2\0" },
		{ "t", true, "Acceleration\0Tags\0" },
		{ "t", true, "Start Time\0End Time\0Tags\0" },
		{ "t", true, "Start Time\0End Time\0Acceleration\0Tags\0" },
		{ "pos", true, "X\0Y\0" },
		{ "p", false, "Exponent\0" },
		{ "org", true, "X\0Y\0" },
		{ "fade", true, "Start Alpha\0Middle Alpha\0End Alpha\0Start In\0End In\0Start Out\0End Out\0" },
		{ "fad", true, "Start Time\0End Time\0" },
		{ "c", false, "Colour\0" },
		{ "1c", false, "Colour\0" },
		{ "2c", false, "Colour\0" },
		{ "3c", false, "Colour\0" },
		{ "4c", false, "Colour\0" },
		{ "alpha", false, "Alpha\0" },
		{ "1a", false, "Alpha\0" },
		{ "2a", false, "Alpha\0" },
		{ "3a", false, "Alpha\0" },
		{ "4a", false, "Alpha\0" },
		{ "an", false, "Alignment\0" },
		{ "a", false, "Alignment\0" },
		{ "b", false, "Weight\0" },
		{ "i", false, "1/0\0" },
		{ "u", false, "1/0\0" },
		{ "s", false, "1/0\0" },
		{ "kf", false, "Duration\0" },
		{ "ko", false, "Duration\0" },
		{ "k", false, "Duration\0" },
		{ "K", false, "Duration\0" },
		{ "q", false, "Wrap Style\0" },
		{ "r", false, "Style\0" },
		{ "fax", false, "Factor\0" },
		{ "fay", false, "Factor\0" }
	};
}

namespace agi {
CalltipProvider::CalltipProvider() {
	for (auto proto : calltip_protos) {
		CalltipProto p;
		std::string tag_name = proto.name;
		p.text = '\\' + tag_name;
		if (proto.has_parens)
			p.text += '(';

		for (const char *arg = proto.args; *arg; ) {
			size_t start = p.text.size();
			p.text += arg;
			size_t end = p.text.size();
			if (proto.has_parens)
				p.text += ',';

			arg += end - start + 1;
			p.args.emplace_back(start, end);
		}

		// replace trailing comma
		if (proto.has_parens)
			p.text.back() = ')';

		protos.emplace(std::move(tag_name), std::move(p));
	}
}

Calltip CalltipProvider::GetCalltip(std::vector<ass::DialogueToken> const& tokens, std::string const& text, size_t pos) {
	namespace dt = ass::DialogueTokenType;

	Calltip ret = { "", 0, 0, 0 };

	size_t idx = 0;
	size_t tag_name_idx = 0;
	size_t commas = 0;
	for (; idx < tokens.size() && pos >= tokens[idx].length; ++idx) {
		switch (tokens[idx].type) {
			case dt::COMMENT:
			case dt::OVR_END:
				tag_name_idx = 0;
				break;
			case dt::TAG_NAME:
				tag_name_idx = idx;
				commas = 0;
				break;
			case dt::ARG_SEP:
				++commas;
				break;
			default: break;
		}
		pos -= tokens[idx].length;
	}

	// Either didn't hit a tag or the override block ended before reaching the
	// current position
	if (tag_name_idx == 0)
		return ret;

	// Find the prototype for this tag
	size_t tag_name_start = 0;
	for (size_t i = 0; i < tag_name_idx; ++i)
		tag_name_start += tokens[i].length;
	auto it = protos.equal_range(text.substr(tag_name_start, tokens[tag_name_idx].length));

	// If there's multiple overloads, check how many total arguments we have
	// and pick the one with the least args >= current arg count
	if (distance(it.first, it.second) > 1) {
		size_t args = commas + 1;
		for (size_t i = idx; i < tokens.size(); ++i) {
			int type = tokens[i].type;
			if (type == dt::ARG_SEP)
				++args;
			else if (type != dt::ARG && type != dt::WHITESPACE)
				break;
		}

		while (it.first != it.second && args > it.first->second.args.size())
			++it.first;
	}

	// Unknown tag or too many arguments
	if (it.first == it.second || it.first->second.args.size() <= commas)
		return ret;

	ret.text = it.first->second.text;
	ret.highlight_start = it.first->second.args[commas].first;
	ret.highlight_end = it.first->second.args[commas].second;
	ret.tag_position = tag_name_start;
	return ret;
}
}
