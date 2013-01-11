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

/// @file dialog_search_replace.h
/// @see dialog_search_replace.cpp
/// @ingroup secondary_ui
///

#include <wx/dialog.h>
#include <wx/string.h>

namespace agi { struct Context; }
class wxCheckBox;
class wxComboBox;
class wxRadioBox;

class SearchReplaceEngine {
	int curLine;
	size_t pos;
	size_t matchLen;
	size_t replaceLen;
	bool LastWasFind;
	bool hasReplace;
	bool isReg;
	bool matchCase;
	bool CanContinue;
	bool hasFocus;
	int field;
	int affect;
	wxString LookFor;
	wxString ReplaceWith;

public:
	agi::Context *context;

	void FindNext();
	void ReplaceNext(bool DoReplace=true);
	void ReplaceAll();
	void OpenDialog(bool HasReplace);
	void OnDialogOpen();

	void SetFocus(bool focus) { hasFocus = focus; }
	bool HasFocus() const { return hasFocus; }

	SearchReplaceEngine();
	friend class DialogSearchReplace;
};

// Instance
extern SearchReplaceEngine Search;

class DialogSearchReplace : public wxDialog {
	friend class SearchReplaceEngine;

	bool hasReplace;

	wxComboBox *FindEdit;
	wxComboBox *ReplaceEdit;
	wxCheckBox *CheckMatchCase;
	wxCheckBox *CheckRegExp;
	wxCheckBox *CheckUpdateVideo;
	wxRadioBox *Affect;
	wxRadioBox *Field;

	void UpdateDropDowns();
	void FindReplace(int mode);	// 0 = find, 1 = replace next, 2 = replace all

public:
	DialogSearchReplace(agi::Context* c, bool withReplace);
	~DialogSearchReplace();
	void UpdateSettings();
};
