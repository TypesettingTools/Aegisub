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
#include "libaegisub/fs.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"

#include <algorithm>
#include <boost/range/algorithm/equal_range.hpp>
#include <tuple>
#include <utility>

namespace agi::hotkey {
namespace {
struct combo_cmp {
	bool operator()(const Combo *a, const Combo *b) {
		return a->Str() < b->Str();
	}

	bool operator()(const Combo *a, std::string_view b) {
		return a->Str() < b;
	}

	bool operator()(std::string_view a, const Combo *b) {
		return a < b->Str();
	}
};

struct hotkey_visitor : json::ConstVisitor {
	std::string_view context;
	std::string_view command;
	Hotkey::HotkeyMap& map;
	bool needs_backup = false;

	hotkey_visitor(std::string_view context, std::string_view command, Hotkey::HotkeyMap& map)
	: context(context), command(command), map(map) { }

	void Visit(std::string const& string) override {
		map.insert(make_pair(command, Combo(context, command, string)));
	}

	void Visit(json::Object const& hotkey) override {
		auto mod_it = hotkey.find("modifiers");
		if (mod_it == end(hotkey)) {
			LOG_E("agi/hotkey/load") << "Hotkey for command '" << command << "' is missing modifiers";
			return;
		}
		auto key_it = hotkey.find("key");
		if (key_it == end(hotkey)) {
			LOG_E("agi/hotkey/load") << "Hotkey for command '" << command << "' is missing the key";
			return;
		}

		std::string key_str;
		json::Array const& arr_mod = mod_it->second;
		for (std::string const& mod : arr_mod) {
			key_str += mod;
			key_str += '-';
		}
		key_str += static_cast<std::string const&>(key_it->second);

		map.insert(make_pair(command, Combo(context, command, std::move(key_str))));
		needs_backup = true;
	}

	void Visit(const json::Array&) override { }
	void Visit(int64_t) override { }
	void Visit(double) override { }
	void Visit(bool) override { }
	void Visit(const json::Null&) override { }
};
}

Hotkey::Hotkey(fs::path const& file, std::string_view default_config)
: config_file(file)
{
	LOG_D("hotkey/init") << "Generating hotkeys.";

	auto root = json_util::file(config_file, default_config);
	for (auto const& hotkey_context : static_cast<json::Object const&>(root))
		BuildHotkey(hotkey_context.first, hotkey_context.second);
	UpdateStrMap();
}

void Hotkey::BuildHotkey(std::string_view context, json::Object const& hotkeys) {
	for (auto const& command : hotkeys) {
		json::Array const& command_hotkeys = command.second;

		hotkey_visitor visitor{context, command.first, cmd_map};
		for (auto const& hotkey : command_hotkeys)
			hotkey.Accept(visitor);
		backup_config_file |= visitor.needs_backup;
	}
}

std::string_view Hotkey::Scan(std::string_view context, std::string_view str, bool always) const {
	std::string_view local, dfault;

	for (auto [index, end] = boost::equal_range(str_map, str, combo_cmp()); index != end; ++index) {
		std::string_view ctext = (*index)->Context();

		if (always && ctext == "Always") {
			LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Always   Command: " << (*index)->CmdName();
			return (*index)->CmdName();
		}
		if (ctext == "Default")
			dfault = (*index)->CmdName();
		else if (ctext == context)
			local = (*index)->CmdName();
	}

	if (local.data()) {
		LOG_D("agi/hotkey/found") << "Found: " << str << "  Context: " << context << "  Command: " << local;
		return local;
	}
	if (dfault.data()) {
		LOG_D("agi/hotkey/found") << "Found: " << str << "  Context (req/found): " << context << "/Default   Command: " << dfault;
		return dfault;
	}

	return "";
}

bool Hotkey::HasHotkey(std::string_view context, std::string_view str) const {
	for (auto [index, end] = boost::equal_range(str_map, str, combo_cmp()); index != end; ++index) {
		if (context == (*index)->Context())
			return true;
	}
	return false;
}

std::vector<std::string> Hotkey::GetHotkeys(std::string_view context, std::string_view command) const {
	std::vector<std::string> ret;

	HotkeyMap::const_iterator it, end;
	for (auto [it, end] = cmd_map.equal_range(command); it != end; ++it) {
		std::string_view ctext = it->second.Context();
		if (ctext == "Always" || ctext == "Default" || ctext == context)
			ret.emplace_back(it->second.Str());
	}

	sort(ret.begin(), ret.end());
	ret.erase(unique(ret.begin(), ret.end()), ret.end());

	return ret;
}

std::string_view Hotkey::GetHotkey(std::string_view context, std::string_view command) const {
	std::string_view ret;
	for (auto [it, end] = cmd_map.equal_range(command); it != end; ++it) {
		std::string_view ctext = it->second.Context();
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

	auto get = [](json::Object& obj, std::string_view key) -> json::UnknownElement& {
		return obj.emplace(key, json::UnknownElement()).first->second;
	};

	for (auto const& combo : str_map) {
		json::Object& context = get(root, combo->Context());
		static_cast<json::Array&>(get(context, combo->CmdName())).push_back(combo->Str());
	}

	if (backup_config_file && fs::FileExists(config_file) && !fs::FileExists(config_file.string() + ".3_1"))
		fs::Copy(config_file, config_file.string() + ".3_1");

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

}
