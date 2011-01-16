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

#include "../include/aegisub/context.h"
#include "../subs_grid.h"
#include "../video_context.h"
#include "../ass_dialogue.h"
#include "../dialog_shift_times.h"
#include "../ass_file.h"

namespace cmd {
/// @defgroup cmd-time Time manipulation commands.
/// @{


/// Changes times of subs so end times begin on next's start time.
class time_continous_end: public Command {
public:
	CMD_NAME("time/continous/end")
	STR_MENU("Change &End")
	STR_DISP("Change End")
	STR_HELP("Changes times of subs so end times begin on next's start time.")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->SubsGrid->GetSelection();
		c->SubsGrid->AdjoinLines(sels.front(), sels.back(), false);
	}
};


/// Changes times of subs so start times begin on previous's end time.
class time_continous_start: public Command {
public:
	CMD_NAME("time/continous/start")
	STR_MENU("Change &Start")
	STR_DISP("Change Start")
	STR_HELP("Changes times of subs so start times begin on previous's end time.")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->SubsGrid->GetSelection();
		c->SubsGrid->AdjoinLines(sels.front(), sels.back(), true);
	}

};


/// Shift selection so first selected line starts at current frame.
class time_frame_current: public Command {
public:
	CMD_NAME("time/frame/current")
	STR_MENU("Shift to Current Frame")
	STR_DISP("Shift to Current Frame")
	STR_HELP("Shift selection so first selected line starts at current frame.")

	void operator()(agi::Context *c) {
		if (!c->videoContext->IsLoaded()) return;

		wxArrayInt sels = c->SubsGrid->GetSelection();
		size_t n=sels.Count();
		if (n == 0) return;

		// Get shifting in ms
		AssDialogue *cur = c->SubsGrid->GetDialogue(sels[0]);
		if (!cur) return;
		int shiftBy = c->videoContext->TimeAtFrame(c->videoContext->GetFrameN(),agi::vfr::START) - cur->Start.GetMS();

		// Update
		for (size_t i=0;i<n;i++) {
			cur = c->SubsGrid->GetDialogue(sels[i]);
			if (cur) {
				cur->Start.SetMS(cur->Start.GetMS()+shiftBy);
				cur->End.SetMS(cur->End.GetMS()+shiftBy);
			}
		}

		// Commit
		c->SubsGrid->ass->Commit(_("shift to frame"), AssFile::COMMIT_TIMES);
	}
};


/// Shift subtitles by time or frames.
class time_shift: public Command {
public:
	CMD_NAME("time/shift")
	STR_MENU("S&hift Times..")
	STR_DISP("Shift Times")
	STR_HELP("Shift subtitles by time or frames.")

	void operator()(agi::Context *c) {
		c->videoContext->Stop();
		DialogShiftTimes(c->parent, c->SubsGrid).ShowModal();
	}
};



/// Set end of selected subtitles to current video frame.
class time_snap_end_video: public Command {
public:
	CMD_NAME("time/snap/end_video")
	STR_MENU("Snap End to Video")
	STR_DISP("Snap End to Video")
	STR_HELP("Set end of selected subtitles to current video frame.")

	void operator()(agi::Context *c) {
		c->SubsGrid->SetSubsToVideo(false);
	}
};


/// Shift selected subtitles so first selected starts at this frame.
class time_snap_frame: public Command {
public:
	CMD_NAME("time/snap/frame")
	STR_MENU("Shift Subtitles to Frame")
	STR_DISP("Shift Subtitles to Frame")
	STR_HELP("Shift selected subtitles so first selected starts at this frame.")

	void operator()(agi::Context *c) {
		if (c->videoContext->IsLoaded()) return;

		wxArrayInt sels = c->SubsGrid->GetSelection();
		if (sels.empty()) return;

		AssDialogue *cur = c->SubsGrid->GetDialogue(sels[0]);
		if (!cur) return;
		int shiftBy = c->videoContext->TimeAtFrame(c->videoContext->GetFrameN(),agi::vfr::START) - cur->Start.GetMS();

		for (size_t i = 0; i < sels.size(); ++i) {
			if (cur = c->SubsGrid->GetDialogue(sels[i])) {
				cur->Start.SetMS(cur->Start.GetMS() + shiftBy);
				cur->End.SetMS(cur->End.GetMS() + shiftBy);
			}
		}

		c->ass->Commit(_("shift to frame"));
	}
};


/// Set start and end of subtitles to the keyframes around current video frame.
class time_snap_scene: public Command {
public:
	CMD_NAME("time/snap/scene")
	STR_MENU("Snap to Scene")
	STR_DISP("Snap to Scene")
	STR_HELP("Set start and end of subtitles to the keyframes around current video frame.")

	void operator()(agi::Context *c) {
		VideoContext *con = c->videoContext;
		if (!con->IsLoaded() || !con->KeyFramesLoaded()) return;

		// Get frames
		wxArrayInt sel = c->SubsGrid->GetSelection();
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
			cur = c->SubsGrid->GetDialogue(sel[i]);
			cur->Start.SetMS(start_ms);
			cur->End.SetMS(end_ms);
		}

		// Commit
		c->SubsGrid->ass->Commit(_("snap to scene"), AssFile::COMMIT_TIMES);
	}
};


/// Set start of selected subtitles to current video frame.
class time_snap_start_video: public Command {
public:
	CMD_NAME("time/snap/start_video")
	STR_MENU("Snap Start to Video")
	STR_DISP("Snap Start to Video")
	STR_HELP("Set start of selected subtitles to current video frame.")

	void operator()(agi::Context *c) {
		c->SubsGrid->SetSubsToVideo(false);
	}
};


/// Sort all subtitles by their end times.
class time_sort_end: public Command {
public:
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
class time_sort_start: public Command {
public:
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
class time_sort_style: public Command {
public:
	CMD_NAME("time/sort/style")
	STR_MENU("St&yle Name")
	STR_DISP("Style Name")
	STR_HELP("Sort all subtitles by their style names.")

	void operator()(agi::Context *c) {
		c->ass->Sort(AssFile::CompStyle);
		c->ass->Commit(_("sort"));
	}
};

/// @}

/// Init time/ commands.
void init_time(CommandManager *cm) {
	cm->reg(new time_continous_end());
	cm->reg(new time_continous_start());
	cm->reg(new time_frame_current());
	cm->reg(new time_shift());
	cm->reg(new time_snap_end_video());
	cm->reg(new time_snap_frame());
	cm->reg(new time_snap_scene());
	cm->reg(new time_snap_start_video());
	cm->reg(new time_sort_end());
	cm->reg(new time_sort_start());
	cm->reg(new time_sort_style());
}

} // namespace cmd
