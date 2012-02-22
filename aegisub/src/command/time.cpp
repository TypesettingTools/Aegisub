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
//
// $Id$

/// @file time.cpp
/// @brief time/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <algorithm>
#endif

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../audio_controller.h"
#include "../audio_timing.h"
#include "../dialog_shift_times.h"
#include "../include/aegisub/context.h"
#include "../main.h"
#include "../selection_controller.h"
#include "../subs_grid.h"
#include "../video_context.h"

namespace {
	using cmd::Command;

	struct validate_video_loaded : public Command {
		CMD_TYPE(COMMAND_VALIDATE)
		bool Validate(const agi::Context *c) {
			return c->videoController->IsLoaded();
		}
	};

	struct validate_adjoinable : public Command {
		CMD_TYPE(COMMAND_VALIDATE)
		bool Validate(const agi::Context *c) {
			SelectionController<AssDialogue>::Selection sel = c->selectionController->GetSelectedSet();
			if (sel.size() < 2) return false;

			size_t found = 0;
			for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ++it) {
				AssDialogue *diag = dynamic_cast<AssDialogue*>(*it);
				if (!diag) continue;

				if (sel.count(diag)) {
					++found;
					if (found == sel.size())
						return true;
				}
				else if (found)
					return false;
			}
			return true;
		}
	};

/// @defgroup cmd-time Time manipulation commands.
/// @{

/// Changes times of subs so end times begin on next's start time.
struct time_continuous_end : public validate_adjoinable {
	CMD_NAME("time/continuous/end")
	STR_MENU("Change &End")
	STR_DISP("Change End")
	STR_HELP("Changes times of subs so end times begin on next's start time")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->subsGrid->GetSelection();
		c->subsGrid->AdjoinLines(sels.front(), sels.back(), false);
	}
};


/// Changes times of subs so start times begin on previous's end time.
struct time_continuous_start : public validate_adjoinable {
	CMD_NAME("time/continuous/start")
	STR_MENU("Change &Start")
	STR_DISP("Change Start")
	STR_HELP("Changes times of subs so start times begin on previous's end time")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->subsGrid->GetSelection();
		c->subsGrid->AdjoinLines(sels.front(), sels.back(), true);
	}

};


/// Shift selection so that the active line starts at current frame.
struct time_frame_current : public validate_video_loaded {
	CMD_NAME("time/frame/current")
	STR_MENU("Shift to &Current Frame")
	STR_DISP("Shift to Current Frame")
	STR_HELP("Shift selection so that the active line starts at current frame")

	void operator()(agi::Context *c) {
		if (!c->videoController->IsLoaded()) return;

		std::set<AssDialogue*> sel = c->selectionController->GetSelectedSet();
		AssDialogue *active_line = c->selectionController->GetActiveLine();

		if (sel.empty() || !active_line) return;

		int shift_by = c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::START) - active_line->Start;

		for (std::set<AssDialogue*>::iterator it = sel.begin(); it != sel.end(); ++it) {
			(*it)->Start = (*it)->Start + shift_by;
			(*it)->End = (*it)->End + shift_by;
		}

		c->ass->Commit(_("shift to frame"), AssFile::COMMIT_DIAG_TIME);
	}
};


/// Shift subtitles by time or frames.
struct time_shift : public Command {
	CMD_NAME("time/shift")
	STR_MENU("S&hift Times...")
	STR_DISP("Shift Times")
	STR_HELP("Shift subtitles by time or frames")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		(new DialogShiftTimes(c))->Show();
	}
};

static void snap_subs_video(agi::Context *c, bool set_start) {
	std::set<AssDialogue*> sel = c->selectionController->GetSelectedSet();

	if (!c->videoController->IsLoaded() || sel.empty()) return;

	int start = c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::START);
	int end = c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::END);

	for (std::set<AssDialogue*>::iterator it = sel.begin(); it != sel.end(); ++it) {
		if (set_start || (*it)->Start > start)
			(*it)->Start = start;
		if (!set_start || (*it)->End < end)
			(*it)->End = end;
	}

	c->ass->Commit(_("timing"), AssFile::COMMIT_DIAG_TIME);
}

