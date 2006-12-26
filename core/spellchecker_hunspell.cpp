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
#include "setup.h"
#if USE_HUNSPELL == 1
#include <hunspell/hunspell.hxx>
#include "spellchecker_hunspell.h"
#include "main.h"


///////////////
// Constructor
HunspellSpellChecker::HunspellSpellChecker() {
	wxString affpath = AegisubApp::folderName + _T("dictionaries/en_US.aff");
	wxString dpath = AegisubApp::folderName + _T("dictionaries/en_US.dic");
	hunspell = new Hunspell(affpath.mb_str(wxConvLocal),dpath.mb_str(wxConvLocal));
	conv = NULL;
	if (hunspell) conv = new wxCSConv(wxString(hunspell->get_dic_encoding(),wxConvUTF8));
}


//////////////
// Destructor
HunspellSpellChecker::~HunspellSpellChecker() {
	delete hunspell;
	hunspell = NULL;
	delete conv;
	conv = NULL;
}


//////////////////////////
// Can add to dictionary?
bool HunspellSpellChecker::CanAddWord(wxString word) {
	if (!hunspell) return false;
	return (word.mb_str(*conv) != NULL);
}


//////////////////////////
// Add word to dictionary
void HunspellSpellChecker::AddWord(wxString word) {
	if (hunspell) hunspell->put_word(word.mb_str(*conv));
}


//////////////////////////////
// Check if the word is valid
bool HunspellSpellChecker::CheckWord(wxString word) {
	if (!hunspell) return true;
	wxCharBuffer buf = word.mb_str(*conv);
	if (buf) return (hunspell->spell(buf) == 1);
	return false;
}


////////////////////////////
// Get suggestions for word
wxArrayString HunspellSpellChecker::GetSuggestions(wxString word) {
	// Array
	wxArrayString suggestions;

	// Word
	wxCharBuffer buf = word.mb_str(*conv);
	if (!buf) return suggestions;

	// Get suggestions
	if (hunspell) {
		// Grab raw from Hunspell
		char **results;
		int n = hunspell->suggest(&results,buf);

		// Convert each
		for (int i=0;i<n;i++) {
			wxString current(results[i],*conv);
			suggestions.Add(current);
			delete results[i];
		}

		// Delete
		delete results;
	}

	// Return them
	return suggestions;
}


//////////////////////////////////////
// Get list of available dictionaries
wxArrayString HunspellSpellChecker::GetLanguageList() {
	wxArrayString list;
	return list;
}


////////////////
// Set language
void HunspellSpellChecker::SetLanguage(wxString language) {
}

#endif
