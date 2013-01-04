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

#include "config.h"

#include "ass_parser.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_info.h"
#include "ass_style.h"
#include "subtitle_format.h"

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

AssParser::AssParser(AssFile *target, int version)
: target(target)
, version(version)
, attach(nullptr)
, state(&AssParser::ParseScriptInfoLine)
{
	std::fill(begin(insertion_positions), end(insertion_positions), nullptr);
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
		attach->Finish();
		InsertLine(attach.release());
		AddLine(data);
	}
	else {
		attach->AddData(data);

		// Done building
		if (data.size() < 80) {
			attach->Finish();
			InsertLine(attach.release());
		}
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
			throw SubtitleFormatParseError("Unknown SSA file format version", 0);
	}

	size_t pos = data.find(':');
	if (pos == data.npos) return;

	InsertLine(new AssInfo(data.substr(0, pos), boost::trim_left_copy(data.substr(pos + 1))));
}

void AssParser::ParseEventLine(std::string const& data) {
	if (boost::starts_with(data, "Dialogue:") || boost::starts_with(data, "Comment:"))
		InsertLine(new AssDialogue(data));
}

void AssParser::ParseStyleLine(std::string const& data) {
	if (boost::starts_with(data, "Style:"))
		InsertLine(new AssStyle(data, version));
}

void AssParser::ParseFontLine(std::string const& data) {
	if (boost::starts_with(data, "fontname: "))
		attach.reset(new AssAttachment(data.substr(10), ENTRY_FONT));
}

void AssParser::ParseGraphicsLine(std::string const& data) {
	if (boost::starts_with(data, "filename: "))
		attach.reset(new AssAttachment(data.substr(10), ENTRY_GRAPHIC));
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
		std::string header = data;
		if (low == "[v4 styles]") {
			header = "[V4+ Styles]";
			version = 0;
			state = &AssParser::ParseStyleLine;
		}
		else if (low == "[v4+ styles]") {
			header = "[V4+ Styles]";
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
		else
			state = &AssParser::UnknownLine;
		return;
	}

	(this->*state)(data);
}

void AssParser::InsertLine(AssEntry *entry) {
	AssEntry *position = insertion_positions[entry->Group()];
	if (position)
		target->Line.insert(++target->Line.iterator_to(*position), *entry);
	else
		target->Line.push_back(*entry);
	insertion_positions[entry->Group()] = entry;
}
