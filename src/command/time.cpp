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

#include "../config.h"

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../audio_controller.h"
#include "../audio_timing.h"
#include "../dialog_manager.h"
#include "../dialog_shift_times.h"
#include "../include/aegisub/context.h"
#include "../options.h"
#include "../selection_controller.h"
#include "../video_context.h"

#include <libaegisub/of_type_adaptor.h>
#include <libaegisub/util.h>

#include <algorithm>

namespace {
	using cmd::Command;

	struct validate_video_loaded : public Command {
		CMD_TYPE(COMMAND_VALIDATE)
		bool Validate(const agi::Context *c) override {
			return c->videoController->IsLoaded();
		}
	};

	struct validate_adjoinable : public Command {
		CMD_TYPE(COMMAND_VALIDATE)
		bool Validate(const agi::Context *c) override {
			SubtitleSelection const& sel = c->selectionController->GetSelectedSet();
			if (sel.size() < 2) return !sel.empty();

			size_t found = 0;
			for (auto diag : c->ass->Line | agi::of_type<AssDialogue>()) {
				if (sel.count(diag)) {
					if (++found == sel.size())
						return true;
				}
				else if (found)
					return false;
			}
			return true;
		}
	};

static void adjoin_lines(agi::Context *c, bool set_start) {
	auto const& sel = c->selectionController->GetSelectedSet();
	AssDialogue *prev = nullptr;
	size_t seen = 0;
	bool prev_sel = false;
	for (auto diag : c->ass->Line | agi::of_type<AssDialogue>()) {
		bool cur_sel = !!sel.count(diag);
		if (prev) {
			// One row selections act as if the previous or next line was selected
			if (set_start && cur_sel && (sel.size() == 1 || prev_sel))
				diag->Start = prev->End;
			else if (!set_start && prev_sel && (cur_sel || sel.size() == 1))
				prev->End = diag->Start;
		}

		if (seen == sel.size())
			break;

		if (cur_sel)
			++seen;

		prev = diag;
		prev_sel = cur_sel;
	}

	c->ass->Commit(_("adjoin"), AssFile::COMMIT_DIAG_TIME);
}

struct time_continuous_end : public validate_adjoinable {
	CMD_NAME("time/continuous/end")
	STR_MENU("Change &End")
	STR_DISP("Change End")
	STR_HELP("Change end times of lines to the next line's start time")

	void operator()(agi::Context *c) override {
		adjoin_lines(c, false);
	}
};

struct time_continuous_start : public validate_adjoinable {
	CMD_NAME("time/continuous/start")
	STR_MENU("Change &Start")
	STR_DISP("Change Start")
	STR_HELP("Change start times of lines to the previous line's end time")

	void operator()(agi::Context *c) override {
		adjoin_lines(c, true);
	}
};

struct time_frame_current : public validate_video_loaded {
	CMD_NAME("time/frame/current")
	STR_MENU("Shift to &Current Frame")
	STR_DISP("Shift to Current Frame")
	STR_HELP("Shift selection so that the active line starts at current frame")

	void operator()(agi::Context *c) override {
		if (!c->videoController->IsLoaded()) return;

		SubtitleSelection const& sel = c->selectionController->GetSelectedSet();
		const AssDialogue *active_line = c->selectionController->GetActiveLine();

		if (sel.empty() || !active_line) return;

		int target_start = std::max(0, c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::START));
		int shift_by = target_start - active_line->Start;

		for (auto line : sel) {
			line->Start = line->Start + shift_by;
			line->End = line->End + shift_by;
		}

		c->ass->Commit(_("shift to frame"), AssFile::COMMIT_DIAG_TIME);
	}
};

struct time_shift : public Command {
	CMD_NAME("time/shift")
	STR_MENU("S&hift Times...")
	STR_DISP("Shift Times")
	STR_HELP("Shift subtitles by time or frames")

	void operator()(agi::Context *c) override {
		c->dialog->Show<DialogShiftTimes>(c);
	}
};

static void snap_subs_video(agi::Context *c, bool set_start) {
	SubtitleSelection const& sel = c->selectionController->GetSelectedSet();

	if (!c->videoController->IsLoaded() || sel.empty()) return;

	int start = c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::START);
	int end = c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::END);

	for (auto line : sel) {
		if (set_start || line->Start > start)
			line->Start = start;
		if (!set_start || line->End < end)
			line->End = end;
	}

	c->ass->Commit(_("timing"), AssFile::COMMIT_DIAG_TIME);
}

struct time_snap_end_video : public validate_video_loaded {
	CMD_NAME("time/snap/end_video")
	STR_MENU("Snap &End to Video")
	STR_DISP("Snap End to Video")
	STR_HELP("Set end of selected subtitles to current video frame")

	void operator()(agi::Context *c) override {
		snap_subs_video(c, false);
	}
};

struct time_snap_scene : public validate_video_loaded {
	CMD_NAME("time/snap/scene")
	STR_MENU("Snap to S&cene")
	STR_DISP("Snap to Scene")
	STR_HELP("Set start and end of subtitles to the keyframes around current video frame")

