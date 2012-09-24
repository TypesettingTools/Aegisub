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

/// @file dialog_jumpto.cpp
/// @brief Dialogue box to enter a time to seek video to
/// @ingroup secondary_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#endif

#include "dialog_jumpto.h"

#include "include/aegisub/context.h"
#include "ass_time.h"
#include "libresrc/libresrc.h"
#include "timeedit_ctrl.h"
#include "validators.h"
#include "video_context.h"

DialogJumpTo::DialogJumpTo(agi::Context *c)
: wxDialog(c->parent, -1, _("Jump to"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS)
, c(c)
, jumpframe(c->videoController->GetFrameN())
{
	SetIcon(GETICON(jumpto_button_16));

	// Set initial values
	wxString maxLength = wxString::Format("%i",c->videoController->GetLength() - 1);

	// Times
	wxStaticText *LabelFrame = new wxStaticText(this,-1,_("Frame: "));
	wxStaticText *LabelTime = new wxStaticText(this,-1,_("Time: "));

	JumpFrame = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(-1,-1),wxTE_PROCESS_ENTER, NumValidator((int)jumpframe));
	JumpFrame->SetMaxLength(maxLength.size());
	JumpTime = new TimeEdit(this, -1, c, AssTime(c->videoController->TimeAtFrame(jumpframe)).GetAssFormated(), wxSize(-1,-1));

	wxGridSizer *TimesSizer = new wxGridSizer(2, 5, 5);

	TimesSizer->Add(LabelFrame, 1, wxALIGN_CENTER_VERTICAL);
	TimesSizer->Add(JumpFrame, wxEXPAND);

	TimesSizer->Add(LabelTime, 1, wxALIGN_CENTER_VERTICAL);
	TimesSizer->Add(JumpTime, wxEXPAND);

	// Buttons
	wxStdDialogButtonSizer *ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);

	// General layout
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TimesSizer, 0, wxALL | wxALIGN_CENTER, 5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT,5);

	// Set sizer
	SetSizerAndFit(MainSizer);
	CenterOnParent();

	Bind(wxEVT_INIT_DIALOG, &DialogJumpTo::OnInitDialog, this);
	Bind(wxEVT_COMMAND_TEXT_ENTER, &DialogJumpTo::OnOK, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogJumpTo::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogJumpTo::EndModal, this, 0), wxID_CANCEL);
	JumpTime->Bind(wxEVT_COMMAND_TEXT_UPDATED, &DialogJumpTo::OnEditTime, this);
	JumpFrame->Bind(wxEVT_COMMAND_TEXT_UPDATED, &DialogJumpTo::OnEditFrame, this);
}

void DialogJumpTo::OnInitDialog(wxInitDialogEvent&) {
	TransferDataToWindow();
	UpdateWindowUI(wxUPDATE_UI_RECURSE);

	// This can't simply be done in the constructor as the value hasn't been set yet
	JumpFrame->SetFocus();
	JumpFrame->SelectAll();
}

void DialogJumpTo::OnOK(wxCommandEvent &) {
	EndModal(0);
	c->videoController->JumpToFrame(std::min<int>(jumpframe, c->videoController->GetLength() - 1));
}

void DialogJumpTo::OnEditTime (wxCommandEvent &) {
	long newframe = c->videoController->FrameAtTime(JumpTime->GetTime());
	if (jumpframe != newframe) {
		jumpframe = newframe;
		JumpFrame->ChangeValue(wxString::Format("%li", jumpframe));
	}
}

void DialogJumpTo::OnEditFrame (wxCommandEvent &event) {
	JumpFrame->GetValue().ToLong(&jumpframe);
	JumpTime->SetTime(c->videoController->TimeAtFrame(jumpframe));
}
