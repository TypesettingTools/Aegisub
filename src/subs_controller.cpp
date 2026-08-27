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

#include "subs_controller.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "project_document.h"
#include "ass_info.h"
#include "ass_style.h"
#include "compat.h"
#include "command/command.h"
#include "format.h"
#include "frame_main.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "project.h"
#include "project_format.h"
#include "selection_controller.h"
#include "subtitle_format.h"
#include "text_selection_controller.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/format_path.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <wx/msgdlg.h>

namespace {
	void autosave_timer_changed(wxTimer *timer) {
		int freq = OPT_GET("App/Auto/Save Every Seconds")->GetInt();
		if (freq > 0 && OPT_GET("App/Auto/Save")->GetBool())
			timer->Start(freq * 1000);
		else
			timer->Stop();
	}
}

struct SubsController::UndoInfo {
	wxString undo_description;
	int commit_id;

	std::vector<std::pair<std::string, std::string>> script_info;
	std::vector<AssStyle> styles;
	std::vector<AssDialogueBase> events;
	std::vector<AssAttachment> attachments;
	std::vector<ExtradataEntry> extradata;

	mutable std::vector<int> selection;
	int active_line_id = 0;
	int pos = 0, sel_start = 0, sel_end = 0;

	UndoInfo(const agi::Context *c, wxString const& d, int commit_id)
	: undo_description(d)
	, commit_id(commit_id)
	, attachments(c->document->Attachments)
	, extradata(c->document->Extradata)
	{
		script_info.reserve(c->document->Info.size());
		for (auto const& info : c->document->Info)
			script_info.emplace_back(info.Key(), info.Value());

		styles.reserve(c->document->Styles.size());
		styles.assign(c->document->Styles.begin(), c->document->Styles.end());

		events.reserve(c->document->Events.size());
		events.assign(c->document->Events.begin(), c->document->Events.end());

		UpdateActiveLine(c);
		UpdateSelection(c);
		UpdateTextSelection(c);
	}

	void Apply(agi::Context *c) const {
		// Keep old dialogue lines alive until after the commit is complete
		// since a bunch of stuff holds references to them
		ProjectDocument old;
		old.Events.swap(c->document->Events);
		c->document->Info.clear();
		c->document->Attachments.clear();
		c->document->Styles.clear();
		c->document->Extradata.clear();

		sort(begin(selection), end(selection));

		AssDialogue *active_line = nullptr;
		Selection new_sel;

		for (auto const& info : script_info)
			c->document->Info.push_back(*new AssInfo(info.first, info.second));
		for (auto const& style : styles)
			c->document->Styles.push_back(*new AssStyle(style));
		c->document->Attachments = attachments;
		for (auto const& event : events) {
			auto copy = new AssDialogue(event);
			c->document->Events.push_back(*copy);
			if (copy->Id == active_line_id)
				active_line = copy;
			if (binary_search(begin(selection), end(selection), copy->Id))
				new_sel.insert(copy);
		}
		c->document->Extradata = extradata;

		c->document->Commit("", ProjectDocument::COMMIT_NEW);
		c->selectionController->SetSelectionAndActive(std::move(new_sel), active_line);

		c->textSelectionController->SetInsertionPoint(pos);
		c->textSelectionController->SetSelection(sel_start, sel_end);
	}

	void UpdateActiveLine(const agi::Context *c) {
		auto line = c->selectionController->GetActiveLine();
		if (line)
			active_line_id = line->Id;
	}

	void UpdateSelection(const agi::Context *c) {
		auto const& sel = c->selectionController->GetSelectedSet();
		selection.clear();
		selection.reserve(sel.size());
		for (const auto diag : sel)
			selection.push_back(diag->Id);
	}

	void UpdateTextSelection(const agi::Context *c) {
		pos = c->textSelectionController->GetInsertionPoint();
		sel_start = c->textSelectionController->GetSelectionStart();
		sel_end = c->textSelectionController->GetSelectionEnd();
	}
};

SubsController::SubsController(agi::Context *context)
: context(context)
, undo_connection(context->document->AddUndoManager(&SubsController::OnCommit, this))
, text_selection_connection(context->textSelectionController->AddSelectionListener(&SubsController::OnTextSelectionChanged, this))
, autosave_queue(agi::dispatch::Create())
{
	autosave_timer_changed(&autosave_timer);
	BindConnection(OPT_SUB("App/Auto/Save", [this] { autosave_timer_changed(&autosave_timer); }));
	BindConnection(OPT_SUB("App/Auto/Save Every Seconds", [this] { autosave_timer_changed(&autosave_timer); }));
	autosave_timer.Bind(wxEVT_TIMER, [this](wxTimerEvent&) { AutoSave(); });
}

