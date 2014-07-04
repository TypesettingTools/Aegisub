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

#include "libaegisub/calltip_provider.h"

#include "libaegisub/ass/dialogue_parser.h"

#include <algorithm>

namespace {
struct proto_lit {
	const char *name;
	const char *args;
};

// NOTE: duplicate tag names sorted by number of arguments
const proto_lit proto_1[] = {
	{"K", "\\KDuration"},
	{"a", "\\aAlignment"},
	{"b", "\\bWeight"},
	{"c", "\\cColour"},
	{"i", "\\i1/0"},
	{"k", "\\kDuration"},
	{"p", "\\pExponent"},
	{"q", "\\qWrap Style"},
	{"r", "\\rStyle"},
	{"s", "\\s1/0"},
	{"t", "\\t(Acceleration,Tags)"},
	{"t", "\\t(Start Time,End Time,Tags)"},
	{"t", "\\t(Start Time,End Time,Acceleration,Tags)"},
	{"u", "\\u1/0"},
	{nullptr, nullptr}
};

const proto_lit proto_2[] = {
	{"1a", "\\1aAlpha"},
	{"1c", "\\1cColour"},
	{"2a", "\\2aAlpha"},
	{"2c", "\\2cColour"},
	{"3a", "\\3aAlpha"},
	{"3c", "\\3cColour"},
	{"4a", "\\4aAlpha"},
	{"4c", "\\4cColour"},
	{"an", "\\anAlignment"},
	{"be", "\\beStrength"},
	{"fe", "\\feEncoding"},
	{"fn", "\\fnFont Name"},
	{"fr", "\\frAngle"},
	{"fs", "\\fsFont Size"},
	{"kf", "\\kfDuration"},
	{"ko", "\\koDuration"},
	{nullptr, nullptr}
};

const proto_lit proto_3[] = {
	{"fax", "\\faxFactor"},
	{"fay", "\\fayFactor"},
	{"frx", "\\frxAngle"},
	{"fry", "\\fryAngle"},
	{"frz", "\\frzAngle"},
	{"fsp", "\\fspSpacing"},
	{"org", "\\org(X,Y)"},
	{"pbo", "\\pboOffset"},
	{"pos", "\\pos(X,Y)"},
	{nullptr, nullptr}
};

const proto_lit proto_4[] = {
	{"blur", "\\blurStrength"},
	{"bord", "\\bordWidth"},
	{"clip", "\\clip(Command)"},
	{"clip", "\\clip(Scale,Command)"},
	{"clip", "\\clip(X1,Y1,X2,Y2)"},
	{"fad", "\\fad(Start Time,End Time)"},
	{"fade", "\\fade(Start Alpha,Middle Alpha,End Alpha,Start In,End In,Start Out,End Out)"},
	{"fscx", "\\fscxScale"},
	{"fscy", "\\fscyScale"},
	{"move", "\\move(X1,Y1,X2,Y2)"},
	{"move", "\\move(X1,Y1,X2,Y2,Start Time,End Time)"},
	{"shad", "\\shadDepth"},
	{nullptr, nullptr}
};

const proto_lit proto_5[] = {
	{"alpha", "\\alphaAlpha"},
	{"iclip", "\\iclip(Command)"},
	{"iclip", "\\iclip(Scale,Command)"},
	{"iclip", "\\iclip(X1,Y1,X2,Y2)"},
	{"xbord", "\\xbordWidth"},
	{"xshad", "\\xshadDepth"},
	{"ybord", "\\ybordWidth"},
	{"yshad", "\\yshadDepth"},
	{nullptr, nullptr}
};

const proto_lit *all_protos[] = {proto_1, proto_2, proto_3, proto_4, proto_5};
}

namespace agi {
Calltip GetCalltip(std::vector<ass::DialogueToken> const& tokens, std::string const& text, size_t pos) {
	namespace dt = ass::DialogueTokenType;

	Calltip ret = { nullptr, 0, 0, 0 };

	size_t idx = 0;
	size_t tag_name_idx = 0;
	size_t commas = 0;
	for (; idx < tokens.size() && pos > 0; ++idx) {
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
		pos -= std::min(pos, tokens[idx].length);
	}

	// Either didn't hit a tag or the override block ended before reaching the
	// current position
	if (tag_name_idx == 0)
		return ret;

	size_t tag_name_start = 0;
	for (size_t i = 0; i < tag_name_idx; ++i)
		tag_name_start += tokens[i].length;
	size_t tag_name_length = tokens[tag_name_idx].length;

	// No tags exist with length over five
	if (tag_name_length > 5)
		return ret;

	auto valid = [&](const proto_lit *it) {
		return it->name && strncmp(it->name, &text[tag_name_start], tag_name_length) == 0;
	};

	// Find the prototype for this tag
	auto proto = all_protos[tag_name_length - 1];
	while (proto->name && strncmp(proto->name, &text[tag_name_start], tag_name_length) < 0)
		++proto;

	if (!valid(proto))
		return ret;

	// If there's multiple overloads, check how many total arguments we have
	// and pick the one with the least args >= current arg count
	if (valid(proto + 1)) {
		size_t args = commas + 1;
		for (size_t i = idx + 1; i < tokens.size(); ++i) {
			int type = tokens[i].type;
			if (type == dt::ARG_SEP)
				++args;
			else if (type != dt::ARG && type != dt::WHITESPACE)
				break;
		}

		auto arg_count = [](const proto_lit *it) -> size_t {
			size_t count = 1;
			for (const char *s = it->args; *s; ++s) {
				if (*s == ',') ++count;
			}
			return count;
		};

		while (valid(proto + 1) && args > arg_count(proto))
			++proto;
	}

	ret.highlight_start = tag_name_length + 1;
	if (proto->args[ret.highlight_start] != '(')
		ret.highlight_end = strlen(proto->args);
	else {
		auto start = proto->args + tag_name_length + 2; // One for slash, one for open paren
		for (; commas > 0; --commas) {
			start = strchr(start, ',');
			if (!start) return ret; // No calltip if there's too many args
			++start;
		}

		ret.highlight_start = std::distance(proto->args, start);
		const char *end = strchr(start, ',');
		if (end)
			ret.highlight_end = std::distance(start, end) + ret.highlight_start;
		else
			ret.highlight_end = strlen(proto->args) - 1; // -1 for close paren
	}

	ret.text = proto->args;
	ret.tag_position = tag_name_start;

	return ret;
}
}
