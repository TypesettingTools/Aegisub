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

/// @file subtitle_format_ass.cpp
/// @brief Reading/writing of SSA-lineage subtitles
/// @ingroup subtitle_io
///


///////////
// Headers
#include "config.h"

#include "ass_dialogue.h"
#include "subtitle_format_ass.h"
#include "text_file_reader.h"
#include "text_file_writer.h"


/// @brief Can read? 
/// @param filename 
/// @return 
///
bool ASSSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".ass") || filename.Right(4).Lower() == _T(".ssa"));
}



/// @brief Get name 
/// @return 
///
wxString ASSSubtitleFormat::GetName() {
	return _T("Advanced Substation Alpha");
}



/// @brief Get read wildcards 
/// @return 
///
wxArrayString ASSSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("ass"));
	formats.Add(_T("ssa"));
	return formats;
}



/// @brief Get write wildcards 
/// @return 
///
wxArrayString ASSSubtitleFormat::GetWriteWildcards() {
	wxArrayString formats;
	formats.Add(_T("ass"));
	formats.Add(_T("ssa"));
	return formats;
}



/// @brief Read file 
/// @param filename 
/// @param encoding 
///
void ASSSubtitleFormat::ReadFile(wxString filename,wxString encoding) {
	using namespace std;

	// Reader
	TextFileReader file(filename,encoding);
	int version = 1;
	if (filename.Right(4).Lower() == _T(".ssa")) version = 0;

	// Parse file
	wxString curgroup;
	int lasttime = -1;
	wxString wxbuffer;
	while (file.HasMoreLines()) {
		// Reads line
		wxbuffer = file.ReadLineFromFile();

		// Make sure that the first non-blank non-comment non-group-header line
		// is really [Script Info]
		if (curgroup.IsEmpty() && !wxbuffer.IsEmpty() && wxbuffer[0] != _T(';') && wxbuffer[0] != _T('[')) {
			curgroup = _T("[Script Info]");
			lasttime = AddLine(curgroup,curgroup,lasttime,version,&curgroup);
		}

		// Convert v4 styles to v4+ styles
		if (!wxbuffer.IsEmpty() && wxbuffer[0] == _T('[')) {
			// Ugly hacks to allow intermixed v4 and v4+ style sections
			wxString low = wxbuffer.Lower();
			if (low == _T("[v4 styles]")) {
				wxbuffer = _T("[V4+ Styles]");
				curgroup = wxbuffer;
				version = 0;
			}
			else if (low == _T("[v4+ styles]")) {
				wxbuffer = _T("[V4+ Styles]");
				curgroup = wxbuffer;
				version = 1;
			}
			else if (low == _T("[v4++ styles]")) {
				wxbuffer = _T("[V4+ Styles]");
				curgroup = wxbuffer;
				version = 2;
			}
			// Not-so-special case for other groups, just set it
			else {
				curgroup = wxbuffer;
				// default from extension in all other sections
				//version = 1;
				//if (filename.Right(4).Lower() == _T(".ssa")) version = 0;
			}
		}

		// Add line
		try {
			lasttime = AddLine(wxbuffer,curgroup,lasttime,version,&curgroup);
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
	if (!wxbuffer.IsEmpty()) AddLine(_T(""),curgroup,lasttime,version);
}



/// @brief Can write to file? 
/// @param filename 
/// @return 
///
bool ASSSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".ass") || filename.Right(4).Lower() == _T(".ssa"));
}



/// @brief Write file 
/// @param _filename 
/// @param encoding  
///
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


