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
#include "config.h"

#include <wx/wxprec.h>
#include <wx/display.h>
#include <wx/dcclient.h>
#include "dialog_splash.h"
#include "options.h"
#include "dialog_tip.h"
#include "libresrc/libresrc.h"
#include "libresrc/icons.h"


///////////////
// Constructor
SplashScreen::SplashScreen(wxWindow *parent)
: wxFrame (parent, -1, _T(""), wxDefaultPosition, wxSize(420,240), wxSTAY_ON_TOP | wxFRAME_NO_TASKBAR , _T("Splash"))
{
	// Set parent
	par = parent;

	// Get splash
	splash = GETIMAGE(splash_misc);

	#if wxUSE_DISPLAY == 1
	// Center on current display
	if (wxDisplay::GetCount() < 1) CentreOnParent();
	else {
		// Get parent position
		wxRect parRect = parent->GetRect();

		// Get display number
		int point = wxDisplay::GetFromPoint(wxPoint(parRect.GetX() + parRect.GetWidth()/2,parRect.GetY() + parRect.GetHeight()/2));
		if (point == wxNOT_FOUND) point = 0;

		// Get display size
		wxDisplay display(point);
		wxRect dr = display.GetGeometry();

		// Calculate position and center it there
		wxRect window = GetScreenRect();
		window = window.CenterIn(dr);
		Move(window.GetLeft(),window.GetTop());
	}
	#else
	// Center on window
	CentreOnParent();
	#endif

	// Prepare
	wxClientDC dc(this);
	dc.DrawBitmap(splash,0,0);

	autoClose = new wxTimer(this,5000);
	autoClose->Start(5000,true);
}


//////////////
// Destructor
SplashScreen::~SplashScreen () {
	// Kill timer
	delete autoClose;
}


///////////////
// Event table
BEGIN_EVENT_TABLE(SplashScreen, wxFrame)
    EVT_MOUSE_EVENTS(SplashScreen::OnMouseEvent)
    EVT_PAINT(SplashScreen::OnPaint)
	EVT_TIMER(5000,SplashScreen::OnTimer)
END_EVENT_TABLE()


///////////
// OnPaint
void SplashScreen::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);
	dc.DrawBitmap(splash,0,0);
}


////////////////
// Mouse events
void SplashScreen::OnMouseEvent(wxMouseEvent& event) {
	if (event.ButtonDown()) {
		// Show tip of the day
		Destroy();
		TipOfTheDay::Show(par);
	}
}


/////////
// Timer
void SplashScreen::OnTimer(wxTimerEvent &event) {
	// Show tip of the day
	Destroy();
	TipOfTheDay::Show(par);
}
