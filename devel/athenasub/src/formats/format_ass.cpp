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
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "section.h"
#include "model.h"
#include "format_ass.h"
#include "format_ass_plain.h"
#include "version.h"
#include "../text_reader.h"
#include "../reader.h"
#include "../text_writer.h"
#include "../writer.h"
#include <iostream>
#include <algorithm>
#include <wx/tokenzr.h>
using namespace Athenasub;


///////
// SSA
StringArray FormatSSA::GetReadExtensions() const
{
	StringArray final;
	final.push_back(".ssa");
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
	final.push_back(".ass");
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
	final.push_back(".ass");
	return final;
}
StringArray FormatASS2::GetWriteExtensions() const
{
	return GetReadExtensions();
}



//////////////////////////////////
// Check if it can read this file
float FormatASSFamily::CanReadFile(Reader &file) const
{
	// Check header
	if (!file->HasMoreLines()) return 0.0f;
	String line = file->ReadLineFromFile();
	line.AsciiMakeLower();
	if (line != "[script info]") return 0.0f;

	float version = 0.0f;
	float sections = 0.25f;
	String section = line;
	for (int i=0; i < 100 && file->HasMoreLines(); i++) {
		// Get line
		line = file->ReadLineFromFile();
		line.AsciiMakeLower();

		// Set section
		if (line[0] == '[' && line[line.Length()-1] == ']') section = line;

		// Check version
		if (section == "[script info]") {
			if (line.StartsWith("scripttype")) {
				int formatVersion = GetVersion();
				String expected = "";
				switch (formatVersion) {
					case 0: expected = "v4.00"; break;
					case 1: expected = "v4.00+"; break;
					case 2: expected = "v4.00++"; break;
				}
				if (line.EndsWith(expected)) version = 0.5f;
				else version = 0.25f;
			}
		}

		// Done when events begin
		if (section == "[events]") {
			sections += 0.25f;
			break;
		}
	}

	// OK
	return sections+version;
}


///////////////
// Constructor
FormatHandlerASS::FormatHandlerASS(int version)
: CFormatHandler(), formatVersion(version)
{
}


//////////////
// Destructor
FormatHandlerASS::~FormatHandlerASS()
{
}


///////////////
// Load a file
void FormatHandlerASS::Load(IModel &model,Reader reader)
{
	// Variables
	int version = 1;
	String curGroup = "-";
	String prevGroup = "-";
	Section section = Section();

	// Read file
	while (reader->HasMoreLines()) {
		// Read a line
		String cur = reader->ReadLineFromFile();
		if (cur.IsEmpty()) continue;

		// Process group
		prevGroup = curGroup;
		ProcessGroup(cur,curGroup,version);

		// Insert group if it doesn't already exist
		if (prevGroup != curGroup) section = GetSection(model,curGroup);
		if (!section) {
			AddSection(model,curGroup);
			section = GetSection(model,curGroup);
		}

		// Skip [] lines
		if (cur[0] == L'[') continue;

		// Create and insert line
		Entry entry = MakeEntry(cur,section,version);
		if (entry) section->AddEntry(entry);
	}

	// Ensure validity
	MakeValid(model);
}


/////////////////////
// Save file to disc
void FormatHandlerASS::Save(const IModel& model,Writer writer) const
{
	// Set up list of sections to write
	StringArray sections;
	sections.push_back("Script Info");
	sections.push_back("V4+ Styles");
	sections.push_back("Events");
	sections.push_back("Fonts");
	sections.push_back("Graphics");

	// Look for remaining sections
	size_t totalSections = GetSectionCount(model);
	for (size_t i=0;i<totalSections;i++) {
		String name = GetSectionByIndex(model,i)->GetName();
		// If not found on the list, add to it
		if (find(sections.begin(),sections.end(),name) == sections.end()) {
			sections.push_back(name);
		}
	}

	// Write sections
	size_t len = sections.size();
	for (size_t i=0;i<len;i++) {
		// See if it exists
		ConstSection section = GetSection(model,sections[i]);
		if (section) {
			// Add a spacer
			if (i != 0) writer->WriteLineToFile("");

			// Write the section
			WriteSection(writer,section);
		}
	}

	writer->Flush();
}


