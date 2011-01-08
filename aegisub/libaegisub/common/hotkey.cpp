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

/// @file hotkey.cpp
/// @brief Hotkey handler
/// @ingroup hotkey menu event window

#include "../config.h"

#ifndef LAGI_PRE
#include <math.h>

#include <memory>
#endif

#include "libaegisub/hotkey.h"

#include "libaegisub/access.h"
#include "libaegisub/cajun/writer.h"
#include "libaegisub/exception.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"


namespace agi {
	namespace hotkey {

Hotkey *hotkey;

std::string Combo::Str() {
	std::string str(key_map[0]);
	for (unsigned int i=1; i < key_map.size(); i++) {
		str.append("-" + key_map[i]);
	}
	return str;
}

std::string Combo::StrMenu() {
	return Str();
}

void Hotkey::ComboInsert(Combo *combo) {
    map.insert(HotkeyMapPair(combo->Str(), combo));
}

Hotkey::~Hotkey() {
	Flush();
}

Hotkey::Hotkey(const std::string &file, const std::string &default_config):
				config_file(file), config_default(default_config) {

	LOG_D("hotkey/init") << "Generating hotkeys.";

	std::istream *stream;

    try {
		stream = agi::io::Open(config_file);
	} catch (const acs::AcsNotFound&) {
		stream = new std::istringstream(config_default);
    }


	json::UnknownElement hotkey_root;
    try {
		hotkey_root = agi::json_util::parse(stream);
	} catch (json::Reader::ParseException& e) {
		// There's definatly a better way to do this.
		std::istringstream *stream = new std::istringstream(config_default);
		hotkey_root = agi::json_util::parse(stream);
	}

	json::Object object = hotkey_root;


	for (json::Object::const_iterator index(object.Begin()); index != object.End(); index++) {
		const json::Object::Member& member = *index;
		const json::Object& obj = member.element;
		BuildHotkey(member.name, obj);
    }
}


void Hotkey::BuildHotkey(std::string context, const json::Object& object) {

	for (json::Object::const_iterator index(object.Begin()); index != object.End(); index++) {
		const json::Object::Member& member = *index;


		const json::Array& array = member.element;
		for (json::Array::const_iterator arr_index(array.Begin()); arr_index != array.End(); arr_index++) {

			Combo *combo = new Combo(context, member.name);

	        const json::Object& obj = *arr_index;

			const json::Array& arr_mod = obj["modifiers"];

			if (arr_mod.Size() >  0) {
				for (json::Array::const_iterator arr_mod_index(arr_mod.Begin()); arr_mod_index != arr_mod.End(); arr_mod_index++) {
					const json::String& key_mod = *arr_mod_index;
					combo->KeyInsert(key_mod.Value());
				} // for arr_mod_index

			}
			const json::String& key = obj["key"];
			combo->KeyInsert(key.Value());

			const json::Boolean& enable = obj["enable"];
			combo->Enable(enable);

			ComboInsert(combo);
		} // for arr_index
	} // for index
}


bool Hotkey::Scan(const std::string context, const std::string str, std::string &cmd) {
	HotkeyMap::iterator index;
	std::pair<HotkeyMap::iterator, HotkeyMap::iterator> range;

	range = map.equal_range(str);
	std::string local, dfault;


	for (index = range.first; index != range.second; ++index) {

		std::string ctext = (*index).second->Context();

		if (ctext == "Always") {
			cmd = (*index).second->CmdName();
			LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Always   Command: " << cmd;
			return 0;
		} else if (ctext == "Default") {
			dfault = (*index).second->CmdName();
		} else if (ctext == context) {
			local = (*index).second->CmdName();
		}
	}

		if (!local.empty()) {
			cmd = local;
			LOG_D("agi/hotkey/found") << "Found: " << str << "  Context: " << context << "  Command: " << local;
			return 0;
		} else if (!dfault.empty()) {
			cmd = dfault;
			LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Default   Command: " << dfault;
			return 0;
		}

		return 1;

}

void Hotkey::Flush() {

	json::Object root;

	HotkeyMap::iterator index;
	for (index = map.begin(); index != map.end(); ++index) {

		Combo::ComboMap combo_map(index->second->Get());

		json::Array modifiers;
		for (int i = 0; i != combo_map.size()-1; i++) {
			modifiers.Insert(json::String(combo_map[i]));
		}

		json::Object hotkey;
		hotkey["modifiers"] = modifiers;
		hotkey["key"] = json::String(combo_map.back());
		hotkey["enable"] = json::Boolean(index->second->IsEnabled());

		json::Object& context_obj = root[index->second->Context()];
		json::Array& combo_array = context_obj[index->second->CmdName()];

		combo_array.Insert(hotkey);
	}

	io::Save file(config_file);
	json::Writer::Write(root, file.Get());

}

	} // namespace toolbar
} // namespace agi
