// Copyright (c) 2010, Amar Takhar
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

/// @file dialog_log.cpp
/// @brief Log window.
/// @ingroup libaegisub
///


////////////
// Includes
#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#endif
#include <string.h>

#include <time.h>
#include "dialog_log.h"

/// @brief Constructor
/// @param parent Parent frame.
LogWindow::LogWindow(wxWindow *parent)
: wxDialog (parent, -1, _("Log window"), wxDefaultPosition, wxSize(700,300), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER, _("Log window"))
{
	// Text sizer
	wxSizer *sizer_text = new wxBoxSizer(wxVERTICAL);

	wxTextCtrl *text_ctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString ,wxDefaultPosition, wxSize(700,300), wxTE_MULTILINE|wxTE_READONLY);
	wxTextAttr attr;
	attr.SetFont(wxFont(8, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	text_ctrl->SetDefaultStyle(attr);
	sizer_text->Add(text_ctrl, 1, wxEXPAND);

	wxSizer *sizer_button = new wxBoxSizer(wxHORIZONTAL);
	sizer_button->Add(new wxButton(this, wxID_OK), 0, wxALIGN_RIGHT | wxALL, 2);


	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(sizer_text, 1 ,wxEXPAND|wxALL, 0);
	sizer->Add(sizer_button, 0 ,wxEXPAND|wxALL, 0);
	sizer->SetSizeHints(this);
	SetSizer(sizer);

	emit_log = new EmitLog(text_ctrl);
	emit_log->Enable();
}


/// @brief Destructor
LogWindow::~LogWindow() {
	delete emit_log;
}


LogWindow::EmitLog::EmitLog(wxTextCtrl *t): text_ctrl(t) {
	const agi::log::Sink *sink = agi::log::log->GetSink();

	for (unsigned int i=0; i < sink->size(); i++) {
		Write((*sink)[i]);
	}
}


void LogWindow::EmitLog::Write(agi::log::SinkMessage *sm) {
#ifndef _WIN32
	tm tmtime;
	localtime_r(&sm->tv.tv_sec, &tmtime);
	wxString log = wxString::Format("%c %02d:%02d:%02d %-6ld <%-25s> [%s:%s:%d]  %s\n",
		agi::log::Severity_ID[sm->severity],
		tmtime.tm_hour,
		tmtime.tm_min,
		tmtime.tm_sec,
		sm->tv.tv_usec,
		sm->section,
		sm->file,
		sm->func,
		sm->line,
		strndup(sm->message, sm->len));
#else
	wxString log = wxString::Format("%c %-6ld <%-25s> [%s:%s:%d]  %s\n",
		agi::log::Severity_ID[sm->severity],
		sm->tv.tv_usec,
		sm->section,
		sm->file,
		sm->func,
		sm->line,
		std::string(sm->message, sm->len));
#endif
	text_ctrl->AppendText(log);
}


void LogWindow::EmitLog::log(agi::log::SinkMessage *sm) {
	delete text_ctrl;
	Write(sm);
}


void LogWindow::OnClose(wxCloseEvent &WXUNUSED(event)) {
	Destroy();
}


BEGIN_EVENT_TABLE(LogWindow, wxDialog)
    EVT_CLOSE(LogWindow::OnClose)
END_EVENT_TABLE()
