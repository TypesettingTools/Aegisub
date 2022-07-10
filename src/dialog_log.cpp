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

#include "compat.h"
#include "dialog_manager.h"
#include "format.h"
#include "include/aegisub/context.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/log.h>

#include <ctime>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace {
class EmitLog final : public agi::log::Emitter {
	wxTextCtrl* text_ctrl;

  public:
	EmitLog(wxTextCtrl* t) : text_ctrl(t) {
		for(auto const& sm : agi::log::log->GetMessages())
			log(sm);
	}

	void log(agi::log::SinkMessage const& sm) override {
		time_t time = sm.time / 1000000000;
#ifndef _WIN32
		tm tmtime;
		localtime_r(&time, &tmtime);
		auto log =
		    fmt_wx("%c %02d:%02d:%02d %-6d <%-25s> [%s:%s:%d]  %s\n",
		           agi::log::Severity_ID[sm.severity], tmtime.tm_hour, tmtime.tm_min, tmtime.tm_sec,
		           (sm.time % 1000000000), sm.section, sm.file, sm.func, sm.line, sm.message);
#else
		auto log =
		    fmt_wx("%c %-6ld.%09ld <%-25s> [%s:%s:%d]  %s\n", agi::log::Severity_ID[sm.severity],
		           (sm.time / 1000000000), (sm.time % 1000000000), sm.section, sm.file, sm.func,
		           sm.line, sm.message);
#endif

		if(wxIsMainThread())
			text_ctrl->AppendText(log);
		else
			agi::dispatch::Main().Async([=] { text_ctrl->AppendText(log); });
	}
};

class LogWindow : public wxDialog {
	agi::log::Emitter* emit_log;

  public:
	LogWindow(agi::Context* c);
	~LogWindow();
};

LogWindow::LogWindow(agi::Context* c)
    : wxDialog(c->parent, -1, _("Log window"), wxDefaultPosition, wxDefaultSize,
               wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER) {
	wxTextCtrl* text_ctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(700, 300),
	                                       wxTE_MULTILINE | wxTE_READONLY);
	text_ctrl->SetDefaultStyle(
	    wxTextAttr(wxNullColour, wxNullColour,
	               wxFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)));

	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(text_ctrl, wxSizerFlags(1).Expand().Border());
	sizer->Add(new wxButton(this, wxID_OK), wxSizerFlags(0).Border().Right());
	SetSizerAndFit(sizer);

	agi::log::log->Subscribe(std::unique_ptr<agi::log::Emitter>(emit_log = new EmitLog(text_ctrl)));
}

LogWindow::~LogWindow() {
	agi::log::log->Unsubscribe(emit_log);
}
} // namespace

void ShowLogWindow(agi::Context* c) {
	c->dialog->Show<LogWindow>(c);
}
