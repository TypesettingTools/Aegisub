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

/// @file subtitle.cpp
/// @brief subtitle/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/choicdlg.h>
#endif

#include <libaegisub/charset_conv.h>

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../compat.h"
#include "../dialog_attachments.h"
#include "../dialog_properties.h"
#include "../dialog_search_replace.h"
#include "../dialog_spellchecker.h"
#include "../frame_main.h"
#include "../include/aegisub/context.h"
#include "../main.h"
#include "../subs_grid.h"
#include "../video_context.h"
#include "../utils.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-subtitle Subtitle commands.
/// @{

struct validate_nonempty_selection : public Command {
	CMD_TYPE(COMMAND_VALIDATE)
	bool Validate(const agi::Context *c) {
		return !c->selectionController->GetSelectedSet().empty();
	}
};

struct validate_nonempty_selection_video_loaded : public Command {
	CMD_TYPE(COMMAND_VALIDATE)
	bool Validate(const agi::Context *c) {
		return c->videoController->IsLoaded() && !c->selectionController->GetSelectedSet().empty();
	}
};

/// Open the attachment list.
struct subtitle_attachment : public Command {
	CMD_NAME("subtitle/attachment")
	STR_MENU("&Attachments..")
	STR_DISP("Attachments")
	STR_HELP("Open the attachment list.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogAttachments(c->parent, c->ass).ShowModal();
	}
};


/// Find words in subtitles.
struct subtitle_find : public Command {
	CMD_NAME("subtitle/find")
	STR_MENU("&Find..")
	STR_DISP("Find")
	STR_HELP("Find words in subtitles.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		Search.OpenDialog(false);
	}
};


/// Find next match of last word.
struct subtitle_find_next : public Command {
	CMD_NAME("subtitle/find/next")
	STR_MENU("Find Next")
	STR_DISP("Find Next")
	STR_HELP("Find next match of last word.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		Search.FindNext();
	}
};

static void insert_subtitle_at_video(agi::Context *c, bool after) {
	int n = c->subsGrid->GetFirstSelRow();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = c->videoController->TimeAtFrame(c->videoController->GetFrameN(), agi::vfr::START);
	def->Start.SetMS(video_ms);
	def->End.SetMS(video_ms + OPT_GET("Timing/Default Duration")->GetInt());
	def->Style = c->subsGrid->GetDialogue(n)->Style;

	// Insert it
	c->subsGrid->BeginBatch();
	c->subsGrid->InsertLine(def, n, after);
	c->subsGrid->SelectRow(n + after);
	c->subsGrid->SetActiveLine(def);
	c->subsGrid->EndBatch();
}

/// Inserts a line after current.
struct subtitle_insert_after : public validate_nonempty_selection {
	CMD_NAME("subtitle/insert/after")
	STR_MENU("&After Current")
	STR_DISP("After Current")
	STR_HELP("Inserts a line after current.")

	void operator()(agi::Context *c) {
		int n = c->subsGrid->GetFirstSelRow();
		int nrows = c->subsGrid->GetRows();

		// Create line to add
		AssDialogue *def = new AssDialogue;
		if (n == nrows-1) {
			def->Start = c->subsGrid->GetDialogue(n)->End;
			def->End = c->subsGrid->GetDialogue(n)->End;
			def->End.SetMS(def->End.GetMS()+OPT_GET("Timing/Default Duration")->GetInt());
		}
		else {
			def->Start = c->subsGrid->GetDialogue(n)->End;
			def->End = c->subsGrid->GetDialogue(n+1)->Start;
		}
		if (def->End.GetMS() < def->Start.GetMS()) def->End.SetMS(def->Start.GetMS()+OPT_GET("Timing/Default Duration")->GetInt());
		def->Style = c->subsGrid->GetDialogue(n)->Style;

		// Insert it
		c->subsGrid->BeginBatch();
		c->subsGrid->InsertLine(def, n, true);
		c->subsGrid->SelectRow(n + 1);
		c->subsGrid->SetActiveLine(def);
		c->subsGrid->EndBatch();
	}
};

/// Inserts a line after current, starting at video time.
struct subtitle_insert_after_videotime : public validate_nonempty_selection_video_loaded {
	CMD_NAME("subtitle/insert/after/videotime")
	STR_MENU("After Current, at Video Time")
	STR_DISP("After Current, at Video Time")
	STR_HELP("Inserts a line after current, starting at video time.")

	void operator()(agi::Context *c) {
		insert_subtitle_at_video(c, true);
	}
};


