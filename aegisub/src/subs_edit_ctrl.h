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


////////////
// Includes
#include "scintilla_text_ctrl.h"
#include "spellchecker_manager.h"
#include "thesaurus.h"


//////////////
// Prototypes
class SubsEditBox;



/// DOCME
/// @class SubsTextEditCtrl
/// @brief DOCME
///
/// DOCME
class SubsTextEditCtrl : public ScintillaTextCtrl {
private:

	/// DOCME
	SpellChecker *spellchecker;

	/// DOCME
	Thesaurus *thesaurus;


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
	void OnCut(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);
	void OnPaste(wxCommandEvent &event);
	void OnUndo(wxCommandEvent &event);
	void OnSelectAll(wxCommandEvent &event);
	void OnAddToDictionary(wxCommandEvent &event);
	void OnUseSuggestion(wxCommandEvent &event);
	void OnUseThesaurusSuggestion(wxCommandEvent &event);
	void OnSetDicLanguage(wxCommandEvent &event);
	void OnSetThesLanguage(wxCommandEvent &event);
	void OnLoseFocus(wxFocusEvent &event);

public:

	/// DOCME
	SubsEditBox *control;

	SubsTextEditCtrl(wxWindow* parent, wxWindowID id, const wxString& value = _T(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr);
	~SubsTextEditCtrl();

	void SetTextTo(const wxString text);
	void UpdateStyle(int start=0,int length=-1);
	void StyleSpellCheck(int start=0,int length=-1);
	void UpdateCallTip();
	void SetStyles();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	EDIT_MENU_SPLIT_PRESERVE = 1400,

	/// DOCME
	EDIT_MENU_SPLIT_ESTIMATE,

	/// DOCME
	EDIT_MENU_CUT,

	/// DOCME
	EDIT_MENU_COPY,

	/// DOCME
	EDIT_MENU_PASTE,

	/// DOCME
	EDIT_MENU_UNDO,

	/// DOCME
	EDIT_MENU_SELECT_ALL,

	/// DOCME
	EDIT_MENU_ADD_TO_DICT,

	/// DOCME
	EDIT_MENU_SUGGESTION,

	/// DOCME
	EDIT_MENU_SUGGESTIONS,

	/// DOCME
	EDIT_MENU_THESAURUS = 1450,

	/// DOCME
	EDIT_MENU_THESAURUS_SUGS,

	/// DOCME
	EDIT_MENU_DIC_LANGUAGE = 1600,

	/// DOCME
	EDIT_MENU_DIC_LANGS,

	/// DOCME
	EDIT_MENU_THES_LANGUAGE = 1700,

	/// DOCME
	EDIT_MENU_THES_LANGS
};


