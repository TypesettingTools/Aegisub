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


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <stdio.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_time.h"
#include "subtitle_format_transtation.h"
#include "text_file_writer.h"


/// @brief Name 
/// @return 
///
wxString TranStationSubtitleFormat::GetName() {
	return _T("TranStation");
}



/// @brief Wildcards 
/// @return 
///
wxArrayString TranStationSubtitleFormat::GetWriteWildcards() {
	wxArrayString formats;
	formats.Add(_T("transtation.txt"));
	return formats;
}



/// @brief Can write file? 
/// @param filename 
/// @return 
///
bool TranStationSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(16).Lower() == _T(".transtation.txt"));
}



/// @brief Write file 
/// @param _filename 
/// @param encoding  
/// @return 
///
void TranStationSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
	// Get FPS
	FPSRational fps_rat = AskForFPS(true);
	if (fps_rat.num <= 0 || fps_rat.den <= 0) return;

	// Open file
	TextFileWriter file(_filename,encoding);

	// Convert to TranStation
	CreateCopy();
	SortLines();
	StripComments();
	RecombineOverlaps();
	MergeIdentical();

	// Write lines
	using std::list;
	AssDialogue *current	= NULL;
	AssDialogue *next		= NULL;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		if (next) 
			current = next;
		next = dynamic_cast<AssDialogue*>(*cur);

		if (current && !current->Comment) {
			// Write text
			file.WriteLineToFile(ConvertLine(current,&fps_rat,(next && !next->Comment) ? next->Start.GetMS() : -1));
			file.WriteLineToFile(_T(""));
		}
	}
	// flush last line
	if (next && !next->Comment)
		file.WriteLineToFile(ConvertLine(next,&fps_rat,-1));

	// Every file must end with this line
	file.WriteLineToFile(_T("SUB["));

	// Clean up
	ClearCopy();
}


/// @brief DOCME
/// @param current     
/// @param fps_rat     
/// @param nextl_start 
///
wxString TranStationSubtitleFormat::ConvertLine(AssDialogue *current, FPSRational *fps_rat, int nextl_start) {
	// Get line data
	AssStyle *style = GetAssFile()->GetStyle(current->Style);
	int valign = 0;
	const wxChar *halign = _T(" "); // default is centered
	const wxChar *type = _T("N"); // no special style
	if (style) {
		if (style->alignment >= 4) valign = 4;
		if (style->alignment >= 7) valign = 9;
		if (style->alignment == 1 || style->alignment == 4 || style->alignment == 7) halign = _T("L");
		if (style->alignment == 3 || style->alignment == 6 || style->alignment == 9) halign = _T("R");
		if (style->italic) type = _T("I");
	}

	// Hack: If an italics-tag (\i1) appears anywhere in the line,
	// make it all italics
	if (current->Text.Find(_T("\\i1")) != wxNOT_FOUND)type = _T("I");

	// Write header
	AssTime start = current->Start;
	AssTime end = current->End;

	// Subtract one frame if the end time of the current line is equal to the
	// start of next one, since the end timestamp is inclusive and the lines
	// would overlap if left as is.
	if (nextl_start > 0 && end.GetMS() == nextl_start)
		end.SetMS(end.GetMS() - ((1000*fps_rat->den)/fps_rat->num));

	FractionalTime ft(_T(":"), fps_rat->num, fps_rat->den, fps_rat->smpte_dropframe);
	wxString header = wxString::Format(_T("SUB[%i%s%s "),valign,halign,type) + ft.FromAssTime(start) + _T(">") + ft.FromAssTime(end) + _T("]\r\n");

	// Process text
	wxString lineEnd = _T("\r\n");
	current->StripTags();
	current->Text.Replace(_T("\\h"),_T(" "),true);
	current->Text.Replace(_T("\\n"),lineEnd,true);
	current->Text.Replace(_T("\\N"),lineEnd,true);
	while (current->Text.Replace(lineEnd+lineEnd,lineEnd,true)) {};

	return header + current->Text;
}