/// Inserts a line before current.
struct subtitle_insert_before : public validate_nonempty_selection {
	CMD_NAME("subtitle/insert/before")
	STR_MENU("&Before Current")
	STR_DISP("Before Current")
	STR_HELP("Inserts a line before current.")

	void operator()(agi::Context *c) {
		int n = c->subsGrid->GetFirstSelRow();

		// Create line to add
		AssDialogue *def = new AssDialogue;
		if (n == 0) {
			def->Start.SetMS(0);
			def->End = c->subsGrid->GetDialogue(n)->Start;
		}
		else if (c->subsGrid->GetDialogue(n-1)->End.GetMS() > c->subsGrid->GetDialogue(n)->Start.GetMS()) {
			def->Start.SetMS(c->subsGrid->GetDialogue(n)->Start.GetMS()-OPT_GET("Timing/Default Duration")->GetInt());
			def->End = c->subsGrid->GetDialogue(n)->Start;
		}
		else {
			def->Start = c->subsGrid->GetDialogue(n-1)->End;
			def->End = c->subsGrid->GetDialogue(n)->Start;
		}
		if (def->End.GetMS() < def->Start.GetMS()) def->End.SetMS(def->Start.GetMS()+OPT_GET("Timing/Default Duration")->GetInt());
		def->Style = c->subsGrid->GetDialogue(n)->Style;

		// Insert it
		c->subsGrid->BeginBatch();
		c->subsGrid->InsertLine(def, n, false);
		c->subsGrid->SelectRow(n);
		c->subsGrid->SetActiveLine(def);
		c->subsGrid->EndBatch();
	}
};


/// Inserts a line before current, starting at video time.
struct subtitle_insert_before_videotime : public validate_nonempty_selection_video_loaded {
	CMD_NAME("subtitle/insert/before/videotime")
	STR_MENU("Before Current, at Video Time")
	STR_DISP("Before Current, at Video Time")
	STR_HELP("Inserts a line before current, starting at video time.")

	void operator()(agi::Context *c) {
		insert_subtitle_at_video(c, false);
	}
};


/// New subtitles.
struct subtitle_new : public Command {
	CMD_NAME("subtitle/new")
	STR_MENU("&New Subtitles")
	STR_DISP("New Subtitles")
	STR_HELP("New subtitles.")

	void operator()(agi::Context *c) {
		c->ass->LoadDefault();
	}
};


/// Opens a subtitles file.
struct subtitle_open : public Command {
	CMD_NAME("subtitle/open")
	STR_MENU("&Open Subtitles..")
	STR_DISP("Open Subtitles")
	STR_HELP("Opens a subtitles file.")

	void operator()(agi::Context *c) {
		wxString path = lagi_wxString(OPT_GET("Path/Last/Subtitles")->GetString()); 
		wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),AssFile::GetWildcardList(0),wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (!filename.empty()) {
			wxGetApp().frame->LoadSubtitles(filename);
		}
	}
};


/// Opens a subtitles file with a specific charset.
struct subtitle_open_charset : public Command {
	CMD_NAME("subtitle/open/charset")
	STR_MENU("&Open Subtitles with Charset..")
	STR_DISP("Open Subtitles with Charset")
	STR_HELP("Opens a subtitles file with a specific charset.")

	void operator()(agi::Context *c) {
		// Initialize charsets
		wxString path = lagi_wxString(OPT_GET("Path/Last/Subtitles")->GetString());

		// Get options and load
		wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),AssFile::GetWildcardList(0),wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (!filename.empty()) {
			wxString charset = wxGetSingleChoice(_("Choose charset code:"), _("Charset"), agi::charset::GetEncodingsList<wxArrayString>(), c->parent, -1, -1, true, 250, 200);
			if (!charset.empty()) {
				wxGetApp().frame->LoadSubtitles(filename,charset);
			}
		}
	}
};


/// Opens the subtitles from the current video file.
struct subtitle_open_video : public Command {
	CMD_NAME("subtitle/open/video")
	STR_MENU("Open Subtitles from &Video")
	STR_DISP("Open Subtitles from Video")
	STR_HELP("Opens the subtitles from the current video file.")
	CMD_TYPE(COMMAND_VALIDATE)

	void operator()(agi::Context *c) {
		wxGetApp().frame->LoadSubtitles(c->videoController->videoName, "binary");
	}

	bool Validate(const agi::Context *c) {
		return c->videoController->IsLoaded() && c->videoController->HasSubtitles();
	}
};


