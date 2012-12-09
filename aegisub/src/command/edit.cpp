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

/// @file edit.cpp
/// @brief edit/ commands.
/// @ingroup command
///

#include "../config.h"

#include <algorithm>

#include <wx/clipbrd.h>
#include <wx/fontdlg.h>
#include <wx/tokenzr.h>

#include "command.h"

#include "../ass_dialogue.h"
#include "../ass_file.h"
#include "../ass_karaoke.h"
#include "../ass_override.h"
#include "../ass_style.h"
#include "../dialog_colorpicker.h"
#include "../dialog_paste_over.h"
#include "../dialog_search_replace.h"
#include "../main.h"
#include "../include/aegisub/context.h"
#include "../subs_edit_ctrl.h"
#include "../subs_grid.h"
#include "../text_selection_controller.h"
#include "../utils.h"
#include "../video_context.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/sliced.hpp>

#include <libaegisub/of_type_adaptor.h>

namespace {
	using namespace boost::adaptors;
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

void paste_lines(agi::Context *c, bool paste_over) {
	wxString data = GetClipboard();
	if (!data) return;

	AssDialogue *rel_line = c->selectionController->GetActiveLine();
	entryIter pos = c->ass->Line.iterator_to(*rel_line);

	AssDialogue *first = 0;
	SubtitleSelection newsel;

	std::vector<bool> pasteOverOptions;
	wxStringTokenizer token (data,"\r\n",wxTOKEN_STRTOK);
	while (token.HasMoreTokens()) {
		// Convert data into an AssDialogue
		wxString curdata = token.GetNextToken();
		curdata.Trim(true);
		curdata.Trim(false);
		AssDialogue *curdiag;
		try {
			// Try to interpret the line as an ASS line
			curdiag = new AssDialogue(curdata);
		}
		catch (...) {
			// Line didn't parse correctly, assume it's plain text that
			// should be pasted in the Text field only
			curdiag = new AssDialogue();
			curdiag->Text = curdata;
			// Make sure pasted plain-text lines always are blank-timed
			curdiag->Start = 0;
			curdiag->End = 0;
		}

		if (!first)
			first = curdiag;

		if (!paste_over) {
			newsel.insert(curdiag);
			c->ass->Line.insert(pos, *curdiag);
		}
		else {
			// Get list of options to paste over, if not asked yet
			if (pasteOverOptions.empty()) {
				DialogPasteOver diag(c->parent);
				if (diag.ShowModal()) {
					delete curdiag;
					return;
				}
				pasteOverOptions = OPT_GET("Tool/Paste Lines Over/Fields")->GetListBool();
			}

			AssDialogue *line = static_cast<AssDialogue *>(&*pos);
			if (pasteOverOptions[0]) line->Layer = curdiag->Layer;
			if (pasteOverOptions[1]) line->Start = curdiag->Start;
			if (pasteOverOptions[2]) line->End = curdiag->End;
			if (pasteOverOptions[3]) line->Style = curdiag->Style;
			if (pasteOverOptions[4]) line->Actor = curdiag->Actor;
			if (pasteOverOptions[5]) line->Margin[0] = curdiag->Margin[0];
			if (pasteOverOptions[6]) line->Margin[1] = curdiag->Margin[1];
			if (pasteOverOptions[7]) line->Margin[2] = curdiag->Margin[2];
			if (pasteOverOptions[8]) line->Effect = curdiag->Effect;
			if (pasteOverOptions[9]) line->Text = curdiag->Text;

			delete curdiag;

			do {
				++pos;
			} while (pos != c->ass->Line.end() && !dynamic_cast<AssDialogue*>(&*pos));
			if (pos == c->ass->Line.end())
				break;
		}
	}

	if (first) {
		c->ass->Commit(_("paste"), paste_over ? AssFile::COMMIT_DIAG_FULL : AssFile::COMMIT_DIAG_ADDREM);

		if (!paste_over)
			c->selectionController->SetSelectionAndActive(newsel, first);
	}
}

template<class T>
T get_value(boost::ptr_vector<AssDialogueBlock> const& blocks, int blockn, T initial, wxString const& tag_name, wxString alt = wxString()) {
	for (auto ovr : blocks | sliced(0, blockn + 1) | reversed | agi::of_type<AssDialogueBlockOverride>()) {
		for (auto tag : ovr->Tags | reversed) {
			if (tag->Name == tag_name || tag->Name == alt)
				return tag->Params[0].Get<T>(initial);
		}
	}
	return initial;
}

/// Get the block index in the text of the position
int block_at_pos(wxString const& text, int pos) {
	int n = 0;
	int max = text.size() - 1;
	for (int i = 0; i <= pos && i <= max; ++i) {
		if (i > 0 && text[i] == '{')
			n++;
		if (text[i] == '}' && i != max && i != pos && i != pos -1 && (i+1 == max || text[i+1] != '{'))
			n++;
	}

	return n;
}

void set_tag(AssDialogue *line, boost::ptr_vector<AssDialogueBlock> &blocks, wxString const& tag, wxString const& value, int &sel_start, int &sel_end, bool at_end = false) {
	if (blocks.empty())
		blocks = line->ParseTags();

	int start = at_end ? sel_end : sel_start;
	int blockn = block_at_pos(line->Text, start);

	AssDialogueBlockPlain *plain = 0;
	AssDialogueBlockOverride *ovr = 0;
	while (blockn >= 0) {
		AssDialogueBlock *block = &blocks[blockn];
		if (dynamic_cast<AssDialogueBlockDrawing*>(block))
			--blockn;
		else if ((plain = dynamic_cast<AssDialogueBlockPlain*>(block))) {
			// Cursor is in a comment block, so try the previous block instead
			if (plain->GetText().StartsWith("{")) {
				--blockn;
				start = line->Text.get().rfind('{', start);
			}
			else
				break;
		}
		else {
			ovr = dynamic_cast<AssDialogueBlockOverride*>(block);
			assert(ovr);
			break;
		}
	}

	// If we didn't hit a suitable block for inserting the override just put
	// it at the beginning of the line
	if (blockn < 0)
		start = 0;

	wxString insert = tag + value;
	int shift = insert.size();
	if (plain || blockn < 0) {
		line->Text = line->Text.get().Left(start) + "{" + insert + "}" + line->Text.get().Mid(start);
		shift += 2;
		blocks = line->ParseTags();
	}
	else if (ovr) {
		wxString alt;
		if (tag == "\\c") alt = "\\1c";
		// Remove old of same
		bool found = false;
		for (size_t i = 0; i < ovr->Tags.size(); i++) {
			wxString name = ovr->Tags[i]->Name;
			if (tag == name || alt == name) {
				shift -= ((wxString)*ovr->Tags[i]).size();
				if (found) {
					delete ovr->Tags[i];
					ovr->Tags.erase(ovr->Tags.begin() + i);
					i--;
				}
				else {
					ovr->Tags[i]->Params[0].Set(value);
					found = true;
				}
			}
		}
		if (!found)
			ovr->AddTag(insert);

		line->UpdateText(blocks);
	}
	else
		assert(false);

	if (!at_end) {
		sel_start += shift;
		sel_end += shift;
	}
}

void commit_text(agi::Context const * const c, wxString const& desc, int sel_start = -1, int sel_end = -1, int *commit_id = 0) {
	SubtitleSelection const& sel = c->selectionController->GetSelectedSet();
	wxString text = c->selectionController->GetActiveLine()->Text;
	for_each(sel.begin(), sel.end(), [&](AssDialogue *d) { d->Text = text; });

	int new_commit_id = c->ass->Commit(desc, AssFile::COMMIT_DIAG_TEXT, commit_id ? *commit_id : -1, sel.size() == 1 ? *sel.begin() : 0);
	if (commit_id)
		*commit_id = new_commit_id;
	if (sel_start >= 0 && sel_end >= 0)
		c->textSelectionController->SetSelection(sel_start, sel_end);
}

void toggle_override_tag(const agi::Context *c, bool (AssStyle::*field), const char *tag, wxString const& undo_msg) {
	AssDialogue *const line = c->selectionController->GetActiveLine();
	AssStyle const* const style = c->ass->GetStyle(line->Style);
	bool state = style ? style->*field : AssStyle().*field;

	boost::ptr_vector<AssDialogueBlock> blocks(line->ParseTags());
	int sel_start = c->textSelectionController->GetSelectionStart();
	int sel_end = c->textSelectionController->GetSelectionEnd();
	int blockn = block_at_pos(line->Text, sel_start);

	state = get_value(blocks, blockn, state, tag);

	set_tag(line, blocks, tag, state ? "0" : "1", sel_start, sel_end);
	if (sel_start != sel_end)
		set_tag(line, blocks, tag, state ? "1" : "0", sel_start, sel_end, true);

	commit_text(c, undo_msg, sel_start, sel_end);
}

void show_color_picker(const agi::Context *c, agi::Color (AssStyle::*field), const char *tag, const char *alt) {
	AssDialogue *const line = c->selectionController->GetActiveLine();
	AssStyle const* const style = c->ass->GetStyle(line->Style);
	agi::Color color = (style ? style->*field : AssStyle().*field);

	boost::ptr_vector<AssDialogueBlock> blocks(line->ParseTags());
	int sel_start = c->textSelectionController->GetSelectionStart();
	int sel_end = c->textSelectionController->GetSelectionEnd();
	int blockn = block_at_pos(line->Text, sel_start);
	int initial_sel_start = sel_start, initial_sel_end = sel_end;

	color = get_value(blocks, blockn, color, tag, alt);
	int commit_id = -1;
	bool ok = GetColorFromUser(c->parent, color, [&](agi::Color new_color) {
		set_tag(line, blocks, tag, new_color.GetAssOverrideFormatted(), sel_start, sel_end);
		commit_text(c, _("set color"), sel_start, sel_end, &commit_id);
	});
	commit_text(c, _("set color"), -1, -1, &commit_id);

	if (!ok) {
		c->ass->Undo();
		c->textSelectionController->SetSelection(initial_sel_start, initial_sel_end);
	}
}

struct edit_color_primary : public Command {
	CMD_NAME("edit/color/primary")
	STR_MENU("Primary Color...")
	STR_DISP("Primary Color")
	STR_HELP("Primary Color")

