// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "ass_file.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_info.h"
#include "ass_style.h"
#include "ass_style_storage.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/fs.h>

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <cassert>

AssFile::AssFile() { }

AssFile::~AssFile() {
	Styles.clear_and_dispose([](AssStyle *e) { delete e; });
	Events.clear_and_dispose([](AssDialogue *e) { delete e; });
}

void AssFile::LoadDefault(bool include_dialogue_line, agi::fs::path const& style_catalog_file) {
	Info.emplace_back("Title", "Default Aegisub file");
	Info.emplace_back("ScriptType", "v4.00+");
	Info.emplace_back("WrapStyle", "0");
	Info.emplace_back("ScaledBorderAndShadow", "yes");
	if (!OPT_GET("Subtitle/Default Resolution/Auto")->GetBool()) {
		Info.emplace_back("PlayResX", std::to_string(OPT_GET("Subtitle/Default Resolution/Width")->GetInt()));
		Info.emplace_back("PlayResY", std::to_string(OPT_GET("Subtitle/Default Resolution/Height")->GetInt()));
	}
	Info.emplace_back("YCbCr Matrix", "None");

	// Add default style
	Styles.push_back(*new AssStyle);

	// Add/replace any catalog styles requested
	if (!style_catalog_file.empty() && agi::fs::FileExists(style_catalog_file)) {
		AssStyleStorage catalog;
		catalog.Load(style_catalog_file);
		catalog.ReplaceIntoFile(*this);
	}

	if (include_dialogue_line)
		Events.push_back(*new AssDialogue);
}

AssFile::AssFile(const AssFile &from)
: Info(from.Info)
, Attachments(from.Attachments)
, Extradata(from.Extradata)
, next_extradata_id(from.next_extradata_id)
{
	Styles.clone_from(from.Styles,
		[](AssStyle const& e) { return new AssStyle(e); },
		[](AssStyle *e) { delete e; });
	Events.clone_from(from.Events,
		[](AssDialogue const& e) { return new AssDialogue(e); },
		[](AssDialogue *e) { delete e; });
}

void AssFile::swap(AssFile& from) throw() {
	Info.swap(from.Info);
	Styles.swap(from.Styles);
	Events.swap(from.Events);
	Attachments.swap(from.Attachments);
	Extradata.swap(from.Extradata);
	std::swap(next_extradata_id, from.next_extradata_id);
}

AssFile& AssFile::operator=(AssFile from) {
	swap(from);
	return *this;
}

EntryList<AssDialogue>::iterator AssFile::iterator_to(AssDialogue& line) {
	using l = EntryList<AssDialogue>;
	bool in_list = !l::node_algorithms::inited(l::value_traits::to_node_ptr(line));
	return in_list ? Events.iterator_to(line) : Events.end();
}

void AssFile::InsertAttachment(agi::fs::path const& filename) {
	AssEntryGroup group = AssEntryGroup::GRAPHIC;

	auto ext = boost::to_lower_copy(filename.extension().string());
	if (ext == ".ttf" || ext == ".ttc" || ext == ".pfb")
		group = AssEntryGroup::FONT;

	Attachments.emplace_back(filename, group);
}

