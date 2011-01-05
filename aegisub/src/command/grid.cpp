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

/// @file grid.cpp
/// @brief grid/ commands.
/// @ingroup command
///

#include "config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "aegisub/context.h"
#include "subs_grid.h"
#include "main.h"
#include "frame_main.h"

namespace cmd {
/// @defgroup cmd-grid Subtitle grid commands.
/// @{



/// Move to the next subtitle line.
class grid_line_next: public Command {
public:
	CMD_NAME("grid/line/next")
	STR_MENU("Next Line")
	STR_DISP("Next Line")
	STR_HELP("Move to the next subtitle line.")

	void operator()(agi::Context *c) {
		c->SubsGrid->NextLine();
	}
};


/// Move to the previous line.
class grid_line_prev: public Command {
public:
	CMD_NAME("grid/line/prev")
	STR_MENU("Previous Line")
	STR_DISP("Previous Line")
	STR_HELP("Move to the previous line.")

	void operator()(agi::Context *c) {
		c->SubsGrid->PrevLine();
	}
};


/// Cycle through tag hiding modes.
class grid_tag_cycle_hiding: public Command {
public:
	CMD_NAME("grid/tag/cycle_hiding")
	STR_MENU("Cycle Tag Hiding")
	STR_DISP("Cycle Tag Hiding")
	STR_HELP("Cycle through tag hiding modes.")

	void operator()(agi::Context *c) {
		int tagMode = OPT_GET("Subtitle/Grid/Hide Overrides")->GetInt();

		// Cycle to next
		tagMode = (tagMode+1)%3;

		// Show on status bar
		wxString message = _("ASS Override Tag mode set to ");
		if (tagMode == 0) message += _("show full tags.");
		if (tagMode == 1) message += _("simplify tags.");
		if (tagMode == 2) message += _("hide tags.");
		wxGetApp().frame->StatusTimeout(message,10000);

		// Set option
		OPT_SET("Subtitle/Grid/Hide Overrides")->SetInt(tagMode);

		// Refresh grid
		c->SubsGrid->Refresh(false);
	}
};


/// Hide override tags in the subtitle grid.
class grid_tags_hide: public Command {
public:
	CMD_NAME("grid/tags/hide")
	STR_MENU("Hide Tags")
	STR_DISP("Hide Tags")
	STR_HELP("Hide override tags in the subtitle grid.")

	void operator()(agi::Context *c) {
//	XXX: Needs fixing.
//		OPT_SET("Subtitle/Grid/Hide Overrides")->SetInt(event.GetId() - cmd::id("subtitle/tags/show"));
//		SubsGrid->Refresh(false);
	}
};


/// Show full override tags in the subtitle grid.
class grid_tags_show: public Command {
public:
	CMD_NAME("grid/tags/show")
	STR_MENU("Show Tags")
	STR_DISP("Show Tags")
	STR_HELP("Show full override tags in the subtitle grid.")

	void operator()(agi::Context *c) {
		//XXX: see grid_tags_hide
	}
};


/// Replace override tags in the subtitle grid with a simplified placeholder.
class grid_tags_simplify: public Command {
public:
	CMD_NAME("grid/tags/simplify")
	STR_MENU("Simplify Tags")
	STR_DISP("Simplify Tags")
	STR_HELP("Replace override tags in the subtitle grid with a simplified placeholder.")

	void operator()(agi::Context *c) {
		//XXX: see grid_tags_hide
	}
};

/// @}


/// Init grid/ commands.
void init_grid(CommandManager *cm) {
	cm->reg(new grid_line_next());
	cm->reg(new grid_line_prev());
	cm->reg(new grid_tag_cycle_hiding());
	cm->reg(new grid_tags_hide());
	cm->reg(new grid_tags_show());
	cm->reg(new grid_tags_simplify());
}

} // namespace cmd