	void operator()(agi::Context *c) {
		show_color_picker(c, &AssStyle::primary, "\\c", "\\1c");
	}
};

struct edit_color_secondary : public Command {
	CMD_NAME("edit/color/secondary")
	STR_MENU("Secondary Color...")
	STR_DISP("Secondary Color")
	STR_HELP("Secondary Color")

	void operator()(agi::Context *c) {
		show_color_picker(c, &AssStyle::secondary, "\\2c", "");
	}
};

struct edit_color_outline : public Command {
	CMD_NAME("edit/color/outline")
	STR_MENU("Outline Color...")
	STR_DISP("Outline Color")
	STR_HELP("Outline Color")

	void operator()(agi::Context *c) {
		show_color_picker(c, &AssStyle::outline, "\\3c", "");
	}
};

struct edit_color_shadow : public Command {
	CMD_NAME("edit/color/shadow")
	STR_MENU("Shadow Color...")
	STR_DISP("Shadow Color")
	STR_HELP("Shadow Color")

	void operator()(agi::Context *c) {
		show_color_picker(c, &AssStyle::shadow, "\\4c", "");
	}
};

struct edit_style_bold : public Command {
	CMD_NAME("edit/style/bold")
	STR_MENU("Bold")
	STR_DISP("Bold")
	STR_HELP("Bold")

