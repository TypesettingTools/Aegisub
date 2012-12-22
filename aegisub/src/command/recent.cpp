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

/// @file recent.cpp
/// @brief recent/ commands, rebuild MRU-based lists.
/// @ingroup command
///

#include "../config.h"

#include <sstream>

#include <wx/event.h>
#include <wx/msgdlg.h>

#include "command.h"

#include "../include/aegisub/context.h"
#include "../audio_controller.h"
#include "../main.h"
#include "../frame_main.h"
#include "../compat.h"
#include "../video_context.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-recent MRU (Most Recently Used) commands.
/// @{

COMMAND_GROUP(recent_audio,     "recent/audio",     _("Recent"), _("Recent"), _("Open recent audio"));
COMMAND_GROUP(recent_keyframes, "recent/keyframe",  _("Recent"), _("Recent"), _("Open recent keyframes"));
COMMAND_GROUP(recent_subtitle,  "recent/subtitle",  _("Recent"), _("Recent"), _("Open recent subtitles"));
COMMAND_GROUP(recent_timecodes, "recent/timecodes", _("Recent"), _("Recent"), _("Open recent timecodes"));
COMMAND_GROUP(recent_video,     "recent/video",     _("Recent"), _("Recent"), _("Open recent video"));

struct recent_audio_entry : public Command {
	CMD_NAME("recent/audio/")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Open recent audio")

	void operator()(agi::Context *c, int id) {
		try {
			c->audioController->OpenAudio(to_wx(config::mru->GetEntry("Audio", id)));
		}
		catch (agi::UserCancelException const&) { }
		catch (agi::Exception const& e) {
			wxMessageBox(to_wx(e.GetChainedMessage()), "Error loading file", wxOK | wxICON_ERROR | wxCENTER, c->parent);
		}
	}
};

struct recent_keyframes_entry : public Command {
	CMD_NAME("recent/keyframes/")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Open recent keyframes")

	void operator()(agi::Context *c, int id) {
		c->videoController->LoadKeyframes(to_wx(config::mru->GetEntry("Keyframes", id)));
	}
};

struct recent_subtitle_entry : public Command {
	CMD_NAME("recent/subtitle/")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Open recent subtitles")

	void operator()(agi::Context *c, int id) {
		wxGetApp().frame->LoadSubtitles(to_wx(config::mru->GetEntry("Subtitle", id)));
	}
};

struct recent_timecodes_entry : public Command {
	CMD_NAME("recent/timecodes/")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Open recent timecodes")

	void operator()(agi::Context *c, int id) {
		c->videoController->LoadTimecodes(to_wx(config::mru->GetEntry("Timecodes", id)));
	}
};

struct recent_video_entry : public Command {
	CMD_NAME("recent/video/")
	STR_MENU("Recent")
	STR_DISP("Recent")
	STR_HELP("Open recent videos")

	void operator()(agi::Context *c, int id) {
		c->videoController->SetVideo(to_wx(config::mru->GetEntry("Video", id)));
	}
};

/// @class mru_wrapper
/// @brief Wrapper class for mru commands to
template<class T>
class mru_wrapper : public T {
	int id;
	std::string full_name;
public:
	const char *name() const { return full_name.c_str(); }
	void operator()(agi::Context *c) {
		T::operator()(c, id);
	}
	mru_wrapper(int id) : id(id) {
		std::stringstream ss;
		ss << T::name();
		ss << id;
		full_name = ss.str();
	}
};
}
/// @}

namespace cmd {
	void init_recent() {
		reg(new recent_audio);
		reg(new recent_keyframes);
		reg(new recent_subtitle);
		reg(new recent_timecodes);
		reg(new recent_video);

		/// @todo 16 is an implementation detail that maybe needs to be exposed
		for (int i = 0; i < 16; ++i) {
			reg(new mru_wrapper<recent_audio_entry>(i));
			reg(new mru_wrapper<recent_keyframes_entry>(i));
			reg(new mru_wrapper<recent_subtitle_entry>(i));
			reg(new mru_wrapper<recent_timecodes_entry>(i));
			reg(new mru_wrapper<recent_video_entry>(i));
		}
	}
}
