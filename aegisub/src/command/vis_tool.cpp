// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file vis_tool.cpp
/// @brief Visual typesetting tools commands
/// @ingroup command visual_ui
///

#include "../config.h"

#include "command.h"

#include "../include/aegisub/context.h"
#include "../video_box.h"
#include "../video_context.h"
#include "../video_display.h"
#include "../visual_tool_clip.h"
#include "../visual_tool_cross.h"
#include "../visual_tool_drag.h"
#include "../visual_tool_rotatexy.h"
#include "../visual_tool_rotatez.h"
#include "../visual_tool_scale.h"
#include "../visual_tool_vector_clip.h"

namespace {
	using cmd::Command;
	/// @defgroup cmd-visual Visual typesetting tools commands
	/// @{

	template<class T>
	struct visual_tool_command : public Command {
		CMD_TYPE(COMMAND_VALIDATE | COMMAND_RADIO)

		bool Validate(const agi::Context *c) {
			return c->videoController->IsLoaded();
		}

		bool IsActive(const agi::Context *c) {
			return c->videoDisplay->ToolIsType(typeid(T));
		}

		void operator()(agi::Context *c) {
			c->videoDisplay->SetTool(new T(c->videoDisplay, c));
		}
	};

	struct visual_mode_cross : public visual_tool_command<VisualToolCross> {
		CMD_NAME("video/tool/cross")
		STR_MENU("Standard")
		STR_DISP("Standard")
		STR_HELP("Standard mode, double click sets position")
	};

	struct visual_mode_drag : public visual_tool_command<VisualToolDrag> {
		CMD_NAME("video/tool/drag")
		STR_MENU("Drag")
		STR_DISP("Drag")
		STR_HELP("Drag subtitles")
	};

	struct visual_mode_rotate_z : public visual_tool_command<VisualToolRotateZ> {
		CMD_NAME("video/tool/rotate/z")
		STR_MENU("Rotate Z")
		STR_DISP("Rotate Z")
		STR_HELP("Rotate subtitles on their Z axis")
	};

	struct visual_mode_rotate_xy : public visual_tool_command<VisualToolRotateXY> {
		CMD_NAME("video/tool/rotate/xy")
		STR_MENU("Rotate XY")
		STR_DISP("Rotate XY")
		STR_HELP("Rotate subtitles on their X and Y axes")
	};

	struct visual_mode_scale : public visual_tool_command<VisualToolScale> {
		CMD_NAME("video/tool/scale")
		STR_MENU("Scale")
		STR_DISP("Scale")
		STR_HELP("Scale subtitles on X and Y axes")
	};

	struct visual_mode_clip : public visual_tool_command<VisualToolClip> {
		CMD_NAME("video/tool/clip")
		STR_MENU("Clip")
		STR_DISP("Clip")
		STR_HELP("Clip subtitles to a rectangle")
	};

	struct visual_mode_vector_clip : public visual_tool_command<VisualToolVectorClip> {
		CMD_NAME("video/tool/vector_clip")
		STR_MENU("Vector Clip")
		STR_DISP("Vector Clip")
		STR_HELP("Clip subtitles to a vectorial area")
	};
}

/// @}

namespace cmd {
	void init_visual_tools() {
		reg(new visual_mode_cross);
		reg(new visual_mode_drag);
		reg(new visual_mode_rotate_z);
		reg(new visual_mode_rotate_xy);
		reg(new visual_mode_scale);
		reg(new visual_mode_clip);
		reg(new visual_mode_vector_clip);
	}
}