	void operator()(agi::Context *c) {
		toggle_override_tag(c, &AssStyle::bold, "\\b", _("toggle bold"));
	}
};

struct edit_style_italic : public Command {
	CMD_NAME("edit/style/italic")
	STR_MENU("Italics")
	STR_DISP("Italics")
	STR_HELP("Italics")

	void operator()(agi::Context *c) {
		toggle_override_tag(c, &AssStyle::italic, "\\i", _("toggle italic"));
	}
};

struct edit_style_underline : public Command {
	CMD_NAME("edit/style/underline")
	STR_MENU("Underline")
	STR_DISP("Underline")
	STR_HELP("Underline")

	void operator()(agi::Context *c) {
		toggle_override_tag(c, &AssStyle::underline, "\\u", _("toggle underline"));
	}
};

struct edit_style_strikeout : public Command {
	CMD_NAME("edit/style/strikeout")
	STR_MENU("Strikeout")
	STR_DISP("Strikeout")
	STR_HELP("Strikeout")

	void operator()(agi::Context *c) {
		toggle_override_tag(c, &AssStyle::strikeout, "\\s", _("toggle strikeout"));
	}
};

struct edit_font : public Command {
	CMD_NAME("edit/font")
	STR_MENU("Font Face...")
	STR_DISP("Font Face")
	STR_HELP("Font Face")

