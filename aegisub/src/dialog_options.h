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

/// @file dialog_options.h
/// @see dialog_options.cpp
/// @ingroup configuration_ui
///




////////////
// Includes
#ifndef AGI_PRE
#include <map>
#include <vector>

#include <wx/dialog.h>
#include <wx/listctrl.h>
#endif

#include "hotkeys.h"
#include "options.h"


//////////////
// Prototypes
class FrameMain;
class DialogInputHotkey;
#if wxUSE_TREEBOOK
class wxTreebook;
#else
#include <wx/choicebk.h>

/// DOCME
typedef wxChoicebook wxTreebook;
#endif



/// DOCME
/// @class OptionsBind
/// @brief DOCME
///
/// DOCME
class OptionsBind {
public:

	/// DOCME
	wxControl *ctrl;

	/// DOCME
	wxString option;

	/// DOCME
	int param;
};



/// DOCME
enum TextType {

	/// DOCME
	TEXT_TYPE_PLAIN,

	/// DOCME
	TEXT_TYPE_NUMBER,

	/// DOCME
	TEXT_TYPE_FILE,

	/// DOCME
	TEXT_TYPE_FOLDER,

	/// DOCME
	TEXT_TYPE_FONT
};


/// DOCME
/// @class DialogOptions
/// @brief DOCME
///
/// DOCME
class DialogOptions: public wxDialog {
private:

	/// DOCME
	bool needsRestart;


	/// DOCME
	wxTreebook *book;

	/// DOCME
	std::vector<OptionsBind> binds;


	/// DOCME
	std::map<wxString,HotkeyType> origKeys;

	/// DOCME
	wxListView *Shortcuts;

	/// DOCME
	bool hotkeysModified;

	void Bind(wxControl *ctrl,wxString option,int param=0);
	void WriteToOptions(bool justApply=false);
	void ReadFromOptions();

	void AddTextControl(wxWindow *parent,wxSizer *sizer,wxString label,wxString option,TextType type=TEXT_TYPE_PLAIN);
	void AddComboControl(wxWindow *parent,wxSizer *sizer,wxString label,wxString option,wxArrayString choices,bool readOnly=true,int bindParam=0);
	void AddCheckBox(wxWindow *parent,wxSizer *sizer,wxString label,wxString option);

	void OnOK(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnApply(wxCommandEvent &event);
	void OnDefaults(wxCommandEvent &event);

	void OnEditHotkey(wxCommandEvent &event);
	void OnClearHotkey(wxCommandEvent &event);
	void OnDefaultHotkey(wxCommandEvent &event);
	void OnDefaultAllHotkey(wxCommandEvent &event);

public:
	DialogOptions(wxWindow *parent);
	~DialogOptions();

	DECLARE_EVENT_TABLE()
};




/// DOCME
/// @class DialogInputHotkey
/// @brief DOCME
///
/// DOCME
class DialogInputHotkey : public wxDialog {

	/// DOCME
	HotkeyType *key;

	/// DOCME
	wxListView *shortcuts;

	void OnKeyDown(wxKeyEvent &event);

public:
	DialogInputHotkey(wxWindow *parent, HotkeyType *key, wxString name, wxListView *Shortcuts);

	DECLARE_EVENT_TABLE()
};