/// Open script properties window.
struct subtitle_properties : public Command {
	CMD_NAME("subtitle/properties")
	STR_MENU("&Properties..")
	STR_DISP("Properties")
	STR_HELP("Open script properties window.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		DialogProperties(c).ShowModal();
	}
};

static void save_subtitles(agi::Context *c, wxString filename) {
	if (filename.empty()) {
		c->videoController->Stop();
		wxString path = lagi_wxString(OPT_GET("Path/Last/Subtitles")->GetString());
		wxFileName origPath(c->ass->filename);
		filename = wxFileSelector(_("Save subtitles file"), path, origPath.GetName() + ".ass", "ass", AssFile::GetWildcardList(1), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, c->parent);
		if (filename.empty()) return;
	}

	try {
		c->ass->Save(filename, true, true);
	}
	catch (const agi::Exception& err) {
		wxMessageBox(lagi_wxString(err.GetMessage()), "Error", wxOK | wxICON_ERROR, NULL);
	}
	catch (const wchar_t *err) {
		wxMessageBox(err, "Error", wxOK | wxICON_ERROR, NULL);
	}
	catch (...) {
		wxMessageBox("Unknown error", "Error", wxOK | wxICON_ERROR, NULL);
	}
}

/// Saves subtitles.
struct subtitle_save : public Command {
	CMD_NAME("subtitle/save")
	STR_MENU("&Save Subtitles")
	STR_DISP("Save Subtitles")
	STR_HELP("Saves subtitles.")

	void operator()(agi::Context *c) {
		save_subtitles(c, c->ass->CanSave() ? c->ass->filename : "");
	}
};


/// Saves subtitles with another name.
struct subtitle_save_as : public Command {
	CMD_NAME("subtitle/save/as")
	STR_MENU("Save Subtitles as..")
	STR_DISP("Save Subtitles as")
	STR_HELP("Saves subtitles with another name.")

	void operator()(agi::Context *c) {
		save_subtitles(c, "");
	}
};

/// Selects all dialogue lines
struct subtitle_select_all : public Command {
	CMD_NAME("subtitle/select/all")
	STR_MENU("Select All")
	STR_DISP("Select All")
	STR_HELP("Selects all dialogue lines.")

	void operator()(agi::Context *c) {
		SelectionController<AssDialogue>::Selection sel;
		transform(c->ass->Line.begin(), c->ass->Line.end(),
			inserter(sel, sel.begin()), cast<AssDialogue*>());
		sel.erase(0);
		c->selectionController->SetSelectedSet(sel);
	}
};

/// Selects all lines that are currently visible on video frame.
struct subtitle_select_visible : public Command {
	CMD_NAME("subtitle/select/visible")
	STR_MENU("Select Visible")
	STR_DISP("Select Visible")
	STR_HELP("Selects all lines that are currently visible on video frame.")
	CMD_TYPE(COMMAND_VALIDATE)

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->subsGrid->SelectVisible();
	}

	bool Validate(agi::Context *c) {
		return c->videoController->IsLoaded();
	}
};


/// Open spell checker.
struct subtitle_spellcheck : public Command {
	CMD_NAME("subtitle/spellcheck")
	STR_MENU("Spe&ll Checker..")
	STR_DISP("Spell Checker")
	STR_HELP("Open spell checker.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		new DialogSpellChecker(c);
	}
};


/// 
struct subtitle_tags_show : public Command {
	CMD_NAME("subtitle/tags/show")
	STR_MENU("XXX: No idea")
	STR_DISP("XXX: No idea")
	STR_HELP("XXX: No idea")

	void operator()(agi::Context *c) {
//XXX: see grid.cpp:grid_tags_hide()
	}
};
}

/// @}

namespace cmd {
	void init_subtitle() {
		reg(new subtitle_attachment);
		reg(new subtitle_find);
		reg(new subtitle_find_next);
		reg(new subtitle_insert_after);
		reg(new subtitle_insert_after_videotime);
		reg(new subtitle_insert_before);
		reg(new subtitle_insert_before_videotime);
		reg(new subtitle_new);
		reg(new subtitle_open);
		reg(new subtitle_open_charset);
		reg(new subtitle_open_video);
		reg(new subtitle_properties);
		reg(new subtitle_save);
		reg(new subtitle_save_as);
		reg(new subtitle_select_all);
		reg(new subtitle_select_visible);
		reg(new subtitle_spellcheck);
		reg(new subtitle_tags_show);
	}
}
