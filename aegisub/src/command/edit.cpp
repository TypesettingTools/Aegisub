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

/// @file edit.cpp
/// @brief edit/ commands.
/// @ingroup command
///

#include "../config.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/clipbrd.h>
#endif

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../dialog_search_replace.h"
#include "../include/aegisub/context.h"
#include "../subs_edit_box.h"
#include "../subs_edit_ctrl.h"
#include "../subs_grid.h"
#include "../video_context.h"

namespace {
	using cmd::Command;
/// @defgroup cmd-edit Editing commands.
/// @{

struct validate_sel_nonempty : public Command {
	CMD_TYPE(COMMAND_VALIDATE)
	bool Validate(const agi::Context *c) {
		return c->selectionController->GetSelectedSet().size() > 0;
	}
};

struct validate_sel_multiple : public Command {
	CMD_TYPE(COMMAND_VALIDATE)
	bool Validate(const agi::Context *c) {
		return c->selectionController->GetSelectedSet().size() > 1;
	}
};

/// Copy subtitles.
struct edit_line_copy : public validate_sel_nonempty {
	CMD_NAME("edit/line/copy")
	STR_MENU("Copy Lines")
	STR_DISP("Copy Lines")
	STR_HELP("Copy subtitles.")

	void operator()(agi::Context *c) {
		if (c->parent->FindFocus() == c->editBox->TextEdit) {
			c->editBox->TextEdit->Copy();
			return;
		}
		c->subsGrid->CopyLines(c->subsGrid->GetSelection());
	}
};


/// Cut subtitles.
struct edit_line_cut: public validate_sel_nonempty {
	CMD_NAME("edit/line/cut")
	STR_MENU("Cut Lines")
	STR_DISP("Cut Lines")
	STR_HELP("Cut subtitles.")

	void operator()(agi::Context *c) {
		if (c->parent->FindFocus() == c->editBox->TextEdit) {
			c->editBox->TextEdit->Cut();
			return;
		}
		c->subsGrid->CutLines(c->subsGrid->GetSelection());
	}
};


/// Delete currently selected lines.
struct edit_line_delete : public validate_sel_nonempty {
	CMD_NAME("edit/line/delete")
	STR_MENU("Delete Lines")
	STR_DISP("Delete Lines")
	STR_HELP("Delete currently selected lines.")

	void operator()(agi::Context *c) {
		c->subsGrid->DeleteLines(c->subsGrid->GetSelection());
	}
};


/// Duplicate the selected lines.
struct edit_line_duplicate : public validate_sel_nonempty {
	CMD_NAME("edit/line/duplicate")
	STR_MENU("&Duplicate Lines")
	STR_DISP("Duplicate Lines")
	STR_HELP("Duplicate the selected lines.")

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->subsGrid->GetSelection();
		c->subsGrid->DuplicateLines(sels.front(), sels.back(), false);
	}
};


/// Duplicate lines and shift by one frame.
struct edit_line_duplicate_shift : public Command {
	CMD_NAME("edit/line/duplicate/shift")
	STR_MENU("&Duplicate and Shift by 1 Frame")
	STR_DISP("Duplicate and Shift by 1 Frame")
	STR_HELP("Duplicate lines and shift by one frame.")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		return !c->selectionController->GetSelectedSet().empty() && c->videoController->IsLoaded();
	}

	void operator()(agi::Context *c) {
		wxArrayInt sels = c->subsGrid->GetSelection();
		c->subsGrid->DuplicateLines(sels.front(), sels.back(), true);
	}
};

static void combine_lines(agi::Context *c, void (*combiner)(AssDialogue *, AssDialogue *), wxString const& message) {
	SelectionController<AssDialogue>::Selection sel = c->selectionController->GetSelectedSet();

	AssDialogue *first = 0;
	entryIter out = c->ass->Line.begin();
	for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ++it) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(*it);
		if (!diag || !sel.count(diag)) {
			*out++ = *it;
			continue;
		}
		if (!first) {
			first = diag;
			*out++ = *it;
			continue;
		}

		combiner(first, diag);

		first->End.SetMS(diag->End.GetMS());
		delete diag;
	}

	c->ass->Line.erase(out, c->ass->Line.end());
	sel.clear();
	sel.insert(first);
	c->selectionController->SetActiveLine(first);
	c->selectionController->SetSelectedSet(sel);
	c->ass->Commit(message);
}

static void combine_karaoke(AssDialogue *first, AssDialogue *second) {
	first->Text += wxString::Format("{\\k%d}%s", (second->Start.GetMS() - first->End.GetMS()) / 10, second->Text);
}

static void combine_concat(AssDialogue *first, AssDialogue *second) {
	first->Text += L"\\N" + second->Text;
}

static void combine_drop(AssDialogue *, AssDialogue *) { }

/// Joins selected lines in a single one, as karaoke.
struct edit_line_join_as_karaoke : public validate_sel_multiple {
	CMD_NAME("edit/line/join/as_karaoke")
	STR_MENU("As &Karaoke")
	STR_DISP("As Karaoke")
	STR_HELP("Joins selected lines in a single one, as karaoke.")

	void operator()(agi::Context *c) {
		combine_lines(c, combine_karaoke, _("join as karaoke"));
	}
};


/// Joins selected lines in a single one, concatenating text together.
struct edit_line_join_concatenate : public validate_sel_multiple {
	CMD_NAME("edit/line/join/concatenate")
	STR_MENU("&Concatenate")
	STR_DISP("Concatenate")
	STR_HELP("Joins selected lines in a single one, concatenating text together.")

	void operator()(agi::Context *c) {
		combine_lines(c, combine_concat, _("join lines"));
	}
};


