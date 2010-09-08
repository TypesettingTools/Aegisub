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

/// @file spellchecker_hunspell.cpp
/// @brief Hunspell-based spell checker implementation
/// @ingroup spelling
///

#include "config.h"

#ifdef WITH_HUNSPELL

#ifndef AGI_PRE
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#endif

#include <libaegisub/log.h>

#include "charset_conv.h"
#include "compat.h"
#include "main.h"
#include "spellchecker_hunspell.h"
#include "standard_paths.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"

/// @brief Constructor
HunspellSpellChecker::HunspellSpellChecker() {
	hunspell = NULL;
	conv = NULL;
	rconv = NULL;
	SetLanguage(lagi_wxString(OPT_GET("Tool/Spell Checker/Language")->GetString()));
}

/// @brief Destructor
HunspellSpellChecker::~HunspellSpellChecker() {
	Reset();
}

/// @brief Reset spelling library
void HunspellSpellChecker::Reset() {
	delete hunspell;
	hunspell = NULL;
	delete conv;
	conv = NULL;
	delete rconv;
	rconv = NULL;
	affpath.Clear();
	dicpath.Clear();
}

/// @brief Can add to dictionary?
/// @param word Word to check.
/// @return Whether word can be added or not.
///
bool HunspellSpellChecker::CanAddWord(wxString word) {
	if (!hunspell) return false;
	try {
		conv->Convert(STD_STR(word));
		return true;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

/// @brief Add word to dictionary
/// @param word Word to add.
///
void HunspellSpellChecker::AddWord(wxString word) {
	// Dictionary OK?
	if (!hunspell) return;

	// Add to currently loaded file
#ifdef WITH_OLD_HUNSPELL
	hunspell->put_word(conv->Convert(STD_STR(word)).c_str());
#else
	hunspell->add(conv->Convert(STD_STR(word)).c_str());
#endif

	// Ensure that the path exists
	wxFileName fn(usrdicpath);
	if (!fn.DirExists()) {
		wxFileName::Mkdir(fn.GetPath());
	}

	// Load dictionary
	wxArrayString dic;
	bool added = false;
	if (fn.FileExists()) {	// Even if you ever want to remove this "if", keep the braces, so the stream closes at the end
		bool first = true;
		TextFileReader reader(usrdicpath, L"UTF-8");
		while (reader.HasMoreLines()) {
			wxString curLine = reader.ReadLineFromFile();
			if (curLine.IsEmpty()) continue;

			if (first) {
				first = false;
				if (curLine.IsNumber()) continue;
			}

			// See if word to be added goes here
			if (!added && curLine.Lower() > word.Lower()) {
				dic.Add(word);
				added = true;
			}

			// Add to memory dictionary
			dic.Add(curLine);
		}
	}

	// Not added yet
	if (!added) dic.Add(word);

	// Write back to disk
	try {
		TextFileWriter writer(usrdicpath, L"UTF-8");
		writer.WriteLineToFile(wxString::Format(L"%i", dic.Count()));
		for (unsigned int i=0;i<dic.Count();i++) writer.WriteLineToFile(dic[i]);
	}
	catch (const agi::Exception&) {
		// Failed to open file
	}
}

/// @brief Check if the word is valid.
/// @param word Word to check
/// @return Whether word is valid or not.
///
bool HunspellSpellChecker::CheckWord(wxString word) {
	if (!hunspell) return true;
	try {
		return hunspell->spell(conv->Convert(STD_STR(word)).c_str()) == 1;
	}
	catch (agi::charset::ConvError const&) {
		return false;
	}
}

/// @brief Get suggestions for word.
/// @param word Word to get suggestions for
/// @return List of suggestions
///
wxArrayString HunspellSpellChecker::GetSuggestions(wxString word) {
	wxArrayString suggestions;
	if (!hunspell) return suggestions;

	try {
		// Grab raw from Hunspell
		char **results;
		int n = hunspell->suggest(&results,conv->Convert(STD_STR(word)).c_str());

		// Convert each
		for (int i=0;i<n;i++) {
			suggestions.Add(rconv->Convert(results[i]));
			delete results[i];
		}

		delete results;
	}
	catch (agi::charset::ConvError const&) {
		return suggestions;
	}

	return suggestions;
}

/// @brief Get list of available dictionaries.
/// @return List of available dictionaries
///
wxArrayString HunspellSpellChecker::GetLanguageList() {
	// Get dir name
	wxString path = StandardPaths::DecodePathMaybeRelative(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()), _T("?data")) + _T("/");
	wxArrayString list;
	wxFileName folder(path);
	if (!folder.DirExists()) return list;

	// Get file lists
	wxArrayString dic;
	wxDir::GetAllFiles(path,&dic,_T("*.dic"),wxDIR_FILES);
	wxArrayString aff;
	wxDir::GetAllFiles(path,&aff,_T("*.aff"),wxDIR_FILES);

	// For each dictionary match, see if it can find the corresponding .aff
	for (unsigned int i=0;i<dic.Count();i++) {
		wxString curAff = dic[i].Left(dic[i].Length()-4) + _T(".aff");
		for (unsigned int j=0;j<aff.Count();j++) {
			// Found match
			if (curAff == aff[j]) {
				wxFileName fname(curAff);
				list.Add(fname.GetName());
				break;
			}
		}
	}

	// Return list
	return list;
}

/// @brief Set language.
/// @param language Language to set
///
void HunspellSpellChecker::SetLanguage(wxString language) {
	// Unload
	Reset();
	if (language.IsEmpty()) return;

	// Get dir name
	//FIXME: this should use ?user instead of ?data; however, since it apparently works already on win32, I'm not gonna mess with it right now :p
	wxString path = StandardPaths::DecodePathMaybeRelative(lagi_wxString(OPT_GET("Path/Dictionary")->GetString()), _T("?data")) + _T("/");
	wxString userPath = StandardPaths::DecodePath(_T("?user/dictionaries/user_"));

	// Get affix and dictionary paths
	affpath = wxString::Format("%s%s.aff", path, language);
	dicpath = wxString::Format("%s%s.dic", path, language);
	usrdicpath = wxString::Format("%s%s.dic", userPath, language);

	LOG_I("dictionary/file") << dicpath;

	// Check if language is available
	if (!wxFileExists(affpath) || !wxFileExists(dicpath)) return;

	// Load
	hunspell = new Hunspell(affpath.mb_str(csConvLocal),dicpath.mb_str(csConvLocal));
	conv = NULL;
	if (hunspell) {
		conv  = new agi::charset::IconvWrapper("wchar_t", hunspell->get_dic_encoding());
		rconv = new agi::charset::IconvWrapper(hunspell->get_dic_encoding(), "wchar_t");
		try {
			TextFileReader reader(usrdicpath, L"UTF-8");
			while (reader.HasMoreLines()) {
				wxString curLine = reader.ReadLineFromFile();
				if (curLine.IsEmpty() || curLine.IsNumber()) continue;
#ifdef WITH_OLD_HUNSPELL
				hunspell->put_word(conv->Convert(STD_STR(curLine)).c_str());
#else
				hunspell->add(conv->Convert(STD_STR(curLine)).c_str());
#endif
			}
		}
		catch (const wchar_t *) {
			// file not found
		}
	}
}

#endif // WITH_HUNSPELL
