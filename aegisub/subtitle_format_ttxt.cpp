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
#include "subtitle_format_ttxt.h"
#include "ass_dialogue.h"
#include "ass_time.h"
#include <wx/xml/xml.h>


///////////////////
// Get format name
wxString TTXTSubtitleFormat::GetName() {
	return _T("MPEG-4 Streaming Text");
}


//////////////////////
// Get read wildcards
wxArrayString TTXTSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("ttxt"));
	return formats;
}


///////////////////////
// Get write wildcards
wxArrayString TTXTSubtitleFormat::GetWriteWildcards() {
	//return GetReadWildcards();
	return wxArrayString();
}


////////////////////
// Can read a file?
bool TTXTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(5).Lower() == _T(".ttxt"));
}


/////////////////////
// Can write a file?
bool TTXTSubtitleFormat::CanWriteFile(wxString filename) {
	return false;
	//return (filename.Right(5).Lower() == _T(".ttxt"));
}


///////////////
// Read a file
void TTXTSubtitleFormat::ReadFile(wxString filename,wxString forceEncoding) {
	// Load default
	LoadDefault(false);

	// Load XML document
	wxXmlDocument doc;
	if (!doc.Load(filename)) throw _T("Failed loading TTXT XML file.");

	// Check root node name
	if (doc.GetRoot()->GetName() != _T("TextStream")) throw _T("Invalid TTXT file.");

	// Get children
	AssDialogue *diag = NULL;
	wxXmlNode *child = doc.GetRoot()->GetChildren();
	int lines = 0;
	while (child) {
		// Line
		if (child->GetName() == _T("TextSample")) {
			// Get properties
			wxString sampleTime = child->GetPropVal(_T("sampleTime"),_T("00:00:00.000"));
			wxString text = child->GetPropVal(_T("text"),_T(""));

			// Parse time
			AssTime time;
			time.ParseASS(sampleTime);

			// Set end time of last line
			if (diag) diag->End = time;
			diag = NULL;

			// Create line
			if (!text.IsEmpty()) {
				// Process text
				wxString finalText;
				finalText.Alloc(text.Length());
				bool in = false;
				bool first = true;
				for (size_t i=0;i<text.Length();i++) {
					if (text[i] == _T('\'')) {
						if (!in && !first) finalText += _T("\\N");
						first = false;
						in = !in;
					}
					else if (in) finalText += text[i];
				}

				// Create dialogue
				diag = new AssDialogue();
				diag->Start = time;
				diag->End.SetMS(time.GetMS()+5000);
				diag->Text = finalText;
				diag->group = _T("[Events]");
				diag->Style = _T("Default");
				diag->Comment = false;
				diag->UpdateData();
				diag->StartMS = diag->Start.GetMS();
				Line->push_back(diag);
				lines++;
			}
		}

		// Header
		else if (child->GetName() == _T("TextStreamHeader")) {
			// TODO
		}

		// Proceed to next child
		child = child->GetNext();
	}

	// No lines?
	if (lines == 0) {
		AssDialogue *line = new AssDialogue();
		line->group = _T("[Events]");
		line->Style = _T("Default");
		line->StartMS = 0;
		line->Start.SetMS(0);
		line->End.SetMS(5000);
		Line->push_back(line);
	}
}


////////////////
// Write a file
void TTXTSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
}
