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
#include "subtitle_format.h"
#include "subtitle_format_ass.h"
#include "subtitle_format_srt.h"
#include "subtitle_format_txt.h"
#include "subtitle_format_ttxt.h"
#include "subtitle_format_mkv.h"
#include "subtitle_format_microdvd.h"
#include "ass_file.h"
#include "vfr.h"


///////////////
// Constructor
SubtitleFormat::SubtitleFormat() {
	Line = NULL;
	Register();
	isCopy = false;
}


//////////////
// Destructor
SubtitleFormat::~SubtitleFormat () {
	Remove();
}


////////
// List
std::list<SubtitleFormat*> SubtitleFormat::formats;
bool SubtitleFormat::loaded = false;


//////////////
// Set target
void SubtitleFormat::SetTarget(AssFile *file) {
	ClearCopy();
	if (!file) Line = NULL;
	else Line = &file->Line;
	assFile = file;
}


///////////////
// Create copy
void SubtitleFormat::CreateCopy() {
	SetTarget(new AssFile(*assFile));
	isCopy = true;
}


//////////////
// Clear copy
void SubtitleFormat::ClearCopy() {
	if (isCopy) {
		delete assFile;
		assFile = NULL;
		isCopy = false;
	}
}


///////////////////
// Clear subtitles
void SubtitleFormat::Clear() {
	assFile->Clear();
}


////////////////
// Load default
void SubtitleFormat::LoadDefault(bool defline) {
	assFile->LoadDefault(defline);
}


////////////
// Add line
int SubtitleFormat::AddLine(wxString data,wxString group,int lasttime,int &version,wxString *outgroup) {
	return assFile->AddLine(data,group,lasttime,version,outgroup);
}


///////////////
// Add formats
void SubtitleFormat::LoadFormats () {
	if (!loaded) {
		new ASSSubtitleFormat();
		new SRTSubtitleFormat();
		new TXTSubtitleFormat();
		new TTXTSubtitleFormat();
		new MicroDVDSubtitleFormat();
		new MKVSubtitleFormat();
	}
	loaded = true;
}


///////////////////
// Destroy formats
void SubtitleFormat::DestroyFormats () {
	std::list<SubtitleFormat*>::iterator cur;
	for (cur=formats.begin();cur!=formats.end();cur = formats.begin()) {
		delete *cur;
	}
	formats.clear();
}


/////////////////////////////
// Get an appropriate reader
SubtitleFormat *SubtitleFormat::GetReader(wxString filename) {
	LoadFormats();
	std::list<SubtitleFormat*>::iterator cur;
	SubtitleFormat *reader;
	for (cur=formats.begin();cur!=formats.end();cur++) {
		reader = *cur;
		if (reader->CanReadFile(filename)) return reader;
	}
	return NULL;
}


/////////////////////////////
// Get an appropriate writer
SubtitleFormat *SubtitleFormat::GetWriter(wxString filename) {
	LoadFormats();
	std::list<SubtitleFormat*>::iterator cur;
	SubtitleFormat *writer;
	for (cur=formats.begin();cur!=formats.end();cur++) {
		writer = *cur;
		if (writer->CanWriteFile(filename)) return writer;
	}
	return NULL;
}


////////////
// Register
void SubtitleFormat::Register() {
	std::list<SubtitleFormat*>::iterator cur;
	for (cur=formats.begin();cur!=formats.end();cur++) {
		if (*cur == this) return;
	}
	formats.push_back(this);
}


//////////
// Remove
void SubtitleFormat::Remove() {
	std::list<SubtitleFormat*>::iterator cur;
	for (cur=formats.begin();cur!=formats.end();cur++) {
		if (*cur == this) {
			formats.erase(cur);
			return;
		}
	}
}


//////////////////////
// Get read wildcards
wxArrayString SubtitleFormat::GetReadWildcards() {
	return wxArrayString();
}


///////////////////////
// Get write wildcards
wxArrayString SubtitleFormat::GetWriteWildcards() {
	return wxArrayString();
}


