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


////////////
// Includes
#include "timeedit_ctrl.h"
#include "ass_time.h"
#include "vfr.h"
#include "options.h"


///////////////
// Constructor
TimeEdit::TimeEdit(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
wxTextCtrl(parent,id,value,pos,size,wxTE_CENTRE | style,validator,name)
{
	SetValue(time.GetASSFormated());
	ready = true;
	byFrame = false;
	isEnd = false;
	modified = false;
	showModified = false;
	time.UpdateFromTextCtrl(this);
	Connect(wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(TimeEdit::OnModified));
}


///////////////
// Event table
BEGIN_EVENT_TABLE(TimeEdit, wxTextCtrl)
	EVT_MOUSE_EVENTS(TimeEdit::OnMouseEvent)
	EVT_KEY_DOWN(TimeEdit::OnKeyDown)
END_EVENT_TABLE()


//////////////////
// Modified event
void TimeEdit::OnModified(wxCommandEvent &event) {
	// Lock
	if (!ready) return;
	ready = false;
	
	// Update
	if (byFrame) Update();
	else time.UpdateFromTextCtrl(this);

	// Colour
	if (showModified && !modified) {
		SetBackgroundColour(Options.AsColour(_T("Edit Box Need Enter Background")));
	}
	modified = true;

	// Done
	ready = true;
	event.Skip();
}


/////////////////////////////
// Set time and update stuff
void TimeEdit::SetTime(int ms) {
	time.SetMS(ms);
	UpdateText();
}


/////////////////////////////////////////
// Toggles between set by frame and time
void TimeEdit::SetByFrame(bool enable) {
	if (enable == byFrame) return;

	// By frames
	if (enable) {
		if (VFR_Output.GetFrameRateType() != NONE) {
			byFrame = true;
			UpdateText();
		}
	}

	// By actual time
	else {
		byFrame = false;
		UpdateText();
	}
}


/////////////////////////////////////
// Update text to reflect time value
void TimeEdit::UpdateText() {
	ready = false;
	if (byFrame) {
		int frame_n = VFR_Output.GetFrameAtTime(time.GetMS(),!isEnd);
		SetValue(wxString::Format(_T("%i"),frame_n));
	}
	else SetValue(time.GetASSFormated());
	ready = true;
}


///////////////
// Mouse event
void TimeEdit::OnMouseEvent(wxMouseEvent &event) {
	if (!byFrame) {
		long from=0,to=0;
		GetSelection(&from,&to);
		if (to != from) SetSelection(to,to);
	}
	event.Skip();
}


//////////
// Update
void TimeEdit::Update() {
	// Update frame
	if (byFrame) {
		long temp;
		GetValue().ToLong(&temp);
		time.SetMS(VFR_Output.GetTimeAtFrame(temp,!isEnd));
	}

	// Update modified status
	if (modified && showModified) {
		SetBackgroundColour(wxNullColour);
		Refresh();
	}
	modified = false;
}


///////////////
// Key pressed
void TimeEdit::OnKeyDown(wxKeyEvent &event) {
	int key = event.GetKeyCode();
	if (byFrame || (key != WXK_BACK && key != WXK_DELETE)) event.Skip();
}
