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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "subtitle_format_txt.h"
#include "text_file_reader.h"
#include "ass_dialogue.h"
#include "options.h"


/////////////
// Can read?
bool TXTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".txt"));
}


/////////////
// Read file
void TXTSubtitleFormat::ReadFile(wxString filename,wxString encoding) {	using namespace std;

	// Reader
	TextFileReader file(filename,encoding,false);

	// Default
	LoadDefault();
	SetIsASS(false);

	// Data
	wxString actor;
	wxString separator = Options.AsText(_T("Text actor separator"));
	wxString comment = Options.AsText(_T("Text comment starter"));
	bool isComment = false;

	// Parse file
	AssDialogue *line = NULL;
	while (file.HasMoreLines()) {
		// Reads line
		wxString value = file.ReadLineFromFile();

		// Check if this isn't a timecodes file
		if (value.Left(10) == _T("# timecode")) {
			throw _T("File is a timecode file, cannot load as subtitles.");
		}

		// Read comment data
		isComment = false;
		if (comment != _T("") && value.Left(comment.Length()) == comment) {
			isComment = true;
			value = value.Mid(comment.Length());
		}

		// Read actor data
		if (!isComment && separator != _T("")) {
			if (value[0] != _T(' ') && value[0] != _T('\t')) {
				size_t pos = value.Find(separator);
				if (pos != -1) {
					actor = value.Left(pos);
					actor.Trim(false);
					actor.Trim(true);
					value = value.Mid(pos+1);
					value.Trim(false);
				}
			}
		}

		// Trim spaces at start
		value.Trim(false);

		// Sets line up
		line = new AssDialogue();
		line->group = _T("[Events]");
		line->Style = _T("Default");
		if (isComment) line->Actor = _T("");
		else line->Actor = actor;
		if (value.IsEmpty()) {
			line->Actor = _T("");
			isComment = true;
		}
		line->Comment = isComment;
		line->Text = value;
		line->StartMS = 0;
		line->Start.SetMS(0);
		line->End.SetMS(0);
		line->UpdateData();
		//line->ParseASSTags();

		// Adds line
		Line->push_back(line);
	}
}
