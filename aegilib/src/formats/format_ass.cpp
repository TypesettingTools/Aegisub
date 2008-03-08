// Copyright (c) 2008, Rodrigo Braz Monteiro
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
// AEGISUB/AEGILIB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "model.h"
#include "format_ass.h"
#include "../text_file_reader.h"
#include <iostream>
#include <wx/tokenzr.h>
using namespace Aegilib;


//////////////
// Extensions
StringArray FormatASS::GetReadExtensions() const
{
	StringArray final;
	final.push_back(L".ass");
	final.push_back(L".ssa");
	return final;
}
StringArray FormatASS::GetWriteExtensions() const
{
	return GetReadExtensions();
}


///////////////
// Constructor
FormatHandlerASS::FormatHandlerASS(Model &_model)
: model(_model)
{
}


//////////////
// Destructor
FormatHandlerASS::~FormatHandlerASS()
{
}


///////////////
// Load a file
void FormatHandlerASS::Load(wxInputStream &file,const String encoding)
{
	// Make text file reader
	TextFileReader reader(file,encoding);

	// Debug
	using namespace std;
	cout << endl << "Dumping file:" << endl;

	// Variables
	int version = 1;
	wxString curGroup = L"-";
	wxString prevGroup = L"-";
	Section *section = NULL;

	// Read file
	while (reader.HasMoreLines()) {
		// Read a line
		wxString cur = reader.ReadLineFromFile();

		// Process group
		prevGroup = curGroup;
		ProcessGroup(cur,curGroup,version);

		// Insert group if it doesn't already exist
		if (prevGroup != curGroup) section = model.GetSection(curGroup);
		if (!section) {
			model.AddSection(curGroup);
			section = model.GetSection(curGroup);
		}

		// Skip [] lines
		if (cur[0] == L'[') continue;

		// Create and insert line
		SectionEntry *entry = MakeEntry(cur,curGroup,version);
		//if (!entry) throw Exception(Exception::Parse_Error);
		section->AddEntry(entry);
	}

	// Debug
	cout << "\nFinished reading file with version=" << version << ".\n\n";
}


///////////////
// Create line
SectionEntry *FormatHandlerASS::MakeEntry(String data,String group,int version)
{
	// Variables
	SectionEntry *final = NULL;

	// Attachments
	if (group == _T("Fonts") || group == _T("Graphics")) {
		// TODO
	}

	// Events
	else if (group == _T("Events")) {
		// Dialogue lines
		if ((data.Left(9) == _T("Dialogue:") || data.Left(8) == _T("Comment:"))) {
			DialogueASS *diag = new DialogueASS(data,version);
			final = diag;

			// Debug
			wxString out = diag->GetStartTime().GetString(2,1) + _T(",") + diag->GetEndTime().GetString(2,1) + _T(",") + diag->GetText();
			std::cout << out.mb_str(wxConvUTF8) << std::endl;
		}

		// Format lines
		else if (data.Left(7) == _T("Format:")) {
			// TODO
			//entry = new AssEntry(_T("Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text"));
		}

		// Garbage
		else {
			// TODO
		}
	}

	// Styles
	else if (group == _T("V4+ Styles")) {
		// TODO
	}

	// Script info
	else if (group == _T("Script Info")) {
		// TODO
	}

	// Return entry
	return final;
}


