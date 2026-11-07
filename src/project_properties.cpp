// Copyright (c) 2026, Aegisub contributors
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

#include "project_properties.h"
#include "text_file_reader.h"

#include <libaegisub/io.h>
#include <libaegisub/util.h>
#include <boost/algorithm/string/trim.hpp>

#include <ostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace {
using Field = std::variant<std::string ProjectProperties::*, int ProjectProperties::*, double ProjectProperties::*>;
const std::vector<std::pair<std::string, Field>> fields = {
	{"Automation Scripts", &ProjectProperties::automation_scripts},
	{"Export Filters", &ProjectProperties::export_filters},
	{"Export Encoding", &ProjectProperties::export_encoding},
	{"Last Style Storage", &ProjectProperties::style_storage},
	{"Audio URI", &ProjectProperties::audio_file},
	{"Audio File", &ProjectProperties::audio_file},
	{"Video File", &ProjectProperties::video_file},
	{"Timecodes File", &ProjectProperties::timecodes_file},
	{"Keyframes File", &ProjectProperties::keyframes_file},
	{"Video Zoom Percent", &ProjectProperties::video_zoom},
	{"Scroll Position", &ProjectProperties::scroll_position},
	{"Active Line", &ProjectProperties::active_row},
	{"Video Position", &ProjectProperties::video_position},
	{"Video AR Mode", &ProjectProperties::ar_mode},
	{"Video AR Value", &ProjectProperties::ar_value},
	{"Aegisub Video Zoom Percent", &ProjectProperties::video_zoom},
	{"Aegisub Scroll Position", &ProjectProperties::scroll_position},
	{"Aegisub Active Line", &ProjectProperties::active_row},
	{"Aegisub Video Position", &ProjectProperties::video_position}
};

constexpr std::string_view pattern_suffix = " Pattern";

bool IsFile(std::string const& key) {
	return key == "Video File" || key == "Audio File" || key == "Keyframes File" || key == "Timecodes File";
}

bool IsAlias(std::string const& key) {
	return key == "Audio URI" || key.starts_with("Aegisub ");
}

bool IsUI(std::string const& key) {
	return key == "Video Zoom Percent" || key == "Scroll Position" || key == "Active Line" || key == "Video Position";
}

bool Split(std::string const& line, std::string &key, std::string &value) {
	if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[')
		return false;

	auto colon = line.find(':');
	if (colon == std::string::npos)
		return false;

	key = boost::trim_copy(line.substr(0, colon));
	value = boost::trim_copy(line.substr(colon + 1));
	return true;
}

void Apply(ProjectProperties &properties, agi::fs::path const& filename, agi::fs::path const& subtitle_directory) {
	// Each file replaces the entire list of candidates for a given medium.
	std::map<std::string, std::vector<std::pair<std::string, bool>>> candidates;
	TextFileReader file(filename, "utf-8", false);

	while (file.HasMoreLines()) {
		auto line = file.ReadLineFromFile();
		std::string key, value;
		if (!Split(line, key, value))
			continue;

		if (key == "Audio URI")
			key = "Audio File";

		bool pattern = key.ends_with(pattern_suffix);
		auto file_key = pattern ? key.substr(0, key.size() - pattern_suffix.size()) : key;
		if (IsFile(file_key)) {
			if (!pattern)
				candidates[file_key].clear();

			candidates[file_key].emplace_back(value, pattern);
			continue;
		}

		properties.ParseProperty(key, value);
	}

	for (auto const& [key, values] : candidates) {
		std::string selected;
		bool inferred = false;

		for (auto const& [value, pattern] : values) {
			selected = value;
			if (pattern) {
				auto matches = agi::fs::Glob((subtitle_directory / agi::fs::path(value)).lexically_normal());
				selected = matches.empty() ? "" : matches.front().generic_string();
			}

			inferred = pattern;
			if (!pattern || !selected.empty())
				break;
		}

		properties.ParseProperty(key, selected);
		properties.inferred_files.erase(key);
		if (inferred)
			properties.inferred_files.insert(key);
	}
}
}

bool ProjectProperties::ParseProperty(std::string const& key, std::string const& value) {
	for (auto const& [name, field] : fields) {
		if (key != name)
			continue;

		std::visit([&](auto member) {
			using T = std::decay_t<decltype(this->*member)>;
			if constexpr (std::is_same_v<T, std::string>)
				this->*member = value;
			else
				agi::util::try_parse(value, &(this->*member));
		}, field);

		if (key == "Audio File" || key == "Audio URI")
			audio_from_video = value == "?video";

		return true;
	}

	if (key.starts_with("Automation Settings ")) {
		automation_settings[key.substr(std::string("Automation Settings ").size())] = value;
		return true;
	}

	return false;
}

std::vector<std::string> ProjectProperties::Serialize(bool save_ui, bool legacy) const {
	std::vector<std::string> lines;

	for (auto const& [key, field] : fields) {
		if (IsAlias(key) || (!save_ui && IsUI(key)))
			continue;

		if (!legacy && inferred_files.count(key))
			continue;

		std::visit([&](auto member) {
			auto value = this->*member;
			if constexpr (std::is_same_v<decltype(value), std::string>) {
				if (key == "Audio File" && audio_from_video)
					value = "?video";

				if (!legacy || !value.empty())
					lines.push_back(key + ": " + value);
			}
			else if (!legacy || value != decltype(value){})
				lines.push_back(key + ": " + std::to_string(value));
		}, field);
	}

	for (auto const& [key, value] : automation_settings)
		lines.push_back("Automation Settings " + key + ": " + value);

	return lines;
}

agi::fs::path ProjectProperties::ConfigFilename(agi::fs::path const& subtitle) {
	return subtitle.parent_path() / (".aegiconf-" + subtitle.filename().string());
}

void ProjectProperties::LoadConfig(agi::fs::path const& subtitle) {
	std::vector<agi::fs::path> directories;
	auto subtitle_directory = agi::fs::Absolute(subtitle).parent_path().lexically_normal();
	auto directory = agi::fs::Canonicalize(subtitle_directory);

	while (true) {
		directories.push_back(directory);

		auto parent = directory.parent_path();
		if (agi::fs::Exists(directory / ".git") || parent == directory || agi::fs::IsMountPoint(directory))
			break;

		directory = parent;
	}

	std::vector<agi::fs::path> configs;
	for (auto it = directories.rbegin(); it != directories.rend(); ++it) {
		auto config = *it / ".aegiconf";
		if (agi::fs::FileExists(config))
			configs.push_back(config);
	}

	auto config = ConfigFilename(agi::fs::Absolute(subtitle));
	if (agi::fs::FileExists(config))
		configs.push_back(config);

	if (configs.empty())
		return;

	// A config cascade replaces all legacy project settings, even if it is empty.
	ProjectProperties properties;
	for (auto const& filename : configs)
		Apply(properties, filename, subtitle_directory);

	*this = std::move(properties);
	audio_from_video = audio_file == "?video";
}
