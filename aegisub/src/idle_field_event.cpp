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

/// @file idle_field_event.cpp
/// @brief Unused event, intended to be used for automatic update of other controls after some idle time from the user
/// @ingroup custom_control
///


///////////
// Headers
#include "config.h"

#include "idle_field_event.h"
#include <wx/event.h>
#include <wx/settings.h>



/// @brief Constructor 
/// @param _control 
/// @param _name    
///
IdleFieldHandler::IdleFieldHandler(wxWindow *_control,wxString _name) {
	control = _control;
	name = _name;
	overriden = false;
	locked = false;
	text = NULL;
	box = NULL;

	// Set colours
	original = control->GetForegroundColour();
	grey = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
	wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	grey = wxColour((grey.Red() + bg.Red()) / 2,(grey.Green() + bg.Green()) / 2,(grey.Blue() + bg.Blue()) / 2);

	// wxTextCtrl
	if (control->IsKindOf(CLASSINFO(wxTextCtrl))) {
		text = (wxTextCtrl*) control;
		Connect(text->GetId(),wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(IdleFieldHandler::OnChange));
	}

	// wxComboBox
	else if (control->IsKindOf(CLASSINFO(wxComboBox))) {
		box = (wxComboBox*) control;
		Connect(box->GetId(),wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(IdleFieldHandler::OnChange));
		Connect(box->GetId(),wxEVT_COMMAND_COMBOBOX_SELECTED,wxCommandEventHandler(IdleFieldHandler::OnChange));
	}

	KillFocus();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(IdleFieldHandler,wxEvtHandler)
	EVT_SET_FOCUS(IdleFieldHandler::OnSetFocus)
	EVT_KILL_FOCUS(IdleFieldHandler::OnKillFocus)
END_EVENT_TABLE()



/// @brief Get Focus event 
/// @param event 
///
void IdleFieldHandler::OnSetFocus(wxFocusEvent &event) {
	SetFocus();
	event.Skip();
}



/// @brief Lose Focus event 
/// @param event 
///
void IdleFieldHandler::OnKillFocus(wxFocusEvent &event) {
	KillFocus();
	event.Skip();
}



/// @brief Get focus 
///
void IdleFieldHandler::SetFocus() {
	if (overriden) {
		// Prepare
		locked = true;
		control->Freeze();
		control->SetForegroundColour(original);

		// Text
		if (text) text->SetValue(_T(""));

		// Box
		if (box) box->SetValue(_T(""));

		// Finish
		overriden = false;
		locked = false;
		control->Thaw();
	}
}



/// @brief Lose Focus 
///
void IdleFieldHandler::KillFocus() {
	bool modify = false;
	if ((text && text->GetValue().IsEmpty()) || (box && box->GetValue().IsEmpty())) modify = true;

	if (modify) {
		// Prepare
		locked = true;
		control->Freeze();
		control->SetForegroundColour(grey);

		// Text
		if (text) text->SetValue(name);

		// Box
		if (box) box->SetValue(name);

		// Finish
		overriden = true;
		locked = false;
		control->Thaw();
	}
}



/// @brief Parent control changed 
/// @param event 
///
void IdleFieldHandler::OnChange(wxCommandEvent &event) {
	if (locked) return;

	overriden = false;
	control->SetForegroundColour(original);
	if (wxWindow::FindFocus() != control) {
		wxFocusEvent focus(wxEVT_KILL_FOCUS,control->GetId());
		focus.SetEventObject(control);
		AddPendingEvent(focus);
	}
	event.Skip();
}