	void operator()(agi::Context *c) {
		AssDialogue *const line = c->selectionController->GetActiveLine();
		boost::ptr_vector<AssDialogueBlock> blocks(line->ParseTags());
		const int blockn = block_at_pos(line->Text, c->textSelectionController->GetInsertionPoint());

		const AssStyle *style = c->ass->GetStyle(line->Style);
		const AssStyle default_style;
		if (!style)
			style = &default_style;

		int sel_start = c->textSelectionController->GetSelectionStart();
		int sel_end = c->textSelectionController->GetSelectionEnd();

		const wxFont startfont(
			get_value(blocks, blockn, (int)style->fontsize, "\\fs"),
			wxFONTFAMILY_DEFAULT,
			get_value(blocks, blockn, style->italic, "\\i") ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			get_value(blocks, blockn, style->bold, "\\b") ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
			get_value(blocks, blockn, style->underline, "\\u"),
			get_value(blocks, blockn, style->font, "\\fn"));

		const wxFont font = wxGetFontFromUser(c->parent, startfont);
		if (!font.Ok() || font == startfont) return;

		if (font.GetFaceName() != startfont.GetFaceName())
			set_tag(line, blocks, "\\fn", font.GetFaceName(), sel_start, sel_end);
		if (font.GetPointSize() != startfont.GetPointSize())
			set_tag(line, blocks, "\\fs", wxString::Format("%d", font.GetPointSize()), sel_start, sel_end);
		if (font.GetWeight() != startfont.GetWeight())
			set_tag(line, blocks, "\\b", wxString::Format("%d", font.GetWeight() == wxFONTWEIGHT_BOLD), sel_start, sel_end);
		if (font.GetStyle() != startfont.GetStyle())
			set_tag(line, blocks, "\\i", wxString::Format("%d", font.GetStyle() == wxFONTSTYLE_ITALIC), sel_start, sel_end);
		if (font.GetUnderlined() != startfont.GetUnderlined())
			set_tag(line, blocks, "\\i", wxString::Format("%d", font.GetUnderlined()), sel_start, sel_end);

		commit_text(c, _("set font"), sel_start, sel_end);
	}
};

/// Find and replace words in subtitles.
struct edit_find_replace : public Command {
	CMD_NAME("edit/find_replace")
	STR_MENU("Find and R&eplace...")
	STR_DISP("Find and Replace")
	STR_HELP("Find and replace words in subtitles")

