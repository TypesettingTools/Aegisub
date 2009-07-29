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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_progress.cpp
/// @brief Progress-bar dialogue box for displaying during long operations
/// @ingroup utility
///


///////////
// Headers
#include "config.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include "dialog_progress.h"
#include "utils.h"


DEFINE_EVENT_TYPE(wxEVT_PROGRESS_UPDATE)



/// @brief Constructor 
/// @param parent  
/// @param title   
/// @param cancel  
/// @param message 
/// @param cur     
/// @param max     
///
DialogProgress::DialogProgress(wxWindow *parent,wxString title,volatile bool *cancel,wxString message,int cur,int max)
: wxDialog(parent,-1,title,wxDefaultPosition,wxDefaultSize,wxBORDER_RAISED/* | wxSTAY_ON_TOP*/)
{
	// Variables
	canceled = cancel;
	if (cancel) *canceled = false;
	virtualMax = max;

	// Gauge
	gauge = new wxGauge(this, -1, 100, wxDefaultPosition, wxSize(300,20), wxGA_HORIZONTAL);
	wxButton *cancelButton = NULL;
	if (cancel) cancelButton = new wxButton(this,wxID_CANCEL);
	text = new wxStaticText(this, -1, message, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(gauge,1,wxEXPAND  | wxALL,5);
	MainSizer->Add(text,0,wxEXPAND | wxALIGN_CENTER | wxBOTTOM,5);
	if (cancel) MainSizer->Add(cancelButton,0,wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	CenterOnParent();
	Connect(0,wxEVT_PROGRESS_UPDATE,wxCommandEventHandler(DialogProgress::OnUpdateProgress));
}



/// @brief Set progress 
/// @param cur 
/// @param max 
/// @return 
///
void DialogProgress::SetProgress(int cur,int max) {
	// Return if there's nothing to do
	int value = cur*100/virtualMax;
	if (gauge->GetValue() == value && virtualMax == max) return;
	virtualMax = max;

	// Check if it's the main thread, if so, just process it now
	if (wxIsMainThread()) {
		gauge->SetValue(MID(0,value,100));
		wxYield();
		return;
	}

	// Otherwise, go on
	{
		wxMutexLocker locker(mutex);
		if (count >= 2) return;
		else count++;
	}

	wxCommandEvent* evt = new wxCommandEvent(wxEVT_PROGRESS_UPDATE,0);
	evt->SetInt(value);
	AddPendingEvent(*evt);
}



/// @brief Update progress 
/// @param event 
///
void DialogProgress::OnUpdateProgress(wxCommandEvent &event)
{
	int value = event.GetInt();
	if (gauge->GetValue() != value) gauge->SetValue(MID(0,value,100));
	wxMutexLocker locker(mutex);
	count--;
}



/// @brief Set progress 
/// @param setto 
///
void DialogProgress::SetText(wxString setto) {
	// Lock
	bool isMain = wxIsMainThread();
	if (!isMain) wxMutexGuiEnter();

	// Update
	text->SetLabel(setto);
	wxYield();

	// Unlock
	if (!isMain) wxMutexGuiLeave();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogProgress,wxDialog)
	EVT_BUTTON(wxID_CANCEL,DialogProgress::OnCancel)
END_EVENT_TABLE()



/// @brief Cancel 
/// @param event 
///
void DialogProgress::OnCancel(wxCommandEvent &event) {
	if (canceled) *canceled = true;
	bool isMain = wxIsMainThread();
	if (!isMain) wxMutexGuiEnter();
	Destroy();
	if (!isMain) wxMutexGuiLeave();
}


/// @brief Thread constructor 
/// @param parent   
/// @param title    
/// @param canceled 
/// @param message  
/// @param cur      
/// @param max      
///
DialogProgressThread::DialogProgressThread(wxWindow *parent,wxString title,volatile bool *canceled,wxString message,int cur,int max)
: wxThread(wxTHREAD_DETACHED)
{
	dialog = new DialogProgress(parent,title,canceled,message,cur,max);
}



/// @brief Thread destructor 
///
DialogProgressThread::~DialogProgressThread() {
}



/// @brief Thread entry point 
/// @return 
///
wxThread::ExitCode DialogProgressThread::Entry() {
	dialog->ShowModal();
	dialog = NULL;
	Delete();
	return 0;
}



/// @brief Close 
///
void DialogProgressThread::Close() {
	wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED,wxID_CANCEL);
	dialog->canceled = NULL;
	dialog->GetEventHandler()->ProcessEvent(event);
}


