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

/// @file help.cpp
/// @brief help/ commands
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/msgdlg.h>
#endif

#include <libaegisub/util_osx.h>

#include "command.h"
#include "../include/aegisub/context.h"

#include "../help_button.h" // help_contents
#include "../main.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-help Help commands.
/// @{

/// Visit Aegisub's bug tracker.
struct help_bugs : public Command {
	CMD_NAME("help/bugs")
	STR_MENU("&Bug Tracker..")
	STR_DISP("Bug Tracker")
	STR_HELP("Visit Aegisub's bug tracker to report bugs and request new features.")

	void operator()(agi::Context *c) {
		if (wxGetMouseState().CmdDown()) {
			if (wxGetMouseState().ShiftDown()) {
				 wxMessageBox(_T("Now crashing with an access violation..."));
				for (char *foo = (char*)0;;) *foo++ = 42;
			} else {
				wxMessageBox(_T("Now crashing with an unhandled exception..."));
				throw c->parent;
			}
		}
		AegisubApp::OpenURL(_T("http://devel.aegisub.org/"));
	}
};


/// Help topics.
struct help_contents : public Command {
	CMD_NAME("help/contents")
	STR_MENU("&Contents..")
	STR_DISP("Contents")
	STR_HELP("Help topics.")

	void operator()(agi::Context *c) {
		HelpButton::OpenPage(_T("Main"));
	}
};


/// Resource files.
struct help_files : public Command {
	CMD_NAME("help/files")
	STR_MENU("&All Files")
	STR_DISP("All Files")
	STR_HELP("Resource files.")

	void operator()(agi::Context *c) {
#ifdef __WXMAC__
		char *shared_path = agi::util::OSX_GetBundleSharedSupportDirectory();
		wxString help_path = wxString::Format(_T("%s/doc"), wxString(shared_path, wxConvUTF8).c_str());
		agi::util::OSX_OpenLocation(help_path.c_str());
		free(shared_path);
#endif
	}
};


/// Visit Aegisub's forums.
struct help_forums : public Command {
	CMD_NAME("help/forums")
	STR_MENU("&Forums..")
	STR_DISP("Forums")
	STR_HELP("Visit Aegisub's forums.")

	void operator()(agi::Context *c) {
		AegisubApp::OpenURL(_T("http://forum.aegisub.org/"));
	}
};


/// Visit Aegisub's official IRC channel.
struct help_irc : public Command {
	CMD_NAME("help/irc")
	STR_MENU("&IRC Channel..")
	STR_DISP("IRC Channel")
	STR_HELP("Visit Aegisub's official IRC channel.")

	void operator()(agi::Context *c) {
		AegisubApp::OpenURL(_T("irc://irc.rizon.net/aegisub"));
	}
};

/// Open the manual page for Visual Typesetting.
struct help_video : public Command {
	CMD_NAME("help/video")
	STR_MENU("&Visual Typesetting..")
	STR_DISP("Visual Typesetting")
	STR_HELP("Open the manual page for Visual Typesetting.")

	void operator()(agi::Context *c) {
		HelpButton::OpenPage("Visual Typesetting");
	}
};

/// Visit Aegisub's official website.
struct help_website : public Command {
	CMD_NAME("help/website")
	STR_MENU("&Website..")
	STR_DISP("Website")
	STR_HELP("Visit Aegisub's official website.")

	void operator()(agi::Context *c) {
		AegisubApp::OpenURL(_T("http://www.aegisub.org/"));
	}
};
}
/// @}

namespace cmd {
	void init_help() {
		reg(new help_bugs);
		reg(new help_contents);
		reg(new help_files);
		reg(new help_forums);
		reg(new help_irc);
		reg(new help_video);
		reg(new help_website);
	}
}
