// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file ass_file.cpp
/// @brief Overall storage of subtitle files, undo management and more
/// @ingroup subs_storage

#include "config.h"

#include "ass_file.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_info.h"
#include "ass_style.h"
#include "options.h"
#include "standard_paths.h"
#include "subtitle_format.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/fs.h>
#include <libaegisub/of_type_adaptor.h>
#include <libaegisub/util.h>

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <list>

namespace std {
	template<>
	void swap(AssFile &lft, AssFile &rgt) {
		lft.swap(rgt);
	}
}

AssFile::AssFile ()
: commitId(0)
, loaded(false)
{
}

AssFile::~AssFile() {
	auto copy = new EntryList;
	copy->swap(Line);
	agi::dispatch::Background().Async([=]{ delete copy; });
}

/// @brief Load generic subs
void AssFile::Load(agi::fs::path const& filename, std::string const& charset) {
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

		swap(temp);
	}
	catch (agi::UserCancelException const&) {
		return;
	}

	// Set general data
	loaded = true;
	this->filename = filename;

	// Add comments and set vars
	SetScriptInfo("ScriptType", "v4.00+");

	// Push the initial state of the file onto the undo stack
	UndoStack.clear();
	RedoStack.clear();
	undoDescription.clear();
	autosavedCommitId = savedCommitId = commitId + 1;
	Commit("", COMMIT_NEW);

	FileOpen(filename);
}

void AssFile::Save(agi::fs::path const& filename, bool setfilename, bool addToRecent, std::string const& encoding) {
	const SubtitleFormat *writer = SubtitleFormat::GetWriter(filename);
	if (!writer)
		throw "Unknown file type.";

	if (setfilename) {
		autosavedCommitId = savedCommitId = commitId;
		this->filename = filename;
		StandardPaths::SetPathValue("?script", filename.parent_path());
	}

	FileSave();

	writer->WriteFile(this, filename, encoding);

	if (addToRecent)
		AddToRecent(filename);
}

agi::fs::path AssFile::AutoSave() {
	if (!loaded || commitId == autosavedCommitId)
		return "";

	auto path = StandardPaths::DecodePath(OPT_GET("Path/Auto/Save")->GetString());
	if (path.empty())
		path = filename.parent_path();

	agi::fs::CreateDirectory(path);

	auto name = filename.filename();
	if (name.empty())
		name = "Untitled";

	path /= str(boost::format("%s.%s.AUTOSAVE.ass") % name % agi::util::strftime("%Y-%m-%d-%H-%M-%S"));

	Save(path, false, false);

	autosavedCommitId = commitId;

	return path;
}

void AssFile::SaveMemory(std::vector<char> &dst) {
	// Check if subs contain at least one style
	// Add a default style if they don't for compatibility with libass/asa
	if (GetStyles().empty())
		InsertLine(new AssStyle);

	// Prepare vector
	dst.clear();
	dst.reserve(0x4000);

	// Write file
	AssEntryGroup group = ENTRY_GROUP_MAX;
	for (auto const& line : Line) {
		if (group != line.Group()) {
			group = line.Group();
			boost::push_back(dst, line.GroupHeader() + "\r\n");
		}
		boost::push_back(dst, line.GetEntryData() + "\r\n");
	}
}

bool AssFile::CanSave() const {
	try {
		return SubtitleFormat::GetWriter(filename)->CanSave(this);
	}
	catch (...) {
		return false;
	}
}

void AssFile::LoadDefault(bool defline) {
	Line.push_back(*new AssInfo("Title", "Default Aegisub file"));
	Line.push_back(*new AssInfo("ScriptType", "v4.00+"));
	Line.push_back(*new AssInfo("WrapStyle", "0"));
	Line.push_back(*new AssInfo("ScaledBorderAndShadow", "yes"));
	Line.push_back(*new AssInfo("Collisions", "Normal"));
	if (!OPT_GET("Subtitle/Default Resolution/Auto")->GetBool()) {
		Line.push_back(*new AssInfo("PlayResX", std::to_string(OPT_GET("Subtitle/Default Resolution/Width")->GetInt())));
		Line.push_back(*new AssInfo("PlayResY", std::to_string(OPT_GET("Subtitle/Default Resolution/Height")->GetInt())));
	}
	Line.push_back(*new AssInfo("YCbCr Matrix", "None"));

	Line.push_back(*new AssStyle);

	if (defline)
		Line.push_back(*new AssDialogue);

	autosavedCommitId = savedCommitId = commitId + 1;
	Commit("", COMMIT_NEW);
	loaded = true;
	StandardPaths::SetPathValue("?script", "");
	FileOpen("");
}