///////////////
// Create line
Entry FormatHandlerASS::MakeEntry(const String &data,Section section,int version)
{
	// Variables
	const String group = section->GetName();
	Entry final;

	// Attachments
	if (group == "Fonts" || group == "Graphics") {
		final = shared_ptr<PlainASS>(new PlainASS(data));
	}

	// Events
	else if (group == "Events") {
		// Dialogue lines
		if ((data.StartsWith("Dialogue:") || data.StartsWith("Comment:"))) {
			shared_ptr<DialogueASS> diag (new DialogueASS(data,version));
			final = diag;
		}

		// Format lines
		else if (data.StartsWith("Format:")) {
			section->SetProperty("Format",data.Mid(7).TrimBoth());
		}

		// Garbage/hard comments
		else {
			final = shared_ptr<PlainASS>(new PlainASS(data));
		}
	}

	// Styles
	else if (group == "V4+ Styles" || group == "V4 Styles+") {
		if (data.StartsWith("Style:")) {
			shared_ptr<StyleASS> style (new StyleASS(data,version));
			final = style;
		}
		if (data.StartsWith("Format:")) {
			section->SetProperty("Format",data.Mid(7).TrimBoth());
		}
	}

	// Script info
	else if (group == "Script Info") {
		// Discard comments
		if (data.StartsWith(";")) return Entry();

		// Parse property
		size_t pos = data.Find(':');
		if (pos == String::npos) return Entry();
		String key = data.Left(pos).TrimBoth();
		String value = data.Mid(pos+1).TrimBoth();

		// Insert property
		section->SetProperty(key,value);
		return Entry();
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
		String low = cur.AsciiLower();
		bool changed = true;

		// SSA file
		if (low == "[v4 styles]") {
			cur = "[V4+ Styles]";
			curGroup = cur;
			version = 0;
		}

		// ASS file
		else if (low == "[v4+ styles]") {
			curGroup = cur;
			version = 1;
		}

		// ASS2 file
		else if (low == "[v4++ styles]") {
			cur = "[V4+ Styles]";
			curGroup = cur;
			version = 2;
		}

		// Other groups
		else {
			String temp = cur;
			temp.TrimBoth();
			if (temp[temp.Length()-1] == ']') curGroup = cur;
			else changed = false;
		}

		// Normalize group name
		if (changed) {
			// Get rid of []
			curGroup = curGroup.Mid(1,curGroup.Length()-2);
			
			// Normalize case
			curGroup.AsciiMakeLower();
			String upper = curGroup.AsciiUpper();
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
	if (curGroup == "Script Info") {
		if (cur.StartsWith("scripttype:",false)) {
			String versionString = cur.Mid(11);
			versionString.TrimBoth();
			versionString.AsciiMakeLower();
			int trueVersion;
			if (versionString == "v4.00") trueVersion = 0;
			else if (versionString == "v4.00+") trueVersion = 1;
			else if (versionString == "v4.00++") trueVersion = 2;
			else THROW_ATHENA_EXCEPTION(Exception::Unknown_Format);
			if (trueVersion != version) {
				// TODO: issue warning?
				version = trueVersion;
			}
		}
	}
}


///////////////////////////////
// Write a section to the file
void FormatHandlerASS::WriteSection(Writer writer,ConstSection section) const
{
	// Write name
	String name = section->GetName();
	writer->WriteLineToFile("[" + name + "]");

	// Write program and library credits
	if (name == "Script Info") {
		String programName = GetHostApplicationName();
		String programURL = GetHostApplicationURL();
		String libVersion = GetLibraryVersionString();
		String libURL = GetLibraryURL();
		writer->WriteLineToFile("; Script generated by " + programName);
		if (!programURL.IsEmpty()) writer->WriteLineToFile("; " + programURL);
		writer->WriteLineToFile("; With " + libVersion);
		if (programURL != libURL) writer->WriteLineToFile("; " + libURL);
	}

	// Write properties
	size_t props = section->GetPropertyCount();
	for (size_t i=0;i<props;i++) {
		String propName = section->GetPropertyName(i);
		writer->WriteLineToFile(propName + ": " + section->GetProperty(propName));
	}

	// Write contents
	size_t entries = section->GetEntryCount();
	for (size_t i=0;i<entries;i++) {
		ConstEntry entry = section->GetEntry(i);
		shared_ptr<const SerializeText> serial = dynamic_pointer_cast<const SerializeText>(entry);
		writer->WriteLineToFile(serial->ToText(formatVersion));
	}
}


///////////////////////
// Validate the format
void FormatHandlerASS::MakeValid(IModel &model)
{
	// Only ASS supported right now
	if (formatVersion != 1) THROW_ATHENA_EXCEPTION(Exception::TODO);

	// Check for [Script Info]
	Section section = GetSection(model,"Script Info");
	if (!section) section = AddSection(model,"Script Info");

	// Check if necessary variables are available
	if (section->GetProperty("PlayResX").IsEmpty()) section->SetProperty("PlayResX","384");	// These two mystical values come from Substation Alpha
	if (section->GetProperty("PlayResY").IsEmpty()) section->SetProperty("PlayResY","288");	// 288 is half of 576, the PAL resolution, and 384 makes it 4:3
	section->SetProperty("ScriptType","v4.00+");

	// Get [V4+ Styles]
	section = GetSection(model,"V4+ Styles");
	if (!section) section = AddSection(model,"V4+ Styles");
	section->SetProperty("Format","Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding");

	// Get [Events]
	section = GetSection(model,"Events");
	if (!section) section = AddSection(model,"Events");
	section->SetProperty("Format","Layer, Start, End, Style, Actor, MarginL, MarginR, MarginV, Effect, Text");
}
