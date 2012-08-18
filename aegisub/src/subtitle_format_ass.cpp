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
#include "ass_file.h"
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

void ASSSubtitleFormat::ReadFile(AssFile *target, wxString const& filename, wxString const& encoding) const {
	using namespace std;

	target->Clear();

	TextFileReader file(filename, encoding);
	int version = filename.Right(4).Lower() != ".ssa";
	AssAttachment *attach = 0;

	while (file.HasMoreLines()) {
		wxString line = file.ReadLineFromFile();
		try {
			target->AddLine(line, &version, &attach);
		}
		catch (const char *err) {
			target->Clear();
			throw AssParseError("Error processing line: " + STD_STR(line) + ": " + err, 0);
		}
	}
}

void ASSSubtitleFormat::WriteFile(const AssFile *src, wxString const& filename, wxString const& encoding) const {
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
