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

#include "ass_parser.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_info.h"
#include "ass_style.h"
#include "string_codec.h"
#include "subtitle_format.h"

#include <libaegisub/ass/uuencode.h>

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

AssParser::AssParser(AssFile *target, int version)
: target(target)
, version(version)
, state(&AssParser::ParseScriptInfoLine)
{
}

AssParser::~AssParser() {
}

void AssParser::ParseAttachmentLine(std::string const& data) {
	bool is_filename = boost::starts_with(data, "fontname: ") || boost::starts_with(data, "filename: ");

	bool valid_data = data.size() > 0 && data.size() <= 80;
	for (auto byte : data) {
		if (byte < 33 || byte >= 97) {
			valid_data = false;
			break;
		}
	}

	// Data is over, add attachment to the file
	if (!valid_data || is_filename) {
		target->Attachments.push_back(*attach.release());
		AddLine(data);
	}
	else {
		attach->AddData(data);

		// Done building
		if (data.size() < 80)
			target->Attachments.push_back(*attach.release());
	}
}

void AssParser::ParseScriptInfoLine(std::string const& data) {
	if (boost::starts_with(data, ";")) {
		// Skip stupid comments added by other programs
		// Of course, we'll add our own in place later... ;)
		return;
	}

	if (boost::starts_with(data, "ScriptType:")) {
		std::string version_str = data.substr(11);
		boost::trim(version_str);
		boost::to_lower(version_str);
		if (version_str == "v4.00")
			version = 0;
		else if (version_str == "v4.00+")
			version = 1;
		else
			throw SubtitleFormatParseError("Unknown SSA file format version", nullptr);
	}

	// Nothing actually supports the Collisions property and malformed values
	// crash VSFilter, so just remove it entirely
	if (boost::starts_with(data, "Collisions:"))
		return;

	size_t pos = data.find(':');
	if (pos == data.npos) return;

	target->Info.push_back(*new AssInfo(data.substr(0, pos), boost::trim_left_copy(data.substr(pos + 1))));
}

void AssParser::ParseEventLine(std::string const& data) {
	if (boost::starts_with(data, "Dialogue:") || boost::starts_with(data, "Comment:"))
		target->Events.push_back(*new AssDialogue(data));
}

void AssParser::ParseStyleLine(std::string const& data) {
	if (boost::starts_with(data, "Style:"))
		target->Styles.push_back(*new AssStyle(data, version));
}

void AssParser::ParseFontLine(std::string const& data) {
	if (boost::starts_with(data, "fontname: "))
		attach.reset(new AssAttachment(data, AssEntryGroup::FONT));
}

void AssParser::ParseGraphicsLine(std::string const& data) {
	if (boost::starts_with(data, "filename: "))
		attach.reset(new AssAttachment(data, AssEntryGroup::GRAPHIC));
}

void AssParser::ParseExtradataLine(std::string const &data) {
	static const boost::regex matcher("Data:[[:space:]]*(\\d+),([^,]+),(.)(.*)");
	boost::match_results<std::string::const_iterator> mr;

	if (boost::regex_match(data, mr, matcher)) {
		auto id = boost::lexical_cast<uint32_t>(mr.str(1));
		auto key = inline_string_decode(mr.str(2));
		auto valuetype = mr.str(3);
		auto value = mr.str(4);
		if (valuetype == "e") {
			// escaped/inline_string encoded
			value = inline_string_decode(value);
		} else if (valuetype == "u") {
			// ass uuencoded
			auto valuedata = agi::ass::UUDecode(value);
			value = std::string(valuedata.begin(), valuedata.end());
		} else {
			// unknown, error?
			value = "";
		}

		// ensure next_extradata_id is always at least 1 more than the largest existing id
		target->next_extradata_id = std::max(id+1, target->next_extradata_id);
		target->Extradata[id] = std::make_pair(key, value);
	}
}

void AssParser::AddLine(std::string const& data) {
	// Special-case for attachments since a line could theoretically be both a
	// valid attachment data line and a valid section header, and if an
	// attachment is in progress it needs to be treated as that
	if (attach.get()) {
		ParseAttachmentLine(data);
		return;
	}

	if (data.empty()) return;

	// Section header
	if (data[0] == '[' && data.back() == ']') {
		// Ugly hacks to allow intermixed v4 and v4+ style sections
		const std::string low = boost::to_lower_copy(data);
		if (low == "[v4 styles]") {
			version = 0;
			state = &AssParser::ParseStyleLine;
		}
		else if (low == "[v4+ styles]") {
			version = 1;
			state = &AssParser::ParseStyleLine;
		}
		else if (low == "[events]")
			state = &AssParser::ParseEventLine;
		else if (low == "[script info]")
			state = &AssParser::ParseScriptInfoLine;
		else if (low == "[graphics]")
			state = &AssParser::ParseGraphicsLine;
		else if (low == "[fonts]")
			state = &AssParser::ParseFontLine;
		else if (low == "[aegisub extradata]")
			state = &AssParser::ParseExtradataLine;
		else
			state = &AssParser::UnknownLine;
		return;
	}

	(this->*state)(data);
}
