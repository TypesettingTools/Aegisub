// Copyright (c) 2007, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file charset_detect.cpp
/// @brief Wrapper around text encoding detection library
/// @ingroup utility
///


///////////
// Headers
#include "config.h"

#ifdef WITH_UNIVCHARDET
#include <list>
#include <wx/intl.h>
#include <wx/choicdlg.h>

#include "text_file_reader.h"
#include "charset_detect.h"
#include "../universalchardet/nsCharSetProber.h"




/// DOCME
struct CharDetResult {

	/// DOCME
	float confidence;

	/// DOCME
	wxString name;


	/// @brief DOCME
	/// @param par 
	/// @return 
	///
	bool operator < (CharDetResult &par) { return confidence > par.confidence; }
};


/// @brief Get encoding 
/// @param filename 
/// @return 
///
wxString CharSetDetect::GetEncoding(wxString filename) {
	std::ifstream file;
#ifdef __WINDOWS__
	file.open(filename.wc_str(),std::ios::in | std::ios::binary);
#else
	file.open(wxFNCONV(filename),std::ios::in | std::ios::binary);
#endif
	if (!file.is_open()) {
		throw _T("Failed opening file for reading.");
	}

	// Loop through it until it finds interesting lines
	while (!file.eof() && !done()) {
		char buffer[512];
		file.read(buffer, 512);
		size_t bytesRead = file.gcount();
		HandleData(buffer, bytesRead);
	}

	// Flag as finished
	DataEnd();

	// Grab every result obtained
	wxString local = wxLocale::GetSystemEncodingName();
	std::list<CharDetResult> results;
	bool gotLocal = false;
	for (int i=0;i<NUM_OF_CHARSET_PROBERS;i++) {
		if (mCharSetProbers[i]) {
			int probes = mCharSetProbers[i]->GetProbeCount();
			for (int j=0;j<probes;j++) {
				float conf = mCharSetProbers[i]->GetConfidence(j);

				// Only bother with those whose confidence is at least 1%
				wxString curName = wxString(mCharSetProbers[i]->GetCharSetName(j),wxConvUTF8);
				if (conf > 0.01f || curName == local) {
					results.push_back(CharDetResult());
					results.back().name = curName;
					results.back().confidence = mCharSetProbers[i]->GetConfidence(j);
				}
			}
		}
	}

	// If you got more than one valid result, ask the user which he wants
	if (results.size() > 1) {
		// Add local
		if (!gotLocal) {
			results.push_back(CharDetResult());
			results.back().name = local;
			results.back().confidence = 0;
		}

		// Sort by confidence
		results.sort();

		// Get choice from user
		wxArrayString choices;
		wxArrayString picked;
		int i = 0;
		for (std::list<CharDetResult>::iterator cur=results.begin();cur!=results.end();cur++) {
			wxString name = (*cur).name;
			if (picked.Index(name) == wxNOT_FOUND) {
				picked.Add(name);

				// Generate name
				wxString choiceStr;
				if ((*cur).confidence > 0.0f) choiceStr = wxString::Format(_T("%f%% - "),(*cur).confidence*100.0f);
				else choiceStr = _T("Unknown - ");
				choiceStr += name;
				if (name == local) choiceStr += _T(" (local)");

				// Insert
				choices.Add(choiceStr);
				i++;
				if (i == 20) break;
			}
		}
		int choice = wxGetSingleChoiceIndex(_("Aegisub could not narrow down the character set to a single one.\nPlease pick one below:"),_("Choose character set"),choices);
		if (choice == -1) throw _T("Canceled");

		// Retrieve name
		i = 0;
		for (std::list<CharDetResult>::iterator cur=results.begin();cur!=results.end();cur++,i++) {
			if (i == choice) result = (*cur).name;
		}
	}

	// Return whatever it got
	return result;
}


/// @brief Report 
/// @param aCharset 
///
void CharSetDetect::Report(const char* aCharset) {
	// Store the result reported
	result = wxString(aCharset,wxConvUTF8);
}

#endif // WITH_UNIVCHARDET


