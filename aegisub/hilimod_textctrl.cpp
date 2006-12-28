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
#include "hilimod_textctrl.h"
#include "options.h"


///////////////
// Constructor
HiliModTextCtrl::HiliModTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
wxTextCtrl(parent,id,value,pos,size,style,validator,name)
{
	UpdateLocked = false;
	isModified = false;
	orig = GetValue();

	Connect(wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(HiliModTextCtrl::OnModified));
}


//////////////////
// Modified event
void HiliModTextCtrl::OnModified(wxCommandEvent &event) {
	if (UpdateLocked) return;
	Modified();
	event.Skip();
}


//////////////////
// Commited event
void HiliModTextCtrl::Commited() {
	if (isModified) {
		orig = GetValue();
		SetBackgroundColour(wxNullColour);
		Refresh(false);
		isModified = false;
	}
}


/////////////
// Set value
void HiliModTextCtrl::SetValue(const wxString& value) {
	UpdateLocked = true;
	orig = value;
	wxTextCtrl::SetValue(value);
	Commited();
	UpdateLocked = false;
}


////////////////
// Was modified
void HiliModTextCtrl::Modified() {
	bool match = GetValue() == orig;

	// Different from original
	if (!isModified && !match) {
		isModified = true;
		SetBackgroundColour(Options.AsColour(_T("Edit Box Need Enter Background")));
		Refresh(false);
	}

	// Same as original
	if (isModified && match) {
		SetBackgroundColour(wxNullColour);
		Refresh(false);
		isModified = false;
	}
}
