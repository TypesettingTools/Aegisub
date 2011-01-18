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

#include "../config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../include/aegisub/context.h"
#include "../subs_grid.h"
#include "../main.h"
#include "../frame_main.h"
#include "../utils.h"


namespace cmd {
/// @defgroup cmd-grid Subtitle grid commands.
/// @{

/// Move to the next subtitle line.
struct grid_line_next : public Command {
	CMD_NAME("grid/line/next")
	STR_MENU("Next Line")
	STR_DISP("Next Line")
	STR_HELP("Move to the next subtitle line.")

	void operator()(agi::Context *c) {
		c->subsGrid->NextLine();
	}
};


/// Move to the previous line.
struct grid_line_prev : public Command {
	CMD_NAME("grid/line/prev")
	STR_MENU("Previous Line")
	STR_DISP("Previous Line")
	STR_HELP("Move to the previous line.")

	void operator()(agi::Context *c) {
		c->subsGrid->PrevLine();
	}
};


/// Cycle through tag hiding modes.
struct grid_tag_cycle_hiding : public Command {
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
	}
};


/// Hide override tags in the subtitle grid.
struct grid_tags_hide : public Command {
	CMD_NAME("grid/tags/hide")
	STR_MENU("Hide Tags")
	STR_DISP("Hide Tags")
	STR_HELP("Hide override tags in the subtitle grid.")

	void operator()(agi::Context *c) {
		OPT_SET("Subtitle/Grid/Hide Overrides")->SetInt(2);
	}
};


/// Show full override tags in the subtitle grid.
struct grid_tags_show : public Command {
	CMD_NAME("grid/tags/show")
	STR_MENU("Show Tags")
	STR_DISP("Show Tags")
	STR_HELP("Show full override tags in the subtitle grid.")

	void operator()(agi::Context *c) {
		OPT_SET("Subtitle/Grid/Hide Overrides")->SetInt(0);
	}
};


/// Replace override tags in the subtitle grid with a simplified placeholder.
struct grid_tags_simplify : public Command {
	CMD_NAME("grid/tags/simplify")
	STR_MENU("Simplify Tags")
	STR_DISP("Simplify Tags")
	STR_HELP("Replace override tags in the subtitle grid with a simplified placeholder.")

	void operator()(agi::Context *c) {
		OPT_SET("Subtitle/Grid/Hide Overrides")->SetInt(1);
	}
};

template<class T, class U>
static bool move_one(T begin, T end, U value) {
	T it = find(begin, end, value);
	assert(it != end);

	T prev = it;
	++prev;
	prev = find_if(prev, end, cast<U>());

	if (prev != end) {
		using std::swap;
		swap(*it, *prev);
		return true;
	}
	return false;
}

/// Swap the active line with the dialogue line above it
struct grid_swap_up : public Command {
	CMD_NAME("grid/swap/up")
	STR_MENU("Move line up")
	STR_DISP("Move line up")
	STR_HELP("Move the selected line up one row")

	void operator()(agi::Context *c) {
		if (AssDialogue *line = c->selectionController->GetActiveLine()) {
			if (move_one(c->ass->Line.rbegin(), c->ass->Line.rend(), line))
				/// todo Maybe add COMMIT_ORDER, as the grid is the only thing
				///      that needs to care about this
				///      Probably not worth it
				c->ass->Commit(_("swap lines"), AssFile::COMMIT_FULL);
		}
	}
};

/// Swap the active line with the dialogue line below it
struct grid_swap_down : public Command {
	CMD_NAME("grid/swap/down")
		STR_MENU("Move line down")
		STR_DISP("Move line down")
		STR_HELP("Move the selected line down one row")

		void operator()(agi::Context *c) {
			if (AssDialogue *line = c->selectionController->GetActiveLine()) {
				if (move_one(c->ass->Line.begin(), c->ass->Line.end(), line))
					c->ass->Commit(_("swap lines"), AssFile::COMMIT_FULL);
			}
	}
};

/// @}


/// Init grid/ commands.
void init_grid(CommandManager *cm) {
	cm->reg(new grid_line_next);
	cm->reg(new grid_line_prev);
	cm->reg(new grid_swap_down);
	cm->reg(new grid_swap_up);
	cm->reg(new grid_tag_cycle_hiding);
	cm->reg(new grid_tags_hide);
	cm->reg(new grid_tags_show);
	cm->reg(new grid_tags_simplify);
}

} // namespace cmd
