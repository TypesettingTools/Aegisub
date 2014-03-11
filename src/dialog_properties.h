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

/// @file dialog_properties.h
/// @see dialog_properties.cpp
/// @ingroup secondary_ui
///

#include <vector>
#include <wx/dialog.h>

class AssFile;
namespace agi { struct Context; }
class wxCheckBox;
class wxComboBox;
class wxTextCtrl;

class DialogProperties : public wxDialog {
	agi::Context *c; ///< Project this dialog is adjusting the properties of

	/// Pairs of a script property and a text control for that property
	std::vector<std::pair<std::string, wxTextCtrl*>> properties;

	// Things that effect rendering
	wxComboBox *WrapStyle;   ///< Wrapping style for long lines
	wxTextCtrl *ResX;        ///< Script x resolution
	wxTextCtrl *ResY;        ///< Script y resolution
	wxCheckBox *ScaleBorder; ///< If script resolution != video resolution how should borders be handled

	/// OK button handler
	void OnOK(wxCommandEvent &event);
	/// Set script resolution to video resolution button
	void OnSetFromVideo(wxCommandEvent &event);
	/// Set a script info field
	/// @param key Name of field
	/// @param value New value
	/// @return Did the value actually need to be changed?
	int SetInfoIfDifferent(std::string const& key, std::string const& value);

	/// Add a property with label and text box for updating the property
	/// @param sizer Sizer to add the label and control to
	/// @param label Label text to use
	/// @param property Script info property name
	void AddProperty(wxSizer *sizer, wxString const& label, std::string const& property);

public:
	/// Constructor
	/// @param c Project context
	DialogProperties(agi::Context *c);
};