//////////////////////
// Process group data
void FormatHandlerASS::ProcessGroup(String cur,String &curGroup,int &version) {
	// Style conversion
	if (!cur.IsEmpty() && cur[0] == '[') {
		wxString low = cur.Lower();
		bool changed = true;

		// SSA file
		if (low == _T("[v4 styles]")) {
			cur = _T("[V4+ Styles]");
			curGroup = cur;
			version = 0;
		}

		// ASS file
		else if (low == _T("[v4+ styles]")) {
			curGroup = cur;
			version = 1;
		}

		// ASS2 file
		else if (low == _T("[v4++ styles]")) {
			cur = _T("[V4+ Styles]");
			curGroup = cur;
			version = 2;
		}

		// Other groups
		else {
			wxString temp = cur;
			temp.Trim(true).Trim(false);
			if (temp[temp.Length()-1] == ']') curGroup = cur;
			else changed = false;
		}

		// Normalize group name
		if (changed) {
			// Get rid of []
			curGroup = curGroup.Mid(1,curGroup.Length()-2);
			
			// Normalize case
			curGroup.MakeLower();
			wxString upper = curGroup.Upper();
			bool raise = true;
			size_t len = curGroup.Length();
			for (size_t i=0;i<len;i++) {
				if (raise) {
					curGroup[i] = upper[i];
					raise = false;
				}
				if (curGroup[i] == L' ') raise = true;
			}
		}
	}

	// Update version with version line
	if (curGroup == _T("Script Info")) {
		if (cur.Left(11).Lower() == _T("scripttype:")) {
			wxString versionString = cur.Mid(11);
			versionString.Trim(true);
			versionString.Trim(false);
			versionString.MakeLower();
			int trueVersion;
			if (versionString == _T("v4.00")) trueVersion = 0;
			else if (versionString == _T("v4.00+")) trueVersion = 1;
			else if (versionString == _T("v4.00++")) trueVersion = 2;
			else throw Exception(Exception::Unknown_Format);
			if (trueVersion != version) {
				// TODO: issue warning?
				version = trueVersion;
			}
		}
	}
}


////////////////////////////
// ASS dialogue constructor
DialogueASS::DialogueASS(String data,int version)
{
	// Try parsing with all different versions
	bool valid = false;
	for (int count=0;!valid && count < 3;count++) {
		valid = Parse(data,version);
		version++;
		if (version > 2) version = 0;
	}
}


//////////////////
// Parse ASS Data
bool DialogueASS::Parse(wxString rawData, int version)
{
	size_t pos = 0;
	wxString temp;

	// Get type
	if (rawData.StartsWith(_T("Dialogue:"))) {
		comment = false;
		pos = 10;
	}
	else if (rawData.StartsWith(_T("Comment:"))) {
		comment = true;
		pos = 9;
	}
	else return false;

	wxStringTokenizer tkn(rawData.Mid(pos),_T(","),wxTOKEN_RET_EMPTY_ALL);
	if (!tkn.HasMoreTokens()) return false;

	// Get first token and see if it has "Marked=" in it
	temp = tkn.GetNextToken().Trim(false).Trim(true);
	if (temp.Lower().StartsWith(_T("marked="))) version = 0;
	else if (version == 0) version = 1;

	// Get layer number
	if (version == 0) layer = 0;
	else {
		long templ;
		temp.ToLong(&templ);
		layer = templ;
	}

	// Get start time
	if (!tkn.HasMoreTokens()) return false;
	start.Parse(tkn.GetNextToken());

	// Get end time
	if (!tkn.HasMoreTokens()) return false;
	end.Parse(tkn.GetNextToken());

	// Get style
	if (!tkn.HasMoreTokens()) return false;
	style = tkn.GetNextToken();
	style.Trim(true);
	style.Trim(false);

	// Get actor
	if (!tkn.HasMoreTokens()) return false;
	actor = tkn.GetNextToken();
	actor.Trim(true);
	actor.Trim(false);

	// Get margins
	for (int i=0;i<3;i++) {
		if (!tkn.HasMoreTokens()) return false;
		long templ;
		tkn.GetNextToken().Trim(false).Trim(true).ToLong(&templ);
		margin[i] = templ;
	}

	// Get bottom margin
	if (version == 1) margin[3] = margin[2];
	bool rollBack = false;
	if (version == 2) {
		if (!tkn.HasMoreTokens()) return false;
		wxString oldTemp = temp;
		temp = tkn.GetNextToken().Trim(false).Trim(true);
		if (!temp.IsNumber()) {
			version = 1;
			rollBack = true;
		}
		else {
			long templ;
			temp.ToLong(&templ);
			margin[3] = templ;
		}
	}

	// Get effect
	if (!rollBack) {
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
	}
	effect = temp;
	effect.Trim(true);
	effect.Trim(false);

	// Get text
	text = rawData.Mid(pos+tkn.GetPosition());

	return true;
}