	void operator()(agi::Context *c) {
		c->videoController->Stop();
		Search.OpenDialog(true);
	}
};

static wxString get_entry_data(AssDialogue *d) { return d->GetEntryData(); }
static void copy_lines(agi::Context *c) {
	SubtitleSelection sel = c->selectionController->GetSelectedSet();
	SetClipboard(join(c->ass->Line
		| agi::of_type<AssDialogue>()
		| filtered([&](AssDialogue *d) { return sel.count(d); })
		| transformed(get_entry_data),
		wxS("")));
}

static void delete_lines(agi::Context *c, wxString const& commit_message) {
	AssDialogue *active = c->selectionController->GetActiveLine();
	SubtitleSelection sel = c->selectionController->GetSelectedSet();

	// Find a line near the active line not being deleted to make the new active line
	AssDialogue *new_active = 0;
	bool hit_active = false;

	for (auto diag : c->ass->Line | agi::of_type<AssDialogue>()) {
		if (diag == active) {
			hit_active = true;
			if (new_active) break;
		}

		if (!sel.count(diag)) {
			new_active = diag;
			if (hit_active) break;
		}
	}

	// Delete selected lines
	c->ass->Line.remove_and_dispose_if([&sel](AssEntry const& e) {
		return sel.count(const_cast<AssDialogue *>(static_cast<const AssDialogue*>(&e)));
	}, [](AssEntry *e) { delete e; });

	// If we didn't get a new active line then we just deleted all the dialogue
	// lines, so make a new one
	if (!new_active) {
		new_active = new AssDialogue;
		c->ass->InsertLine(new_active);
	}

	c->ass->Commit(commit_message, AssFile::COMMIT_DIAG_ADDREM);

	sel.clear();
	sel.insert(new_active);
	c->selectionController->SetSelectionAndActive(sel, new_active);
}

/// Copy subtitles.
struct edit_line_copy : public validate_sel_nonempty {
	CMD_NAME("edit/line/copy")
	STR_MENU("&Copy Lines")
	STR_DISP("Copy Lines")
	STR_HELP("Copy subtitles")

	void operator()(agi::Context *c) {
		// Ideally we'd let the control's keydown handler run and only deal
		// with the events not processed by it, but that doesn't seem to be
		// possible with how wx implements key event handling - the native
		// platform processing is evoked only if the wx event is unprocessed,
		// and there's no way to do something if the native platform code leaves
		// it unprocessed

		if (wxTextEntryBase *ctrl = dynamic_cast<wxTextEntryBase*>(c->parent->FindFocus()))
			ctrl->Copy();
		else {
			copy_lines(c);
		}
	}
};

/// Cut subtitles.
struct edit_line_cut: public validate_sel_nonempty {
	CMD_NAME("edit/line/cut")
	STR_MENU("Cu&t Lines")
	STR_DISP("Cut Lines")
	STR_HELP("Cut subtitles")

	void operator()(agi::Context *c) {
		if (wxTextEntryBase *ctrl = dynamic_cast<wxTextEntryBase*>(c->parent->FindFocus()))
			ctrl->Cut();
		else {
			copy_lines(c);
			delete_lines(c, _("cut lines"));
		}
	}
};

/// Delete currently selected lines.
struct edit_line_delete : public validate_sel_nonempty {
	CMD_NAME("edit/line/delete")
	STR_MENU("De&lete Lines")
	STR_DISP("Delete Lines")
	STR_HELP("Delete currently selected lines")

