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

/// @file dialog_log.h
/// @see dialog_log.cpp
/// @ingroup libaegisub
///




////////////
// Includes
#ifndef AGI_PRE
#include <wx/dialog.h>
#include <wx/textctrl.h>
#endif

#include <libaegisub/log.h>

/// @class AboutScreen
/// @brief About dialogue.

class LogWindow: public wxDialog {
public:

	class EmitLog: public agi::log::Emitter {
		wxTextCtrl *text_ctrl;
		void Write(agi::log::SinkMessage *sm);
	public:
		EmitLog(wxTextCtrl *t);
		void Prime();
		void Set(wxTextCtrl *text_ctrl_p) { text_ctrl = text_ctrl_p; }
    	void log(agi::log::SinkMessage *sm);
	};


	LogWindow(wxWindow *parent);
	~LogWindow();
	DECLARE_EVENT_TABLE()
private:
	void OnClose(wxCloseEvent &event);
	EmitLog *emit_log;
};

