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
#include "thesaurus_myspell.h"
#include "mythes.hxx"
#include "main.h"


///////////////
// Constructor
MySpellThesaurus::MySpellThesaurus() {
	wxString idxpath = AegisubApp::folderName + _T("dictionaries/th_en_US.idx");
	wxString datpath = AegisubApp::folderName + _T("dictionaries/th_en_US.dat");
	mythes = new MyThes(idxpath.mb_str(wxConvLocal),datpath.mb_str(wxConvLocal));
	conv = NULL;
	if (mythes) conv = new wxCSConv(wxString(mythes->get_th_encoding(),wxConvUTF8));
}


//////////////
// Destructor
MySpellThesaurus::~MySpellThesaurus() {
	delete mythes;
	mythes = NULL;
	delete conv;
	conv = NULL;
}


///////////////////
// Get suggestions
void MySpellThesaurus::Lookup(wxString word,ThesaurusEntryArray &result) {
	// Loaded?
	if (!mythes) return;

	// Grab raw from MyThes
	mentry *me;
	wxCharBuffer buf = word.Lower().mb_str(*conv);
	int n = mythes->Lookup(buf,strlen(buf),&me);

	// Each entry
	for (int i=0;i<n;i++) {
		ThesaurusEntry entry;
		entry.name = wxString(me[i].defn,*conv);
		for (int j=0;j<me[i].count;j++) entry.words.Add(wxString(me[i].psyns[j],*conv));
		result.push_back(entry);
	}

	// Clean up
	mythes->CleanUpAfterLookup(&me,n);
}


/////////////////////
// Get language list
wxArrayString MySpellThesaurus::GetLanguageList() {
	wxArrayString list;
	return list;
}


////////////////
// Set language
void MySpellThesaurus::SetLanguage(wxString language) {
}
