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

#include "ass_time.h"
#include "async_video_provider.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "project.h"
#include "timeedit_ctrl.h"
#include "validators.h"
#include "video_controller.h"

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace {
class DialogJumpTo final : public wxDialog {
	agi::Context *c;       ///< Project context
	long jumpframe;        ///< Target frame to jump to
	TimeEdit *JumpTime;    ///< Target time edit control
	wxTextCtrl *JumpFrame; ///< Target frame edit control

	/// Enter/OK button handler
	void OnOK(wxCommandEvent &event);
	/// Update target frame on target time changed
	void OnEditTime(wxCommandEvent &event);
	/// Update target time on target frame changed
	void OnEditFrame(wxCommandEvent &event);
	/// Dialog initializer to set default focus and selection
	void OnInitDialog(wxInitDialogEvent&);

public:
	DialogJumpTo(agi::Context *c);
};

DialogJumpTo::DialogJumpTo(agi::Context *c)
: wxDialog(c->parent, -1, _("Jump to"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS)
, c(c)
, jumpframe(c->videoController->GetFrameN())
{
	SetIcon(GETICON(jumpto_button_16));

	auto LabelFrame = new wxStaticText(this, -1, _("Frame: "));
	auto LabelTime = new wxStaticText(this, -1, _("Time: "));

	JumpFrame = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(-1,-1),wxTE_PROCESS_ENTER, IntValidator((int)jumpframe));
	JumpFrame->SetMaxLength(std::to_string(c->project->VideoProvider()->GetFrameCount() - 1).size());
	JumpTime = new TimeEdit(this, -1, c, AssTime(c->videoController->TimeAtFrame(jumpframe)).GetAssFormated(), wxSize(-1,-1));

	auto TimesSizer = new wxGridSizer(2, 5, 5);

	TimesSizer->Add(LabelFrame, 1, wxALIGN_CENTER_VERTICAL);
	TimesSizer->Add(JumpFrame, wxEXPAND);

	TimesSizer->Add(LabelTime, 1, wxALIGN_CENTER_VERTICAL);
	TimesSizer->Add(JumpTime, wxEXPAND);

	auto ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);

	// General layout
	auto MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TimesSizer, 0, wxALL | wxALIGN_CENTER, 5);
	MainSizer->Add(ButtonSizer, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 5);
	SetSizerAndFit(MainSizer);
	CenterOnParent();

	Bind(wxEVT_INIT_DIALOG, &DialogJumpTo::OnInitDialog, this);
	Bind(wxEVT_TEXT_ENTER, &DialogJumpTo::OnOK, this);
	Bind(wxEVT_BUTTON, &DialogJumpTo::OnOK, this, wxID_OK);
	JumpTime->Bind(wxEVT_TEXT, &DialogJumpTo::OnEditTime, this);
	JumpFrame->Bind(wxEVT_TEXT, &DialogJumpTo::OnEditFrame, this);
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
	c->videoController->JumpToFrame(jumpframe);
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
}

void ShowJumpToDialog(agi::Context *c) {
	DialogJumpTo(c).ShowModal();
}
