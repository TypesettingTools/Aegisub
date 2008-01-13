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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include "dialog_jumpto.h"
#include "vfr.h"
#include "video_context.h"
#include "utils.h"


///////
// IDs
enum {
	TEXT_JUMP_TIME = 1100,
	TEXT_JUMP_FRAME
};


///////////////
// Constructor
DialogJumpTo::DialogJumpTo (wxWindow *parent)
: wxDialog(parent, -1, _("Jump to"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS , _T("JumpTo"))
{
	// Set icon
	SetIcon(BitmapToIcon(wxBITMAP(jumpto_button)));

	// Set initial values
	ready = false;
	jumpframe = VideoContext::Get()->GetFrameN();
	jumptime.SetMS(VFR_Output.GetTimeAtFrame(jumpframe));

	// Times
	wxStaticText *LabelFrame = new wxStaticText(this,-1,_("Frame: "),wxDefaultPosition,wxSize(60,20));
	wxStaticText *LabelTime = new wxStaticText(this,-1,_("Time: "),wxDefaultPosition,wxSize(60,20));
	JumpFrame = new wxTextCtrl(this,TEXT_JUMP_FRAME,wxString::Format(_T("%i"),jumpframe),wxDefaultPosition,wxSize(60,20),wxTE_PROCESS_ENTER);
	JumpTime = new TimeEdit(this,TEXT_JUMP_TIME,jumptime.GetASSFormated(),wxDefaultPosition,wxSize(60,20),wxTE_PROCESS_ENTER);
	wxSizer *FrameSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *TimeSizer = new wxBoxSizer(wxHORIZONTAL);
	FrameSizer->Add(LabelFrame,0,wxALIGN_CENTER_VERTICAL,0);
	FrameSizer->Add(JumpFrame,1,wxLEFT,5);
	TimeSizer->Add(LabelTime,0,wxALIGN_CENTER_VERTICAL,0);
	TimeSizer->Add(JumpTime,1,wxLEFT,5);
	wxSizer *TimesSizer = new wxStaticBoxSizer(wxVERTICAL, this, _T(""));
	TimesSizer->Add(FrameSizer,0,wxEXPAND | wxBOTTOM,5);
	TimesSizer->Add(TimeSizer,0,wxEXPAND,0);

	// Buttons
	wxButton *OKButton = new wxButton(this,wxID_OK);
	wxButton *CancelButton = new wxButton(this,wxID_CANCEL);
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
	ready = true;
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogJumpTo, wxDialog)
	EVT_TEXT_ENTER(TEXT_JUMP_FRAME,DialogJumpTo::OnKey)
	EVT_TEXT_ENTER(TEXT_JUMP_TIME,DialogJumpTo::OnKey)
	EVT_BUTTON(wxID_CANCEL,DialogJumpTo::OnCloseButton)
	EVT_BUTTON(wxID_OK,DialogJumpTo::OnOK)
	EVT_TEXT(TEXT_JUMP_TIME, DialogJumpTo::OnEditTime)
	EVT_TEXT(TEXT_JUMP_FRAME, DialogJumpTo::OnEditFrame)
END_EVENT_TABLE()


/////////
// Close
void DialogJumpTo::OnCloseButton (wxCommandEvent &event) { OnClose(false); }
void DialogJumpTo::OnOK (wxCommandEvent &event) { OnClose(true); }


//////////////////
// On Key pressed
void DialogJumpTo::OnKey(wxCommandEvent &event) {
	EndModal(0);
	VideoContext::Get()->JumpToFrame(jumpframe);
}


////////////////////////
// On OK button pressed
void DialogJumpTo::OnClose(bool ok) {
	EndModal(0);
	if (ok)	VideoContext::Get()->JumpToFrame(jumpframe);
}


////////////////////////
// Time editbox changed
void DialogJumpTo::OnEditTime (wxCommandEvent &event) {
	if (ready) {
		ready = false;

		// Update frame
		long newframe = VFR_Output.GetFrameAtTime(JumpTime->time.GetMS());
		if (jumpframe != newframe) {
			jumpframe = newframe;
			JumpFrame->SetValue(wxString::Format(_T("%i"),jumpframe));
		}

		ready = true;
	}
	else event.Skip();
}


/////////////////////////
// Frame editbox changed
void DialogJumpTo::OnEditFrame (wxCommandEvent &event) {
	if (ready) {
		ready = false;

		// Update frame
		JumpFrame->GetValue().ToLong(&jumpframe);
		JumpFrame->SetValue(wxString::Format(_T("%i"),jumpframe));

		// Update time
		int newtime = VFR_Output.GetTimeAtFrame(jumpframe);
		if (jumptime.GetMS() != newtime) {
			jumptime.SetMS(newtime);
			JumpTime->SetValue(jumptime.GetASSFormated());
		}

		ready = true;
	}
	else event.Skip();
}
