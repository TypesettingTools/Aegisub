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

#include "ass_dialogue.h"
#include "compat.h"
#include "text_file_reader.h"
#include "text_file_writer.h"

DEFINE_SIMPLE_EXCEPTION(AssParseError, SubtitleFormatParseError, "subtitle_io/parse/ass")

ASSSubtitleFormat::ASSSubtitleFormat()
: SubtitleFormat("Advanced Substation Alpha")
{
}

wxArrayString ASSSubtitleFormat::GetReadWildcards() const {
	wxArrayString formats;
	formats.Add("ass");
	formats.Add("ssa");
	return formats;
}

wxArrayString ASSSubtitleFormat::GetWriteWildcards() const {
	wxArrayString formats;
	formats.Add("ass");
	formats.Add("ssa");
	return formats;
}

void ASSSubtitleFormat::ReadFile(wxString const& filename, wxString const& encoding) {
	using namespace std;

	TextFileReader file(filename, encoding);
	int version = filename.Right(4).Lower() != ".ssa";

	wxString curgroup;
	while (file.HasMoreLines()) {
		wxString line = file.ReadLineFromFile();

		// Make sure that the first non-blank non-comment non-group-header line
		// is really [Script Info]
		if (curgroup.empty() && !line.empty() && line[0] != ';' && line[0] != '[') {
			curgroup = "[Script Info]";
			AddLine(curgroup, curgroup, version, &curgroup);
		}

		// Convert v4 styles to v4+ styles
		if (!line.empty() && line[0] == '[') {
			// Ugly hacks to allow intermixed v4 and v4+ style sections
			wxString low = line.Lower();
			if (low == "[v4 styles]") {
				line = "[V4+ Styles]";
				curgroup = line;
				version = 0;
			}
			else if (low == "[v4+ styles]") {
				line = "[V4+ Styles]";
				curgroup = line;
				version = 1;
			}
			else if (low == "[v4++ styles]") {
				line = "[V4+ Styles]";
				curgroup = line;
				version = 2;
			}
			// Not-so-special case for other groups, just set it
			else {
				curgroup = line;
			}
		}

		try {
			AddLine(line, curgroup, version, &curgroup);
		}
		catch (const char *err) {
			Clear();
			throw AssParseError("Error processing line: " + STD_STR(line) + ": " + err, 0);
		}
	}
}

void ASSSubtitleFormat::WriteFile(wxString const& filename, wxString const& encoding) {
	TextFileWriter file(filename, encoding);
	bool ssa = filename.Right(4).Lower() == ".ssa";

	std::list<AssEntry*>::iterator last = Line->end(); --last;
	wxString group = Line->front()->group;
	for (std::list<AssEntry*>::iterator cur=Line->begin(); cur!=Line->end(); ++cur) {
		// Add a blank line between each group
		if ((*cur)->group != group) {
			file.WriteLineToFile("");
			group = (*cur)->group;
		}

		// Only add a line break if there is a next line
		bool lineBreak = cur != last;

		// Write line
		if (ssa) file.WriteLineToFile((*cur)->GetSSAText(), lineBreak);
		else file.WriteLineToFile((*cur)->GetEntryData(), lineBreak);
	}
}
