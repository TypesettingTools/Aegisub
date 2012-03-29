// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file subtitle_format_transtation.cpp
/// @brief Reading/writing Transtation-compatible subtitles
/// @ingroup subtitle_io
///

#include "config.h"

#ifndef AGI_PRE
#include <stdio.h>
#endif

#include "subtitle_format_transtation.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_time.h"
#include "text_file_writer.h"

TranStationSubtitleFormat::TranStationSubtitleFormat()
: SubtitleFormat("TranStation")
{
}

wxArrayString TranStationSubtitleFormat::GetWriteWildcards() const {
	wxArrayString formats;
	formats.Add("transtation.txt");
	return formats;
}

bool TranStationSubtitleFormat::CanWriteFile(wxString const& filename) const {
	return filename.Lower().EndsWith(".transtation.txt");
}

void TranStationSubtitleFormat::WriteFile(const AssFile *src, wxString const& filename, wxString const& encoding) const {
	FractionalTime ft = AskForFPS(true);
	if (!ft.FPS().IsLoaded()) return;

	TextFileWriter file(filename, encoding);

	// Convert to TranStation
	AssFile copy(*src);
	copy.Sort();
	StripComments(copy.Line);
	RecombineOverlaps(copy.Line);
	MergeIdentical(copy.Line);

	AssDialogue *prev = 0;
	for (std::list<AssEntry*>::iterator it = copy.Line.begin(); it != copy.Line.end(); ++it) {
		AssDialogue *cur = dynamic_cast<AssDialogue*>(*it);

		if (prev && cur) {
			file.WriteLineToFile(ConvertLine(&copy, prev, &ft, cur->Start));
			file.WriteLineToFile("");
		}

		if (cur)
			prev = cur;
	}

	// flush last line
	if (prev)
		file.WriteLineToFile(ConvertLine(&copy, prev, &ft, -1));

	// Every file must end with this line
	file.WriteLineToFile("SUB[");
}

wxString TranStationSubtitleFormat::ConvertLine(AssFile *file, AssDialogue *current, FractionalTime *ft, int nextl_start) const {
	int valign = 0;
	const char *halign = " "; // default is centered
	const char *type = "N"; // no special style
	if (AssStyle *style = file->GetStyle(current->Style)) {
		if (style->alignment >= 4) valign = 4;
		if (style->alignment >= 7) valign = 9;
		if (style->alignment == 1 || style->alignment == 4 || style->alignment == 7) halign = "L";
		if (style->alignment == 3 || style->alignment == 6 || style->alignment == 9) halign = "R";
		if (style->italic) type = "I";
	}

	// Hack: If an italics-tag (\i1) appears anywhere in the line,
	// make it all italics
	if (current->Text.Find("\\i1") != wxNOT_FOUND) type = "I";

	// Write header
	AssTime end = current->End;

	// Subtract one frame if the end time of the current line is equal to the
	// start of next one, since the end timestamp is inclusive and the lines
	// would overlap if left as is.
	if (nextl_start > 0 && end == nextl_start)
		end = ft->FPS().TimeAtFrame(ft->FPS().FrameAtTime(end, agi::vfr::END) - 1, agi::vfr::END);

	wxString header = wxString::Format("SUB[%i%s%s ", valign, halign, type) + ft->ToSMPTE(current->Start) + ">" + ft->ToSMPTE(end) + "]\r\n";

	// Process text
	wxString lineEnd = "\r\n";
	current->StripTags();
	current->Text.Replace("\\h", " ", true);
	current->Text.Replace("\\n", lineEnd, true);
	current->Text.Replace("\\N", lineEnd, true);
	while (current->Text.Replace(lineEnd + lineEnd, lineEnd, true));

	return header + current->Text;
}
