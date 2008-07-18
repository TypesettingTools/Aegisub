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
#include "ass_dialogue.h"
#include "subtitle_format_encore.h"
#include "text_file_writer.h"


////////
// Name
wxString EncoreSubtitleFormat::GetName() {
	return _T("Adobe Encore");
}


/////////////
// Wildcards
wxArrayString EncoreSubtitleFormat::GetWriteWildcards() {
	wxArrayString formats;
	formats.Add(_T("encore.txt"));
	return formats;
}


///////////////////
// Can write file?
bool EncoreSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(11).Lower() == _T(".encore.txt"));
}


//////////////
// Write file
void EncoreSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
	// Get FPS
	double fps = AskForFPS(true);
	if (fps <= 0.0) return;

	// Open file
	TextFileWriter file(_filename,encoding);

	// Convert to encore
	CreateCopy();
	SortLines();
	StripComments();
	RecombineOverlaps();
	MergeIdentical();
	ConvertTags(1,_T("\r\n"));

	// Write lines
	using std::list;
	int i = 0;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current && !current->Comment) {
			// Time stamps
			wxString timeStamps = wxString::Format(_T("%i "),++i) + current->Start.GetSMPTE(fps) + _T(" ") + current->End.GetSMPTE(fps);

			// Convert : to ; if it's NTSC
			if (fps > 26.0) timeStamps.Replace(_T(":"),_T(";"));

			// Write
			file.WriteLineToFile(timeStamps + current->Text);
		}
	}

	// Clean up
	ClearCopy();
}
