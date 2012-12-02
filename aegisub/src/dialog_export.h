// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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

/// @file dialog_export.h
/// @see dialog_export.cpp
/// @ingroup export
///

#include <map>

#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include <libaegisub/scoped_ptr.h>

class AssExporter;
namespace agi { struct Context; }

class DialogExport : public wxDialog {
	agi::Context *c;

	/// The export transform engine
	agi::scoped_ptr<AssExporter> exporter;

	/// The description of the currently selected export filter
	wxTextCtrl *filter_description;

	/// A list of all registered export filters
	wxCheckListBox *filter_list;

	/// A list of available target charsets
	wxChoice *charset_list;

	wxSizer *opt_sizer;

	void OnProcess(wxCommandEvent &);
	void OnMoveUp(wxCommandEvent &);
	void OnMoveDown(wxCommandEvent &);
	void OnSelectAll(wxCommandEvent &);
	void OnSelectNone(wxCommandEvent &);
	void OnCheck(wxCommandEvent &);
	void OnChange(wxCommandEvent &);

	/// Set all the checkboxes
	void SetAll(bool new_value);
	/// Update which options sizers are shown
	void RefreshOptions();

public:
	DialogExport(agi::Context *c);
	~DialogExport();
};
