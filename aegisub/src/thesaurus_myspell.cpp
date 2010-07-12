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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file thesaurus_myspell.cpp
/// @brief MySpell-based thesaurus implementation
/// @ingroup thesaurus
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/log.h>
#endif

#include <libaegisub/log.h>

#include "compat.h"
#include "mythes.hxx"
#include "main.h"
#include "standard_paths.h"
#include "thesaurus_myspell.h"
#include "utils.h"


/// @brief Constructor 
///
MySpellThesaurus::MySpellThesaurus() {
	conv = NULL;
	mythes = NULL;
	SetLanguage(lagi_wxString(OPT_GET("Tool/Thesaurus/Language")->GetString()));
}



/// @brief Destructor 
///
MySpellThesaurus::~MySpellThesaurus() {
	delete mythes;
	mythes = NULL;
	delete conv;
	conv = NULL;
}



/// @brief Get suggestions 
/// @param word   
/// @param result 
/// @return 
///
void MySpellThesaurus::Lookup(wxString word,ThesaurusEntryArray &result) {
	// Loaded?
	if (!mythes) return;

	// Grab raw from MyThes
	mentry *me;
	wxCharBuffer buf = word.Lower().mb_str(*conv);
	if (!buf) return;
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



/// @brief Get language list 
/// @return 
///
wxArrayString MySpellThesaurus::GetLanguageList() {
	// Get dir name
	wxString path = StandardPaths::DecodePathMaybeRelative(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()), _T("?data")) + _T("/");
	wxArrayString list;
	wxFileName folder(path);
	if (!folder.DirExists()) return list;

	// Get file lists
	wxArrayString idx;
	wxDir::GetAllFiles(path,&idx,_T("*.idx"),wxDIR_FILES);
	wxArrayString dat;
	wxDir::GetAllFiles(path,&dat,_T("*.dat"),wxDIR_FILES);

	// For each idxtionary match, see if it can find the corresponding .dat
	for (unsigned int i=0;i<idx.Count();i++) {
		wxString curdat = idx[i].Left(MAX(0,signed(idx[i].Length())-4)) + _T(".dat");
		for (unsigned int j=0;j<dat.Count();j++) {
			// Found match
			if (curdat == dat[j]) {
				wxFileName fname(curdat);
				wxString name = fname.GetName();
				if (name.Left(3) == _T("th_")) name = name.Mid(3);
				list.Add(name);
				break;
			}
		}
	}

	// Return list
	return list;
}



/// @brief Set language 
/// @param language 
///
void MySpellThesaurus::SetLanguage(wxString language) {
	// Unload
	delete mythes;
	mythes = NULL;
	delete conv;
	conv = NULL;

	// Unloading
	if (language.IsEmpty()) return;

	// Get dir name
	wxString path = StandardPaths::DecodePathMaybeRelative(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()), _T("?data")) + _T("/");

	// Get affix and dictionary paths
	wxString idxpath = path + _T("th_") + language + _T(".idx");
	wxString datpath = path + _T("th_") + language + _T(".dat");

	// Check if language is available
	if (!wxFileExists(idxpath) || !wxFileExists(datpath)) return;

	LOG_I("thesaurus/file") << "Using thesaurus: " << datpath.c_str();

	// Load
	mythes = new MyThes(idxpath.mb_str(wxConvLocal),datpath.mb_str(wxConvLocal));
	conv = NULL;
	if (mythes) conv = new wxCSConv(wxString(mythes->get_th_encoding(),wxConvUTF8));
}