	void operator()(agi::Context *c) {
		delete_lines(c, _("delete lines"));
	}
};

struct in_selection : public std::unary_function<AssEntry, bool> {
	SubtitleSelectionController::Selection const& sel;
	in_selection(SubtitleSelectionController::Selection const& sel) : sel(sel) { }
	bool operator()(AssEntry const& e) const {
		const AssDialogue *d = dynamic_cast<const AssDialogue*>(&e);
		return d && sel.count(const_cast<AssDialogue *>(d));
	}
};

static void duplicate_lines(agi::Context *c, bool shift) {
	in_selection sel(c->selectionController->GetSelectedSet());
	SubtitleSelectionController::Selection new_sel;
	AssDialogue *new_active = 0;

	entryIter start = c->ass->Line.begin();
	entryIter end = c->ass->Line.end();
	while (start != end) {
		// Find the first line in the selection
		start = find_if(start, end, sel);
		if (start == end) break;

		// And the last line in this contiguous selection
		entryIter insert_pos = find_if_not(start, end, sel);
		entryIter last = std::prev(insert_pos);

		// Duplicate each of the selected lines, inserting them in a block
		// after the selected block
		do {
			AssDialogue *new_diag = static_cast<AssDialogue*>(start->Clone());

			c->ass->Line.insert(insert_pos, *new_diag);
			new_sel.insert(new_diag);
			if (!new_active)
				new_active = new_diag;

			if (shift) {
				int pos = c->videoController->FrameAtTime(new_diag->End, agi::vfr::END) + 1;
				new_diag->Start = c->videoController->TimeAtFrame(pos, agi::vfr::START);
				new_diag->End = c->videoController->TimeAtFrame(pos, agi::vfr::END);
			}
		} while (start++ != last);

		// Skip over the lines we just made
		start = insert_pos;
	}

	if (new_sel.empty()) return;

	c->ass->Commit(_("duplicate lines"), AssFile::COMMIT_DIAG_ADDREM);

	c->selectionController->SetSelectionAndActive(new_sel, new_active);
}

/// Duplicate the selected lines.
struct edit_line_duplicate : public validate_sel_nonempty {
	CMD_NAME("edit/line/duplicate")
	STR_MENU("&Duplicate Lines")
	STR_DISP("Duplicate Lines")
	STR_HELP("Duplicate the selected lines")

	void operator()(agi::Context *c) {
		duplicate_lines(c, false);
	}
};


/// Duplicate lines and shift by one frame.
struct edit_line_duplicate_shift : public Command {
	CMD_NAME("edit/line/duplicate/shift")
	STR_MENU("D&uplicate and Shift by 1 Frame")
	STR_DISP("Duplicate and Shift by 1 Frame")
	STR_HELP("Duplicate lines and shift by one frame")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		return !c->selectionController->GetSelectedSet().empty() && c->videoController->IsLoaded();
	}

	void operator()(agi::Context *c) {
		duplicate_lines(c, true);
	}
};

static void combine_lines(agi::Context *c, void (*combiner)(AssDialogue *, AssDialogue *), wxString const& message) {
	SubtitleSelection sel = c->selectionController->GetSelectedSet();

	AssDialogue *first = 0;
	for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(&*it++);
		if (!diag || !sel.count(diag))
			continue;
		if (!first) {
			first = diag;
			continue;
		}

		combiner(first, diag);
		first->End = std::max(first->End, diag->End);
		delete diag;
	}

	sel.clear();
	sel.insert(first);
	c->selectionController->SetSelectionAndActive(sel, first);
	c->ass->Commit(message, AssFile::COMMIT_DIAG_ADDREM | AssFile::COMMIT_DIAG_FULL);
}

static void combine_karaoke(AssDialogue *first, AssDialogue *second) {
	first->Text = wxString::Format("%s{\\k%d}%s", first->Text.get(), (second->Start - first->End) / 10, second->Text.get());
}

static void combine_concat(AssDialogue *first, AssDialogue *second) {
	first->Text = first->Text + " " + second->Text;
}

static void combine_drop(AssDialogue *, AssDialogue *) { }

