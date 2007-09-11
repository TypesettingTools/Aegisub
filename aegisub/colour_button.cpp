// Copyright (c) 2006, Rodrigo Braz Monteiro
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
#include <wx/dcmemory.h>
#include "colour_button.h"
#include "dialog_colorpicker.h"


///////////////
// Constructor
ColourButton::ColourButton(wxWindow* parent, wxWindowID id, const wxSize& size, wxColour col) {
	// Variables
	linkColour = NULL;

	// Create base
	wxBitmapButton::Create(parent,id,wxBitmap(size.GetWidth(),size.GetHeight()),wxDefaultPosition,wxDefaultSize,wxBU_AUTODRAW);
	bmp = GetBitmapLabel();

	// Set colour
	SetColour(col);

	// Connect to click event
	Connect(GetId(),wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(ColourButton::OnClick));
}


//////////////
// Destructor
ColourButton::~ColourButton() {
}


//////////////
// Set colour
void ColourButton::SetColour(wxColour col) {
	// Set colour
	colour = col;

	// Draw colour
	{
		wxMemoryDC dc;
		dc.SelectObject(bmp);
		dc.SetBrush(wxBrush(colour));
		dc.DrawRectangle(0,0,bmp.GetWidth(),bmp.GetHeight());
	}

	// Set bitmap
	SetBitmapLabel(bmp);

	// Set link colour
	if (linkColour) *linkColour = colour;
}


//////////////
// Get Colour
wxColour ColourButton::GetColour() {
	return colour;
}


/////////
// Click
void ColourButton::OnClick(wxCommandEvent &event) {
	DialogColorPicker dlg(GetParent(), colour);
	if (dlg.ShowModal() == wxID_OK) {
		SetColour(dlg.GetColor());
	}
	event.Skip();
}


///////////////////
// Set Link Colour
void ColourButton::SetLinkColour(wxColour *col) {
	linkColour = col;
	if (linkColour) SetColour(*linkColour);
}
