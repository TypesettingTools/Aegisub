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

#include "libaegisub/hotkey.h"

#include "libaegisub/cajun/writer.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"
#include "libaegisub/split.h"

#include <algorithm>
#include <boost/range/algorithm/equal_range.hpp>
#include <tuple>

namespace agi { namespace hotkey {
namespace {
struct combo_cmp {
	bool operator()(const Combo *a, const Combo *b) {
		return a->Str() < b->Str();
	}

	bool operator()(const Combo *a, std::string const& b) {
		return a->Str() < b;
	}

	bool operator()(std::string const& a, const Combo *b) {
		return a < b->Str();
	}
};
}

Hotkey::Hotkey(fs::path const& file, std::pair<const char *, size_t> default_config)
: config_file(file)
{
	LOG_D("hotkey/init") << "Generating hotkeys.";

	auto root = json_util::file(config_file, default_config);
	for (auto const& hotkey_context : static_cast<json::Object const&>(root))
		BuildHotkey(hotkey_context.first, hotkey_context.second);
	UpdateStrMap();
}

void Hotkey::BuildHotkey(std::string const& context, json::Object const& hotkeys) {
	for (auto const& command : hotkeys) {
		json::Array const& command_hotkeys = command.second;

		for (json::Object const& hotkey : command_hotkeys) {
			auto mod_it = hotkey.find("modifiers");
			if (mod_it == end(hotkey)) {
				LOG_E("agi/hotkey/load") << "Hotkey for command '" << command.first << "' is missing modifiers";
				continue;
			}
			auto key_it = hotkey.find("key");
			if (key_it == end(hotkey)) {
				LOG_E("agi/hotkey/load") << "Hotkey for command '" << command.first << "' is missing the key";
				continue;
			}

			std::string key_str;
			json::Array const& arr_mod = mod_it->second;
			for (std::string const& mod : arr_mod) {
				key_str += mod;
				key_str += '-';
			}
			key_str += static_cast<std::string const&>(key_it->second);

			cmd_map.insert(make_pair(command.first, Combo(context, command.first, std::move(key_str))));
		}
	}
}

std::string Hotkey::Scan(std::string const& context, std::string const& str, bool always) const {
	const std::string *local = nullptr, *dfault = nullptr;

	std::vector<const Combo *>::const_iterator index, end;
	for (std::tie(index, end) = boost::equal_range(str_map, str, combo_cmp()); index != end; ++index) {
		std::string const& ctext = (*index)->Context();

		if (always && ctext == "Always") {
			LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Always   Command: " << (*index)->CmdName();
			return (*index)->CmdName();
		}
		if (ctext == "Default")
			dfault = &(*index)->CmdName();
		else if (ctext == context)
			local = &(*index)->CmdName();
	}

	if (local) {
		LOG_D("agi/hotkey/found") << "Found: " << str << "  Context: " << context << "  Command: " << *local;
		return *local;
	}
	if (dfault) {
		LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Default   Command: " << *dfault;
		return *dfault;
	}

	return "";
}

bool Hotkey::HasHotkey(std::string const& context, std::string const& str) const {
	std::vector<const Combo *>::const_iterator index, end;
	for (std::tie(index, end) = boost::equal_range(str_map, str, combo_cmp()); index != end; ++index) {
		if (context == (*index)->Context())
			return true;
	}
	return false;
}

std::vector<std::string> Hotkey::GetHotkeys(std::string const& context, std::string const& command) const {
	std::vector<std::string> ret;

	HotkeyMap::const_iterator it, end;
	for (std::tie(it, end) = cmd_map.equal_range(command); it != end; ++it) {
		std::string const& ctext = it->second.Context();
		if (ctext == "Always" || ctext == "Default" || ctext == context)
			ret.emplace_back(it->second.Str());
	}

	sort(ret.begin(), ret.end());
	ret.erase(unique(ret.begin(), ret.end()), ret.end());

	return ret;
}

std::string Hotkey::GetHotkey(std::string const& context, std::string const& command) const {
	std::string ret;
	HotkeyMap::const_iterator it, end;
	for (std::tie(it, end) = cmd_map.equal_range(command); it != end; ++it) {
		std::string const& ctext = it->second.Context();
		if (ctext == context) return it->second.Str();
		if (ctext == "Default")
			ret = it->second.Str();
		else if (ret.empty() && ctext == "Always")
			ret = it->second.Str();
	}
	return ret;
}

void Hotkey::Flush() {
	json::Object root;

	for (auto const& combo : str_map) {
		auto keys = combo->Str();
		if (keys.empty()) continue;

		json::Object hotkey;
		json::Array& modifiers = hotkey["modifiers"];
		for (auto tok : agi::Split(keys, '-')) {
			if (end(tok) == end(keys))
				hotkey.insert(make_pair("key", agi::str(tok)));
			else
				modifiers.push_back(agi::str(tok));
		}

		json::Object& context = root[combo->Context()];
		json::Array& combo_array = context[combo->CmdName()];
		combo_array.push_back(std::move(hotkey));
	}

	io::Save file(config_file);
	JsonWriter::Write(root, file.Get());
}

void Hotkey::UpdateStrMap() {
	str_map.clear();
	str_map.reserve(cmd_map.size());
	for (auto const& combo : cmd_map)
		str_map.push_back(&combo.second);

	sort(begin(str_map), end(str_map), combo_cmp());
}

void Hotkey::SetHotkeyMap(HotkeyMap new_map) {
	cmd_map = std::move(new_map);
	UpdateStrMap();
	Flush();
	HotkeysChanged();
}

} }