/// Joins selected lines in a single one, as karaoke.
struct edit_line_join_as_karaoke : public validate_sel_multiple {
	CMD_NAME("edit/line/join/as_karaoke")
	STR_MENU("As &Karaoke")
	STR_DISP("As Karaoke")
	STR_HELP("Joins selected lines in a single one, as karaoke")

	void operator()(agi::Context *c) {
		combine_lines(c, combine_karaoke, _("join as karaoke"));
	}
};


/// Joins selected lines in a single one, concatenating text together.
struct edit_line_join_concatenate : public validate_sel_multiple {
	CMD_NAME("edit/line/join/concatenate")
	STR_MENU("&Concatenate")
	STR_DISP("Concatenate")
	STR_HELP("Joins selected lines in a single one, concatenating text together")

	void operator()(agi::Context *c) {
		combine_lines(c, combine_concat, _("join lines"));
	}
};


/// Joins selected lines in a single one, keeping text of first and discarding remaining.
struct edit_line_join_keep_first : public validate_sel_multiple {
	CMD_NAME("edit/line/join/keep_first")
	STR_MENU("Keep &First")
	STR_DISP("Keep First")
	STR_HELP("Joins selected lines in a single one, keeping text of first and discarding remaining")

	void operator()(agi::Context *c) {
		combine_lines(c, combine_drop, _("join lines"));
	}
};


/// Paste subtitles.
struct edit_line_paste : public Command {
	CMD_NAME("edit/line/paste")
	STR_MENU("&Paste Lines")
	STR_DISP("Paste Lines")
	STR_HELP("Paste subtitles")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *) {
		bool can_paste = false;
		if (wxTheClipboard->Open()) {
			can_paste = wxTheClipboard->IsSupported(wxDF_TEXT);
			wxTheClipboard->Close();
		}
		return can_paste;
	}

	void operator()(agi::Context *c) {
		if (wxTextEntryBase *ctrl = dynamic_cast<wxTextEntryBase*>(c->parent->FindFocus()))
			ctrl->Paste();
		else
			paste_lines(c, false);
	}
};


/// Paste subtitles over others.
struct edit_line_paste_over : public Command {
	CMD_NAME("edit/line/paste/over")
	STR_MENU("Paste Lines &Over...")
	STR_DISP("Paste Lines Over")
	STR_HELP("Paste subtitles over others")
	CMD_TYPE(COMMAND_VALIDATE)

	bool Validate(const agi::Context *c) {
		bool can_paste = !c->selectionController->GetSelectedSet().empty();
		if (can_paste && wxTheClipboard->Open()) {
			can_paste = wxTheClipboard->IsSupported(wxDF_TEXT);
			wxTheClipboard->Close();
		}
		return can_paste;
	}

	void operator()(agi::Context *c) {
		paste_lines(c, true);
	}
};


/// Recombine subtitles when they have been split and merged.
struct edit_line_recombine : public validate_sel_multiple {
	CMD_NAME("edit/line/recombine")
	STR_MENU("Recom&bine Lines")
	STR_DISP("Recombine Lines")
	STR_HELP("Recombine subtitles when they have been split and merged")

	void operator()(agi::Context *c) {
		c->subsGrid->RecombineLines();
	}
};


/// Uses karaoke timing to split line into multiple smaller lines.
struct edit_line_split_by_karaoke : public validate_sel_nonempty {
	CMD_NAME("edit/line/split/by_karaoke")
	STR_MENU("Split Lines (by karaoke)")
	STR_DISP("Split Lines (by karaoke)")
	STR_HELP("Uses karaoke timing to split line into multiple smaller lines")

	void operator()(agi::Context *c) {
		AssKaraoke::SplitLines(c->selectionController->GetSelectedSet(), c);
	}
};

