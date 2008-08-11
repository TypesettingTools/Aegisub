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
#include <wx/intl.h>
#include <wx/choicdlg.h>
#include "subtitle_format.h"
#include "subtitle_format_ass.h"
#include "subtitle_format_srt.h"
#include "subtitle_format_txt.h"
#include "subtitle_format_ttxt.h"
#include "subtitle_format_mkv.h"
#include "subtitle_format_microdvd.h"
#include "subtitle_format_encore.h"
#include "subtitle_format_transtation.h"
#include "subtitle_format_dvd.h"
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
		new EncoreSubtitleFormat();
		new TranStationSubtitleFormat();
#ifdef __WXDEBUG__
		new DVDSubtitleFormat();
#endif
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
	final = wxString(_("All Supported Formats")) + _T(" (") + temp1.Left(temp1.Length()-1) + _T(")|") + temp2.Left(temp2.Length()-1) + _T("|") + final.Left(final.Length()-1);

	// Return final list
	return final;
}


/////////////////////////////////
// Ask the user to enter the FPS
double SubtitleFormat::AskForFPS(bool palNtscOnly) {
	wxArrayString choices;
	
	// Video FPS
	bool vidLoaded = !palNtscOnly && VFR_Output.IsLoaded();
	if (vidLoaded) {
		wxString vidFPS;
		if (VFR_Output.GetFrameRateType() == VFR) vidFPS = _T("VFR");
		else vidFPS = wxString::Format(_T("%.3f"),VFR_Output.GetAverage());
		choices.Add(wxString::Format(_T("From video (%s)"),vidFPS.c_str()));
	}
	
	// Standard FPS values
	if (!palNtscOnly) {
		choices.Add(_("15.000 FPS"));
		choices.Add(_("23.976 FPS (Decimated NTSC)"));
		choices.Add(_("24.000 FPS (FILM)"));
	}
	choices.Add(_("25.000 FPS (PAL)"));
	choices.Add(_("29.970 FPS (NTSC)"));
	if (!palNtscOnly) {
		choices.Add(_("30.000 FPS"));
		choices.Add(_("59.940 FPS (NTSC x2)"));
		choices.Add(_("60.000 FPS"));
		choices.Add(_("119.880 FPS (NTSC x4)"));
		choices.Add(_("120.000 FPS"));
	}

	// Ask
	int choice = wxGetSingleChoiceIndex(_("Please choose the appropriate FPS for the subtitles:"),_("FPS"),choices);
	if (choice == -1) return 0.0;

	// PAL/NTSC choice
	if (palNtscOnly) {
		if (choice == 0) return 25.0;
		else return 30.0 / 1.001;
	}

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


////////////////////////////
// Remove all comment lines
void SubtitleFormat::StripComments() {
	using std::list;
	list<AssEntry*>::iterator next;
	
	for (list<AssEntry*>::iterator cur = Line->begin(); cur != Line->end(); cur = next) {
		next = cur;
		next++;
		
		AssDialogue *dlg = AssEntry::GetAsDialogue(*cur);
		if (dlg && (dlg->Comment || dlg->Text.IsEmpty())) {
			delete *cur;
			Line->erase(cur);
		}
	}
}


/////////////////////////////////
// Remove all non-dialogue lines
void SubtitleFormat::StripNonDialogue() {
	using std::list;
	list<AssEntry*>::iterator next;
	
	for (list<AssEntry*>::iterator cur = Line->begin(); cur != Line->end(); cur = next) {
		next = cur;
		next++;
		
		if (!AssEntry::GetAsDialogue(*cur)) {
			delete *cur;
			Line->erase(cur);
		}
	}
}


///////////////////////////////////////////
// Helper function for RecombineOverlaps()
static void InsertLineSortedIntoList(std::list<AssEntry*> &list, std::list<AssEntry*>::iterator next, AssDialogue *newdlg) {
	std::list<AssEntry*>::iterator insertpos = next;
	bool inserted = false;
	while (insertpos != list.end()) {
		AssDialogue *candidate = AssEntry::GetAsDialogue(*insertpos);
		if (candidate && candidate->Start >= newdlg->Start) {
			list.insert(insertpos, newdlg);
			inserted = true;
			break;
		}
		insertpos++;
	}
	if (!inserted) {
		list.push_back(newdlg);
	}
}

///////////////////////////////////////////////////////////
// Split and merge lines so there are no overlapping lines
// http://malakith.net/aegiwiki/Split-merge_algorithm_for_converting_to_simple_subtitle_formats
void SubtitleFormat::RecombineOverlaps() {
	using std::list;
	list<AssEntry*>::iterator next;
	
	for (list<AssEntry*>::iterator cur = Line->begin(); cur != Line->end(); cur = next) {
		next = cur;
		next++;
		
		if (next == Line->end()) break;
		
		AssDialogue *prevdlg = AssEntry::GetAsDialogue(*cur);
		AssDialogue *curdlg = AssEntry::GetAsDialogue(*next);
		
		if (curdlg && prevdlg && prevdlg->End > curdlg->Start) {
			// Use names like in the algorithm description and prepare for erasing
			// old dialogues from the list
			list<AssEntry*>::iterator prev = cur;
			cur = next;
			next++;
			
			// std::list::insert() inserts items before the given iterator, so
			// we need 'next' for inserting. 'prev' and 'cur' can safely be erased
			// from the list now.
			Line->erase(prev);
			Line->erase(cur);
			
			//Is there an A part before the overlap?
			if (curdlg->Start > prevdlg->Start) {
				// Produce new entry with correct values
				AssDialogue *newdlg = AssEntry::GetAsDialogue(prevdlg->Clone());
				newdlg->Start = prevdlg->Start;
				newdlg->End = curdlg->Start;
				newdlg->Text = prevdlg->Text;
				
				InsertLineSortedIntoList(*Line, next, newdlg);
			}
			
			// Overlapping A+B part
			{
				AssDialogue *newdlg = AssEntry::GetAsDialogue(prevdlg->Clone());
				newdlg->Start = curdlg->Start;
				newdlg->End = (prevdlg->End < curdlg->End) ? prevdlg->End : curdlg->End;
				// Put an ASS format hard linewrap between lines
				newdlg->Text = curdlg->Text + _T("\\N") + prevdlg->Text;
				
				InsertLineSortedIntoList(*Line, next, newdlg);
			}
			
			// Is there an A part after the overlap?
			if (prevdlg->End > curdlg->End) {
				// Produce new entry with correct values
				AssDialogue *newdlg = AssEntry::GetAsDialogue(prevdlg->Clone());
				newdlg->Start = curdlg->End;
				newdlg->End = prevdlg->End;
				newdlg->Text = prevdlg->Text;
				
				InsertLineSortedIntoList(*Line, next, newdlg);
			}
			
			// Is there a B part after the overlap?
			if (curdlg->End > prevdlg->End) {
				// Produce new entry with correct values
				AssDialogue *newdlg = AssEntry::GetAsDialogue(prevdlg->Clone());
				newdlg->Start = prevdlg->End;
				newdlg->End = curdlg->End;
				newdlg->Text = curdlg->Text;
				
				InsertLineSortedIntoList(*Line, next, newdlg);
			}
			
			next--;
		}
	}
}


////////////////////////////////////////////////
// Merge identical lines that follow each other
void SubtitleFormat::MergeIdentical() {
	using std::list;
	list<AssEntry*>::iterator next;
	
	for (list<AssEntry*>::iterator cur = Line->begin(); cur != Line->end(); cur = next) {
		next = cur;
		next++;
		
		if (next == Line->end()) break;
		
		AssDialogue *curdlg = AssEntry::GetAsDialogue(*cur);
		AssDialogue *nextdlg = AssEntry::GetAsDialogue(*next);
		
		if (curdlg && nextdlg && curdlg->End == nextdlg->Start && curdlg->Text == nextdlg->Text) {
			// Merge timing
			nextdlg->Start = (nextdlg->Start < curdlg->Start ? nextdlg->Start : curdlg->Start);
			nextdlg->End = (nextdlg->End > curdlg->End ? nextdlg->End : curdlg->End);
			
			// Remove duplicate line
			delete *cur;
			Line->erase(cur);
		}
	}
}

