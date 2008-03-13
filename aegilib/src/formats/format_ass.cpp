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
// AEGISUB/GORGONSUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "section.h"
#include "model.h"
#include "format_ass.h"
#include "version.h"
#include "../text_file_reader.h"
#include "../text_file_writer.h"
#include <iostream>
#include <wx/tokenzr.h>
using namespace Gorgonsub;


///////
// SSA
StringArray FormatSSA::GetReadExtensions() const
{
	StringArray final;
	final.push_back(L".ssa");
	return final;
}
StringArray FormatSSA::GetWriteExtensions() const
{
	return GetReadExtensions();
}


///////
// ASS
StringArray FormatASS::GetReadExtensions() const
{
	StringArray final;
	final.push_back(L".ass");
	return final;
}
StringArray FormatASS::GetWriteExtensions() const
{
	return GetReadExtensions();
}


////////
// ASS2
StringArray FormatASS2::GetReadExtensions() const
{
	StringArray final;
	final.push_back(L".ass");
	return final;
}
StringArray FormatASS2::GetWriteExtensions() const
{
	return GetReadExtensions();
}


///////////////
// Constructor
FormatHandlerASS::FormatHandlerASS(Model &_model,int version)
: model(_model), formatVersion(version)
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

	// Variables
	int version = 1;
	wxString curGroup = L"-";
	wxString prevGroup = L"-";
	SectionPtr section = SectionPtr();

	// Read file
	while (reader.HasMoreLines()) {
		// Read a line
		wxString cur = reader.ReadLineFromFile();
		if (cur.IsEmpty()) continue;

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
		SectionEntryPtr entry = MakeEntry(cur,section,version);
		if (entry) section->AddEntry(entry);
	}
}


/////////////////////
// Save file to disc
void FormatHandlerASS::Save(wxOutputStream &file,const String encoding)
{
	// Make text file writer
	TextFileWriter writer(file,encoding);

	// Set up list of sections to write
	wxArrayString sections;
	sections.Add(L"Script Info");
	sections.Add(L"V4+ Styles");
	sections.Add(L"Events");
	sections.Add(L"Fonts");
	sections.Add(L"Graphics");

	// Look for remaining sections
	size_t totalSections = model.GetSectionCount();
	for (size_t i=0;i<totalSections;i++) {
		String name = model.GetSectionByIndex(i)->GetName();
		if (sections.Index(name,false,false) == wxNOT_FOUND) sections.Add(name);
	}

	// Write sections
	size_t len = sections.size();
	for (size_t i=0;i<len;i++) {
		// See if it exists
		SectionPtr section = model.GetSection(sections[i]);
		if (section) {
			// Add a spacer
			if (i != 0) writer.WriteLineToFile(_T(""));

			// Write the section
			WriteSection(writer,section);
		}
	}
}


///////////////
// Create line
SectionEntryPtr FormatHandlerASS::MakeEntry(const String &data,SectionPtr section,int version)
{
	// Variables
	const String group = section->GetName();
	SectionEntryPtr final;

	// Attachments
	if (group == _T("Fonts") || group == _T("Graphics")) {
		final = shared_ptr<PlainASS>(new PlainASS(data));
	}

	// Events
	else if (group == _T("Events")) {
		// Dialogue lines
		if ((data.Left(9) == _T("Dialogue:") || data.Left(8) == _T("Comment:"))) {
			shared_ptr<DialogueASS> diag (new DialogueASS(data,version));
			final = diag;
		}

		// Format lines
		else if (data.Left(7) == _T("Format:")) {
			section->SetProperty(_T("Format"),data.Mid(7).Trim(true).Trim(false));
		}

		// Garbage/hard comments
		else {
			final = shared_ptr<PlainASS>(new PlainASS(data));
		}
	}

	// Styles
	else if (group == _T("V4+ Styles")) {
		if (data.Left(6) == _T("Style:")) {
			shared_ptr<StyleASS> style (new StyleASS(data,version));
			final = style;
		}
		if (data.Left(7) == _T("Format:")) {
			section->SetProperty(_T("Format"),data.Mid(7).Trim(true).Trim(false));
		}
	}

	// Script info
	else if (group == _T("Script Info")) {
		// Discard comments
		if (data.Left(1) == _T(";")) return SectionEntryPtr();

		// Parse property
		size_t pos = data.Find(_T(':'));
		if (pos == wxNOT_FOUND) return SectionEntryPtr();
		wxString key = data.Left(pos).Trim(true).Trim(false);
		wxString value = data.Mid(pos+1).Trim(true).Trim(false);

		// Insert property
		section->SetProperty(key,value);
		return SectionEntryPtr();
	}

	// Unknown group, just leave it intact
	else {
		final = shared_ptr<PlainASS>(new PlainASS(data));
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


///////////////////////////////
// Write a section to the file
void FormatHandlerASS::WriteSection(TextFileWriter &writer,SectionPtr section)
{
	// Write name
	wxString name = section->GetName();
	writer.WriteLineToFile(_T("[") + name + _T("]"));

	// Write program and library credits
	if (name == _T("Script Info")) {
		wxString programName = GetHostApplicationName();
		wxString programURL = GetHostApplicationURL();
		wxString libVersion = GetLibraryVersionString();
		wxString libURL = GetLibraryURL();
		writer.WriteLineToFile(_T("; Script generated by ") + programName);
		if (!programURL.IsEmpty()) writer.WriteLineToFile(_T("; ") + programURL);
		writer.WriteLineToFile(_T("; With ") + libVersion);
		if (programURL != libURL) writer.WriteLineToFile(_T("; ") + libURL);
	}

	// Write properties
	size_t props = section->GetPropertyCount();
	for (size_t i=0;i<props;i++) {
		String propName = section->GetPropertyName(i);
		writer.WriteLineToFile(propName + _T(": ") + section->GetProperty(propName));
	}

	// Write contents
	size_t entries = section->GetEntryCount();
	for (size_t i=0;i<entries;i++) {
		SectionEntryConstPtr entry = section->GetEntry(i);
		shared_ptr<const SerializeText> serial = dynamic_pointer_cast<const SerializeText>(entry);
		writer.WriteLineToFile(serial->ToText(formatVersion));
	}
}
