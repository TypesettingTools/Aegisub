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

/// @file subtitle_format_mkv.cpp
/// @brief Importing subtitles (plain, SSA, ASS) from Matroska files
/// @ingroup subtitle_io matroska
///


///////////
// Headers
#include "config.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "mkv_wrap.h"
#include "subtitle_format_mkv.h"


/// @brief Can read? 
/// @param filename 
/// @return 
///
bool MKVSubtitleFormat::CanReadFile(wxString filename) {
	if (filename.Right(4).Lower() == _T(".mkv") || filename.Right(4).Lower() == _T(".mks")
			|| filename.Right(4).Lower() == _T(".mka"))
		return true;
	else
		return false;
}



/// @brief Get name 
/// @return 
///
wxString MKVSubtitleFormat::GetName() {
	return _T("Matroska");
}



/// @brief Get read wildcards 
/// @return 
///
wxArrayString MKVSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("mkv"));
	formats.Add(_T("mka"));
	formats.Add(_T("mks"));
	return formats;
}



/// @brief Read file 
/// @param filename 
/// @param encoding 
///
void MKVSubtitleFormat::ReadFile(wxString filename,wxString encoding) {
	// Open matroska
	MatroskaWrapper wrap;
	wrap.Open(filename,false);

	// Read subtitles in a temporary object
	wrap.GetSubtitles(GetAssFile());

	// Close matroska
	wrap.Close();
}



/// @brief Can write to file? 
/// @param filename 
/// @return 
///
bool MKVSubtitleFormat::CanWriteFile(wxString filename) {
	return false;
}



/// @brief Write file 
/// @param _filename 
/// @param encoding  
///
void MKVSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
}