/// Set end of selected subtitles to current video frame.
struct time_snap_end_video : public validate_video_loaded {
	CMD_NAME("time/snap/end_video")
	STR_MENU("Snap &End to Video")
	STR_DISP("Snap End to Video")
	STR_HELP("Set end of selected subtitles to current video frame")

	void operator()(agi::Context *c) {
		snap_subs_video(c, false);
	}
};

/// Set start and end of subtitles to the keyframes around current video frame.
struct time_snap_scene : public validate_video_loaded {
	CMD_NAME("time/snap/scene")
	STR_MENU("Snap to S&cene")
	STR_DISP("Snap to Scene")
	STR_HELP("Set start and end of subtitles to the keyframes around current video frame")

	void operator()(agi::Context *c) {
		VideoContext *con = c->videoController;
		if (!con->IsLoaded() || !con->KeyFramesLoaded()) return;

		// Get frames
		wxArrayInt sel = c->subsGrid->GetSelection();
		int curFrame = con->GetFrameN();
		int prev = 0;
		int next = 0;

		const std::vector<int> &keyframes = con->GetKeyFrames();
		if (curFrame < keyframes.front()) {
			next = keyframes.front();
		}
		else if (curFrame >= keyframes.back()) {
			prev = keyframes.back();
			next = con->GetLength();
		}
		else {
			std::vector<int>::const_iterator kf = std::lower_bound(keyframes.begin(), keyframes.end(), curFrame);
			if (*kf == curFrame) {
				prev = *kf;
				next = *(kf + 1);
			}
			else {
				prev = *(kf - 1);
				next = *kf;
			}
		}

		// Get times
		int start_ms = con->TimeAtFrame(prev,agi::vfr::START);
		int end_ms = con->TimeAtFrame(next-1,agi::vfr::END);
		AssDialogue *cur;

		// Update rows
		for (size_t i=0;i<sel.Count();i++) {
			cur = c->subsGrid->GetDialogue(sel[i]);
			cur->Start = start_ms;
			cur->End = end_ms;
		}

		// Commit
		c->ass->Commit(_("snap to scene"), AssFile::COMMIT_DIAG_TIME);
	}
};

struct time_add_lead_in : public Command {
	CMD_NAME("time/lead/in")
	STR_MENU("Add lead in")
	STR_DISP("Add lead in")
	STR_HELP("Add lead in")
	void operator()(agi::Context *c) {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->AddLeadIn();
	}
};

struct time_add_lead_out : public Command {
	CMD_NAME("time/lead/out")
	STR_MENU("Add lead out")
	STR_DISP("Add lead out")
	STR_HELP("Add lead out")
	void operator()(agi::Context *c) {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->AddLeadOut();
	}
};


/// Set start of selected subtitles to current video frame.
struct time_snap_start_video : public validate_video_loaded {
	CMD_NAME("time/snap/start_video")
	STR_MENU("Snap &Start to Video")
	STR_DISP("Snap Start to Video")
	STR_HELP("Set start of selected subtitles to current video frame")

	void operator()(agi::Context *c) {
		snap_subs_video(c, true);
	}
};

/// Switch to the next timeable thing (line or syllable)
struct time_next : public Command {
	CMD_NAME("time/next")
	STR_MENU("Next Line")
	STR_DISP("Next Line")
	STR_HELP("Next line or syllable")
	void operator()(agi::Context *c) {
		c->audioController->Stop();
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->Next();
	}
};

/// Switch to the previous timeable thing (line or syllable)
struct time_prev : public Command {
	CMD_NAME("time/prev")
	STR_MENU("Previous Line")
	STR_DISP("Previous Line")
	STR_HELP("Previous line or syllable")
	void operator()(agi::Context *c) {
		c->audioController->Stop();
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->Prev();
	}
};
}

/// @}

namespace cmd {
	void init_time() {
		reg(new time_add_lead_in);
		reg(new time_add_lead_out);
		reg(new time_continuous_end);
		reg(new time_continuous_start);
		reg(new time_frame_current);
		reg(new time_next);
		reg(new time_prev);
		reg(new time_shift);
		reg(new time_snap_end_video);
		reg(new time_snap_scene);
		reg(new time_snap_start_video);
	}
}
