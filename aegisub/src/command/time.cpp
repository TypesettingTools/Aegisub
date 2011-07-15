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

#include "../selection_controller.h"
#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../audio_controller.h"
#include "../audio_timing.h"
#include "../dialog_shift_times.h"
#include "../include/aegisub/context.h"
#include "../main.h"
#include "../subs_grid.h"
#include "../video_context.h"

namespace cmd {
/// @defgroup cmd-time Time manipulation commands.
/// @{


/// Changes times of subs so end times begin on next's start time.
struct time_continuous_end : public Command {
	CMD_NAME("time/continuous/end")
	STR_MENU("Change &End")
	STR_DISP("Change End")
	STR_HELP("Changes times of subs so end times begin on next's start time.")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->subsGrid->GetSelection();
		c->subsGrid->AdjoinLines(sels.front(), sels.back(), false);
	}
};


/// Changes times of subs so start times begin on previous's end time.
struct time_continuous_start : public Command {
	CMD_NAME("time/continuous/start")
	STR_MENU("Change &Start")
	STR_DISP("Change Start")
	STR_HELP("Changes times of subs so start times begin on previous's end time.")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->subsGrid->GetSelection();
		c->subsGrid->AdjoinLines(sels.front(), sels.back(), true);
	}

};


/// Shift selection so first selected line starts at current frame.
struct time_frame_current : public Command {
	CMD_NAME("time/frame/current")
	STR_MENU("Shift to Current Frame")
	STR_DISP("Shift to Current Frame")
	STR_HELP("Shift selection so first selected line starts at current frame.")

	void operator()(agi::Context *c) {
		if (!c->videoController->IsLoaded()) return;

		wxArrayInt sels = c->subsGrid->GetSelection();
		size_t n=sels.Count();
		if (n == 0) return;

		// Get shifting in ms
		AssDialogue *cur = c->subsGrid->GetDialogue(sels[0]);
		if (!cur) return;
		int shiftBy = c->videoController->TimeAtFrame(c->videoController->GetFrameN(),agi::vfr::START) - cur->Start.GetMS();

		// Update
		for (size_t i=0;i<n;i++) {
			cur = c->subsGrid->GetDialogue(sels[i]);
			if (cur) {
				cur->Start.SetMS(cur->Start.GetMS()+shiftBy);
				cur->End.SetMS(cur->End.GetMS()+shiftBy);
			}
		}

		// Commit
		c->ass->Commit(_("shift to frame"), AssFile::COMMIT_TIMES);
	}
};


/// Shift subtitles by time or frames.
struct time_shift : public Command {
	CMD_NAME("time/shift")
	STR_MENU("S&hift Times..")
	STR_DISP("Shift Times")
	STR_HELP("Shift subtitles by time or frames.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogShiftTimes(c).ShowModal();
	}
};



/// Set end of selected subtitles to current video frame.
struct time_snap_end_video : public Command {
	CMD_NAME("time/snap/end_video")
	STR_MENU("Snap End to Video")
	STR_DISP("Snap End to Video")
	STR_HELP("Set end of selected subtitles to current video frame.")

	void operator()(agi::Context *c) {
		c->subsGrid->SetSubsToVideo(false);
	}
};


/// Shift selected subtitles so first selected starts at this frame.
struct time_snap_frame : public Command {
	CMD_NAME("time/snap/frame")
	STR_MENU("Shift Subtitles to Frame")
	STR_DISP("Shift Subtitles to Frame")
	STR_HELP("Shift selected subtitles so first selected starts at this frame.")

	void operator()(agi::Context *c) {
		if (c->videoController->IsLoaded()) return;

		wxArrayInt sels = c->subsGrid->GetSelection();
		if (sels.empty()) return;

		AssDialogue *cur = c->subsGrid->GetDialogue(sels[0]);
		if (!cur) return;
		int shiftBy = c->videoController->TimeAtFrame(c->videoController->GetFrameN(),agi::vfr::START) - cur->Start.GetMS();

		for (size_t i = 0; i < sels.size(); ++i) {
			if (cur = c->subsGrid->GetDialogue(sels[i])) {
				cur->Start.SetMS(cur->Start.GetMS() + shiftBy);
				cur->End.SetMS(cur->End.GetMS() + shiftBy);
			}
		}

		c->ass->Commit(_("shift to frame"));
	}
};


