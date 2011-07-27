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

/// @file subtitle_format_txt.cpp
/// @brief Importing/exporting subtitles to untimed plain text
/// @ingroup subtitle_io
///


///////////
// Headers
#include "config.h"

#include "ass_dialogue.h"
#include "compat.h"
#include "dialog_text_import.h"
#include "main.h"
#include "subtitle_format_txt.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "version.h"


/// @brief Can read? 
/// @param filename 
/// @return 
///
bool TXTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".txt"));
}



/// @brief Can write? 
/// @param filename 
/// @return 
///
bool TXTSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".txt") && filename.Right(11).Lower() != _T(".encore.txt") && filename.Right(16).Lower() != _T(".transtation.txt"));
}



/// @brief Get name 
/// @return 
///
wxString TXTSubtitleFormat::GetName() {
	return _T("Plain-Text");
}



/// @brief Get read wildcards 
/// @return 
///
wxArrayString TXTSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("txt"));
	return formats;
}



/// @brief Get write wildcards 
/// @return 
///
wxArrayString TXTSubtitleFormat::GetWriteWildcards() {
	return GetReadWildcards();
}



/// @brief Read file 
/// @param filename 
/// @param encoding 
/// @return 
///
void TXTSubtitleFormat::ReadFile(wxString filename,wxString encoding) {	using namespace std;
	// Import options
	DialogTextImport dlg;
	if (dlg.ShowModal() == wxID_CANCEL) return;

	// Reader
	TextFileReader file(filename,encoding,false);

	// Default
	LoadDefault(false);

	// Data
	wxString actor;
	wxString separator = lagi_wxString(OPT_GET("Tool/Import/Text/Actor Separator")->GetString());
	wxString comment = lagi_wxString(OPT_GET("Tool/Import/Text/Comment Starter")->GetString());
	bool isComment = false;
	int lines = 0;

	// Parse file
	AssDialogue *line = NULL;
	while (file.HasMoreLines()) {
		// Reads line
		wxString value = file.ReadLineFromFile();
		if(value.IsEmpty()) continue;

		// Check if this isn't a timecodes file
		if (value.StartsWith(_T("# timecode"))) {
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
				int pos = value.Find(separator);
				if (pos != wxNOT_FOUND) {
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
		line = new AssDialogue;
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
		line->Start.SetMS(0);
		line->End.SetMS(0);

		// Adds line
		Line->push_back(line);
		lines++;
	}

	// No lines?
	if (lines == 0) {
		line = new AssDialogue;
		line->group = _T("[Events]");
		line->Style = _T("Default");
		line->Start.SetMS(0);
		line->End.SetMS(OPT_GET("Timing/Default Duration")->GetInt());
		Line->push_back(line);
	}
}



/// @brief Write file 
/// @param filename 
/// @param encoding 
///
void TXTSubtitleFormat::WriteFile(wxString filename,wxString encoding) {	using namespace std;
	size_t num_actor_names = 0, num_dialogue_lines = 0;

	// Detect number of lines with Actor field filled out
	for (list<AssEntry*>::iterator l = Line->begin(); l != Line->end(); ++l) {
		AssDialogue *dia = dynamic_cast<AssDialogue*>(*l);
		if (dia && !dia->Comment) {
			num_dialogue_lines++;
			if (!dia->Actor.IsEmpty())
				num_actor_names++;
		}
	}

	// If too few lines have Actor filled out, don't write it
	bool write_actors = num_actor_names > num_dialogue_lines/2;
	bool strip_formatting = true;

	TextFileWriter file(filename, encoding);
	file.WriteLineToFile(wxString("# Exported by Aegisub ") + GetAegisubShortVersionString());

	// Write the file
	for (list<AssEntry*>::iterator l = Line->begin(); l != Line->end(); ++l) {
		AssDialogue *dia = dynamic_cast<AssDialogue*>(*l);

		if (dia) {
			wxString out_line;

			if (dia->Comment) {
				out_line = _T("# ");
			}

			if (write_actors) {
				out_line += dia->Actor + _T(": ");
			}

			wxString out_text;
			if (strip_formatting) {
				dia->ParseASSTags();
				for (std::vector<AssDialogueBlock*>::iterator block = dia->Blocks.begin(); block != dia->Blocks.end(); ++block) {
					if ((*block)->GetType() == BLOCK_PLAIN) {
						out_text += (*block)->GetText();
					}
				}
				dia->ClearBlocks();
			}
			else {
				out_text = dia->Text;
			}
			out_line += out_text;

			if (!out_text.IsEmpty()) {
				file.WriteLineToFile(out_line);
			}
		}
		else {
			// Not a dialogue line
			// TODO: should any non-dia lines cause blank lines in output?
			//file.WriteLineToFile(_T(""));
		}
	}
}


