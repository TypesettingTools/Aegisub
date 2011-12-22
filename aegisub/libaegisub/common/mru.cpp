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
//
// $Id$

/// @file mru.cpp
/// @brief Most Recently Used (MRU) Lists
/// @ingroup libaegisub

#include "libaegisub/cajun/writer.h"

#include "libaegisub/access.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"
#include "libaegisub/mru.h"

namespace agi {

MRUManager::MRUManager(const std::string &config, const std::string &default_config): config_name(config) {
	LOG_D("agi/mru") << "Loading MRU List";

	json::Object root_new = json_util::file(config, default_config);

	json::Object::const_iterator index_object(root_new.begin()), index_objectEnd(root_new.end());

	for (; index_object != index_objectEnd; ++index_object)
		Load(index_object->first, index_object->second);
}


MRUManager::~MRUManager() {
}

MRUManager::MRUListMap &MRUManager::Find(std::string const& key) {
	MRUMap::iterator index = mru.find(key);
	if (index == mru.end())
		throw MRUErrorInvalidKey("Invalid key value");
	return index->second;
}


void MRUManager::Add(const std::string &key, const std::string &entry) {
	MRUListMap &map = Find(key);
	map.remove(entry);
	map.push_front(entry);
	Prune(map);

	Flush();
}


void MRUManager::Remove(const std::string &key, const std::string &entry) {
	Find(key).remove(entry);

	Flush();
}


const MRUManager::MRUListMap* MRUManager::Get(const std::string &key) {
	return &Find(key);
}

std::string const& MRUManager::GetEntry(const std::string &key, size_t entry) {
	const MRUManager::MRUListMap *map = Get(key);

	if (entry > map->size())
		throw MRUErrorIndexOutOfRange("Requested element index is out of range.");

	MRUListMap::const_iterator index = map->begin();
	advance(index, entry);
	return *index;
}


void MRUManager::Flush() {
	json::Object out;

	for (MRUMap::const_iterator i = mru.begin(); i != mru.end(); ++i) {
		json::Array &array = out[i->first];
		copy(i->second.begin(), i->second.end(), std::back_inserter(array));
	}

	json::Writer::Write(out, io::Save(config_name).Get());
}


/// @brief Prune MRUListMap to the desired length.
/// This uses the user-set values for MRU list length.
inline void MRUManager::Prune(MRUListMap& map) {
	map.resize(std::min<size_t>(16, map.size()));
}

/// @brief Load MRU Lists.
/// @param key List name.
/// @param array json::Array of values.
void MRUManager::Load(const std::string &key, const json::Array& array) {
	try {
		copy(array.begin(), array.end(), back_inserter(mru[key]));
	}
	catch (json::Exception const&) {
		// Out of date MRU file; just discard the data and skip it
	}
	Prune(mru[key]);
}

}
