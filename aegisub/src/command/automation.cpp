// Copyright (c) 2005-2010, Niels Martin Hansen
// Copyright (c) 2005-2010, Rodrigo Braz Monteiro
// Copyright (c) 2010, Amar Takhar
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//	 this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//	 this list of conditions and the following disclaimer in the documentation
//	 and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//	 may be used to endorse or promote products derived from this software
//	 without specific prior written permission.
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

/// @file automation.cpp
/// @brief am/ (automation) commands
/// @ingroup command
///

#include "../config.h"


#include "command.h"

#include "../auto4_base.h"
#include "../dialog_automation.h"
#include "../dialog_manager.h"
#include "../frame_main.h"
#include "../include/aegisub/context.h"
#include "../main.h"
#include "../options.h"
#include "../utils.h"
#include "../video_context.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-am Automation commands
/// @{

struct reload_all : public Command {
	CMD_NAME("am/reload")
	STR_MENU("&Reload Automation scripts")
	STR_DISP("Reload Automation scripts")
	STR_HELP("Reload all Automation scripts and rescan the autoload folder")

	void operator()(agi::Context *c) {
		wxGetApp().global_scripts->Reload();
		c->local_scripts->Reload();
		StatusTimeout(_("Reloaded all Automation scripts"));
	}
};

struct reload_autoload : public Command {
	CMD_NAME("am/reload/autoload")
	STR_MENU("R&eload autoload Automation scripts")
	STR_DISP("Reload autoload Automation scripts")
	STR_HELP("Rescan the Automation autoload folder")

	void operator()(agi::Context *c) {
		wxGetApp().global_scripts->Reload();
		StatusTimeout(_("Reloaded autoload Automation scripts"));
	}
};

struct open_manager : public Command {
	CMD_NAME("am/manager")
	STR_MENU("&Automation...")
	STR_DISP("Automation")
	STR_HELP("Open automation manager")

	void operator()(agi::Context *c) {
		c->dialog->Show<DialogAutomation>(c);
	}
};

struct meta : public Command {
	CMD_NAME("am/meta")
	STR_MENU("&Automation...")
	STR_DISP("Automation")
	STR_HELP("Open automation manager")

	void operator()(agi::Context *c) {
		if (wxGetMouseState().CmdDown()) {
			if (wxGetMouseState().ShiftDown())
				cmd::call("am/reload", c);
			else
				cmd::call("am/reload/autoload", c);
		}
		else
			cmd::call("am/manager", c);
	}
};

/// @}
}

namespace cmd {
	void init_automation() {
		reg(new meta);
		reg(new open_manager);
		reg(new reload_all);
		reg(new reload_autoload);
	}
}