/// Set start and end of subtitles to the keyframes around current video frame.
struct time_snap_scene : public Command {
	CMD_NAME("time/snap/scene")
	STR_MENU("Snap to Scene")
	STR_DISP("Snap to Scene")
	STR_HELP("Set start and end of subtitles to the keyframes around current video frame.")

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
			cur->Start.SetMS(start_ms);
			cur->End.SetMS(end_ms);
		}

		// Commit
		c->ass->Commit(_("snap to scene"), AssFile::COMMIT_TIMES);
	}
};

struct time_add_lead_in : public Command {
	CMD_NAME("time/lead/in")
	STR_MENU("Add lead in")
	STR_DISP("Add lead in")
	STR_HELP("Add lead in")
	void operator()(agi::Context *c) {
		if (AssDialogue *line = c->selectionController->GetActiveLine()) {
			line->Start.SetMS(line->Start.GetMS() - OPT_GET("Audio/Lead/IN")->GetInt());
			c->ass->Commit(_("add lead in"), AssFile::COMMIT_TIMES);
		}
	}
};

struct time_add_lead_out : public Command {
	CMD_NAME("time/lead/out")
	STR_MENU("Add lead out")
	STR_DISP("Add lead out")
	STR_HELP("Add lead out")
	void operator()(agi::Context *c) {
		if (AssDialogue *line = c->selectionController->GetActiveLine()) {
			line->End.SetMS(line->End.GetMS() + OPT_GET("Audio/Lead/OUT")->GetInt());
			c->ass->Commit(_("add lead out"), AssFile::COMMIT_TIMES);
		}
	}
};


/// Set start of selected subtitles to current video frame.
struct time_snap_start_video : public Command {
	CMD_NAME("time/snap/start_video")
	STR_MENU("Snap Start to Video")
	STR_DISP("Snap Start to Video")
	STR_HELP("Set start of selected subtitles to current video frame.")

	void operator()(agi::Context *c) {
		c->subsGrid->SetSubsToVideo(false);
	}
};


/// Sort all subtitles by their end times.
struct time_sort_end : public Command {
	CMD_NAME("time/sort/end")
	STR_MENU("&End Time")
	STR_DISP("End Time")
	STR_HELP("Sort all subtitles by their end times.")

	void operator()(agi::Context *c) {
		c->ass->Sort(AssFile::CompEnd);
		c->ass->Commit(_("sort"));
	}
};


/// Sort all subtitles by their start times.
struct time_sort_start : public Command {
	CMD_NAME("time/sort/start")
	STR_MENU("&Start Time")
	STR_DISP("Start Time")
	STR_HELP("Sort all subtitles by their start times.")

	void operator()(agi::Context *c) {
		c->ass->Sort();
		c->ass->Commit(_("sort"));
	}
};


/// Sort all subtitles by their style names.
struct time_sort_style : public Command {
	CMD_NAME("time/sort/style")
	STR_MENU("St&yle Name")
	STR_DISP("Style Name")
	STR_HELP("Sort all subtitles by their style names.")

	void operator()(agi::Context *c) {
		c->ass->Sort(AssFile::CompStyle);
		c->ass->Commit(_("sort"));
	}
};

/// Switch to the next timeable thing (line or syllable)
struct time_next : public Command {
	CMD_NAME("time/next")
	STR_MENU("Next line")
	STR_DISP("Next line")
	STR_HELP("Next line or syllable")
	void operator()(agi::Context *c) {
		c->audioController->Stop();
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->Next();
		c->audioController->PlayPrimaryRange();
	}
};

/// Switch to the previous timeable thing (line or syllable)
struct time_prev : public Command {
	CMD_NAME("time/prev")
	STR_MENU("Previous line")
	STR_DISP("Previous line")
	STR_HELP("Previous line or syllable")
	void operator()(agi::Context *c) {
		c->audioController->Stop();
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->Prev();
		c->audioController->PlayPrimaryRange();
	}
};

/// @}

/// Init time/ commands.
void init_time(CommandManager *cm) {
	cm->reg(new time_add_lead_in);
	cm->reg(new time_add_lead_out);
	cm->reg(new time_continuous_end);
	cm->reg(new time_continuous_start);
	cm->reg(new time_frame_current);
	cm->reg(new time_next);
	cm->reg(new time_prev);
	cm->reg(new time_shift);
	cm->reg(new time_snap_end_video);
	cm->reg(new time_snap_frame);
	cm->reg(new time_snap_scene);
	cm->reg(new time_snap_start_video);
	cm->reg(new time_sort_end);
	cm->reg(new time_sort_start);
	cm->reg(new time_sort_style);
}

} // namespace cmd
