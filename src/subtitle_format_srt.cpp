// Copyright (c) 2006, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file subtitle_format_srt.cpp
/// @brief Reading/writing SubRip format subtitles (.SRT)
/// @ingroup subtitle_io
///

#include "subtitle_format_srt.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "options.h"
#include "text_file_reader.h"
#include "text_file_writer.h"

#include <libaegisub/format.h>
#include <libaegisub/of_type_adaptor.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
#include <map>

DEFINE_EXCEPTION(SRTParseError, SubtitleFormatParseError);

namespace {
class SrtTagParser {
	struct FontAttribs {
		std::string face;
		std::string size;
		std::string color;
	};

	enum TagType {
		// leave 0 unused so indexing an unknown tag in the map won't clash
		TAG_BOLD_OPEN = 1,
		TAG_BOLD_CLOSE,
		TAG_ITALICS_OPEN,
		TAG_ITALICS_CLOSE,
		TAG_UNDERLINE_OPEN,
		TAG_UNDERLINE_CLOSE,
		TAG_STRIKEOUT_OPEN,
		TAG_STRIKEOUT_CLOSE,
		TAG_FONT_OPEN,
		TAG_FONT_CLOSE
	};

	const boost::regex tag_matcher;
	const boost::regex attrib_matcher;
	const boost::regex is_quoted;
	std::map<std::string, TagType> tag_name_cases;

public:
	SrtTagParser()
	: tag_matcher("^(.*?)<(/?b|/?i|/?u|/?s|/?font)([^>]*)>(.*)$", boost::regex::icase)
	, attrib_matcher(R"(^[[:space:]]+(face|size|color)=('[^']*'|"[^"]*"|[^[:space:]]+))", boost::regex::icase)
	, is_quoted(R"(^(['"]).*\1$)")
	{
		tag_name_cases["b"]  = TAG_BOLD_OPEN;
		tag_name_cases["/b"] = TAG_BOLD_CLOSE;
		tag_name_cases["i"]  = TAG_ITALICS_OPEN;
		tag_name_cases["/i"] = TAG_ITALICS_CLOSE;
		tag_name_cases["u"]  = TAG_UNDERLINE_OPEN;
		tag_name_cases["/u"] = TAG_UNDERLINE_CLOSE;
		tag_name_cases["s"]  = TAG_STRIKEOUT_OPEN;
		tag_name_cases["/s"] = TAG_STRIKEOUT_CLOSE;
		tag_name_cases["font"] = TAG_FONT_OPEN;
		tag_name_cases["/font"] = TAG_FONT_CLOSE;
	}

	std::string ToAss(std::string srt)
	{
		int bold_level = 0;
		int italics_level = 0;
		int underline_level = 0;
		int strikeout_level = 0;
		std::vector<FontAttribs> font_stack;

		std::string ass; // result to be built

		while (!srt.empty())
		{
			boost::smatch result;
			if (!regex_match(srt, result, tag_matcher))
			{
				// no more tags could be matched, end of string
				ass.append(srt);
				break;
			}

			// we found a tag, translate it
			std::string pre_text  = result.str(1);
			std::string tag_name  = result.str(2);
			std::string tag_attrs = result.str(3);
			std::string post_text = result.str(4);

			// the text before the tag goes through unchanged
			ass.append(pre_text);
			// the text after the tag is the input for next iteration
			srt = post_text;

			boost::to_lower(tag_name);
			switch (tag_name_cases[tag_name])
			{
			case TAG_BOLD_OPEN:
				if (bold_level == 0)
					ass.append("{\\b1}");
				bold_level++;
				break;
			case TAG_BOLD_CLOSE:
				if (bold_level == 1)
					ass.append("{\\b}");
				if (bold_level > 0)
					bold_level--;
				break;
			case TAG_ITALICS_OPEN:
				if (italics_level == 0)
					ass.append("{\\i1}");
				italics_level++;
				break;
			case TAG_ITALICS_CLOSE:
				if (italics_level == 1)
					ass.append("{\\i}");
				if (italics_level > 0)
					italics_level--;
				break;
			case TAG_UNDERLINE_OPEN:
				if (underline_level == 0)
					ass.append("{\\u1}");
				underline_level++;
				break;
			case TAG_UNDERLINE_CLOSE:
				if (underline_level == 1)
					ass.append("{\\u}");
				if (underline_level > 0)
					underline_level--;
				break;
			case TAG_STRIKEOUT_OPEN:
				if (strikeout_level == 0)
					ass.append("{\\s1}");
				strikeout_level++;
				break;
			case TAG_STRIKEOUT_CLOSE:
				if (strikeout_level == 1)
					ass.append("{\\s}");
				if (strikeout_level > 0)
					strikeout_level--;
				break;
			case TAG_FONT_OPEN:
				{
					// new attributes to fill in
					FontAttribs new_attribs;
					FontAttribs old_attribs;
					// start out with any previous ones on stack
					if (font_stack.size() > 0)
						old_attribs = font_stack.back();
					new_attribs = old_attribs;
					// now find all attributes on this font tag
					boost::smatch result;
					while (regex_search(tag_attrs, result, attrib_matcher))
					{
						// get attribute name and values
						std::string attr_name = result.str(1);
						std::string attr_value = result.str(2);

						// clean them
						boost::to_lower(attr_name);
						if (regex_match(attr_value, is_quoted))
							attr_value = attr_value.substr(1, attr_value.size() - 2);

						// handle the attributes
						if (attr_name == "face")
							new_attribs.face = agi::format("{\\fn%s}", attr_value);
						else if (attr_name == "size")
							new_attribs.size = agi::format("{\\fs%s}", attr_value);
						else if (attr_name == "color")
							new_attribs.color = agi::format("{\\c%s}", agi::Color(attr_value).GetAssOverrideFormatted());

						// remove this attribute to prepare for the next
						tag_attrs = result.suffix().str();
					}

					// the attributes changed from old are then written out
					if (new_attribs.face != old_attribs.face)
						ass.append(new_attribs.face);
					if (new_attribs.size != old_attribs.size)
						ass.append(new_attribs.size);
					if (new_attribs.color != old_attribs.color)
						ass.append(new_attribs.color);

					// lastly dump the new attributes state onto the stack
					font_stack.push_back(new_attribs);
				}
				break;
			case TAG_FONT_CLOSE:
				{
					// this requires a font stack entry
					if (font_stack.empty())
						break;
					// get the current attribs
					FontAttribs cur_attribs = font_stack.back();
					// remove them from the stack
					font_stack.pop_back();
					// grab the old attributes if there are any
					FontAttribs old_attribs;
					if (font_stack.size() > 0)
						old_attribs = font_stack.back();
					// then restore the attributes to previous settings
					if (cur_attribs.face != old_attribs.face)
					{
						if (old_attribs.face.empty())
							ass.append("{\\fn}");
						else
							ass.append(old_attribs.face);
					}
					if (cur_attribs.size != old_attribs.size)
					{
						if (old_attribs.size.empty())
							ass.append("{\\fs}");
						else
							ass.append(old_attribs.size);
					}
					if (cur_attribs.color != old_attribs.color)
					{
						if (old_attribs.color.empty())
							ass.append("{\\c}");
						else
							ass.append(old_attribs.color);
					}
				}
				break;
			default:
				// unknown tag, replicate it in the output
				ass.append("<").append(tag_name).append(tag_attrs).append(">");
				break;
			}
		}

		// make it a little prettier, join tag groups
		boost::replace_all(ass, "}{", "");

		return ass;
	}
};

