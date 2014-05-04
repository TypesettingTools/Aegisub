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

/// @file subtitle_format_txt.cpp
/// @brief Importing/exporting subtitles to untimed plain text
/// @ingroup subtitle_io
///

#include "subtitle_format_txt.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "dialog_text_import.h"
#include "options.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"
#include "version.h"

#include <libaegisub/path.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

TXTSubtitleFormat::TXTSubtitleFormat()
: SubtitleFormat("Plain-Text")
{
}

std::vector<std::string> TXTSubtitleFormat::GetReadWildcards() const {
	std::vector<std::string> formats;
	formats.push_back("txt");
	return formats;
}

std::vector<std::string> TXTSubtitleFormat::GetWriteWildcards() const {
	return GetReadWildcards();
}

bool TXTSubtitleFormat::CanWriteFile(agi::fs::path const& filename) const {
	auto str = filename.string();
	return boost::iends_with(str, ".txt") && !(boost::iends_with(str, ".encore.txt") || boost::iends_with(str, ".transtation.txt"));
}

void TXTSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename, agi::vfr::Framerate const& fps, std::string const& encoding) const {
	DialogTextImport dlg;
	if (dlg.ShowModal() == wxID_CANCEL) return;

	TextFileReader file(filename, encoding, false);

	target->LoadDefault(false, OPT_GET("Subtitle Format/TXT/Default Style Catalog")->GetString());

	std::string actor;
	std::string separator = OPT_GET("Tool/Import/Text/Actor Separator")->GetString();
	std::string comment = OPT_GET("Tool/Import/Text/Comment Starter")->GetString();

	// Parse file
	while (file.HasMoreLines()) {
		std::string value = file.ReadLineFromFile();
		if (value.empty() && !OPT_GET("Tool/Import/Text/Include Blank")->GetBool()) continue;

		// Check if this isn't a timecodes file
		if (boost::starts_with(value, "# timecode"))
			throw SubtitleFormatParseError("File is a timecode file, cannot load as subtitles.", nullptr);

		// Read comment data
		bool isComment = false;
		if (!comment.empty() && boost::starts_with(value, comment)) {
			isComment = true;
			value.erase(0, comment.size());
		}

		// Read actor data
		if (!isComment && !separator.empty() && !value.empty()) {
			if (value[0] != ' ' && value[0] != '\t') {
				size_t pos = value.find(separator);
				if (pos != std::string::npos) {
					actor = value.substr(0, pos);
					boost::trim(actor);
					value.erase(0, pos + 1);
				}
			}
		}

		// Trim spaces at start
		boost::trim_left(value);

		if (value.empty())
			isComment = true;

		// Sets line up
		auto line = new AssDialogue;
		line->Actor = isComment ? std::string() : actor;
		line->Comment = isComment;
		line->Text = value;
		line->End = 0;

		target->Events.push_back(*line);
	}
}

void TXTSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, agi::vfr::Framerate const& fps, std::string const& encoding) const {
	size_t num_actor_names = 0, num_dialogue_lines = 0;

	// Detect number of lines with Actor field filled out
	for (auto const& dia : src->Events) {
		if (!dia.Comment) {
			num_dialogue_lines++;
			if (!dia.Actor.get().empty())
				num_actor_names++;
		}
	}

	// If too few lines have Actor filled out, don't write it
	bool write_actors = num_actor_names > num_dialogue_lines/2;
	bool strip_formatting = true;

	TextFileWriter file(filename, encoding);
	file.WriteLineToFile(std::string("# Exported by Aegisub ") + GetAegisubShortVersionString());

	// Write the file
	for (auto const& dia : src->Events) {
		std::string out_line;

		if (dia.Comment)
			out_line = "# ";

		if (write_actors)
			out_line += dia.Actor.get() + ": ";

		std::string out_text = strip_formatting ? dia.GetStrippedText() : dia.Text;
		out_line += out_text;

		if (!out_text.empty())
			file.WriteLineToFile(out_line);
	}
}
