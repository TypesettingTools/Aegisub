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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file colour_button.cpp
/// @brief Push-button that displays a colour for label, and brings up colour selection dialogue when pressed
/// @ingroup custom_control
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/dcmemory.h>
#endif

#include "colour_button.h"
#include "config.h"
#include "dialog_colorpicker.h"

ColourButton::ColourButton(wxWindow* parent, wxWindowID id, const wxSize& size, wxColour col)
: wxBitmapButton(parent, id, wxBitmap(size))
, bmp(GetBitmapLabel())
, colour(col)
{
	{
		wxMemoryDC dc;
		dc.SelectObject(bmp);
		dc.SetBrush(wxBrush(colour));
		dc.DrawRectangle(0,0,bmp.GetWidth(),bmp.GetHeight());
	}
	SetBitmapLabel(bmp);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ColourButton::OnClick, this);
}

ColourButton::~ColourButton() {
}

/// @brief Callback for the color picker dialog
/// @param col New color
void ColourButton::SetColour(wxColour col) {
	colour = col;

	// Draw colour
	{
		wxMemoryDC dc;
		dc.SelectObject(bmp);
		dc.SetBrush(wxBrush(colour));
		dc.DrawRectangle(0,0,bmp.GetWidth(),bmp.GetHeight());
	}
	SetBitmapLabel(bmp);

	// Trigger a click event on this as some stuff relies on that to know
	// when the color has changed
	wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	evt.SetClientData(this);
	AddPendingEvent(evt);
}

wxColour ColourButton::GetColour() {
	return colour;
}

/// @brief Click handler
/// @param event 
void ColourButton::OnClick(wxCommandEvent &event) {
	if (event.GetClientData() != this)
		GetColorFromUser<ColourButton, &ColourButton::SetColour>(GetParent(), colour, this);
	else
		event.Skip();
}
