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
//
// $Id$

/// @file dialog_text_import.cpp
/// @brief Set-up import from plain text files
/// @ingroup secondary_ui
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/sizer.h>
#include <wx/stattext.h>
#endif

#include "compat.h"
#include "dialog_text_import.h"
#include "main.h"
#include "options.h"


/// @brief Constructor 
///
DialogTextImport::DialogTextImport()
: wxDialog(NULL , -1, _("Text import options"),wxDefaultPosition,wxDefaultSize)
{
	// Main controls
	wxFlexGridSizer *fg = new wxFlexGridSizer(2, 5, 5);
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	edit_separator = new wxTextCtrl(this, EDIT_ACTOR_SEPARATOR, lagi_wxString(OPT_GET("Tool/Import/Text/Actor Separator")->GetString()));
	edit_comment = new wxTextCtrl(this, EDIT_COMMENT_STARTER, lagi_wxString(OPT_GET("Tool/Import/Text/Comment Starter")->GetString()));

	// Dialog layout
	fg->Add(new wxStaticText(this, -1, _("Actor separator:")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(edit_separator, 0, wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Comment starter:")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(edit_comment, 0, wxEXPAND);

	main_sizer->Add(fg, 1, wxALL|wxEXPAND, 5);
	main_sizer->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND, 5);
	main_sizer->SetSizeHints(this);
	SetSizer(main_sizer);
}


/// @brief Destructor 
///
DialogTextImport::~DialogTextImport()
{
}


/// @brief DOCME
/// @param event 
///
void DialogTextImport::OnOK(wxCommandEvent &event)
{
	// Set options
	OPT_SET("Tool/Import/Text/Actor Separator")->SetString(STD_STR(edit_separator->GetValue()));
	OPT_SET("Tool/Import/Text/Comment Starter")->SetString(STD_STR(edit_comment->GetValue()));

	EndModal(wxID_OK);
}


/// @brief DOCME
/// @param event 
///
void DialogTextImport::OnCancel(wxCommandEvent &event)
{
	EndModal(wxID_CANCEL);
}

///////////////
// Event table
BEGIN_EVENT_TABLE(DialogTextImport,wxDialog)
	EVT_BUTTON(wxID_OK,DialogTextImport::OnOK)
	EVT_BUTTON(wxID_CANCEL,DialogTextImport::OnCancel)
END_EVENT_TABLE()


