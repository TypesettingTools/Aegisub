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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "subtitle_format_transtation.h"
#include "text_file_writer.h"
#include <stdio.h>


////////
// Name
wxString TranStationSubtitleFormat::GetName() {
	return _T("TranStation");
}


/////////////
// Wildcards
wxArrayString TranStationSubtitleFormat::GetWriteWildcards() {
	wxArrayString formats;
	formats.Add(_T("transtation.txt"));
	return formats;
}


///////////////////
// Can write file?
bool TranStationSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(16).Lower() == _T(".transtation.txt"));
}


//////////////
// Write file
void TranStationSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
	// Get FPS
	double fps = AskForFPS(true);
	if (fps <= 0.0) return;

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
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current && !current->Comment) {
			// Get line data
			AssStyle *style = GetAssFile()->GetStyle(current->Style);
			int valign = 0;
			wxChar *halign = _T(" "); // default is centered
			wxChar *type = _T("N"); // no special style
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
			// Subtract half a frame duration from end time, since it is inclusive
			// and we otherwise run the risk of having two lines overlap in a
			// frame, when they should run right into each other.
			end.SetMS(end.GetMS() - (int)(500.0/fps));
			wxString header = wxString::Format(_T("SUB[%i%s%s "),valign,halign,type) + start.GetSMPTE(fps) + _T(">") + end.GetSMPTE(fps) + _T("]");
			file.WriteLineToFile(header);

			// Process text
			wxString lineEnd = _T("\r\n");
			current->StripTags();
			current->Text.Replace(_T("\\h"),_T(" "),true);
			current->Text.Replace(_T("\\n"),lineEnd,true);
			current->Text.Replace(_T("\\N"),lineEnd,true);
			while (current->Text.Replace(lineEnd+lineEnd,lineEnd,true));

			// Write text
			file.WriteLineToFile(current->Text);
			file.WriteLineToFile(_T(""));
		}
	}

	// Every file must end with this line
	file.WriteLineToFile(_T("SUB["));

	// Clean up
	ClearCopy();
}