void AssFile::swap(AssFile &that) throw() {
	// Intentionally does not swap undo stack related things
	using std::swap;
	swap(loaded, that.loaded);
	swap(commitId, that.commitId);
	swap(undoDescription, that.undoDescription);
	swap(Line, that.Line);
}

AssFile::AssFile(const AssFile &from)
: undoDescription(from.undoDescription)
, commitId(from.commitId)
, filename(from.filename)
, loaded(from.loaded)
{
	Line.clone_from(from.Line, std::mem_fun_ref(&AssEntry::Clone), delete_ptr());
}
AssFile& AssFile::operator=(AssFile from) {
	std::swap(*this, from);
	return *this;
}

void AssFile::InsertLine(AssEntry *entry) {
	if (Line.empty()) {
		Line.push_back(*entry);
		return;
	}

	// Search for insertion point
	entryIter it = Line.end();
	do {
		--it;
		if (it->Group() <= entry->Group()) {
			Line.insert(++it, *entry);
			return;
		}
	} while (it != Line.begin());

	Line.push_front(*entry);
}

void AssFile::InsertAttachment(agi::fs::path const& filename) {
	AssEntryGroup group = ENTRY_GRAPHIC;

	auto ext = boost::to_lower_copy(filename.extension().string());
	if (ext == ".ttf" || ext == ".ttc" || ext == ".pfb")
		group = ENTRY_FONT;

	InsertLine(new AssAttachment(filename, group));
}

std::string AssFile::GetScriptInfo(std::string const& key) const {
	for (const auto info : Line | agi::of_type<AssInfo>()) {
		if (boost::iequals(key, info->Key()))
			return info->Value();
	}

	return "";
}

int AssFile::GetScriptInfoAsInt(std::string const& key) const {
	return atoi(GetScriptInfo(key).c_str());
}

void AssFile::SetScriptInfo(std::string const& key, std::string const& value) {
	for (auto info : Line | agi::of_type<AssInfo>()) {
		if (boost::iequals(key, info->Key())) {
			if (value.empty())
				delete info;
			else
				info->SetValue(value);
			return;
		}
	}

	if (!value.empty())
		InsertLine(new AssInfo(key, value));
}

void AssFile::GetResolution(int &sw,int &sh) const {
	sw = GetScriptInfoAsInt("PlayResX");
	sh = GetScriptInfoAsInt("PlayResY");

	// Gabest logic?
	if (sw == 0 && sh == 0) {
		sw = 384;
		sh = 288;
	} else if (sw == 0) {
		if (sh == 1024)
			sw = 1280;
		else
			sw = sh * 4 / 3;
	} else if (sh == 0) {
		// you are not crazy; this doesn't make any sense
		if (sw == 1280)
			sh = 1024;
		else
			sh = sw * 3 / 4;
	}
}

std::vector<std::string> AssFile::GetStyles() const {
	std::vector<std::string> styles;
	for (auto style : Line | agi::of_type<AssStyle>())
		styles.push_back(style->name);
	return styles;
}

AssStyle *AssFile::GetStyle(std::string const& name) {
	for (auto style : Line | agi::of_type<AssStyle>()) {
		if (boost::iequals(style->name, name))
			return style;
	}
	return nullptr;
}

void AssFile::AddToRecent(agi::fs::path const& file) const {
	config::mru->Add("Subtitle", file);
	OPT_SET("Path/Last/Subtitles")->SetString(file.parent_path().string());
}