/// Joins selected lines in a single one, keeping text of first and discarding remaining.
struct edit_line_join_keep_first : public validate_sel_multiple {
	CMD_NAME("edit/line/join/keep_first")
	STR_MENU("Keep &First")
	STR_DISP("Keep First")
	STR_HELP("Joins selected lines in a single one, keeping text of first and discarding remaining.")

	void operator()(agi::Context *c) {
		combine_lines(c, combine_drop, _("join lines"));
	}
};


/// Paste subtitles.
struct edit_line_paste : public Command {
	CMD_NAME("edit/line/paste")
	STR_MENU("Paste Lines")
	STR_DISP("Paste Lines")
	STR_HELP("Paste subtitles.")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		if (wxTheClipboard->Open()) {
			bool can_paste = wxTheClipboard->IsSupported(wxDF_TEXT);
			wxTheClipboard->Close();
			return can_paste;
		}
		return false;
	}

	void operator()(agi::Context *c) {
		if (c->parent->FindFocus() == c->editBox->TextEdit) {
			c->editBox->TextEdit->Paste();
			return;
		}
		c->subsGrid->PasteLines(c->subsGrid->GetFirstSelRow());
	}
};


/// Paste subtitles over others.
struct edit_line_paste_over : public Command {
	CMD_NAME("edit/line/paste/over")
	STR_MENU("Paste Lines Over..")
	STR_DISP("Paste Lines Over")
	STR_HELP("Paste subtitles over others.")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		if (wxTheClipboard->Open()) {
			bool can_paste = wxTheClipboard->IsSupported(wxDF_TEXT);
			wxTheClipboard->Close();
			return can_paste && c->selectionController->GetSelectedSet().size();
		}
		return false;
	}

	void operator()(agi::Context *c) {
		c->subsGrid->PasteLines(c->subsGrid->GetFirstSelRow(),true);
	}
};


/// Recombine subtitles when they have been split and merged.
struct edit_line_recombine : public validate_sel_multiple {
	CMD_NAME("edit/line/recombine")
	STR_MENU("Recombine Lines")
	STR_DISP("Recombine Lines")
	STR_HELP("Recombine subtitles when they have been split and merged.")

	void operator()(agi::Context *c) {
		c->subsGrid->RecombineLines();
	}
};


/// Uses karaoke timing to split line into multiple smaller lines.
struct edit_line_split_by_karaoke : public validate_sel_nonempty {
	CMD_NAME("edit/line/split/by_karaoke")
	STR_MENU("Split Lines (by karaoke)")
	STR_DISP("Split Lines (by karaoke)")
	STR_HELP("Uses karaoke timing to split line into multiple smaller lines.")

	void operator()(agi::Context *c) {
		c->subsGrid->BeginBatch();
		wxArrayInt sels = c->subsGrid->GetSelection();
		bool didSplit = false;
		for (int i = sels.size() - 1; i >= 0; --i) {
			didSplit |= c->subsGrid->SplitLineByKaraoke(sels[i]);
		}
		if (didSplit) {
			c->ass->Commit(_("splitting"));
		}
		c->subsGrid->EndBatch();
	}
};


/// Swaps the two selected lines.
struct edit_line_swap : public Command {
	CMD_NAME("edit/line/swap")
	STR_MENU("Swap Lines")
	STR_DISP("Swap Lines")
	STR_HELP("Swaps the two selected lines.")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		return c->selectionController->GetSelectedSet().size() == 2;
	}

	void operator()(agi::Context *c) {
		SelectionController<AssDialogue>::Selection sel = c->selectionController->GetSelectedSet();
		if (sel.size() == 2) {
			entryIter a = find(c->ass->Line.begin(), c->ass->Line.end(), *sel.begin());
			entryIter b = find(c->ass->Line.begin(), c->ass->Line.end(), *sel.rbegin());

			using std::swap;
			swap(*a, *b);
			c->ass->Commit(_("swap lines"));
		}
	}
};


/// Redoes last action.
struct edit_redo : public Command {
	CMD_NAME("edit/redo")
	STR_MENU("&Redo")
	STR_DISP("Redo")
	STR_HELP("Redoes last action.")
	CMD_TYPE(COMMAND_VALIDATE | COMMAND_DYNAMIC_NAME)

	bool Validate(const agi::Context *c) {
		return !c->ass->IsRedoStackEmpty();
	}

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->ass->Redo();
	}
};


/// Find and replace words in subtitles.
struct edit_search_replace : public Command {
	CMD_NAME("edit/search_replace")
	STR_MENU("Search and &Replace..")
	STR_DISP("Search and Replace")
	STR_HELP("Find and replace words in subtitles.")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		Search.OpenDialog(true);
	}
};


/// Undoes last action.
struct edit_undo : public Command {
	CMD_NAME("edit/undo")
	STR_MENU("&Undo")
	STR_DISP("Undo")
	STR_HELP("Undoes last action.")
	CMD_TYPE(COMMAND_VALIDATE | COMMAND_DYNAMIC_NAME)

	bool Validate(const agi::Context *c) {
		return !c->ass->IsUndoStackEmpty();
	}

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		c->ass->Undo();
	}
};

}
/// @}

namespace cmd {
	void init_edit() {
		reg(new edit_line_copy);
		reg(new edit_line_cut);
		reg(new edit_line_delete);
		reg(new edit_line_duplicate);
		reg(new edit_line_duplicate_shift);
		reg(new edit_line_join_as_karaoke);
		reg(new edit_line_join_concatenate);
		reg(new edit_line_join_keep_first);
		reg(new edit_line_paste);
		reg(new edit_line_paste_over);
		reg(new edit_line_recombine);
		reg(new edit_line_split_by_karaoke);
		reg(new edit_line_swap);
		reg(new edit_redo);
		reg(new edit_search_replace);
		reg(new edit_undo);
	}
}