/////////////////////
// Get wildcard list
wxString SubtitleFormat::GetWildcards(int mode) {
	// Ensure it's loaded
	LoadFormats();

	// Variables
	wxArrayString all;
	wxArrayString cur;
	wxString wild;
	wxString final;
	wxString temp1;
	wxString temp2;

	// For each format
	std::list<SubtitleFormat*>::iterator curIter;
	SubtitleFormat *format;
	for (curIter=formats.begin();curIter!=formats.end();curIter++) {
		// Get list
		format = *curIter;
		if (mode == 0) cur = format->GetReadWildcards();
		else if (mode == 1) cur = format->GetWriteWildcards();
		temp1.Clear();
		temp2.Clear();

		// Has wildcards
		if (cur.Count()) {
			// Process entries
			for (unsigned int i=0;i<cur.Count();i++) {
				wild = _T("*.") + cur[i];
				all.Add(wild);
				temp1 += wild + _T(",");
				temp2 += wild + _T(";");
			}

			// Assemble final name
			final += format->GetName() + _T(" (") + temp1.Left(temp1.Length()-1) + _T(")|") + temp2.Left(temp2.Length()-1) + _T("|");
		}
	}

	// Add "all formats" list
	temp1.Clear();
	temp2.Clear();
	for (unsigned int i=0;i<all.Count();i++) {
		temp1 += all[i] + _T(",");
		temp2 += all[i] + _T(";");
	}
	final = _("All Supported Formats (") + temp1.Left(temp1.Length()-1) + _T(")|") + temp2.Left(temp2.Length()-1) + _T("|") + final.Left(final.Length()-1);

	// Return final list
	return final;
}


/////////////////////////////////
// Ask the user to enter the FPS
double SubtitleFormat::AskForFPS() {
	wxArrayString choices;
	
	// Video FPS
	bool vidLoaded = VFR_Output.IsLoaded();
	if (vidLoaded) {
		wxString vidFPS;
		if (VFR_Output.GetFrameRateType() == VFR) vidFPS = _T("VFR");
		else vidFPS = wxString::Format(_T("%.3f"),VFR_Output.GetAverage());
		choices.Add(wxString::Format(_T("From video (%s)"),vidFPS.c_str()));
	}
	
	// Standard FPS values
	choices.Add(_("15.000 FPS"));
	choices.Add(_("23.976 FPS (Decimated NTSC)"));
	choices.Add(_("24.000 FPS (FILM)"));
	choices.Add(_("25.000 FPS (PAL)"));
	choices.Add(_("29.970 FPS (NTSC)"));
	choices.Add(_("30.000 FPS"));
	choices.Add(_("59.940 FPS (NTSC x2)"));
	choices.Add(_("60.000 FPS"));
	choices.Add(_("119.880 FPS (NTSC x4)"));
	choices.Add(_("120.000 FPS"));

	// Ask
	int choice = wxGetSingleChoiceIndex(_("Please choose the appropriate FPS for the subtitles:"),_("FPS"),choices);
	if (choice == -1) return 0.0;

	// Get FPS from choice
	if (vidLoaded) choice--;
	switch (choice) {
		case -1: return -1.0; break; // VIDEO
		case 0: return 15.0; break;
		case 1: return 24.0 / 1.001; break;
		case 2: return 24.0; break;
		case 3: return 25.0; break;
		case 4: return 30.0 / 1.001; break;
		case 5: return 30.0; break;
		case 6: return 60.0 / 1.001; break;
		case 7: return 60.0; break;
		case 8: return 120.0 / 1.001; break;
		case 9: return 120.0; break;
	}

	// fubar
	return 0.0;
}


//////////////
// Sort lines
void SubtitleFormat::SortLines() {
	Line->sort(LessByPointedToValue<AssEntry>());
}


////////////////
// Convert tags
void SubtitleFormat::ConvertTags(int format,wxString lineEnd) {
	using std::list;
	list<AssEntry*>::iterator next;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current) {
			// Strip tags
			if (format == 1) current->StripTags();
			else if (format == 2) current->ConvertTagsToSRT();

			// Replace line breaks
			current->Text.Replace(_T("\\h"),_T(" "),true);
			current->Text.Replace(_T("\\n"),lineEnd,true);
			current->Text.Replace(_T("\\N"),lineEnd,true);
			while (current->Text.Replace(lineEnd+lineEnd,lineEnd,true));
		}
	}
}


////////////////////////////////////////////
// Merge identical and/or overlapping lines
void SubtitleFormat::Merge(bool identical,bool overlaps,bool stripComments) {
	using std::list;
	list<AssEntry*>::iterator next;
	list<AssEntry*>::iterator prev = Line->end();
	AssDialogue *previous = NULL;

	// Loop through each line
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur=next) {
		next = cur;
		next++;

		// Dialogue line
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current) {
			// Strip comments and empty lines
			if ((current->Comment && stripComments) || current->Text.IsEmpty()) {
				delete *cur;
				Line->erase(cur);
			}

			// Proper line
			else {
				// Check for duplication
				if (previous != NULL) {
					if (previous->Text == current->Text) {
						if (abs(current->Start.GetMS() - previous->End.GetMS()) < 20) {
							current->Start = (current->Start < previous->Start ? current->Start : previous->Start);
							current->End = (current->End > previous->End ? current->End : previous->End);
							delete *prev;
							Line->erase(prev);
						}
					}
				}

				// Set as previous	
				prev = cur;
				previous = current;
			}
		}

		// Other line, delete it
		else {
			delete *cur;
			Line->erase(cur);
		}
	}
}
