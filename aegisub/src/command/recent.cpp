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

/// @file recent.cpp
/// @brief recent/ commands, rebuild MRU-based lists.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/event.h>
#endif

#include "command.h"

#include "../include/aegisub/context.h"
#include "../main.h"
#include "../frame_main.h"
#include "../compat.h"
#include "../video_context.h"

namespace cmd {
/// @defgroup cmd-recent MRU (Most Recently Used) commands.
/// @{


/// Open recent audio.
class recent_audio: public Command {
public:
	CMD_NAME("recent/audio")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Open recent audio.")

	void operator()(agi::Context *c) {
//		c->audioController->OpenAudio(lagi_wxString(config::mru->GetEntry("Audio", event.GetId()-cmd::id("recent/audio"))));
	}
};


/// Recent keyframes.
class recent_keyframe: public Command {
public:
	CMD_NAME("recent/keyframe")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Recent keyframes.")

	void operator()(agi::Context *c) {
//		VideoContext::Get()->LoadKeyframes(lagi_wxString(config::mru->GetEntry("Keyframes", event.GetId()-cmd::id("recent/keyframe"))));
	}
};


/// Recently opened subtitles.
class recent_subtitle: public Command {
public:
	CMD_NAME("recent/subtitle")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Recently opened subtitles.")

	void operator()(agi::Context *c) {
//		int number = event.GetId()-cmd::id("recent/subtitle");
//		wxGetApp().frame->LoadSubtitles(lagi_wxString(config::mru->GetEntry("Subtitle", number)));
	}
};


/// Recent timecodes.
class recent_timecode: public Command {
public:
	CMD_NAME("recent/timecode")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Recent timecodes.")

	void operator()(agi::Context *c) {
//		int number = event.GetId()-cmd::id("recent/timecode");
//		wxGetApp().frame->LoadVFR(lagi_wxString(config::mru->GetEntry("Timecodes", number)));
	}
};


/// Recently opened videos.
class recent_video: public Command {
public:
	CMD_NAME("recent/video")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Recently opened videos.")

	void operator()(agi::Context *c) {
//		int number = event.GetId()-cmd::id("recent/video");
//		wxGetApp().frame->LoadVideo(lagi_wxString(config::mru->GetEntry("Video", number)));
	}
};

/// @}

/// Init recent/ commands.
void init_recent(CommandManager *cm) {
	cm->reg(new recent_audio());
	cm->reg(new recent_keyframe());
	cm->reg(new recent_subtitle());
	cm->reg(new recent_timecode());
	cm->reg(new recent_video());
}

} // namespace cmd
