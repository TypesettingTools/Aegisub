// Copyright (c) 2026, Marko Hohos <hohosmarko@gmail.com>
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

#include "subtitle_format_lrc.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "boost/regex/v5/regex.hpp"
#include "boost/regex/v5/regex_search.hpp"
#include "libaegisub/ass/time.h"
#include "options.h"
#include "text_file_reader.h"
#include "text_file_writer.h"

#include <libaegisub/format.h>
#include <libaegisub/of_type_adaptor.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <format>
#include <sstream>

DEFINE_EXCEPTION(LRCParseError, SubtitleFormatParseError);

namespace {
	struct LRCLine {
		agi::Time start_time;
		std::string lyric;
	};

	agi::Time ParseTime(std::string time){
		char delim;
		int minutes{0};
		int seconds{0};
		int milliseconds{0};

		// Remove the '[' and ']'
		time.erase(0, 1);
		time.pop_back();

		std::istringstream ss{time};
		ss >> minutes >> delim >> seconds >> delim >> milliseconds;

		milliseconds *= 10;

		return agi::Time{(minutes * 60 + seconds) * 1000 + milliseconds};
	}
	std::string StringifyTime(agi::Time time){
		int milliseconds = time;

		int seconds = milliseconds / 1000;
		milliseconds -= seconds * 1000;

		int minutes = seconds / 60;
		seconds -= minutes * 60;

		// Reduce to 2 digits
		milliseconds /= 10;

		return std::format("[{:0>2}:{:0>2}.{:0>2}]", minutes, seconds, milliseconds);
	}

	LRCLine ParseLine(std::string line, int line_num){
		// "[mm:ss.ff]" eg.("[01:53.21]")
		static const boost::regex timestamp_regex("^\\[([0-9]{1,2}:[0-9]{1,2}.[0-9]{1,2})\\]");

		boost::smatch timestamp_match;
		if(!boost::regex_search(line, timestamp_match, timestamp_regex)){
			throw LRCParseError(std::format("Parsing LRC: Expected timestamp at line {}", line_num));
		}

		std::string timestamp = timestamp_match[0];
		line.erase(0, timestamp.size());
		// There is always a space between end of timestamp and lyric text
		boost::trim_left(line);

		return LRCLine{ParseTime(timestamp), line};
	}
}

LRCSubtitleFormat::LRCSubtitleFormat(): SubtitleFormat("Lyric"){

}
std::vector<std::string> LRCSubtitleFormat::GetReadWildcards() const {
	return {"lrc"};
}
std::vector<std::string> LRCSubtitleFormat::GetWriteWildcards() const {
	return GetReadWildcards();
}
void LRCSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename, agi::vfr::Framerate const&, const char *forceEncoding) const {
	TextFileReader file(filename, forceEncoding);
	target->LoadDefault(false, OPT_GET("Subtitle Format/LRC/Default Style Catalog")->GetString());

	AssDialogue *line{nullptr};
	int line_num = 0;
	LRCLine currentLine{ParseLine(file.ReadLineFromFile(), line_num++)};
	while(file.HasMoreLines()){
		LRCLine nextLine{ParseLine(file.ReadLineFromFile(), line_num++)};
		
		// New line
		line = new AssDialogue{};
		line->Start = currentLine.start_time;
		// LRC contains only start time, not the end, so have to take the end from next line
		line->End = nextLine.start_time;
		line->Text = currentLine.lyric;
		target->Events.push_back(*line);

		currentLine = std::move(nextLine);
	}
}
void LRCSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, agi::vfr::Framerate const&, const char *encoding) const {
	TextFileWriter file(filename, encoding);

	for(auto it = src->Events.cbegin(); it != src->Events.cend(); ++it){
		const auto &dialog = *it;
		std::string out_line = StringifyTime(dialog.Start);

		// Add the LRC spacing
		if(!dialog.Text->empty()){
			out_line += " ";
			out_line += dialog.Text;
		}

		if(out_line.size()){
			file.WriteLineToFile(out_line);
		}

		// If last lyric, write end
		if(it == std::prev(src->Events.cend())){
			out_line = StringifyTime(dialog.End);
			file.WriteLineToFile(out_line, false);
		}
	}
}