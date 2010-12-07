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

/// @file dialog_search_replace.h
/// @see dialog_search_replace.cpp
/// @ingroup secondary_ui
///




///////////
// Headers
#ifndef AGI_PRE
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#endif


//////////////
// Prototypes
class SubtitlesGrid;



/// DOCME
/// @class SearchReplaceEngine
/// @brief DOCME
///
/// DOCME
class SearchReplaceEngine {
private:

	/// DOCME
	int curLine;

	/// DOCME
	size_t pos;

	/// DOCME
	size_t matchLen;

	/// DOCME
	size_t replaceLen;

	/// DOCME
	bool Modified;

	/// DOCME
	bool LastWasFind;

	/// DOCME
	bool hasReplace;

	wxString *GetText(int n,int field);
	
public:

	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	bool isReg;

	/// DOCME
	bool matchCase;

	/// DOCME
	bool updateVideo;

	/// DOCME
	bool CanContinue;

	/// DOCME
	bool hasFocus;

	/// DOCME
	int field;

	/// DOCME
	int affect;

	/// DOCME
	wxString LookFor;

	/// DOCME
	wxString ReplaceWith;

	void FindNext();
	void ReplaceNext(bool DoReplace=true);
	void ReplaceAll();
	void OpenDialog(bool HasReplace);
	void OnDialogOpen();

	SearchReplaceEngine();
	friend class DialogSearchReplace;
};

// Instance
extern SearchReplaceEngine Search;



/// DOCME
/// @class DialogSearchReplace
/// @brief DOCME
///
/// DOCME
class DialogSearchReplace : public wxDialog {
	friend class SearchReplaceEngine;

private:

	/// DOCME
	bool hasReplace;


	/// DOCME
	wxComboBox *FindEdit;

	/// DOCME
	wxComboBox *ReplaceEdit;

	/// DOCME
	wxCheckBox *CheckMatchCase;

	/// DOCME
	wxCheckBox *CheckRegExp;

	/// DOCME
	wxCheckBox *CheckUpdateVideo;

	/// DOCME
	wxRadioBox *Affect;

	/// DOCME
	wxRadioBox *Field;

	void UpdateDropDowns();
	void FindReplace(int mode);	// 0 = find, 1 = replace next, 2 = replace all

	void OnClose (wxCommandEvent &event);
	void OnFindNext (wxCommandEvent &event);
	void OnReplaceNext (wxCommandEvent &event);
	void OnReplaceAll (wxCommandEvent &event);
	void OnSetFocus (wxFocusEvent &event);
	void OnKillFocus (wxFocusEvent &event);
	void OnKeyDown (wxKeyEvent &event);

public:
	DialogSearchReplace(wxWindow *parent,bool hasReplace,wxString name);
	~DialogSearchReplace();
	void UpdateSettings();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	BUTTON_FIND_NEXT,

	/// DOCME
	BUTTON_REPLACE_NEXT,

	/// DOCME
	BUTTON_REPLACE_ALL,

	/// DOCME
	CHECK_MATCH_CASE,

	/// DOCME
	CHECK_REGEXP,

	/// DOCME
	CHECK_UPDATE_VIDEO
};


