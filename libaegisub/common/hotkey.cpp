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

/// @file hotkey.cpp
/// @brief Hotkey handler
/// @ingroup hotkey menu event window

#include "libaegisub/hotkey.h"

#include "libaegisub/cajun/writer.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <tuple>

namespace agi {
	namespace hotkey {

std::string Combo::Str() const {
	return boost::algorithm::join(key_map, "-");
}

std::string Combo::StrMenu() const {
	return Str();
}

void Hotkey::ComboInsert(Combo const& combo) {
	str_map.insert(make_pair(combo.Str(), combo));
	cmd_map.insert(make_pair(combo.CmdName(), combo));
}

Hotkey::Hotkey(fs::path const& file, std::pair<const char *, size_t> default_config)
: config_file(file)
{
	LOG_D("hotkey/init") << "Generating hotkeys.";

	auto root = json_util::file(config_file, default_config);
	for (auto const& hotkey_context : static_cast<json::Object const&>(root))
		BuildHotkey(hotkey_context.first, hotkey_context.second);
}

void Hotkey::BuildHotkey(std::string const& context, json::Object const& hotkeys) {
	for (auto const& command : hotkeys) {
		const json::Array& command_hotkeys = command.second;

		for (json::Object const& hotkey : command_hotkeys) {
			std::vector<std::string> keys;

			auto it = hotkey.find("modifiers");
			if (it == end(hotkey)) {
				LOG_E("agi/hotkey/load") << "Hotkey for command '" << command.first << "' is missing modifiers";
				continue;
			}

			json::Array const& arr_mod = it->second;
			keys.reserve(arr_mod.size() + 1);
			copy(arr_mod.begin(), arr_mod.end(), back_inserter(keys));

			it = hotkey.find("key");
			if (it == end(hotkey)) {
				LOG_E("agi/hotkey/load") << "Hotkey for command '" << command.first << "' is missing the key";
				continue;
			}

			keys.push_back(it->second);
			ComboInsert(Combo(context, command.first, keys));
		}
	}
}

std::string Hotkey::Scan(const std::string &context, const std::string &str, bool always) const {
	std::string local, dfault;

	HotkeyMap::const_iterator index, end;
	for (std::tie(index, end) = str_map.equal_range(str); index != end; ++index) {
		std::string const& ctext = index->second.Context();

		if (always && ctext == "Always") {
			LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Always   Command: " << index->second.CmdName();
			return index->second.CmdName();
		}
		if (ctext == "Default")
			dfault = index->second.CmdName();
		else if (ctext == context)
			local = index->second.CmdName();
	}

	if (!local.empty()) {
		LOG_D("agi/hotkey/found") << "Found: " << str << "  Context: " << context << "  Command: " << local;
		return local;
	}
	if (!dfault.empty()) {
		LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Default   Command: " << dfault;
		return dfault;
	}

	return "";
}

bool Hotkey::HasHotkey(const std::string &context, const std::string &str) const {
	HotkeyMap::const_iterator index, end;
	for (std::tie(index, end) = str_map.equal_range(str); index != end; ++index) {
		std::string const& ctext = index->second.Context();

		if (ctext == context)
			return true;
	}
	return false;
}

std::vector<std::string> Hotkey::GetHotkeys(const std::string &context, const std::string &command) const {
	std::vector<std::string> ret;

	HotkeyMap::const_iterator it, end;
	for (std::tie(it, end) = cmd_map.equal_range(command); it != end; ++it) {
		std::string ctext = it->second.Context();
		if (ctext == "Always" || ctext == "Default" || ctext == context)
			ret.emplace_back(it->second.StrMenu());
	}

	sort(ret.begin(), ret.end());
	ret.erase(unique(ret.begin(), ret.end()), ret.end());

	return ret;
}

std::string Hotkey::GetHotkey(const std::string &context, const std::string &command) const {
	std::string ret;
	HotkeyMap::const_iterator it, end;
	for (std::tie(it, end) = cmd_map.equal_range(command); it != end; ++it) {
		std::string ctext = it->second.Context();
		if (ctext == context) return it->second.StrMenu();
		if (ctext == "Default")
			ret = it->second.StrMenu();
		else if (ret.empty() && ctext == "Always")
			ret = it->second.StrMenu();
	}
	return ret;
}

void Hotkey::Flush() {
	json::Object root;

	for (auto const& combo : str_map | boost::adaptors::map_values) {
		json::Object hotkey;
		if (combo.Get().size()) {
			hotkey["key"] = combo.Get().back();
			json::Array& modifiers = hotkey["modifiers"];
			modifiers.insert(modifiers.end(), combo.Get().begin(), combo.Get().end() - 1);
		}

		json::Object& context = root[combo.Context()];
		json::Array& combo_array = context[combo.CmdName()];
		combo_array.push_back(std::move(hotkey));
	}

	io::Save file(config_file);
	JsonWriter::Write(root, file.Get());
}

void Hotkey::SetHotkeyMap(HotkeyMap const& new_map) {
	cmd_map = new_map;

	str_map.clear();
	for (auto const& combo : cmd_map | boost::adaptors::map_values)
		str_map.insert(make_pair(combo.Str(), combo));

	Flush();
	HotkeysChanged();
}

	} // namespace toolbar
} // namespace agi
