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

#include "command.h"

#include "../include/aegisub/context.h"
#include "../libresrc/libresrc.h"
#include "../options.h"
#include "../utils.h"
#include "../video_context.h"

#include <libaegisub/make_unique.h>

namespace {
	using cmd::Command;

struct timecode_close final : public Command {
	CMD_NAME("timecode/close")
	CMD_ICON(close_timecodes_menu)
	STR_MENU("Close Timecodes File")
	STR_DISP("Close Timecodes File")
	STR_HELP("Close the currently open timecodes file")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) override {
		return c->videoController->OverTimecodesLoaded();
	}

	void operator()(agi::Context *c) override {
		c->videoController->CloseTimecodes();
	}
};

struct timecode_open final : public Command {
	CMD_NAME("timecode/open")
	CMD_ICON(open_timecodes_menu)
	STR_MENU("Open Timecodes File...")
	STR_DISP("Open Timecodes File")
	STR_HELP("Open a VFR timecodes v1 or v2 file")

	void operator()(agi::Context *c) override {
		auto str = _("All Supported Formats") + " (*.txt)|*.txt|" + _("All Files") + " (*.*)|*.*";
		auto filename = OpenFileSelector(_("Open Timecodes File"), "Path/Last/Timecodes", "", "", str, c->parent);
		if (!filename.empty())
			c->videoController->LoadTimecodes(filename);
	}
};

struct timecode_save final : public Command {
	CMD_NAME("timecode/save")
	CMD_ICON(save_timecodes_menu)
	STR_MENU("Save Timecodes File...")
	STR_DISP("Save Timecodes File")
	STR_HELP("Save a VFR timecodes v2 file")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) override {
		return c->videoController->TimecodesLoaded();
	}

	void operator()(agi::Context *c) override {
		auto str = _("All Supported Formats") + " (*.txt)|*.txt|" + _("All Files") + " (*.*)|*.*";
		auto filename = SaveFileSelector(_("Save Timecodes File"), "Path/Last/Timecodes", "", "", str, c->parent);
		if (!filename.empty())
			c->videoController->SaveTimecodes(filename);
	}
};
}

namespace cmd {
	void init_timecode() {
		reg(agi::make_unique<timecode_close>());
		reg(agi::make_unique<timecode_open>());
		reg(agi::make_unique<timecode_save>());
	}
}
