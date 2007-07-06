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
#include "toggle_bitmap.h"
#include <wx/wxprec.h>
#include <wx/tglbtn.h>


///////////////
// Constructor
ToggleBitmap::ToggleBitmap(wxWindow *parent,wxWindowID id,const wxBitmap &image,const wxSize &size)
: wxControl (parent,id,wxDefaultPosition,wxDefaultSize,wxSUNKEN_BORDER)
{
	// Set variables
	img = image;
	state = false;

	// Set size
	int w,h;
	if (size.GetWidth() != -1) w = size.GetWidth();
	else w = img.GetWidth();
	if (size.GetHeight() != -1) h = size.GetHeight();
	else h = img.GetHeight();
	SetClientSize(w,h);
	GetSize(&w,&h);
	SetSizeHints(w,h,w,h);
}


/////////////
// Get state
bool ToggleBitmap::GetValue() {
	return state;
}


/////////////
// Set state
void ToggleBitmap::SetValue(bool _state) {
	// Set flag
	state = _state;

	// Refresh
	Refresh(false);
}


//////////////
// Draw image
void ToggleBitmap::DrawImage(wxDC &dc) {
	// Get size
	int w,h;
	GetClientSize(&w,&h);

	// Get background color
	wxColour bgColor;
	if (state) bgColor = wxColour(0,255,0);
	else bgColor = wxColour(255,0,0);
	wxColor sysCol = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT);
	int r,g,b;
	r = (sysCol.Red() + bgColor.Red()) / 2;
	g = (sysCol.Green() + bgColor.Green()) / 2;
	b = (sysCol.Blue() + bgColor.Blue()) / 2;
	bgColor.Set(r,g,b);

	// Draw background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(bgColor));
	dc.DrawRectangle(0,0,w,h);
	
	// Draw bitmap
	dc.DrawBitmap(img,(w-img.GetWidth())/2,(h-img.GetHeight())/2,true);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(ToggleBitmap,wxControl)
    EVT_MOUSE_EVENTS(ToggleBitmap::OnMouseEvent)
    EVT_PAINT(ToggleBitmap::OnPaint)
END_EVENT_TABLE()


////////////////
// Mouse events
void ToggleBitmap::OnMouseEvent(wxMouseEvent &event) {
	// Get mouse position
	int x = event.GetX();
	int y = event.GetY();
	int w,h;
	GetClientSize(&w,&h);
	bool inside = true;
	if (x < 0 || y < y || x >= w || y >= h) inside = false;

	// Left click to toggle state
	if (inside && event.ButtonDown(wxMOUSE_BTN_LEFT)) {
		// Update
		state = !state;
		Refresh(false);

		// Send event
		wxCommandEvent sendEvent(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,GetId());
		AddPendingEvent(sendEvent);
	}
}


///////////////
// Paint event
void ToggleBitmap::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	DrawImage(dc);
}
