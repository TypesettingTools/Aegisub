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

wxString MicroDVDSubtitleFormat::GetName() {
	return _T("MicroDVD");
}

wxArrayString MicroDVDSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("sub"));
	return formats;
}

wxArrayString MicroDVDSubtitleFormat::GetWriteWildcards() {
	return GetReadWildcards();
}

bool MicroDVDSubtitleFormat::CanReadFile(wxString filename) {
	// Return false immediately if extension is wrong
	if (filename.Right(4).Lower() != _T(".sub")) return false;

	// Since there is an infinity of .sub formats, load first line and check if it's valid
	TextFileReader file(filename);
	if (file.HasMoreLines()) {
		wxRegEx exp(_T("^[\\{\\[]([0-9]+)[\\}\\]][\\{\\[]([0-9]+)[\\}\\]](.*)$"),wxRE_ADVANCED);
		return exp.Matches(file.ReadLineFromFile());
	}

	return false;
}

bool MicroDVDSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".sub"));
}

void MicroDVDSubtitleFormat::ReadFile(wxString filename,wxString forceEncoding) {
	TextFileReader file(filename);
	wxRegEx exp(_T("^[\\{\\[]([0-9]+)[\\}\\]][\\{\\[]([0-9]+)[\\}\\]](.*)$"),wxRE_ADVANCED);

	LoadDefault(false);

	agi::vfr::Framerate cfr;
	const agi::vfr::Framerate *rate = &cfr;

	bool isFirst = true;
	FPSRational fps_rat;
	double fps = 0.0;
	while (file.HasMoreLines()) {
		wxString line = file.ReadLineFromFile();
		if (exp.Matches(line)) {
			long f1,f2;
			exp.GetMatch(line,1).ToLong(&f1);
			exp.GetMatch(line,2).ToLong(&f2);
			wxString text = exp.GetMatch(line,3);

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

			// Start and end times
			int start,end;
			start = rate->TimeAtFrame(f1,agi::vfr::START);
			end = rate->TimeAtFrame(f2,agi::vfr::END);

			text.Replace(_T("|"),_T("\\N"));

			AssDialogue *line = new AssDialogue();
			line->group = _T("[Events]");
			line->Style = _T("Default");
			line->Start.SetMS(start);
			line->End.SetMS(end);
			line->Text = text;
			Line->push_back(line);
		}
	}
}

void MicroDVDSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
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
	ConvertTags(1,_T("|"));

	TextFileWriter file(filename,encoding);

	// Write FPS line
	if (!rate->IsVFR()) {
		file.WriteLineToFile(wxString::Format(_T("{1}{1}%.6f"),rate->FPS()));
	}

	// Write lines
	using std::list;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = dynamic_cast<AssDialogue*>(*cur);
		if (current && !current->Comment) {
			int start = rate->FrameAtTime(current->Start.GetMS(),agi::vfr::START);
			int end = rate->FrameAtTime(current->End.GetMS(),agi::vfr::END);

			file.WriteLineToFile(wxString::Format(_T("{%i}{%i}%s"),start,end,current->Text.c_str()));
		}
	}

	ClearCopy();
}
