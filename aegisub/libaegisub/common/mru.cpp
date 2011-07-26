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

#ifndef LAGI_PRE
#include <fstream>
#include <time.h>
#endif

#include "libaegisub/cajun/writer.h"

#include "libaegisub/access.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"
#include "libaegisub/mru.h"
#include "libaegisub/io.h"

namespace agi {

MRUManager::MRUManager(const std::string &config, const std::string &default_config): config_name(config) {
	LOG_D("agi/mru") << "Loading MRU List";

	json::UnknownElement root = json_util::file(config, default_config);
	const json::Object& root_new = (json::Object)root;

	json::Object::const_iterator index_object(root_new.Begin()), index_objectEnd(root_new.End());

	for (; index_object != index_objectEnd; ++index_object) {
		const json::Object::Member& member = *index_object;
		const std::string &member_name = member.name;
		const json::UnknownElement& element = member.element;

		Load(member_name, (json::Array)element);
	}
}


MRUManager::~MRUManager() {
	Flush();

	for (MRUMap::iterator i = mru.begin(); i != mru.end(); ++i) {
		delete i->second;
	}
}


void MRUManager::Add(const std::string &key, const std::string &entry) {
	MRUMap::iterator index;

	if ((index = mru.find(key)) != mru.end()) {
		MRUListMap &map = *index->second;

		// Remove the file before adding it.
		Remove(key, entry);

		map.insert(std::pair<time_t, std::string>(time(NULL), entry));

		Prune(map);

	} else {
		throw MRUErrorInvalidKey("Invalid key value");
	}
}


void MRUManager::Remove(const std::string &key, const std::string &entry) {
	MRUMap::iterator index;

	if ((index = mru.find(key)) != mru.end()) {
		MRUListMap &map = *index->second;
		for (MRUListMap::iterator map_idx = map.begin(); map_idx != map.end();) {
			if (map_idx->second == entry)
				map.erase(map_idx++);
			else
				++map_idx;
		}
	} else {
		throw MRUErrorInvalidKey("Invalid key value");
	}

}


const MRUManager::MRUListMap* MRUManager::Get(const std::string &key) {
	MRUMap::iterator index;

	if ((index = mru.find(key)) != mru.end()) {
		return index->second;
	} else {
		throw MRUErrorInvalidKey("Invalid key value");
	}
}


const std::string MRUManager::GetEntry(const std::string &key, const int entry) {
	const MRUManager::MRUListMap *map = Get(key);

	MRUListMap::const_iterator index = map->begin();

	if ((unsigned int)entry > map->size())
		throw MRUErrorIndexOutOfRange("Requested element index is out of range.");

	std::advance(index, entry);

	return index->second;
}


void MRUManager::Flush() {
	json::Object out;

	for (MRUMap::const_iterator i = mru.begin(); i != mru.end(); ++i) {
		json::Array array;
		MRUListMap *map_list = i->second;

		for (MRUListMap::const_iterator i_lst = map_list->begin(); i_lst != map_list->end(); ++i_lst) {
			json::Object obj;
			obj["time"] = json::Number((double)i_lst->first);
			obj["entry"] = json::String(i_lst->second);
			array.Insert(obj);
		}

		out[i->first] = array;
	}

	io::Save file(config_name);
	std::ofstream& ofp = file.Get();
	json::Writer::Write(out, ofp);

}


/// @brief Prune MRUListMap to the desired length.
/// This uses the user-set values for MRU list length.
inline void MRUManager::Prune(MRUListMap& map) {
	unsigned int size = 16;

	MRUListMap::iterator index = map.begin();;

	if (map.size() >= size) {
		std::advance(index, size);

		// Use a range incase the storage number shrinks.
		map.erase(index, map.end());
	}
}

/// @brief Load MRU Lists.
/// @param key List name.
/// @param array json::Array of values.
void MRUManager::Load(const std::string &key, const json::Array& array) {
	json::Array::const_iterator index(array.Begin()), indexEnd(array.End());

	MRUListMap *map = new MRUListMap();

	for (; index != indexEnd; ++index) {
		const json::Object& obj = *index;

		time_t time = (time_t)(json::Number)obj["time"];
		std::string entry = (json::String)obj["entry"];

		map->insert(make_pair(time, entry));
	}

	mru[key] = map;
	Prune(*map);
}


}
