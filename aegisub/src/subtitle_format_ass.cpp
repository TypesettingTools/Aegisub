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
//
// $Id$

/// @file subtitle_format_ass.cpp
/// @brief Reading/writing of SSA-lineage subtitles
/// @ingroup subtitle_io
///

#include "config.h"

#include "subtitle_format_ass.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "text_file_reader.h"
#include "text_file_writer.h"

DEFINE_SIMPLE_EXCEPTION(AssParseError, SubtitleFormatParseError, "subtitle_io/parse/ass")

AssSubtitleFormat::AssSubtitleFormat()
: SubtitleFormat("Advanced Substation Alpha")
{
}

wxArrayString AssSubtitleFormat::GetReadWildcards() const {
	wxArrayString formats;
	formats.Add("ass");
	formats.Add("ssa");
	return formats;
}

wxArrayString AssSubtitleFormat::GetWriteWildcards() const {
	wxArrayString formats;
	formats.Add("ass");
	formats.Add("ssa");
	return formats;
}

void AssSubtitleFormat::ReadFile(AssFile *target, wxString const& filename, wxString const& encoding) const {
	target->Clear();

	TextFileReader file(filename, encoding);
	int version = filename.Right(4).Lower() != ".ssa";
	AssAttachment *attach = 0;

	while (file.HasMoreLines()) {
		wxString line = file.ReadLineFromFile();
		try {
			AddLine(target, line, &version, &attach);
		}
		catch (const char *err) {
			target->Clear();
			throw AssParseError("Error processing line: " + STD_STR(line) + ": " + err, 0);
		}
	}
}

void AssSubtitleFormat::WriteFile(const AssFile *src, wxString const& filename, wxString const& encoding) const {
	TextFileWriter file(filename, encoding);
	bool ssa = filename.Right(4).Lower() == ".ssa";

	wxString group = src->Line.front()->group;
	for (LineList::const_iterator cur = src->Line.begin(); cur != src->Line.end(); ++cur) {
		// Add a blank line between each group
		if ((*cur)->group != group) {
			file.WriteLineToFile("");
			group = (*cur)->group;
		}

		file.WriteLineToFile(ssa ? (*cur)->GetSSAText() : (*cur)->GetEntryData(), true);
	}
}

void AssSubtitleFormat::AddLine(AssFile *target, wxString data, int *version, AssAttachment **attach) {
	// Is this line an attachment filename?
	bool isFilename = data.StartsWith("fontname: ") || data.StartsWith("filename: ");

	// If there's an attachment in progress, deal with it first as an
	// attachment data line can appear to be other things
	if (*attach) {
		// Check if it's valid data
		bool validData = data.size() > 0 && data.size() <= 80;
		for (size_t i = 0; i < data.size(); ++i) {
			if (data[i] < 33 || data[i] >= 97) {
				validData = false;
				break;
			}
		}

		// Data is over, add attachment to the file
		if (!validData || isFilename) {
			(*attach)->Finish();
			target->Line.push_back(*attach);
			*attach = NULL;
		}
		else {
			// Insert data
			(*attach)->AddData(data);

			// Done building
			if (data.Length() < 80) {
				(*attach)->Finish();
				target->Line.push_back(*attach);
				*attach = NULL;
				return;
			}
		}
	}

	if (data.empty()) return;

	// Section header
	if (data[0] == '[' && data.Last() == ']') {
		// Ugly hacks to allow intermixed v4 and v4+ style sections
		wxString low = data.Lower();
		if (low == "[v4 styles]") {
			data = "[V4+ Styles]";
			*version = 0;
		}
		else if (low == "[v4+ styles]") {
			data = "[V4+ Styles]";
			*version = 1;
		}

		target->Line.push_back(new AssEntry(data, data));
		return;
	}

	// If the first nonblank line isn't a header pretend it starts with [Script Info]
	if (target->Line.empty())
		target->Line.push_back(new AssEntry("[Script Info]", "[Script Info]"));

	wxString group = target->Line.back()->group;
	wxString lowGroup = group.Lower();

	// Attachment
	if (lowGroup == "[fonts]" || lowGroup == "[graphics]") {
		if (isFilename) {
			*attach = new AssAttachment(data.Mid(10), group);
		}
	}
	// Dialogue
	else if (lowGroup == "[events]") {
		if (data.StartsWith("Dialogue:") || data.StartsWith("Comment:"))
			target->Line.push_back(new AssDialogue(data));
		else if (data.StartsWith("Format:"))
			target->Line.push_back(new AssEntry("Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text", group));
	}
	// Style
	else if (lowGroup == "[v4+ styles]") {
		if (data.StartsWith("Style:"))
			target->Line.push_back(new AssStyle(data, *version));
		else if (data.StartsWith("Format:"))
			target->Line.push_back(new AssEntry("Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding", group));
	}
	// Script info
	else if (lowGroup == "[script info]") {
		// Comment
		if (data.StartsWith(";")) {
			// Skip stupid comments added by other programs
			// Of course, we'll add our own in place later... ;)
			return;
		}

		if (data.StartsWith("ScriptType:")) {
			wxString versionString = data.Mid(11).Trim(true).Trim(false).Lower();
			int trueVersion;
			if (versionString == "v4.00")
				trueVersion = 0;
			else if (versionString == "v4.00+")
				trueVersion = 1;
			else
				throw "Unknown SSA file format version";
			if (trueVersion != *version) {
				wxLogMessage("Warning: File has the wrong extension.");
				*version = trueVersion;
			}
		}

		// Everything
		target->Line.push_back(new AssEntry(data, group));
	}
	// Unrecognized group
	else {
		target->Line.push_back(new AssEntry(data, group));
	}
}