std::string AssFile::GetScriptInfo(std::string const& key) const {
	for (auto const& info : Info) {
		if (boost::iequals(key, info.Key()))
			return info.Value();
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
	for (auto it = Info.begin(); it != Info.end(); ++it) {
		if (boost::iequals(key, it->Key())) {
			if (value.empty())
				Info.erase(it);
			else
				it->SetValue(value);
			return;
		}
	}

	if (!value.empty())
		Info.emplace_back(key, value);
}

void AssFile::GetResolution(int &sw, int &sh) const {
	sw = GetScriptInfoAsInt("PlayResX");
	sh = GetScriptInfoAsInt("PlayResY");

	// Gabest logic: default is 384x288, assume 1280x1024 if either height or
	// width are that, otherwise assume 4:3 if only heigh or width are set.
	// Why 1280x1024? Who the fuck knows. Clearly just Gabest trolling everyone.
	if (sw == 0 && sh == 0) {
		sw = 384;
		sh = 288;
	}
	else if (sw == 0)
		sw = sh == 1024 ? 1280 : sh * 4 / 3;
	else if (sh == 0)
		sh = sw == 1280 ? 1024 : sw * 3 / 4;
}

std::vector<std::string> AssFile::GetStyles() const {
	std::vector<std::string> styles;
	for (auto& style : Styles)
		styles.push_back(style.name);
	return styles;
}

AssStyle *AssFile::GetStyle(std::string const& name) {
	for (auto& style : Styles) {
		if (boost::iequals(style.name, name))
			return &style;
	}
	return nullptr;
}

int AssFile::Commit(wxString const& desc, int type, int amend_id, AssDialogue *single_line) {
	if (type == COMMIT_NEW || (type & COMMIT_DIAG_ADDREM) || (type & COMMIT_ORDER)) {
		int i = 0;
		for (auto& event : Events)
			event.Row = i++;
	}

	PushState({desc, &amend_id, single_line});

	std::set<const AssDialogue*> changed_lines;
	if (single_line)
		changed_lines.insert(single_line);

	AnnounceCommit(type, changed_lines);

	return amend_id;
}

bool AssFile::CompStart(AssDialogue const& lft, AssDialogue const& rgt) {
	return lft.Start < rgt.Start;
}
bool AssFile::CompEnd(AssDialogue const& lft, AssDialogue const& rgt) {
	return lft.End < rgt.End;
}
bool AssFile::CompStyle(AssDialogue const& lft, AssDialogue const& rgt) {
	return lft.Style < rgt.Style;
}
bool AssFile::CompActor(AssDialogue const& lft, AssDialogue const& rgt) {
	return lft.Actor < rgt.Actor;
}
bool AssFile::CompEffect(AssDialogue const& lft, AssDialogue const& rgt) {
	return lft.Effect < rgt.Effect;
}
bool AssFile::CompLayer(AssDialogue const& lft, AssDialogue const& rgt) {
	return lft.Layer < rgt.Layer;
}

void AssFile::Sort(CompFunc comp, std::set<AssDialogue*> const& limit) {
	Sort(Events, comp, limit);
}

void AssFile::Sort(EntryList<AssDialogue> &lst, CompFunc comp, std::set<AssDialogue*> const& limit) {
	if (limit.empty()) {
		lst.sort(comp);
		return;
	}

	// Sort each selected block separately, leaving everything else untouched
	for (auto begin = lst.begin(); begin != lst.end(); ++begin) {
		if (!limit.count(&*begin)) continue;
		auto end = begin;
		while (end != lst.end() && limit.count(&*end)) ++end;

		// sort doesn't support only sorting a sublist, so move them to a temp list
		EntryList<AssDialogue> tmp;
		tmp.splice(tmp.begin(), lst, begin, end);
		tmp.sort(comp);
		lst.splice(end, tmp);

		begin = --end;
	}
}


uint32_t AssFile::AddExtradata(std::string const& key, std::string const& value) {
	// next_extradata_id must not exist
	assert(Extradata.find(next_extradata_id) == Extradata.end());
	Extradata[next_extradata_id] = std::make_pair(key, value);
	return next_extradata_id++; // return old value, then post-increment
}

std::map<std::string, std::string> AssFile::GetExtradata(std::vector<uint32_t> const& id_list) const {
	// If multiple IDs have the same key name, the last ID wins
	std::map<std::string, std::string> result;
	for (auto id : id_list) {
		auto it = Extradata.find(id);
		if (it != Extradata.end())
			result[it->second.first] = it->second.second;
	}
	return result;
}

void AssFile::CleanExtradata() {
	// Collect all IDs existing in the database
	// Then remove all IDs found to be in use from this list
	// Remaining is then all garbage IDs
	std::vector<uint32_t> ids;
	for (auto& it : Extradata)
		ids.push_back(it.first);
	if (ids.empty()) return;

	// For each line, find which IDs it actually uses and remove them from the unused-list
	for (auto& line : Events) {
		// Find the ID for each unique key in the line
		std::map<std::string, uint32_t> key_ids;
		for (auto id : line.ExtradataIds.get()) {
			auto ed_it = Extradata.find(id);
			if (ed_it == Extradata.end())
				continue;
			key_ids[ed_it->second.first] = id;
		}
		// Update the line's ID list to only contain the actual ID for any duplicate keys
		// Also mark found IDs as used in the cleaning list
		std::vector<uint32_t> new_ids;
		for (auto& keyid : key_ids) {
			new_ids.push_back(keyid.second);
			ids.erase(std::remove(ids.begin(), ids.end(), keyid.second));
		}
		line.ExtradataIds = new_ids;
	}

	// The ids list should contain only unused IDs now
	for (auto id : ids) {
		Extradata.erase(id);
	}
}

