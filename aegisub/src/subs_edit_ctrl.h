// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file subs_edit_ctrl.h
/// @see subs_edit_ctrl.cpp
/// @ingroup main_ui
///

#ifndef AGI_PRE
#include <memory>
#endif

#include "scintilla_text_ctrl.h"

class SpellChecker;
class SubsEditBox;
class SubtitlesGrid;
class Thesaurus;

/// DOCME
/// @class SubsTextEditCtrl
/// @brief DOCME
///
/// DOCME
class SubsTextEditCtrl : public ScintillaTextCtrl {
	/// DOCME
	std::auto_ptr<SpellChecker> spellchecker;

	/// DOCME
	std::auto_ptr<Thesaurus> thesaurus;

	SubtitlesGrid *grid;


	/// DOCME
	wxString currentWord;

	/// DOCME
	wxArrayString sugs;

	/// DOCME
	wxArrayString thesSugs;

	/// DOCME
	int currentWordPos;


	/// DOCME
	wxArrayString proto;

	/// DOCME
	int tipProtoN;

	void ShowPopupMenu(int activePos=-1);

	void OnMouseEvent(wxMouseEvent &event);
	void OnSplitLinePreserve(wxCommandEvent &event);
	void OnSplitLineEstimate(wxCommandEvent &event);
	void OnAddToDictionary(wxCommandEvent &event);
	void OnUseSuggestion(wxCommandEvent &event);
	void OnSetDicLanguage(wxCommandEvent &event);
	void OnSetThesLanguage(wxCommandEvent &event);
	void OnLoseFocus(wxFocusEvent &event);

public:
	SubsTextEditCtrl(wxWindow* parent, wxSize size, long style, SubtitlesGrid *grid);
	~SubsTextEditCtrl();

	void SetTextTo(wxString text);
	void UpdateStyle(int start=0,int length=-1);
	void StyleSpellCheck(int start=0,int length=-1);
	void UpdateCallTip(wxStyledTextEvent &);
	void SetStyles();

	DECLARE_EVENT_TABLE()
};
