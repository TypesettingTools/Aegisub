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

/// @file subtitle_format_encore.cpp
/// @brief Reading/writing Adobe Encore subtitle format
/// @ingroup subtitle_io
///

#include "subtitle_format_encore.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "text_file_writer.h"

#include <libaegisub/ass/smpte.h>
#include <libaegisub/format.h>

EncoreSubtitleFormat::EncoreSubtitleFormat()
: SubtitleFormat("Adobe Encore")
{
}

std::vector<std::string> EncoreSubtitleFormat::GetWriteWildcards() const {
	return {"encore.txt"};
}

void EncoreSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, agi::vfr::Framerate const& video_fps, std::string const&) const {
	agi::vfr::Framerate fps = AskForFPS(false, true, video_fps);
	if (!fps.IsLoaded()) return;

	// Convert to encore
	AssFile copy(*src);
	copy.Sort();
	StripComments(copy);
	RecombineOverlaps(copy);
	MergeIdentical(copy);
	StripTags(copy);
	ConvertNewlines(copy, "\r\n");

	// Encore wants ; for NTSC and : for PAL
	// The manual suggests no other frame rates are supported
	agi::SmpteFormatter ft(fps, fps.NeedsDropFrames() ? ';' : ':');

	// Write lines
	int i = 0;
	TextFileWriter file(filename, "UTF-8");
	for (auto const& current : copy.Events)
		file.WriteLineToFile(agi::format("%i %s %s %s", ++i, ft.ToSMPTE(current.Start), ft.ToSMPTE(current.End), current.Text));
}