AssTime ReadSRTTime(std::string const& ts)
{
	// For the sake of your sanity, please do not read this function.

	int d, h, m, s, ms;
	d = h = m = s = ms = 0;

	size_t ci = 0;
	int ms_chars = 0;

	for (bool milliseconds = false; ci < ts.size() && !milliseconds; ++ci)
	{
		switch (ts[ci])
		{
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			s = s * 10 + (ts[ci] - '0');
			break;
		case ':':
			d = h;
			h = m;
			m = s;
			s = 0;
			break;
		case ',':
			milliseconds = true;
			break;
		default:
			ci = ts.size();
		}
	}

	for (; ci < ts.size(); ++ci)
	{
		if (!isdigit(ts[ci])) break;

		ms = ms * 10 + (ts[ci] - '0');
		ms_chars++;
	}

	ms *= pow(10, 3 - ms_chars);

	return ms + 1000*(s + 60*(m + 60*(h + d*24)));
}

std::string WriteSRTTime(AssTime const& ts)
{
	return agi::format("%02d:%02d:%02d,%03d", ts.GetTimeHours(), ts.GetTimeMinutes(), ts.GetTimeSeconds(), ts.GetTimeMiliseconds());
}

}

SRTSubtitleFormat::SRTSubtitleFormat()
: SubtitleFormat("SubRip")
{
}