SubsController::~SubsController() {
	// Make sure there are no autosaves in progress
	autosave_queue->Sync([]{ });
}

void SubsController::SetSelectionController(SelectionController *selection_controller) {
	active_line_connection = selection_controller->AddActiveLineListener(&SubsController::OnActiveLineChanged, this);
	selection_connection = selection_controller->AddSelectionListener(&SubsController::OnSelectionChanged, this);
}

ProjectProperties SubsController::Load(agi::fs::path const& filename, const char *charset) {
	ProjectDocument temp;
	bool const is_project = agi::fs::HasExtension(filename, "aegi");
	if (is_project)
		ProjectFormat::Read(temp, filename);
	else
		SubtitleFormat::GetReader(filename, charset)->ReadFile(&temp, filename, context->project->Timecodes(), charset);

	context->document->swap(temp);
	auto props = context->document->Properties;

	if (is_project)
		SetFileName(filename);
	else {
		this->filename.clear();
		context->path->SetToken("?script", filename.parent_path());
		config::mru->Add("Subtitle", filename);
	}

	// Push the initial state of the file onto the undo stack
	undo_stack.clear();
	redo_stack.clear();
	context->document->Commit("", ProjectDocument::COMMIT_NEW);
	autosaved_commit_id = commit_id;
	saved_commit_id = is_project ? commit_id : commit_id - 1; // Imports are new projects.

	// Save backup of file
	if (is_project && OPT_GET("App/Auto/Backup")->GetBool()) {
		auto path_str = OPT_GET("Path/Auto/Backup")->GetString();
		agi::fs::path path;
		if (path_str.empty())
			path = filename.parent_path();
		else
			path = context->path->Decode(path_str);
		agi::fs::CreateDirectory(path);
		agi::fs::Copy(filename, path/(filename.stem().string() + ".ORIGINAL" + filename.extension().string()));
	}

	FileOpen(filename);
	return props;
}

