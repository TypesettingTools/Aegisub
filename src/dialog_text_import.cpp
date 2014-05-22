// Copyright (c) 2007, Alysson Souza e Silva
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

#include "options.h"
#include "validators.h"

#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>

namespace {
/// @class DialogTextImport
/// @brief Plain text import separator character selection dialog
///
/// A simple dialog to let the user select the format of a plain text file
/// being imported into Aegisub
class DialogTextImport final : public wxDialog {
	std::string seperator;
	std::string comment;
	bool include_blank;

public:
	DialogTextImport();
};

DialogTextImport::DialogTextImport()
: wxDialog(nullptr , -1, _("Text import options"))
, seperator(OPT_GET("Tool/Import/Text/Actor Separator")->GetString())
, comment(OPT_GET("Tool/Import/Text/Comment Starter")->GetString())
, include_blank(OPT_GET("Tool/Import/Text/Include Blank")->GetBool())
{
	auto make_text_ctrl = [=](std::string *var) {
		return new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, 0, StringBinder(var));
	};

	auto fg = new wxFlexGridSizer(2, 5, 5);
	fg->Add(new wxStaticText(this, -1, _("Actor separator:")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(make_text_ctrl(&seperator), 0, wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Comment starter:")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(make_text_ctrl(&comment), 0, wxEXPAND);

	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(fg, 1, wxALL|wxEXPAND, 5);
	main_sizer->Add(new wxCheckBox(this, -1, _("Include blank lines"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&include_blank)), 0, wxLEFT|wxRIGHT|wxALIGN_RIGHT, 5);
	main_sizer->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND, 5);
	SetSizerAndFit(main_sizer);

	Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
		TransferDataFromWindow();

		OPT_SET("Tool/Import/Text/Actor Separator")->SetString(seperator);
		OPT_SET("Tool/Import/Text/Comment Starter")->SetString(comment);
		OPT_SET("Tool/Import/Text/Include Blank")->SetBool(include_blank);

		EndModal(wxID_OK);
	}, wxID_OK);
}
}

bool ShowPlainTextImportDialog() {
	return DialogTextImport().ShowModal() == wxID_OK;
}
