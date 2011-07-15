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

/// @file tool.cpp
/// @brief tool/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/utils.h>
#endif

#include "command.h"

#include "../include/aegisub/context.h"

#include "../dialog_fonts_collector.h"
#include "../standard_paths.h" // tool_assdraw
#include "../video_context.h" // tool_font_collector
#include "../dialog_export.h"
#include "../dialog_resample.h"
#include "../dialog_selection.h"
#include "../dialog_styling_assistant.h"
#include "../dialog_style_manager.h"
#include "../dialog_timing_processor.h"
#include "../dialog_translation.h"
#include "../dialog_kara_timing_copy.h"
#include "../subs_grid.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-tool Various tool and utilities
/// @{


/// Launch ASSDraw3 tool for vector drawing.
struct tool_assdraw : public Command {
	CMD_NAME("tool/assdraw")
	STR_MENU("ASSDraw3...")
	STR_DISP("ASSDraw3")
	STR_HELP("Launch ASSDraw3 tool for vector drawing.")

	void operator()(agi::Context *c) {
		wxExecute(_T("\"") + StandardPaths::DecodePath(_T("?data/ASSDraw3.exe")) + _T("\""));
	}
};

/// Saves a copy of subtitles with processing applied to it.
struct tool_export : public Command {
	CMD_NAME("tool/export")
	STR_MENU("Export Subtitles..")
	STR_DISP("Export Subtitles")
	STR_HELP("Saves a copy of subtitles with processing applied to it.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogExport(c->parent, c->ass).ShowModal();
	}
};


/// Open fonts collector.
struct tool_font_collector : public Command {
	CMD_NAME("tool/font_collector")
	STR_MENU("&Fonts Collector..")
	STR_DISP("Fonts Collector")
	STR_HELP("Open fonts collector.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogFontsCollector(c->parent, c->ass).ShowModal();
	}
};


/// Selects lines based on defined criterea.
struct tool_line_select : public Command {
	CMD_NAME("tool/line/select")
	STR_MENU("Select Lines..")
	STR_DISP("Select Lines")
	STR_HELP("Selects lines based on defined criterea.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogSelection(c->parent, c->subsGrid).ShowModal();
	}
};


/// Changes resolution and modifies subtitles to conform to change.
struct tool_resampleres : public Command {
	CMD_NAME("tool/resampleres")
	STR_MENU("Resample Resolution..")
	STR_DISP("Resample Resolution")
	STR_HELP("Changes resolution and modifies subtitles to conform to change.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogResample(c).ShowModal();
	}
};


/// Open styling assistant.
struct tool_style_assistant : public Command {
	CMD_NAME("tool/style/assistant")
	STR_MENU("St&yling Assistant..")
	STR_DISP("Styling Assistant")
	STR_HELP("Open styling assistant.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		if (!c->stylingAssistant) c->stylingAssistant = new DialogStyling(c);
		c->stylingAssistant->Show(true);
	}
};


/// Open styles manager.
struct tool_style_manager : public Command {
	CMD_NAME("tool/style/manager")
	STR_MENU("&Styles Manager..")
	STR_DISP("Styles Manager")
	STR_HELP("Open styles manager.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogStyleManager(c).ShowModal();
	}
};


/// Open Kanji timer.
struct tool_time_kanji : public Command {
	CMD_NAME("tool/time/kanji")
	STR_MENU("Kanji Timer..")
	STR_DISP("Kanji Timer")
	STR_HELP("Open Kanji timer.")

	void operator()(agi::Context *c) {
		DialogKanjiTimer(c).ShowModal();
	}
};


/// Launch timing post-processor.
struct tool_time_postprocess : public Command {
	CMD_NAME("tool/time/postprocess")
	STR_MENU("Timing Post-Processor..")
	STR_DISP("Timing Post-Processor")
	STR_HELP("Runs a post-processor for timing to deal with lead-ins, lead-outs, scene timing and etc.")

	void operator()(agi::Context *c) {
		DialogTimingProcessor(c).ShowModal();
	}
};


/// Open translation assistant.
struct tool_translation_assistant : public Command {
	CMD_NAME("tool/translation_assistant")
	STR_MENU("&Translation Assistant..")
	STR_DISP("Translation Assistant")
	STR_HELP("Open translation assistant.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		int start = c->subsGrid->GetFirstSelRow();
		if (start == -1) start = 0;
		DialogTranslation(c, start, true).ShowModal();
	}
};
}
/// @}

namespace cmd {
	void init_tool() {
		reg(new tool_assdraw);
		reg(new tool_export);
		reg(new tool_font_collector);
		reg(new tool_line_select);
		reg(new tool_resampleres);
		reg(new tool_style_assistant);
		reg(new tool_style_manager);
		reg(new tool_time_kanji);
		reg(new tool_time_postprocess);
		reg(new tool_translation_assistant);
	}
}