void SubsController::Save(agi::fs::path const& filename, const char *encoding) {
	(void)encoding;
	if (!agi::fs::HasExtension(filename, "aegi"))
		throw agi::InvalidInputException("Aegisub projects must use the .aegi extension.");

	int old_autosaved_commit_id = autosaved_commit_id, old_saved_commit_id = saved_commit_id;
	try {
		autosaved_commit_id = saved_commit_id = commit_id;

		// Have to set this now for the sake of things that want to save paths
		// relative to the script in the header
		this->filename = filename;
		context->project->SetSubtitlesFilename(filename);

		context->document->CleanExtradata();
		ProjectFormat::Write(*context->document, filename);
		FileSave();
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
	ProjectDocument blank;
	blank.swap(*context->document);
	context->document->LoadDefault(true, OPT_GET("Subtitle Format/ASS/Default Style Catalog")->GetString());
	context->document->Commit("", ProjectDocument::COMMIT_NEW);
	FileOpen(filename);
}

int SubsController::TryToClose(bool allow_cancel) const {
	if (!IsModified())
		return wxYES;

	int flags = wxYES_NO;
	if (allow_cancel)
		flags |= wxCANCEL;
	int result = wxMessageBox(fmt_tl("Do you want to save changes to %s?", Filename()), _("Unsaved changes"), flags, context->parent);
	if (result == wxYES) {
		cmd::call("subtitle/save", context);
		// If it fails saving, return cancel anyway
		return IsModified() ? wxCANCEL : wxYES;
	}
	return result;
}

void SubsController::AutoSave() {
	if (commit_id == autosaved_commit_id)
		return;

	auto directory = context->path->Decode(OPT_GET("Path/Auto/Save")->GetString());
	if (directory.empty())
		directory = filename.parent_path();

	auto name = filename.filename();
	if (name.empty())
		name = from_wx(_("Untitled"));

	autosaved_commit_id = commit_id;
	auto frame = context->frame;
	auto subs_copy = new ProjectDocument(*context->document);
	autosave_queue->Async([subs_copy, name, directory, frame] {
		wxString msg;
		std::unique_ptr<ProjectDocument> subs(subs_copy);

		try {
			agi::fs::CreateDirectory(directory);
			auto path = directory / agi::format("%s.%s.AUTOSAVE.aegi", name.string(),
			                                     agi::util::strftime("%Y-%m-%d-%H-%M-%S"));
			ProjectFormat::Write(*subs, path);
			msg = fmt_tl("File backup saved as \"%s\".", path);
		}
		catch (const agi::Exception& err) {
			msg = _("Exception when attempting to autosave file: ") + to_wx(err.GetMessage());
		}
		catch (...) {
			msg = _("Unhandled exception when attempting to autosave file.");
		}

		agi::dispatch::Main().Async([frame, msg] {
			frame->StatusTimeout(msg);
		});
	});
}

bool SubsController::CanSave() const {
	return agi::fs::HasExtension(filename, "aegi");
}

void SubsController::SetFileName(agi::fs::path const& path) {
	filename = path;
	context->path->SetToken("?script", path.parent_path());
	config::mru->Add("Subtitle", path);
	OPT_SET("Path/Last/Subtitles")->SetString(filename.parent_path().string());
}

void SubsController::OnCommit(ProjectDocumentCommit c) {
	if (c.message.empty() && !undo_stack.empty()) return;

	commit_id = next_commit_id++;
	// Allow coalescing only if it's the last change and the file has not been
	// saved since the last change
	if (commit_id == *c.commit_id+1 && redo_stack.empty() && saved_commit_id+1 != commit_id) {
		// If only one line changed just modify it instead of copying the file
		if (c.single_line && c.single_line->Group() == AssEntryGroup::DIALOGUE) {
			for (auto& diag : undo_stack.back().events) {
				if (diag.Id == c.single_line->Id) {
					diag = *c.single_line;
					break;
				}
			}
			*c.commit_id = commit_id;
			return;
		}

		undo_stack.pop_back();
	}

	// Make sure the file has at least one style and one dialogue line
	if (context->document->Styles.empty())
		context->document->Styles.push_back(*new AssStyle);
	if (context->document->Events.empty()) {
		context->document->Events.push_back(*new AssDialogue);
		context->document->Events.back().Row = 0;
	}

	redo_stack.clear();

	undo_stack.emplace_back(context, c.message, commit_id);

	int depth = std::max<int>(OPT_GET("Limits/Undo Levels")->GetInt(), 2);
	while ((int)undo_stack.size() > depth)
		undo_stack.pop_front();

	if (undo_stack.size() > 1 && OPT_GET("App/Auto/Save on Every Change")->GetBool() && !filename.empty() && CanSave())
		Save(filename);

	*c.commit_id = commit_id;
}

void SubsController::OnActiveLineChanged() {
	if (!undo_stack.empty())
		undo_stack.back().UpdateActiveLine(context);
}

void SubsController::OnSelectionChanged() {
	if (!undo_stack.empty())
		undo_stack.back().UpdateSelection(context);
}

void SubsController::OnTextSelectionChanged() {
	if (!undo_stack.empty())
		undo_stack.back().UpdateTextSelection(context);
}

void SubsController::Undo() {
	if (undo_stack.size() <= 1) return;
	redo_stack.splice(redo_stack.end(), undo_stack, std::prev(undo_stack.end()));

	commit_id = undo_stack.back().commit_id;

	text_selection_connection.Block();
	undo_stack.back().Apply(context);
	text_selection_connection.Unblock();
}

void SubsController::Redo() {
	if (redo_stack.empty()) return;
	undo_stack.splice(undo_stack.end(), redo_stack, std::prev(redo_stack.end()));

	commit_id = undo_stack.back().commit_id;

	text_selection_connection.Block();
	undo_stack.back().Apply(context);
	text_selection_connection.Unblock();
}

wxString SubsController::GetUndoDescription() const {
	return IsUndoStackEmpty() ? wxString() : undo_stack.back().undo_description;
}

wxString SubsController::GetRedoDescription() const {
	return IsRedoStackEmpty() ? wxString() : redo_stack.back().undo_description;
}

agi::fs::path SubsController::Filename() const {
	if (!filename.empty()) return filename;

	// Apple HIG says "untitled" should not be capitalised
#ifndef __WXMAC__
	return from_wx(_("Untitled"));
#else
	return from_wx(_("untitled"));
#endif
}
