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

/// @file menu.cpp
/// @brief menu/ commands, related to activating/deactivating/populating menu items
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#endif

#include "command.h"

#include "../include/aegisub/context.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-menu Main menu dropdown and submenu related commands.
/// @{

COMMAND_GROUP(main_audio, "main/audio", "Audio", "Audio", "Audio manipulation.");
COMMAND_GROUP(main_automation, "main/automation", "Automation", "Automation", "Automation manipulation and scripts.");
COMMAND_GROUP(main_edit, "main/edit", "&Edit", "Edit", "Editing operations.");
COMMAND_GROUP(main_edit_sort_lines, "main/edit/sort lines", "Sort lines", "Sort lines", "Sort lines by column.");
COMMAND_GROUP(main_file, "main/file", "&File", "File", "Operations on subtitles.");
COMMAND_GROUP(main_help, "main/help", "Help", "Help", "Help options.");
COMMAND_GROUP(main_subtitle, "main/subtitle", "&Subtitle", "Subtitle", "Subtitle manipulation.");
COMMAND_GROUP(main_subtitle_insert_lines, "main/subtitle/insert lines", "&Insert Lines", "Insert Lines", "Insert lines into currently active subtitle file.");
COMMAND_GROUP(main_subtitle_sort_lines, "main/subtitle/sort lines", "Sort Lines", "Sort Lines", "Sort lines by column.");
COMMAND_GROUP(main_subtitle_join_lines, "main/subtitle/join lines", "Join Lines", "Join Lines", "Merge 2 or more lines together.");
COMMAND_GROUP(main_timing, "main/timing", "&Timing", "Timing", "Time manipulation.");
COMMAND_GROUP(main_timing_make_times_continuous, "main/timing/make times continuous", "Make Times Continuous", "Make Times Continuous", "Make time continuous.");
COMMAND_GROUP(main_video, "main/video", "&Video", "Video", "Video operations.");
COMMAND_GROUP(main_video_override_ar, "main/video/override ar", "Override AR", "Override AR", "Override Aspect Ratio");
COMMAND_GROUP(main_video_set_zoom, "main/video/set zoom", "Set Zoom", "Set Zoom", "Set zoom level.");
COMMAND_GROUP(main_view, "main/view", "View", "View", "View options.");
}

/// @}

namespace cmd {
	void init_menu() {
		reg(new main_audio);
		reg(new main_automation);
		reg(new main_edit);
		reg(new main_edit_sort_lines);
		reg(new main_file);
		reg(new main_help);
		reg(new main_subtitle);
		reg(new main_subtitle_insert_lines);
		reg(new main_subtitle_join_lines);
		reg(new main_subtitle_sort_lines);
		reg(new main_timing);
		reg(new main_timing_make_times_continuous);
		reg(new main_video);
		reg(new main_video_override_ar);
		reg(new main_video_set_zoom);
		reg(new main_view);
	}
}
