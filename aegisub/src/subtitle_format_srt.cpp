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

/// @file subtitle_format_srt.cpp
/// @brief Reading/writing SubRip format subtitles (.SRT)
/// @ingroup subtitle_io
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/regex.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "subtitle_format_srt.h"
#include "text_file_reader.h"
#include "text_file_writer.h"


DEFINE_SIMPLE_EXCEPTION(SRTParseError, SubtitleFormatParseError, "subtitle_io/parse/srt")


/// @brief Can read? 
/// @param filename 
/// @return 
///
bool SRTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".srt"));
}



/// @brief Can write? 
/// @param filename 
/// @return 
///
bool SRTSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".srt"));
}



/// @brief Get name 
/// @return 
///
wxString SRTSubtitleFormat::GetName() {
	return _T("SubRip");
}



/// @brief Get read wildcards 
/// @return 
///
wxArrayString SRTSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("srt"));
	return formats;
}



/// @brief Get write wildcards 
/// @return 
///
wxArrayString SRTSubtitleFormat::GetWriteWildcards() {
	return GetReadWildcards();
}



/// @brief Read file 
/// @param filename 
/// @param encoding 
///
void SRTSubtitleFormat::ReadFile(wxString filename,wxString encoding) {
	using namespace std;

	// Reader
	TextFileReader file(filename,encoding);

	// Default
	LoadDefault(false);

	// See parsing algorithm at <http://devel.aegisub.org/wiki/SubtitleFormats/SRT>

	// "hh:mm:ss,fff --> hh:mm:ss,fff" (e.g. "00:00:04,070 --> 00:00:10,04")
	/// @todo: move the full parsing of SRT timestamps here, instead of having it in AssTime
	wxRegEx timestamp_regex(L"^([0-9]{2}:[0-9]{2}:[0-9]{2},[0-9]{3}) --> ([0-9]{2}:[0-9]{2}:[0-9]{2},[0-9]{3})");
	if (!timestamp_regex.IsValid())
		throw agi::InternalError("Parsing SRT: Failed compiling regex", 0);

	int state = 1;
	int line_num = 0;
	int linebreak_debt = 0;
	AssDialogue *line = 0;
	while (file.HasMoreLines()) {
		wxString text_line = file.ReadLineFromFile();
		line_num++;
		text_line.Trim(true).Trim(false);

		switch (state) {
			case 1:
			{
				// start of file, no subtitles found yet
				if (text_line.IsEmpty())
					// ignore blank lines
					break;
				if (text_line.IsNumber()) {
					// found the line number, throw it away and hope for timestamps
					state = 2;
					break;
				}
				if (timestamp_regex.Matches(text_line)) {
					goto found_timestamps;
				}
				char cvtbuf[16]; sprintf(cvtbuf, "%d", line_num);
				throw SRTParseError(std::string("Parsing SRT: Expected subtitle index at line ") + cvtbuf, 0);
			}
			case 2:
			{
				// want timestamps
				if (timestamp_regex.Matches(text_line) == false) {
					// bad format
					char cvtbuf[16]; sprintf(cvtbuf, "%d", line_num);
					throw SRTParseError(std::string("Parsing SRT: Expected timestamp pair at line ") + cvtbuf, 0);
				}
found_timestamps:
				if (line != 0) {
					// finalise active line
					line->ParseSRTTags();
					line = 0;
				}
				// create new subtitle
				line = new AssDialogue();
				line->group = L"[Events]";
				line->Style = _T("Default");
				line->Comment = false;
				// this parsing should best be moved out of AssTime
				line->Start.ParseSRT(timestamp_regex.GetMatch(text_line, 1));
				line->End.ParseSRT(timestamp_regex.GetMatch(text_line, 2));
				// store pointer to subtitle, we'll continue working on it
				Line->push_back(line);
				// next we're reading the text
				state = 3;
				break;
			}
			case 3:
			{
				// reading first line of subtitle text
				if (text_line.IsEmpty()) {
					// that's not very interesting... blank subtitle?
					state = 5;
					linebreak_debt = 1;
					break;
				}
				line->Text.Append(text_line);
				state = 4;
				break;
			}
			case 4:
			{
				// reading following line of subtitle text
				if (text_line.IsEmpty()) {
					// blank line, next may begin a new subtitle
					state = 5;
					linebreak_debt = 1;
					break;
				}
				line->Text.Append(L"\\N").Append(text_line);
				break;
			}
			case 5:
			{
				// blank line in subtitle text
				linebreak_debt++;
				if (text_line.IsEmpty()) {
					// multiple blank lines in a row, just add a line break...
					break;
				}
				if (text_line.IsNumber()) {
					// must be a subtitle index!
					// go for timestamps next
					state = 2;
					break;
				}
				if (timestamp_regex.Matches(text_line)) {
					goto found_timestamps;
				}
				// assume it's a continuation of the subtitle text
				// resolve our line break debt and append the line text
				while (linebreak_debt-- > 0)
					line->Text.Append(L"\\N");
				line->Text.Append(text_line);
				state = 4;
				break;
			}
			default:
			{
				char cvtbuf[16]; sprintf(cvtbuf, "%d", state);
				throw agi::InternalError(std::string("Parsing SRT: Reached unexpected state ") + cvtbuf, 0);
			}
		}
	}

	if (state == 1 || state == 2) {
		throw SRTParseError(std::string("Parsing SRT: Incomplete file"), 0);
	}

	if (line) {
		// an unfinalised line
		line->ParseSRTTags();
	}
}



/// @brief Write file 
/// @param _filename 
/// @param encoding  
///
void SRTSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
	// Open file
	TextFileWriter file(_filename,encoding);

	// Convert to SRT
	CreateCopy();
	SortLines();
	StripComments();
	// Tags must be converted in two passes
	// First ASS style overrides are converted to SRT but linebreaks are kept
	ConvertTags(2,_T("\\N"));
	// Then we can recombine overlaps, this requires ASS style linebreaks
	RecombineOverlaps();
	MergeIdentical();
	// And finally convert linebreaks
	ConvertTags(0,_T("\r\n"));
	// Otherwise unclosed overrides might affect lines they shouldn't, see bug #809 for example

	// Write lines
	int i=1;
	using std::list;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = dynamic_cast<AssDialogue*>(*cur);
		if (current && !current->Comment) {
			// Write line
			file.WriteLineToFile(wxString::Format(_T("%i"),i));
			file.WriteLineToFile(current->Start.GetSRTFormated() + _T(" --> ") + current->End.GetSRTFormated());
			file.WriteLineToFile(current->Text);
			file.WriteLineToFile(_T(""));

			i++;
		}
	}

	// Clean up
	ClearCopy();
}


