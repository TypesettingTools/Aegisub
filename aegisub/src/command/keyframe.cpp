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

/// @file keyframe.cpp
/// @brief keyframe/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/filedlg.h>
#endif

#include "command.h"

#include "../include/aegisub/context.h"
#include "../main.h"
#include "../compat.h"
#include "../video_context.h"

namespace cmd {
/// @defgroup cmd-keyframed Keyframe commands.
/// @{


/// Closes the currently open keyframes list.
class keyframe_close: public Command {
public:
	CMD_NAME("keyframe/close")
	STR_MENU("Close Keyframes")
	STR_DISP("Close Keyframes")
	STR_HELP("Closes the currently open keyframes list.")

	void operator()(agi::Context *c) {
		c->videoContext->CloseKeyframes();
	}
};


/// Opens a keyframe list file.
class keyframe_open: public Command {
public:
	CMD_NAME("keyframe/open")
	STR_MENU("Open Keyframes..")
	STR_DISP("Open Keyframes")
	STR_HELP("Opens a keyframe list file.")

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Keyframes")->GetString());
		wxString filename = wxFileSelector(
			_T("Select the keyframes file to open"),
			path,
			_T("")
			,_T(".txt"),
			_T("All supported formats (*.txt, *.pass, *.stats, *.log)|*.txt;*.pass;*.stats;*.log|All files (*.*)|*.*"),
			wxFD_FILE_MUST_EXIST | wxFD_OPEN);

		if (filename.empty()) return;
		OPT_SET("Path/Last/Keyframes")->SetString(STD_STR(filename));
		c->videoContext->LoadKeyframes(filename);
	}
};


/// Saves the current keyframe list.
class keyframe_save: public Command {
public:
	CMD_NAME("keyframe/save")
	STR_MENU("Save Keyframes..")
	STR_DISP("Save Keyframes")
	STR_HELP("Saves the current keyframe list.")

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Keyframes")->GetString());
		wxString filename = wxFileSelector(_T("Select the Keyframes file to open"),path,_T(""),_T("*.key.txt"),_T("Text files (*.txt)|*.txt"),wxFD_OVERWRITE_PROMPT | wxFD_SAVE);
		if (filename.empty()) return;
		OPT_SET("Path/Last/Keyframes")->SetString(STD_STR(filename));
		c->videoContext->SaveKeyframes(filename);
	}
};

/// @}

/// Init keyframe/ commands.
void init_keyframe(CommandManager *cm) {
	cm->reg(new keyframe_close());
	cm->reg(new keyframe_open());
	cm->reg(new keyframe_save());
}

} // namespace cmd
