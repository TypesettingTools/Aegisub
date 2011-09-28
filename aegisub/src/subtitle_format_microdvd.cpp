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

/// @file subtitle_format_microdvd.cpp
/// @brief Reading/writing MicroDVD subtitle format (.SUB)
/// @ingroup subtitle_io
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/regex.h>
#endif

#include "ass_dialogue.h"
#include "ass_time.h"
#include "subtitle_format_microdvd.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "video_context.h"

MicroDVDSubtitleFormat::MicroDVDSubtitleFormat()
: SubtitleFormat("MicroDVD")
{
}

wxArrayString MicroDVDSubtitleFormat::GetReadWildcards() const {
	wxArrayString formats;
	formats.Add("sub");
	return formats;
}

wxArrayString MicroDVDSubtitleFormat::GetWriteWildcards() const {
	return GetReadWildcards();
}

bool MicroDVDSubtitleFormat::CanReadFile(wxString const& filename) const {
	// Return false immediately if extension is wrong
	if (filename.Right(4).Lower() != ".sub") return false;

	// Since there is an infinity of .sub formats, load first line and check if it's valid
	TextFileReader file(filename);
	if (file.HasMoreLines()) {
		wxRegEx exp("^[\\{\\[]([0-9]+)[\\}\\]][\\{\\[]([0-9]+)[\\}\\]](.*)$", wxRE_ADVANCED);
		return exp.Matches(file.ReadLineFromFile());
	}

	return false;
}

void MicroDVDSubtitleFormat::ReadFile(wxString const& filename, wxString const& forceEncoding) {
	TextFileReader file(filename);
	wxRegEx exp("^[\\{\\[]([0-9]+)[\\}\\]][\\{\\[]([0-9]+)[\\}\\]](.*)$", wxRE_ADVANCED);

	LoadDefault(false);

	agi::vfr::Framerate cfr;
	const agi::vfr::Framerate *rate = &cfr;

	bool isFirst = true;
	FPSRational fps_rat;
	double fps = 0.0;
	while (file.HasMoreLines()) {
		wxString line = file.ReadLineFromFile();
		if (exp.Matches(line)) {
			long f1, f2;
			exp.GetMatch(line, 1).ToLong(&f1);
			exp.GetMatch(line, 2).ToLong(&f2);
			wxString text = exp.GetMatch(line, 3);

			// If it's the first, check if it contains fps information
			if (isFirst) {
				if (f1 == 1 && f2 == 1) {
					// Convert fps
					try {
						text.ToDouble(&fps);
					}
					catch (...) { }
				}
				isFirst = false;

				// If it wasn't an fps line, ask the user for it
				if (fps <= 0.0) {
					fps_rat = AskForFPS();
					if (fps_rat.num == 0) return;
					else if (fps_rat.num > 0) cfr = double(fps_rat.num)/fps_rat.den;
					else rate = &VideoContext::Get()->FPS();
				}
				else {
					cfr = fps;
					continue;
				}
			}

			text.Replace("|", "\\N");

			AssDialogue *line = new AssDialogue;
			line->group = "[Events]";
			line->Style = "Default";
			line->Start.SetMS(rate->TimeAtFrame(f1, agi::vfr::START));
			line->End.SetMS(rate->TimeAtFrame(f2, agi::vfr::END));
			line->Text = text;
			Line->push_back(line);
		}
	}
}

void MicroDVDSubtitleFormat::WriteFile(wxString const& filename, wxString const& encoding) {
	agi::vfr::Framerate cfr;
	const agi::vfr::Framerate *rate = &cfr;

	FPSRational fps_rat = AskForFPS();
	if (fps_rat.num == 0 || fps_rat.den == 0) return;
	double fps = double(fps_rat.num) / fps_rat.den;
	if (fps > 0.0) cfr = fps;
	else rate = &VideoContext::Get()->FPS();

	// Convert file
	CreateCopy();
	SortLines();
	StripComments();
	RecombineOverlaps();
	MergeIdentical();
	ConvertTags(1, "|");

	TextFileWriter file(filename, encoding);

	// Write FPS line
	if (!rate->IsVFR()) {
		file.WriteLineToFile(wxString::Format("{1}{1}%.6f", rate->FPS()));
	}

	// Write lines
	for (std::list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		if (AssDialogue *current = dynamic_cast<AssDialogue*>(*cur)) {
			int start = rate->FrameAtTime(current->Start.GetMS(), agi::vfr::START);
			int end = rate->FrameAtTime(current->End.GetMS(), agi::vfr::END);

			file.WriteLineToFile(wxString::Format("{%i}{%i}%s", start, end, current->Text));
		}
	}

	ClearCopy();
}
