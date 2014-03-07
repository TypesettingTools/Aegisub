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

/// @file subtitle_format_transtation.cpp
/// @brief Reading/writing Transtation-compatible subtitles
/// @ingroup subtitle_io
///

#include "config.h"

#include <cstdio>

#include "subtitle_format_transtation.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_time.h"
#include "text_file_writer.h"

#include <libaegisub/of_type_adaptor.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

TranStationSubtitleFormat::TranStationSubtitleFormat()
: SubtitleFormat("TranStation")
{
}

std::vector<std::string> TranStationSubtitleFormat::GetWriteWildcards() const {
	std::vector<std::string> formats;
	formats.push_back("transtation.txt");
	return formats;
}

void TranStationSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, std::string const& encoding) const {
	agi::vfr::Framerate fps = AskForFPS(false, true);
	if (!fps.IsLoaded()) return;

	// Convert to TranStation
	AssFile copy(*src);
	copy.Sort();
	StripComments(copy);
	RecombineOverlaps(copy);
	MergeIdentical(copy);
	StripTags(copy);
	ConvertNewlines(copy, "\r\n");

	SmpteFormatter ft(fps);
	TextFileWriter file(filename, encoding);
	AssDialogue *prev = nullptr;
	for (auto cur : copy.Events | agi::of_type<AssDialogue>()) {
		if (prev) {
			file.WriteLineToFile(ConvertLine(&copy, prev, fps, ft, cur->Start));
			file.WriteLineToFile("");
		}

		prev = cur;
	}

	// flush last line
	if (prev)
		file.WriteLineToFile(ConvertLine(&copy, prev, fps, ft, -1));

	// Every file must end with this line
	file.WriteLineToFile("SUB[");
}

std::string TranStationSubtitleFormat::ConvertLine(AssFile *file, AssDialogue *current, agi::vfr::Framerate const& fps, SmpteFormatter const& ft, int nextl_start) const {
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
	if (current->Text.get().find("\\i1") != std::string::npos) type = "I";

	// Write header
	AssTime end = current->End;

	// Subtract one frame if the end time of the current line is equal to the
	// start of next one, since the end timestamp is inclusive and the lines
	// would overlap if left as is.
	if (nextl_start > 0 && end == nextl_start)
		end = fps.TimeAtFrame(fps.FrameAtTime(end, agi::vfr::END) - 1, agi::vfr::END);

	std::string header = str(boost::format("SUB[%i%s%s %s>%s]\r\n") % valign % halign % type % ft.ToSMPTE(current->Start) % ft.ToSMPTE(end));
	return header + current->Text.get();
}
