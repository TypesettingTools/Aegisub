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


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/taskbar.h>
#include "systray.h"
#include "dict_window.h"


///////////////
// Constructor
Systray::Systray(wxFrame *mast) {
	master = mast;
	SetIcon(wxICON(wxicon),_T("TrayDict"));
}


//////////////
// Destructor
Systray::~Systray() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(Systray,wxTaskBarIcon)
	EVT_TASKBAR_LEFT_UP(Systray::OnLeftClick)
	EVT_MENU(SYSTRAY_OPEN,Systray::OnOpen)
	EVT_MENU(SYSTRAY_EXIT,Systray::OnExit)
END_EVENT_TABLE()


//////////////
// Left click
void Systray::OnLeftClick(wxTaskBarIconEvent &event) {
	if (master->IsShown()) master->Hide();
	else BringUp();
}


////////
// Open
void Systray::OnOpen(wxCommandEvent &event) {
	if (master->IsShown()) master->Hide();
	else BringUp();
}


////////
// Exit
void Systray::OnExit(wxCommandEvent &event) {
	master->Destroy();
}


/////////
// Popup
wxMenu* Systray::CreatePopupMenu() {
	wxMenu *popup = new wxMenu();
	if (master->IsShown()) popup->Append(SYSTRAY_OPEN,_T("Hide"));
	else popup->Append(SYSTRAY_OPEN,_T("Show"));
	popup->Append(SYSTRAY_EXIT,_T("Exit"));
	return popup;
}


////////////
// Bring up
void Systray::BringUp() {
	//if (IsOk()) RemoveIcon();
	master->Show();
	master->Restore();
	master->Raise();
	((DictWindow*)master)->entry->SetFocus();
}
