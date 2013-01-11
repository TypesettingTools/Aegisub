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

#include "config.h"

#include "subtitle_format_txt.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "dialog_text_import.h"
#include "options.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"
#include "version.h"

#include <libaegisub/of_type_adaptor.h>

TXTSubtitleFormat::TXTSubtitleFormat()
: SubtitleFormat("Plain-Text")
{
}

wxArrayString TXTSubtitleFormat::GetReadWildcards() const {
	wxArrayString formats;
	formats.Add("txt");
	return formats;
}

wxArrayString TXTSubtitleFormat::GetWriteWildcards() const {
	return GetReadWildcards();
}

bool TXTSubtitleFormat::CanWriteFile(wxString const& filename) const {
	return (filename.Right(4).Lower() == ".txt" && filename.Right(11).Lower() != ".encore.txt" && filename.Right(16).Lower() != ".transtation.txt");
}

void TXTSubtitleFormat::ReadFile(AssFile *target, wxString const& filename, wxString const& encoding) const {
	using namespace std;
	DialogTextImport dlg;
	if (dlg.ShowModal() == wxID_CANCEL) return;

	TextFileReader file(filename, encoding, false);

	target->LoadDefault(false);

	wxString actor;
	wxString separator = to_wx(OPT_GET("Tool/Import/Text/Actor Separator")->GetString());
	wxString comment = to_wx(OPT_GET("Tool/Import/Text/Comment Starter")->GetString());

	// Parse file
	while (file.HasMoreLines()) {
		wxString value = file.ReadLineFromFile();
		if(value.empty()) continue;

		// Check if this isn't a timecodes file
		if (value.StartsWith("# timecode"))
			throw SubtitleFormatParseError("File is a timecode file, cannot load as subtitles.", 0);

		// Read comment data
		bool isComment = false;
		if (!comment.empty() && value.StartsWith(comment)) {
			isComment = true;
			value = value.Mid(comment.size());
		}

		// Read actor data
		if (!isComment && !separator.empty()) {
			if (value[0] != ' ' && value[0] != '\t') {
				int pos = value.Find(separator);
				if (pos != wxNOT_FOUND) {
					actor = value.Left(pos);
					actor.Trim(false);
					actor.Trim(true);
					value = value.Mid(pos+1);
				}
			}
		}

		// Trim spaces at start
		value.Trim(false);

		if (value.empty())
			isComment = true;

		// Sets line up
		AssDialogue *line = new AssDialogue;
		line->Actor = isComment ? wxString() : actor;
		line->Comment = isComment;
		line->Text = value;
		line->End = 0;

		// Adds line
		target->Line.push_back(*line);
	}
}

void TXTSubtitleFormat::WriteFile(const AssFile *src, wxString const& filename, wxString const& encoding) const {
	size_t num_actor_names = 0, num_dialogue_lines = 0;

	// Detect number of lines with Actor field filled out
	for (auto dia : src->Line | agi::of_type<AssDialogue>()) {
		if (!dia->Comment) {
			num_dialogue_lines++;
			if (!dia->Actor.get().empty())
				num_actor_names++;
		}
	}

	// If too few lines have Actor filled out, don't write it
	bool write_actors = num_actor_names > num_dialogue_lines/2;
	bool strip_formatting = true;

	TextFileWriter file(filename, encoding);
	file.WriteLineToFile(wxString("# Exported by Aegisub ") + GetAegisubShortVersionString());

	// Write the file
	for (auto dia : src->Line | agi::of_type<AssDialogue>()) {
		wxString out_line;

		if (dia->Comment)
			out_line = "# ";

		if (write_actors)
			out_line += dia->Actor + ": ";

		wxString out_text = strip_formatting ? dia->GetStrippedText() : dia->Text;
		out_line += out_text;

		if (!out_text.empty())
			file.WriteLineToFile(out_line);
	}
}
