// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

#include "libaegisub/mru.h"

#include "libaegisub/cajun/writer.h"

#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"
#include "libaegisub/option.h"
#include "libaegisub/option_value.h"

namespace {
const char* mru_names[] = {
	"Audio", "Find", "Keyframes", "Replace", "Subtitle", "Timecodes", "Video",
};

const char* option_names[] = {
	"Limits/MRU", "Limits/Find Replace", "Limits/MRU", "Limits/Find Replace",
	"Limits/MRU", "Limits/MRU",          "Limits/MRU",
};

int mru_index(const char* key) {
	int i;
	switch(*key) {
		case 'A': i = 0; break;
		case 'F': i = 1; break;
		case 'K': i = 2; break;
		case 'R': i = 3; break;
		case 'S': i = 4; break;
		case 'T': i = 5; break;
		case 'V': i = 6; break;
		default: return -1;
	}
	return strcmp(key, mru_names[i]) == 0 ? i : -1;
}
} // namespace

namespace agi {
MRUManager::MRUManager(agi::fs::path const& config, std::pair<const char*, size_t> default_config,
                       agi::Options* options)
    : config_name(config), options(options) {
	LOG_D("agi/mru") << "Loading MRU List";

	auto root = json_util::file(config, default_config);
	for(auto const& it : static_cast<json::Object const&>(root))
		Load(it.first.c_str(), it.second);
}

MRUManager::MRUListMap& MRUManager::Find(const char* key) {
	auto index = mru_index(key);
	if(index == -1) throw MRUError("Invalid key value");
	return mru[index];
}

void MRUManager::Add(const char* key, agi::fs::path const& entry) {
	MRUListMap& map = Find(key);
	auto it = find(begin(map), end(map), entry);
	if(it == begin(map) && it != end(map)) return;
	if(it != end(map))
		rotate(begin(map), it, it + 1);
	else {
		map.insert(begin(map), entry);
		Prune(key, map);
	}

	Flush();
}

void MRUManager::Remove(const char* key, agi::fs::path const& entry) {
	auto& map = Find(key);
	map.erase(remove(begin(map), end(map), entry), end(map));
	Flush();
}

const MRUManager::MRUListMap* MRUManager::Get(const char* key) {
	return &Find(key);
}

agi::fs::path const& MRUManager::GetEntry(const char* key, const size_t entry) {
	const auto map = Get(key);
	if(entry >= map->size()) throw MRUError("Requested element index is out of range.");

	return *next(map->begin(), entry);
}

void MRUManager::Flush() {
	json::Object out;

	for(size_t i = 0; i < mru.size(); ++i) {
		json::Array& array = out[mru_names[i]];
		for(auto const& p : mru[i])
			array.push_back(p.string());
	}

	agi::JsonWriter::Write(out, io::Save(config_name).Get());
}

void MRUManager::Prune(const char* key, MRUListMap& map) const {
	size_t limit = 16u;
	if(options) {
		int idx = mru_index(key);
		if(idx != -1) limit = (size_t)options->Get(option_names[idx])->GetInt();
	}
	map.resize(std::min(limit, map.size()));
}

void MRUManager::Load(const char* key, const json::Array& array) {
	int idx = mru_index(key);
	if(idx == -1) return;

	try {
		mru[idx].reserve(array.size());
		for(std::string const& str : array)
			mru[idx].push_back(str);
	} catch(json::Exception const&) {
		// Out of date MRU file; just discard the data and skip it
	}
	Prune(key, mru[idx]);
}

} // namespace agi
