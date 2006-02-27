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
#include "subtitle_format_reader.h"
#include "subtitle_format_ass.h"
#include "subtitle_format_srt.h"
#include "subtitle_format_txt.h"
#include "ass_file.h"


///////////////
// Constructor
SubtitleFormatReader::SubtitleFormatReader() {
	Line = NULL;
	Register();
}


//////////////
// Destructor
SubtitleFormatReader::~SubtitleFormatReader() {
	Remove();
}


//////////////
// Set target
void SubtitleFormatReader::SetTarget(AssFile *file) {
	if (!file) Line = NULL;
	else Line = &file->Line;
	assFile = file;
}


////////
// List
std::list<SubtitleFormatReader*> SubtitleFormatReader::readers;
bool SubtitleFormatReader::loaded = false;


/////////////////////////////
// Get an appropriate reader
SubtitleFormatReader *SubtitleFormatReader::GetReader(wxString filename) {
	LoadReaders();
	std::list<SubtitleFormatReader*>::iterator cur;
	SubtitleFormatReader *reader;
	for (cur=readers.begin();cur!=readers.end();cur++) {
		reader = *cur;
		if (reader->CanReadFile(filename)) return reader;
	}
	return NULL;
}


////////////
// Register
void SubtitleFormatReader::Register() {
	std::list<SubtitleFormatReader*>::iterator cur;
	for (cur=readers.begin();cur!=readers.end();cur++) {
		if (*cur == this) return;
	}
	readers.push_back(this);
}


//////////
// Remove
void SubtitleFormatReader::Remove() {
	std::list<SubtitleFormatReader*>::iterator cur;
	for (cur=readers.begin();cur!=readers.end();cur++) {
		if (*cur == this) {
			readers.erase(cur);
			return;
		}
	}
}


///////////////////
// Clear subtitles
void SubtitleFormatReader::Clear() {
	assFile->Clear();
}


////////////////
// Load default
void SubtitleFormatReader::LoadDefault() {
	assFile->LoadDefault();
}


///////////////////
// Set if it's ASS
void SubtitleFormatReader::SetIsASS(bool isASS) {
	assFile->IsASS = isASS;
}


////////////
// Add line
int SubtitleFormatReader::AddLine(wxString data,wxString group,int lasttime,bool &IsSSA) {
	return assFile->AddLine(data,group,lasttime,IsSSA);
}


///////////////
// Add loaders
void SubtitleFormatReader::LoadReaders () {
	if (!loaded) {
		new ASSSubtitleFormatReader();
		new SRTSubtitleFormatReader();
		new TXTSubtitleFormatReader();
	}
	loaded = true;
}


///////////////////
// Destroy loaders
void SubtitleFormatReader::DestroyReaders () {
	SubtitleFormatReader *reader;
	std::list<SubtitleFormatReader*>::iterator cur,next;
	for (cur=readers.begin();cur!=readers.end();cur = next) {
		next = cur;
		next++;
		reader = *cur;
		readers.erase(cur);
		delete reader;
	}
	readers.clear();
}
