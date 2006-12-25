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
}


//////////////
// Destructor
MySpellThesaurus::~MySpellThesaurus() {
	delete mythes;
	mythes = NULL;
}


///////////////////
// Get suggestions
wxArrayString MySpellThesaurus::GetSuggestions(wxString word) {
	// Array
	wxArrayString suggestions;

	// Get suggestions
	if (mythes) {
		// Grab raw from MyThes
		mentry *me;
		wxCharBuffer buf = word.mb_str(wxConvUTF8);
		int n = mythes->Lookup(buf,strlen(buf),&me);

		// Each entry
		for (int i=0;i<n;i++) {
			suggestions.Add(wxString(me[i].defn,wxConvUTF8));
			for (int j=0;j<me[i].count;j++) suggestions.Add(wxString(_T("+")) + wxString(me[i].psyns[j],wxConvUTF8));
		}

		// Delete
		mythes->CleanUpAfterLookup(&me,n);
	}

	// Return them
	return suggestions;
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
