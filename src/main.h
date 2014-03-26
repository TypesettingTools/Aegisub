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

/// @file main.h
/// @see main.cpp
/// @ingroup main
///

#include <wx/app.h>

#include "aegisublocale.h"

#ifndef wxUSE_EXCEPTIONS
#error wxWidgets is compiled without exceptions support. Aegisub requires exceptions support in wxWidgets to run safely.
#endif

class FrameMain;

namespace Automation4 { class AutoloadScriptManager; }

class AegisubApp : public wxApp {
	friend class FrameMain;

	bool OnInit() override;
	int OnExit() override;
	int OnRun() override;

	void OnUnhandledException() override;
	void OnFatalException() override;
	bool OnExceptionInMainLoop() override;

	/// @brief Handle wx assertions and redirect to the logging system.
	/// @param file File name
	/// @param line Line number
	/// @param func Function name
	/// @param cond Condition
	/// @param msg  Message
	void OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg) override;

	// This function wraps all event handler calls anywhere in the application and is
	// our ticket to catch exceptions happening in event handlers.
	void HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const override;

	FrameMain *frame = nullptr;
public:
	AegisubApp();
	AegisubLocale locale;
	Automation4::AutoloadScriptManager *global_scripts = nullptr;

	// Apple events
	void MacOpenFile(const wxString &filename);
};

wxDECLARE_APP(AegisubApp);