	void operator()(agi::Context *c) override {
		VideoContext *con = c->videoController;
		if (!con->IsLoaded() || !con->KeyFramesLoaded()) return;

		int curFrame = con->GetFrameN();
		int prev = 0;
		int next = 0;

		auto const& keyframes = con->GetKeyFrames();
		if (curFrame < keyframes.front())
			next = keyframes.front();
		else if (curFrame >= keyframes.back()) {
			prev = keyframes.back();
			next = con->GetLength();
		}
		else {
			auto kf = std::lower_bound(keyframes.begin(), keyframes.end(), curFrame);
			if (*kf == curFrame) {
				prev = *kf;
				next = *(kf + 1);
			}
			else {
				prev = *(kf - 1);
				next = *kf;
			}
		}

		int start_ms = con->TimeAtFrame(prev,agi::vfr::START);
		int end_ms = con->TimeAtFrame(next-1,agi::vfr::END);

		for (auto line : c->selectionController->GetSelectedSet()) {
			line->Start = start_ms;
			line->End = end_ms;
		}

		c->ass->Commit(_("snap to scene"), AssFile::COMMIT_DIAG_TIME);
	}
};

struct time_add_lead_both : public Command {
	CMD_NAME("time/lead/both")
	STR_MENU("Add lead in and out")
	STR_DISP("Add lead in and out")
	STR_HELP("Add both lead in and out to the selected lines")
	void operator()(agi::Context *c) override {
		if (AudioTimingController *tc = c->audioController->GetTimingController()) {
			tc->AddLeadIn();
			tc->AddLeadOut();
		}
	}
};

struct time_add_lead_in : public Command {
	CMD_NAME("time/lead/in")
	STR_MENU("Add lead in")
	STR_DISP("Add lead in")
	STR_HELP("Add the lead in time to the selected lines")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->AddLeadIn();
	}
};

struct time_add_lead_out : public Command {
	CMD_NAME("time/lead/out")
	STR_MENU("Add lead out")
	STR_DISP("Add lead out")
	STR_HELP("Add the lead out time to the selected lines")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->AddLeadOut();
	}
};

struct time_length_increase : public Command {
	CMD_NAME("time/length/increase")
	STR_MENU("Increase length")
	STR_DISP("Increase length")
	STR_HELP("Increase the length of the current timing unit")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->ModifyLength(1, false);
	}
};

struct time_length_increase_shift : public Command {
	CMD_NAME("time/length/increase/shift")
	STR_MENU("Increase length and shift")
	STR_DISP("Increase length and shift")
	STR_HELP("Increase the length of the current timing unit and shift the following items")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->ModifyLength(1, true);
	}
};

struct time_length_decrease : public Command {
	CMD_NAME("time/length/decrease")
	STR_MENU("Decrease length")
	STR_DISP("Decrease length")
	STR_HELP("Decrease the length of the current timing unit")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->ModifyLength(-1, false);
	}
};

struct time_length_decrease_shift : public Command {
	CMD_NAME("time/length/decrease/shift")
	STR_MENU("Decrease length and shift")
	STR_DISP("Decrease length and shift")
	STR_HELP("Decrease the length of the current timing unit and shift the following items")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->ModifyLength(-1, true);
	}
};

struct time_start_increase : public Command {
	CMD_NAME("time/start/increase")
	STR_MENU("Shift start time forward")
	STR_DISP("Shift start time forward")
	STR_HELP("Shift the start time of the current timing unit forward")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->ModifyStart(1);
	}
};

struct time_start_decrease : public Command {
	CMD_NAME("time/start/decrease")
	STR_MENU("Shift start time backward")
	STR_DISP("Shift start time backward")
	STR_HELP("Shift the start time of the current timing unit backward")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->ModifyStart(-1);
	}
};

struct time_snap_start_video : public validate_video_loaded {
	CMD_NAME("time/snap/start_video")
	STR_MENU("Snap &Start to Video")
	STR_DISP("Snap Start to Video")
	STR_HELP("Set start of selected subtitles to current video frame")

	void operator()(agi::Context *c) override {
		snap_subs_video(c, true);
	}
};

struct time_next : public Command {
	CMD_NAME("time/next")
	STR_MENU("Next Line")
	STR_DISP("Next Line")
	STR_HELP("Next line or syllable")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->Next(AudioTimingController::TIMING_UNIT);
	}
};

struct time_prev : public Command {
	CMD_NAME("time/prev")
	STR_MENU("Previous Line")
	STR_DISP("Previous Line")
	STR_HELP("Previous line or syllable")
	void operator()(agi::Context *c) override {
		if (c->audioController->GetTimingController())
			c->audioController->GetTimingController()->Prev();
	}
};
}

namespace cmd {
	void init_time() {
		reg(agi::util::make_unique<time_add_lead_both>());
		reg(agi::util::make_unique<time_add_lead_in>());
		reg(agi::util::make_unique<time_add_lead_out>());
		reg(agi::util::make_unique<time_continuous_end>());
		reg(agi::util::make_unique<time_continuous_start>());
		reg(agi::util::make_unique<time_frame_current>());
		reg(agi::util::make_unique<time_length_decrease>());
		reg(agi::util::make_unique<time_length_decrease_shift>());
		reg(agi::util::make_unique<time_length_increase>());
		reg(agi::util::make_unique<time_length_increase_shift>());
		reg(agi::util::make_unique<time_next>());
		reg(agi::util::make_unique<time_prev>());
		reg(agi::util::make_unique<time_shift>());
		reg(agi::util::make_unique<time_snap_end_video>());
		reg(agi::util::make_unique<time_snap_scene>());
		reg(agi::util::make_unique<time_snap_start_video>());
		reg(agi::util::make_unique<time_start_decrease>());
		reg(agi::util::make_unique<time_start_increase>());
	}
}
