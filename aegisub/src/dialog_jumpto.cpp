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
#include "utils.h"
#include "video_context.h"

DialogJumpTo::DialogJumpTo(agi::Context *c)
: wxDialog(c->parent, -1, _("Jump to"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS , "JumpTo")
, c(c)
, jumpframe(c->videoController->GetFrameN())
{
	SetIcon(BitmapToIcon(GETIMAGE(jumpto_button_24)));

	// Set initial values
	AssTime jumptime;
	jumptime.SetMS(c->videoController->TimeAtFrame(jumpframe));
	wxString maxLength = wxString::Format("%i",c->videoController->GetLength() - 1);

	// Times
	wxStaticText *LabelFrame = new wxStaticText(this,-1,_("Frame: "),wxDefaultPosition,wxSize(60,20));
	wxStaticText *LabelTime = new wxStaticText(this,-1,_("Time: "),wxDefaultPosition,wxSize(60,20));
	JumpFrame = new wxTextCtrl(this,-1,wxString::Format("%i",jumpframe),wxDefaultPosition,wxSize(60,20),wxTE_PROCESS_ENTER);
	JumpFrame->SetMaxLength(maxLength.size());
	JumpTime = new TimeEdit(this,-1,jumptime.GetASSFormated(),wxDefaultPosition,wxSize(60,20),wxTE_PROCESS_ENTER);
	wxSizer *FrameSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *TimeSizer = new wxBoxSizer(wxHORIZONTAL);
	FrameSizer->Add(LabelFrame,0,wxALIGN_CENTER_VERTICAL,0);
	FrameSizer->Add(JumpFrame,1,wxLEFT,5);
	TimeSizer->Add(LabelTime,0,wxALIGN_CENTER_VERTICAL,0);
	TimeSizer->Add(JumpTime,1,wxLEFT,5);
	wxSizer *TimesSizer = new wxStaticBoxSizer(wxVERTICAL, this, "");
	TimesSizer->Add(FrameSizer,0,wxEXPAND | wxBOTTOM,5);
	TimesSizer->Add(TimeSizer,0,wxEXPAND,0);

	// Buttons
	wxButton *OKButton = new wxButton(this, wxID_OK);
	wxButton *CancelButton = new wxButton(this, wxID_CANCEL);
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->Add(OKButton,1,wxRIGHT,5);
	ButtonSizer->Add(CancelButton,0,0,0);

	// General layout
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TimesSizer,0,wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);
	CenterOnParent();

	Bind(wxEVT_COMMAND_TEXT_ENTER, &DialogJumpTo::OnOK, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogJumpTo::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&DialogJumpTo::EndModal, this, 0), wxID_CANCEL);
	JumpTime->Bind(wxEVT_COMMAND_TEXT_UPDATED, &DialogJumpTo::OnEditTime, this);
	JumpFrame->Bind(wxEVT_COMMAND_TEXT_UPDATED, &DialogJumpTo::OnEditFrame, this);
}

void DialogJumpTo::OnOK(wxCommandEvent &) {
	EndModal(0);
	c->videoController->JumpToFrame(std::min<int>(jumpframe, c->videoController->GetLength() - 1));
}

void DialogJumpTo::OnEditTime (wxCommandEvent &) {
	long newframe = c->videoController->FrameAtTime(JumpTime->time.GetMS());
	if (jumpframe != newframe) {
		jumpframe = newframe;
		JumpFrame->ChangeValue(wxString::Format("%i", jumpframe));
	}
}

void DialogJumpTo::OnEditFrame (wxCommandEvent &event) {
	JumpFrame->GetValue().ToLong(&jumpframe);
	JumpFrame->ChangeValue(wxString::Format("%i", jumpframe));

	int newtime = c->videoController->TimeAtFrame(jumpframe);
	if (JumpTime->time.GetMS() != newtime) {
		JumpTime->time.SetMS(newtime);
		JumpTime->ChangeValue(JumpTime->time.GetASSFormated());
	}
}
