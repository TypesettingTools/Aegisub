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

/// @file dialog_paste_over.h
/// @see dialog_paste_over.cpp
/// @ingroup secondary_ui
///


#pragma once


///////////
// Headers
#include <wx/dialog.h>
#include <wx/checklst.h>



/// DOCME
/// @class DialogPasteOver
/// @brief DOCME
///
/// DOCME
class DialogPasteOver : public wxDialog {
private:

	/// DOCME
	wxCheckListBox *ListBox;

	/// DOCME
	wxArrayInt options;

	void OnOK(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnTimes(wxCommandEvent &event);
	void OnText(wxCommandEvent &event);
	void OnAll(wxCommandEvent &event);
	void OnNone(wxCommandEvent &event);

public:
	DialogPasteOver(wxWindow *parent);
	~DialogPasteOver();

	wxArrayInt GetOptions();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	Paste_Over_Times = 1620,

	/// DOCME
	Paste_Over_Text,

	/// DOCME
	Paste_Over_All,

	/// DOCME
	Paste_Over_None
};


