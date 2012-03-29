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

/// @file subtitle_format_encore.cpp
/// @brief Reading/writing Adobe Encore subtitle format
/// @ingroup subtitle_io
///

#include "config.h"

#include "subtitle_format_encore.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "text_file_writer.h"

EncoreSubtitleFormat::EncoreSubtitleFormat()
: SubtitleFormat("Adobe Encore")
{
}

wxArrayString EncoreSubtitleFormat::GetWriteWildcards() const {
	wxArrayString formats;
	formats.Add("encore.txt");
	return formats;
}

bool EncoreSubtitleFormat::CanWriteFile(wxString const& filename) const {
	return filename.EndsWith(".encore.txt");
}

void EncoreSubtitleFormat::WriteFile(const AssFile *src, wxString const& filename, wxString const&) const {
	agi::vfr::Framerate fps = AskForFPS(false, true);
	if (!fps.IsLoaded()) return;

	// Convert to encore
	AssFile copy(*src);
	copy.Sort();
	StripComments(copy.Line);
	RecombineOverlaps(copy.Line);
	MergeIdentical(copy.Line);
	StripTags(copy.Line);
	ConvertNewlines(copy.Line, "\r\n");


	// Encode wants ; for NTSC and : for PAL
	// The manual suggests no other frame rates are supported
	char sep = fps.NeedsDropFrames() ? ';' : ':';
	SmpteFormatter ft(fps, sep);

	// Write lines
	int i = 0;
	TextFileWriter file(filename, "UTF-8");
	for (LineList::const_iterator cur = copy.Line.begin(); cur != copy.Line.end(); ++cur) {
		if (AssDialogue *current = dynamic_cast<AssDialogue*>(*cur)) {
			++i;
			file.WriteLineToFile(wxString::Format("%i %s %s %s", i, ft.ToSMPTE(current->Start), ft.ToSMPTE(current->End), current->Text));
		}
	}
}
