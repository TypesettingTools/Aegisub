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
#include "utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/of_type_adaptor.h>

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/path.hpp>

namespace std {
	template<>
	void swap(AssFile &lft, AssFile &rgt) {
		lft.swap(rgt);
	}
}

AssFile::~AssFile() {
	auto copy = new EntryList;
	copy->swap(Line);
	agi::dispatch::Background().Async([=]{
		copy->clear_and_dispose([](AssEntry *e) { delete e; });
		delete copy;
	});
}

void AssFile::LoadDefault(bool defline) {
	Line.push_back(*new AssInfo("Title", "Default Aegisub file"));
	Line.push_back(*new AssInfo("ScriptType", "v4.00+"));
	Line.push_back(*new AssInfo("WrapStyle", "0"));
	Line.push_back(*new AssInfo("ScaledBorderAndShadow", "yes"));
	if (!OPT_GET("Subtitle/Default Resolution/Auto")->GetBool()) {
		Line.push_back(*new AssInfo("PlayResX", std::to_string(OPT_GET("Subtitle/Default Resolution/Width")->GetInt())));
		Line.push_back(*new AssInfo("PlayResY", std::to_string(OPT_GET("Subtitle/Default Resolution/Height")->GetInt())));
	}
	Line.push_back(*new AssInfo("YCbCr Matrix", "None"));

	Line.push_back(*new AssStyle);

	if (defline)
		Line.push_back(*new AssDialogue);
}

void AssFile::swap(AssFile &that) throw() {
	Line.swap(that.Line);
}

AssFile::AssFile(const AssFile &from) {
	Line.clone_from(from.Line, std::mem_fun_ref(&AssEntry::Clone), [](AssEntry *e) { delete e; });
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
	AssEntryGroup group = AssEntryGroup::GRAPHIC;

	auto ext = boost::to_lower_copy(filename.extension().string());
	if (ext == ".ttf" || ext == ".ttc" || ext == ".pfb")
		group = AssEntryGroup::FONT;

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

std::string AssFile::GetUIState(std::string const& key) const {
	auto value = GetScriptInfo("Aegisub " + key);
	if (value.empty())
		value = GetScriptInfo(key);
	return value;
}

int AssFile::GetUIStateAsInt(std::string const& key) const {
	return atoi(GetUIState(key).c_str());
}

void AssFile::SaveUIState(std::string const& key, std::string const& value) {
	if (OPT_GET("App/Save UI State")->GetBool())
		SetScriptInfo("Aegisub " + key, value);
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

int AssFile::Commit(wxString const& desc, int type, int amend_id, AssEntry *single_line) {
	AssFileCommit c = { desc, &amend_id, single_line };
	PushState(c);

	std::set<const AssEntry*> changed_lines;
	if (single_line)
		changed_lines.insert(single_line);

	AnnounceCommit(type, changed_lines);

	return amend_id;
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
	inline bool is_dialogue(AssEntry *e, std::set<AssDialogue*> const& limit) {
		AssDialogue *d = dynamic_cast<AssDialogue*>(e);
		return d && (limit.empty() || limit.count(d));
	}
}

void AssFile::Sort(EntryList &lst, CompFunc comp, std::set<AssDialogue*> const& limit) {
	auto compE = [&](AssEntry const& a, AssEntry const& b) {
		return comp(static_cast<const AssDialogue*>(&a), static_cast<const AssDialogue*>(&b));
	};

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
