// Copyright (c) 2007, Rodrigo Braz Monteiro
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
//   * Neither the name of the TrayDict Group nor the names of its contributors
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
// TRAYDICT
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "gecko_display.h"
#include "gecko_controller.h"
#include "main.h"


///////////////
// Constructor
GeckoDisplay::GeckoDisplay(wxWindow *parent)
: wxPanel(parent)
{
	controller = NULL;
	controller = new GeckoController(this,TrayDict::folderName);
	controller->AddRef();
}


//////////////
// Destructor
GeckoDisplay::~GeckoDisplay()
{
	controller->Release();
	//delete controller;
}


////////////////////
// Initialize gecko
void GeckoDisplay::InitGecko()
{
}


///////////////
// Append text
void GeckoDisplay::AppendText(wxString text)
{

}


////////////
// Set text
void GeckoDisplay::SetText(wxString text)
{

}


///////////////
// Event table
BEGIN_EVENT_TABLE(GeckoDisplay,wxPanel)
	EVT_SIZE(GeckoDisplay::OnSize)
	EVT_SET_FOCUS(GeckoDisplay::OnSetFocus)
	EVT_KILL_FOCUS(GeckoDisplay::OnKillFocus)
END_EVENT_TABLE()


////////
// Size
void GeckoDisplay::OnSize(wxSizeEvent &event)
{
	if (controller) controller->SetSize(event.GetSize());
}


/////////
// Focus
void GeckoDisplay::OnSetFocus(wxFocusEvent &event)
{
}

void GeckoDisplay::OnKillFocus(wxFocusEvent &event)
{
}
