// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "config.h"

#include "subs_controller.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "command/command.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "subtitle_format.h"
#include "text_file_reader.h"
#include "video_context.h"

#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <wx/msgdlg.h>

struct SubsController::UndoInfo {
	AssFile file;
	wxString undo_description;
	int commit_id;
	UndoInfo(AssFile const& f, wxString const& d, int c) : file(f), undo_description(d), commit_id(c) { }
};

SubsController::SubsController(agi::Context *context)
: context(context)
, undo_connection(context->ass->AddUndoManager(&SubsController::OnCommit, this))
, commit_id(0)
, saved_commit_id(0)
, autosaved_commit_id(0)
{
}

void SubsController::Load(agi::fs::path const& filename, std::string const& charset) {
	if (TryToClose() == wxCANCEL) return;

	// Make sure that file isn't actually a timecode file
	try {
		TextFileReader testSubs(filename, charset);
		std::string cur = testSubs.ReadLineFromFile();
		if (boost::starts_with(cur, "# timecode")) {
			context->videoController->LoadTimecodes(filename);
			return;
		}
	}
	catch (...) {
		// if trying to load the file as timecodes fails it's fairly
		// safe to assume that it is in fact not a timecode file
	}

	const SubtitleFormat *reader = SubtitleFormat::GetReader(filename);

	try {
		AssFile temp;
		reader->ReadFile(&temp, filename, charset);

		bool found_style = false;
		bool found_dialogue = false;

		// Check if the file has at least one style and at least one dialogue line
		for (auto const& line : temp.Line) {
			AssEntryGroup type = line.Group();
			if (type == ENTRY_STYLE) found_style = true;
			if (type == ENTRY_DIALOGUE) found_dialogue = true;
			if (found_style && found_dialogue) break;
		}

		// And if it doesn't add defaults for each
		if (!found_style)
			temp.InsertLine(new AssStyle);
		if (!found_dialogue)
			temp.InsertLine(new AssDialogue);

		context->ass->swap(temp);
	}
	catch (agi::UserCancelException const&) {
		return;
	}
	catch (agi::fs::FileNotFound const&) {
		wxMessageBox(filename.wstring() + " not found.", "Error", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		config::mru->Remove("Subtitle", filename);
		return;
	}
	catch (agi::Exception const& err) {
		wxMessageBox(to_wx(err.GetChainedMessage()), "Error", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		return;
	}
	catch (std::exception const& err) {
		wxMessageBox(to_wx(err.what()), "Error", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		return;
	}
	catch (...) {
		wxMessageBox("Unknown error", "Error", wxOK | wxICON_ERROR | wxCENTER, context->parent);
		return;
	}

	SetFileName(filename);

	// Push the initial state of the file onto the undo stack
	undo_stack.clear();
	redo_stack.clear();
	autosaved_commit_id = saved_commit_id = commit_id + 1;
	context->ass->Commit("", AssFile::COMMIT_NEW);

	// Save backup of file
	if (CanSave() && OPT_GET("App/Auto/Backup")->GetBool()) {
		auto path_str = OPT_GET("Path/Auto/Backup")->GetString();
		agi::fs::path path;
		if (path_str.empty())
			path = filename.parent_path();
		else
			path = config::path->Decode(path_str);
		agi::fs::CreateDirectory(path);
		agi::fs::Copy(filename, path/(filename.stem().string() + ".ORIGINAL" + filename.extension().string()));
	}

	FileOpen(filename);
}

void SubsController::Save(agi::fs::path const& filename, std::string const& encoding) {
	const SubtitleFormat *writer = SubtitleFormat::GetWriter(filename);
	if (!writer)
		throw "Unknown file type.";

	int old_autosaved_commit_id = autosaved_commit_id, old_saved_commit_id = saved_commit_id;
	try {
		autosaved_commit_id = saved_commit_id = commit_id;

		// Have to set this now for the sake of things that want to save paths
		// relative to the script in the header
		this->filename = filename;
		config::path->SetToken("?script", filename.parent_path());

		FileSave();

		writer->WriteFile(context->ass, filename, encoding);
	}
	catch (...) {
		autosaved_commit_id = old_autosaved_commit_id;
		saved_commit_id = old_saved_commit_id;
		throw;
	}

	SetFileName(filename);
}

void SubsController::Close() {
	undo_stack.clear();
	redo_stack.clear();
	autosaved_commit_id = saved_commit_id = commit_id + 1;
	filename.clear();
	context->ass->Line.clear();
	context->ass->LoadDefault();
	context->ass->Commit("", AssFile::COMMIT_NEW);
}

int SubsController::TryToClose(bool allow_cancel) const {
	if (!IsModified())
		return wxYES;

	int flags = wxYES_NO;
	if (allow_cancel)
		flags |= wxCANCEL;
	int result = wxMessageBox(wxString::Format(_("Do you want to save changes to %s?"), Filename().wstring()), _("Unsaved changes"), flags, context->parent);
	if (result == wxYES) {
		cmd::call("subtitle/save", context);
		// If it fails saving, return cancel anyway
		return IsModified() ? wxCANCEL : wxYES;
	}
	return result;
}

agi::fs::path SubsController::AutoSave() {
	if (commit_id == autosaved_commit_id)
		return "";

	auto path = config::path->Decode(OPT_GET("Path/Auto/Save")->GetString());
	if (path.empty())
		path = filename.parent_path();

	agi::fs::CreateDirectory(path);

	auto name = filename.filename();
	if (name.empty())
		name = "Untitled";

	path /= str(boost::format("%s.%s.AUTOSAVE.ass") % name.string() % agi::util::strftime("%Y-%m-%d-%H-%M-%S"));

	SubtitleFormat::GetWriter(path)->WriteFile(context->ass, path);
	autosaved_commit_id = commit_id;

	return path;
}

bool SubsController::CanSave() const {
	try {
		return SubtitleFormat::GetWriter(filename)->CanSave(context->ass);
	}
	catch (...) {
		return false;
	}
}

void SubsController::SetFileName(agi::fs::path const& path) {
	filename = path;
	config::path->SetToken("?script", path.parent_path());
	config::mru->Add("Subtitle", path);
	OPT_SET("Path/Last/Subtitles")->SetString(filename.parent_path().string());
}

void SubsController::OnCommit(AssFileCommit c) {
	if (c.message.empty() && !undo_stack.empty()) return;

	++commit_id;
	// Allow coalescing only if it's the last change and the file has not been
	// saved since the last change
	if (commit_id == *c.commit_id+1 && redo_stack.empty() && saved_commit_id+1 != commit_id && autosaved_commit_id+1 != commit_id) {
		// If only one line changed just modify it instead of copying the file
		if (c.single_line) {
			entryIter this_it = context->ass->Line.begin(), undo_it = undo_stack.back().file.Line.begin();
			while (&*this_it != c.single_line) {
				++this_it;
				++undo_it;
			}
			undo_stack.back().file.Line.insert(undo_it, *c.single_line->Clone());
			delete &*undo_it;
		}
		else
			undo_stack.back().file = *context->ass;

		*c.commit_id = commit_id;
		return;
	}

	redo_stack.clear();

	undo_stack.emplace_back(*context->ass, c.message, commit_id);

	int depth = std::max<int>(OPT_GET("Limits/Undo Levels")->GetInt(), 2);
	while ((int)undo_stack.size() > depth)
		undo_stack.pop_front();

	if (undo_stack.size() > 1 && OPT_GET("App/Auto/Save on Every Change")->GetBool() && !filename.empty() && CanSave())
		Save(filename);

	*c.commit_id = commit_id;
}

void SubsController::Undo() {
	if (undo_stack.size() <= 1) return;

	redo_stack.emplace_back(AssFile(), undo_stack.back().undo_description, commit_id);
	context->ass->swap(redo_stack.back().file);
	undo_stack.pop_back();
	*context->ass = undo_stack.back().file;
	commit_id = undo_stack.back().commit_id;

	context->ass->Commit("", AssFile::COMMIT_NEW);
}

void SubsController::Redo() {
	if (redo_stack.empty()) return;

	context->ass->swap(redo_stack.back().file);
	commit_id = redo_stack.back().commit_id;
	undo_stack.emplace_back(*context->ass, redo_stack.back().undo_description, commit_id);
	redo_stack.pop_back();

	context->ass->Commit("", AssFile::COMMIT_NEW);
}

wxString SubsController::GetUndoDescription() const {
	return IsUndoStackEmpty() ? "" : undo_stack.back().undo_description;
}

wxString SubsController::GetRedoDescription() const {
	return IsRedoStackEmpty() ? "" : redo_stack.back().undo_description;
}

agi::fs::path SubsController::Filename() const {
	if (!filename.empty()) return filename;

	// Apple HIG says "untitled" should not be capitalised
#ifndef __WXMAC__
	return _("Untitled").wx_str();
#else
	return _("untitled").wx_str();
#endif
}