void split_lines(agi::Context *c, bool estimate) {
	int pos = c->textSelectionController->GetSelectionStart();

	AssDialogue *n1 = c->selectionController->GetActiveLine();
	AssDialogue *n2 = new AssDialogue(*n1);
	c->ass->Line.insert(++c->ass->Line.iterator_to(*n1), *n2);

	wxString orig = n1->Text;
	n1->Text = orig.Left(pos).Trim(true); // Trim off trailing whitespace
	n2->Text = orig.Mid(pos).Trim(false); // Trim off leading whitespace

	if (estimate && orig.size()) {
		double splitPos = double(pos) / orig.size();
		n2->Start = n1->End = (int)((n1->End - n1->Start) * splitPos) + n1->Start;
	}

	c->ass->Commit(_("split"), AssFile::COMMIT_DIAG_ADDREM | (estimate ? AssFile::COMMIT_DIAG_FULL : AssFile::COMMIT_DIAG_TEXT));
}

struct edit_line_split_estimate : public validate_sel_nonempty {
	CMD_NAME("edit/line/split/estimate")
	STR_MENU("Split at cursor (estimate times)")
	STR_DISP("Split at cursor (estimate times)")
	STR_HELP("Split the current line at the cursor, dividing the original line's duration between the new ones")

	void operator()(agi::Context *c) {
		split_lines(c, true);
	}
};

struct edit_line_split_preserve : public validate_sel_nonempty {
	CMD_NAME("edit/line/split/preserve")
	STR_MENU("Split at cursor (preserve times)")
	STR_DISP("Split at cursor (preserve times)")
	STR_HELP("Split the current line at the cursor, setting both lines to the original line's times")

	void operator()(agi::Context *c) {
		split_lines(c, false);
	}
};

/// Redoes last action.
struct edit_redo : public Command {
	CMD_NAME("edit/redo")
	STR_HELP("Redoes last action")
	CMD_TYPE(COMMAND_VALIDATE | COMMAND_DYNAMIC_NAME)

	wxString StrMenu(const agi::Context *c) const {
		return c->ass->IsRedoStackEmpty() ?
			_("Nothing to &redo") :
		wxString::Format(_("&Redo %s"), c->ass->GetRedoDescription());
	}
	wxString StrDisplay(const agi::Context *c) const {
		return c->ass->IsRedoStackEmpty() ?
			_("Nothing to redo") :
		wxString::Format(_("Redo %s"), c->ass->GetRedoDescription());
	}

	bool Validate(const agi::Context *c) {
		return !c->ass->IsRedoStackEmpty();
	}

	void operator()(agi::Context *c) {
		c->ass->Redo();
	}
};

/// Undoes last action.
struct edit_undo : public Command {
	CMD_NAME("edit/undo")
	STR_HELP("Undoes last action")
	CMD_TYPE(COMMAND_VALIDATE | COMMAND_DYNAMIC_NAME)

	wxString StrMenu(const agi::Context *c) const {
		return c->ass->IsUndoStackEmpty() ?
			_("Nothing to &undo") :
			wxString::Format(_("&Undo %s"), c->ass->GetUndoDescription());
	}
	wxString StrDisplay(const agi::Context *c) const {
		return c->ass->IsUndoStackEmpty() ?
			_("Nothing to undo") :
			wxString::Format(_("Undo %s"), c->ass->GetUndoDescription());
	}

	bool Validate(const agi::Context *c) {
		return !c->ass->IsUndoStackEmpty();
	}

	void operator()(agi::Context *c) {
		c->ass->Undo();
	}
};

}
/// @}

namespace cmd {
	void init_edit() {
		reg(new edit_color_primary);
		reg(new edit_color_secondary);
		reg(new edit_color_outline);
		reg(new edit_color_shadow);
		reg(new edit_font);
		reg(new edit_find_replace);
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
		reg(new edit_line_split_estimate);
		reg(new edit_line_split_preserve);
		reg(new edit_style_bold);
		reg(new edit_style_italic);
		reg(new edit_style_underline);
		reg(new edit_style_strikeout);
		reg(new edit_redo);
		reg(new edit_undo);
	}
}
