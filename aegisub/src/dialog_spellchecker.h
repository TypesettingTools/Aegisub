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

/// @file dialog_spellchecker.h
/// @see dialog_spellchecker.cpp
/// @ingroup unused spelling
///

#ifndef AGI_PRE
#include <map>

#include <wx/dialog.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#endif

namespace agi { struct Context; }
class SpellChecker;

/// DOCME
/// @class DialogSpellChecker
/// @brief DOCME
///
/// DOCME
class DialogSpellChecker : public wxDialog {
	agi::Context *context;

	/// DOCME
	SpellChecker *spellchecker;


	/// DOCME
	std::map<wxString,wxString> autoReplace;

	/// DOCME
	wxArrayString autoIgnore;

	/// DOCME
	wxArrayString langCodes;


	/// DOCME

	/// DOCME
	int wordStart,wordEnd;

	/// DOCME
	int lastLine;

	/// DOCME
	int lastPos;

	/// DOCME
	int firstLine;


	/// DOCME
	wxTextCtrl *origWord;

	/// DOCME
	wxTextCtrl *replaceWord;

	/// DOCME
	wxListBox *suggestList;

	/// DOCME
	wxComboBox *language;
	wxButton *addButton;

	bool FindOrDie();
	bool FindNext(int startLine=-1,int startPos=-1);
	bool GetFirstMatch();
	void SetWord(wxString word);
	void Replace();

	void OnChangeLanguage(wxCommandEvent &event);
	void OnChangeSuggestion(wxCommandEvent &event);
	void OnTakeSuggestion(wxCommandEvent &event);

	void OnClose(wxCommandEvent &event);
	void OnReplace(wxCommandEvent &event);
	void OnReplaceAll(wxCommandEvent &event);
	void OnIgnore(wxCommandEvent &event);
	void OnIgnoreAll(wxCommandEvent &event);
	void OnAdd(wxCommandEvent &event);

public:
	DialogSpellChecker(agi::Context *context);
	~DialogSpellChecker();

	DECLARE_EVENT_TABLE()
};