int AssFile::Commit(wxString const& desc, int type, int amendId, AssEntry *single_line) {
	std::set<const AssEntry*> changed_lines;
	if (single_line)
		changed_lines.insert(single_line);

	++commitId;
	// Allow coalescing only if it's the last change and the file has not been
	// saved since the last change
	if (commitId == amendId+1 && RedoStack.empty() && savedCommitId+1 != commitId && autosavedCommitId+1 != commitId) {
		// If only one line changed just modify it instead of copying the file
		if (single_line) {
			entryIter this_it = Line.begin(), undo_it = UndoStack.back().Line.begin();
			while (&*this_it != single_line) {
				++this_it;
				++undo_it;
			}
			UndoStack.back().Line.insert(undo_it, *single_line->Clone());
			delete &*undo_it;
		}
		else {
			UndoStack.back() = *this;
		}
		AnnounceCommit(type, changed_lines);
		return commitId;
	}

	RedoStack.clear();

	// Place copy on stack
	undoDescription = desc;
	UndoStack.push_back(*this);

	// Cap depth
	int depth = std::max<int>(OPT_GET("Limits/Undo Levels")->GetInt(), 2);
	while ((int)UndoStack.size() > depth) {
		UndoStack.pop_front();
	}

	if (UndoStack.size() > 1 && OPT_GET("App/Auto/Save on Every Change")->GetBool() && !filename.empty() && CanSave())
		Save(filename);

	AnnounceCommit(type, changed_lines);
	return commitId;
}

void AssFile::Undo() {
	if (UndoStack.size() <= 1) return;

	RedoStack.emplace_back();
	swap(RedoStack.back());
	UndoStack.pop_back();
	*this = UndoStack.back();

	AnnounceCommit(COMMIT_NEW, std::set<const AssEntry*>());
}

void AssFile::Redo() {
	if (RedoStack.empty()) return;

	swap(RedoStack.back());
	UndoStack.push_back(*this);
	RedoStack.pop_back();

	AnnounceCommit(COMMIT_NEW, std::set<const AssEntry*>());
}

wxString AssFile::GetUndoDescription() const {
	return IsUndoStackEmpty() ? "" : UndoStack.back().undoDescription;
}

wxString AssFile::GetRedoDescription() const {
	return IsRedoStackEmpty() ? "" : RedoStack.back().undoDescription;
}

bool AssFile::CompStart(const AssDialogue* lft, const AssDialogue* rgt) {
	return lft->Start < rgt->Start;
}
bool AssFile::CompEnd(const AssDialogue* lft, const AssDialogue* rgt) {
	return lft->End < rgt->End;
}
bool AssFile::CompStyle(const AssDialogue* lft, const AssDialogue* rgt) {
	return lft->Style < rgt->Style;
}
bool AssFile::CompActor(const AssDialogue* lft, const AssDialogue* rgt) {
	return lft->Actor < rgt->Actor;
}
bool AssFile::CompEffect(const AssDialogue* lft, const AssDialogue* rgt) {
	return lft->Effect < rgt->Effect;
}
bool AssFile::CompLayer(const AssDialogue* lft, const AssDialogue* rgt) {
	return lft->Layer < rgt->Layer;
}

void AssFile::Sort(CompFunc comp, std::set<AssDialogue*> const& limit) {
	Sort(Line, comp, limit);
}
namespace {
	struct AssEntryComp : public std::binary_function<AssEntry, AssEntry, bool> {
		AssFile::CompFunc comp;
		bool operator()(AssEntry const&a, AssEntry const&b) const {
			return comp(static_cast<const AssDialogue*>(&a), static_cast<const AssDialogue*>(&b));
		}
	};

	inline bool is_dialogue(AssEntry *e, std::set<AssDialogue*> const& limit) {
		AssDialogue *d = dynamic_cast<AssDialogue*>(e);
		return d && (limit.empty() || limit.count(d));
	}
}

void AssFile::Sort(EntryList &lst, CompFunc comp, std::set<AssDialogue*> const& limit) {
	AssEntryComp compE;
	compE.comp = comp;
	// Sort each block of AssDialogues separately, leaving everything else untouched
	for (entryIter begin = lst.begin(); begin != lst.end(); ++begin) {
		if (!is_dialogue(&*begin, limit)) continue;
		entryIter end = begin;
		while (end != lst.end() && is_dialogue(&*end, limit)) ++end;

		// used instead of std::list::sort for partial list sorting
		EntryList tmp;
		tmp.splice(tmp.begin(), lst, begin, end);
		tmp.sort(compE);
		lst.splice(end, tmp);

		begin = --end;
	}
}

AssFile *AssFile::top;