std::vector<std::string> SRTSubtitleFormat::GetReadWildcards() const {
	return {"srt"};
}

std::vector<std::string> SRTSubtitleFormat::GetWriteWildcards() const {
	return GetReadWildcards();
}

enum ParseState {
	STATE_INITIAL,
	STATE_TIMESTAMP,
	STATE_FIRST_LINE_OF_BODY,
	STATE_REST_OF_BODY,
	STATE_LAST_WAS_BLANK
};

void SRTSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename, agi::vfr::Framerate const& fps, std::string const& encoding) const {
	using namespace std;

	TextFileReader file(filename, encoding);
	target->LoadDefault(false, OPT_GET("Subtitle Format/SRT/Default Style Catalog")->GetString());

	// See parsing algorithm at <http://devel.aegisub.org/wiki/SubtitleFormats/SRT>

	// "hh:mm:ss,fff --> hh:mm:ss,fff" (e.g. "00:00:04,070 --> 00:00:10,04")
	const boost::regex timestamp_regex("^([0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2},[0-9]{1,}) --> ([0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2},[0-9]{1,})");

	SrtTagParser tag_parser;

	ParseState state = STATE_INITIAL;
	int line_num = 0;
	int linebreak_debt = 0;
	AssDialogue *line = nullptr;
	std::string text;
	while (file.HasMoreLines()) {
		std::string text_line = file.ReadLineFromFile();
		++line_num;
		boost::trim(text_line);

		boost::smatch timestamp_match;
		switch (state) {
			case STATE_INITIAL:
				// ignore leading blank lines
				if (text_line.empty()) break;
				if (all(text_line, boost::is_digit())) {
					// found the line number, throw it away and hope for timestamps
					state = STATE_TIMESTAMP;
					break;
				}
				if (regex_search(text_line, timestamp_match, timestamp_regex))
					goto found_timestamps;

				throw SRTParseError(agi::format("Parsing SRT: Expected subtitle index at line %d", line_num));

			case STATE_TIMESTAMP:
				if (!regex_search(text_line, timestamp_match, timestamp_regex))
					throw SRTParseError(agi::format("Parsing SRT: Expected timestamp pair at line %d", line_num));
found_timestamps:
				if (line) {
					// finalize active line
					line->Text = tag_parser.ToAss(text);
					text.clear();
				}

				// create new subtitle
				line = new AssDialogue;
				line->Start = ReadSRTTime(timestamp_match.str(1));
				line->End = ReadSRTTime(timestamp_match.str(2));
				// store pointer to subtitle, we'll continue working on it
				target->Events.push_back(*line);
				// next we're reading the text
				state = STATE_FIRST_LINE_OF_BODY;
				break;

			case STATE_FIRST_LINE_OF_BODY:
				if (text_line.empty()) {
					// that's not very interesting... blank subtitle?
					state = STATE_LAST_WAS_BLANK;
					// no previous line that needs a line break after
					linebreak_debt = 0;
					break;
				}
				text.append(text_line);
				state = STATE_REST_OF_BODY;
				break;

			case STATE_REST_OF_BODY:
				if (text_line.empty()) {
					// Might be either the gap between two subtitles or just a
					// blank line in the middle of a subtitle, so defer adding
					// the line break until we check what's on the next line
					state = STATE_LAST_WAS_BLANK;
					linebreak_debt = 1;
					break;
				}
				text.append("\\N");
				text.append(text_line);
				break;

			case STATE_LAST_WAS_BLANK:
				++linebreak_debt;
				if (text_line.empty()) break;
				if (all(text_line, boost::is_digit())) {
					// Hopefully it's the start of a new subtitle, and the
					// previous blank line(s) were the gap between subtitles
					state = STATE_TIMESTAMP;
					break;
				}
				if (regex_search(text_line, timestamp_match, timestamp_regex))
					goto found_timestamps;

				// assume it's a continuation of the subtitle text
				// resolve our line break debt and append the line text
				while (linebreak_debt-- > 0)
					text.append("\\N");
				text.append(text_line);
				state = STATE_REST_OF_BODY;
				break;
		}
	}

	if (state == 1 || state == 2)
		throw SRTParseError("Parsing SRT: Incomplete file");

	if (line) // an unfinalized line
		line->Text = tag_parser.ToAss(text);
}

void SRTSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, agi::vfr::Framerate const& fps, std::string const& encoding) const {
	TextFileWriter file(filename, encoding);

	// Convert to SRT
	AssFile copy(*src);
	copy.Sort();
	StripComments(copy);
	RecombineOverlaps(copy);
	MergeIdentical(copy);
#ifdef _WIN32
	ConvertNewlines(copy, "\r\n", false);
#else
	ConvertNewlines(copy, "\n", false);
#endif

	// Write lines
	int i=0;
	for (auto const& current : copy.Events) {
		file.WriteLineToFile(std::to_string(++i));
		file.WriteLineToFile(WriteSRTTime(current.Start) + " --> " + WriteSRTTime(current.End));
		file.WriteLineToFile(ConvertTags(&current));
		file.WriteLineToFile("");
	}
}

bool SRTSubtitleFormat::CanSave(const AssFile *file) const {
	std::string supported_tags[] = { "\\b", "\\i", "\\s", "\\u" };

	if (!file->Attachments.empty())
		return false;

	auto def = boost::flyweight<std::string>("Default");
	for (auto const& line : file->Events) {
		if (line.Style != def)
			return false;

		auto blocks = line.ParseTags();
		for (auto ovr : blocks | agi::of_type<AssDialogueBlockOverride>()) {
			// Verify that all overrides used are supported
			for (auto const& tag : ovr->Tags) {
				if (!std::binary_search(supported_tags, std::end(supported_tags), tag.Name))
					return false;
			}
		}
	}

	return true;
}

std::string SRTSubtitleFormat::ConvertTags(const AssDialogue *diag) const {
	std::string final;
	std::map<char, bool> tag_states;
	tag_states['i'] = false;
	tag_states['b'] = false;
	tag_states['u'] = false;
	tag_states['s'] = false;

	for (auto& block : diag->ParseTags()) {
		if (AssDialogueBlockOverride* ovr = dynamic_cast<AssDialogueBlockOverride*>(block.get())) {
			// Iterate through overrides
			for (auto const& tag : ovr->Tags) {
				if (tag.IsValid() && tag.Name.size() == 2) {
					auto it = tag_states.find(tag.Name[1]);
					if (it != tag_states.end()) {
						bool temp = tag.Params[0].Get(false);
						if (temp && !it->second)
							final += agi::format("<%c>", it->first);
						if (!temp && it->second)
							final += agi::format("</%c>", it->first);
						it->second = temp;
					}
				}
			}
		}
		// Plain text
		else if (AssDialogueBlockPlain *plain = dynamic_cast<AssDialogueBlockPlain*>(block.get()))
			final += plain->GetText();
	}

	// Ensure all tags are closed
	// Otherwise unclosed overrides might affect lines they shouldn't, see bug #809 for example
	for (auto it : tag_states) {
		if (it.second)
			final += agi::format("</%c>", it.first);
	}

	return final;
}
