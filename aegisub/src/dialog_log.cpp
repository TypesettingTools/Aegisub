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

/// @file dialog_log.cpp
/// @brief Log window.
/// @ingroup libaegisub
///

#include "config.h"

#include "dialog_log.h"

#include "compat.h"
#include "include/aegisub/context.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/log.h>

#include <algorithm>
#include <ctime>
#include <functional>
#include <string>

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

class EmitLog : public agi::log::Emitter {
	wxTextCtrl *text_ctrl;
public:
	EmitLog(wxTextCtrl *t)
	: text_ctrl(t)
	{
		for (auto sm : agi::log::log->GetMessages())
			log(&sm);
	}

	void log(agi::log::SinkMessage *sm) {
#ifndef _WIN32
		tm tmtime;
		localtime_r(&sm->tv.tv_sec, &tmtime);
		auto log = wxString::Format("%c %02d:%02d:%02d %-6ld <%-25s> [%s:%s:%d]  %s\n",
			agi::log::Severity_ID[sm->severity],
			(int)tmtime.tm_hour,
			(int)tmtime.tm_min,
			(int)tmtime.tm_sec,
			(long)sm->tv.tv_usec,
			sm->section,
			sm->file,
			sm->func,
			sm->line,
			to_wx(sm->message));
#else
		auto log = wxString::Format("%c %-6ld <%-25s> [%s:%s:%d]  %s\n",
			agi::log::Severity_ID[sm->severity],
			sm->tv.tv_usec,
			sm->section,
			sm->file,
			sm->func,
			sm->line,
			to_wx(sm->message));
#endif

		if (wxIsMainThread())
			text_ctrl->AppendText(log);
		else
			agi::dispatch::Main().Async([=]{ text_ctrl->AppendText(log); });
	}
};

LogWindow::LogWindow(agi::Context *c)
: wxDialog(c->parent, -1, _("Log window"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
{
	wxTextCtrl *text_ctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(700,300), wxTE_MULTILINE|wxTE_READONLY);
	text_ctrl->SetDefaultStyle(wxTextAttr(wxNullColour, wxNullColour, wxFont(8, wxMODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)));

	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(text_ctrl, wxSizerFlags(1).Expand().Border());
	sizer->Add(new wxButton(this, wxID_OK), wxSizerFlags(0).Border().Right());
	SetSizerAndFit(sizer);

	agi::log::log->Subscribe(emit_log = new EmitLog(text_ctrl));
}

LogWindow::~LogWindow() {
	agi::log::log->Unsubscribe(emit_log);
}
