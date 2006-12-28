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
#include "subtitle_format_ass.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "ass_dialogue.h"


/////////////
// Can read?
bool ASSSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".ass") || filename.Right(4).Lower() == _T(".ssa"));
}


////////////
// Get name
wxString ASSSubtitleFormat::GetName() {
	return _T("Advanced Substation Alpha");
}


//////////////////////
// Get read wildcards
wxArrayString ASSSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("ass"));
	formats.Add(_T("ssa"));
	return formats;
}


///////////////////////
// Get write wildcards
wxArrayString ASSSubtitleFormat::GetWriteWildcards() {
	wxArrayString formats;
	formats.Add(_T("ass"));
	formats.Add(_T("ssa"));
	return formats;
}


/////////////
// Read file
void ASSSubtitleFormat::ReadFile(wxString filename,wxString encoding) {
	using namespace std;

	// Reader
	TextFileReader file(filename,encoding);
	bool IsSSA = filename.Right(4).Lower() == _T(".ssa");

	// Parse file
	wxString curgroup;
	int lasttime = -1;
	wxString wxbuffer;
	while (file.HasMoreLines()) {
		// Reads line
		wxbuffer = file.ReadLineFromFile();

		// Convert v4 styles to v4+ styles
		// Ugly hacks to allow intermixed v4 and v4+ style sections
		if (wxbuffer.Lower() == _T("[v4 styles]")) {
			wxbuffer = _T("[V4+ Styles]");
			curgroup = wxbuffer;
			IsSSA = true;
		}
		else if (wxbuffer.Lower() == _T("[v4+ styles]")) {
			curgroup = wxbuffer;
			IsSSA = false;
		}
		// Not-so-special case for other groups, just set it
		else if (wxbuffer[0] == _T('[')) {
			curgroup = wxbuffer;
			// default from extension in all other sections
			IsSSA = filename.Right(4).Lower() == _T(".ssa");
		}

		// Add line
		try {
			lasttime = AddLine(wxbuffer,curgroup,lasttime,IsSSA,&curgroup);
		}
		catch (const wchar_t *err) {
			Clear();
			throw wxString(_T("Error processing line: ")) + wxbuffer + _T(": ") + wxString(err);
		}
		catch (...) {
			Clear();
			throw wxString(_T("Error processing line: ")) + wxbuffer;
		}
	}

	// Add one last empty line in case it didn't end with one
	if (!wxbuffer.IsEmpty()) AddLine(_T(""),curgroup,lasttime,IsSSA);
}


//////////////////////
// Can write to file?
bool ASSSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".ass") || filename.Right(4).Lower() == _T(".ssa"));
}


//////////////
// Write file
void ASSSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
	// Open file
	TextFileWriter file(_filename,encoding);
	bool ssa = _filename.Right(4).Lower() == _T(".ssa");

	// Write lines
	using std::list;
	AssEntry *entry;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();) {
		// Get entry
		entry = *cur;

		// Only add a line break if there is a next line
		cur++;
		bool lineBreak = cur != Line->end();

		// Write line
		if (ssa) file.WriteLineToFile(entry->GetSSAText(),lineBreak);
		else file.WriteLineToFile(entry->GetEntryData(),lineBreak);
	}
}
