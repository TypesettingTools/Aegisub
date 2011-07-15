// Copyright (c) 2005-2010, Niels Martin Hansen
// Copyright (c) 2005-2010, Rodrigo Braz Monteiro
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

/// @file timecode.cpp
/// @brief timecode/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/filedlg.h>
#endif

#include "command.h"

#include "../include/aegisub/context.h"
#include "../video_context.h"
#include "../main.h"
#include "../compat.h"
#include "../subs_edit_box.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-timecode Timecode commands.
/// @{

/// Closes the currently open timecodes file.
struct timecode_close : public Command {
	CMD_NAME("timecode/close")
	STR_MENU("Close Timecodes File")
	STR_DISP("Close Timecodes File")
	STR_HELP("Closes the currently open timecodes file.")

	bool Validate(const agi::Context *c) {
		return c->videoController->OverTimecodesLoaded();
	}

	void operator()(agi::Context *c) {
		c->videoController->CloseTimecodes();
	}
};


/// Opens a VFR timecodes v1 or v2 file.
struct timecode_open : public Command {
	CMD_NAME("timecode/open")
	STR_MENU("Open Timecodes File..")
	STR_DISP("Open Timecodes File")
	STR_HELP("Opens a VFR timecodes v1 or v2 file.")

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Timecodes")->GetString());
		wxString str = wxString(_("All Supported Types")) + _T("(*.txt)|*.txt|") + _("All Files") + _T(" (*.*)|*.*");
		wxString filename = wxFileSelector(_("Open timecodes file"),path,_T(""),_T(""),str,wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (!filename.empty()) {
			c->videoController->LoadTimecodes(filename);
			OPT_SET("Path/Last/Timecodes")->SetString(STD_STR(filename));
		}
	}
};


/// Saves a VFR timecodes v2 file.
struct timecode_save : public Command {
	CMD_NAME("timecode/save")
	STR_MENU("Save Timecodes File..")
	STR_DISP("Save Timecodes File")
	STR_HELP("Saves a VFR timecodes v2 file.")

	bool Validate(const agi::Context *c) {
		return c->videoController->TimecodesLoaded();
	}

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Timecodes")->GetString());
		wxString str = wxString(_("All Supported Types")) + _T("(*.txt)|*.txt|") + _("All Files") + _T(" (*.*)|*.*");
		wxString filename = wxFileSelector(_("Save timecodes file"),path,_T(""),_T(""),str,wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (!filename.empty()) {
			c->videoController->SaveTimecodes(filename);
			OPT_SET("Path/Last/Timecodes")->SetString(STD_STR(filename));
		}
	}
};
}
/// @}

namespace cmd {
	void init_timecode() {
		reg(new timecode_close);
		reg(new timecode_open);
		reg(new timecode_save);
	}
}
