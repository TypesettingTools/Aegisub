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

#include <libaegisub/util.h>

#include <type_traits>
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

bool IsAlias(std::string const& key) {
	return key == "Audio URI" || key.starts_with("Aegisub ");
}

bool IsUI(std::string const& key) {
	return key == "Video Zoom Percent" || key == "Scroll Position" || key == "Active Line" || key == "Video Position";
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

std::vector<std::string> ProjectProperties::Serialize(bool save_ui) const {
	std::vector<std::string> lines;

	for (auto const& [key, field] : fields) {
		if (IsAlias(key) || (!save_ui && IsUI(key)))
			continue;

		std::visit([&](auto member) {
			auto value = this->*member;
			if constexpr (std::is_same_v<decltype(value), std::string>) {
				if (key == "Audio File" && audio_from_video)
					value = "?video";

				if (!value.empty())
					lines.push_back(key + ": " + value);
			}
			else if (value != decltype(value){})
				lines.push_back(key + ": " + std::to_string(value));
		}, field);
	}

	for (auto const& [key, value] : automation_settings)
		lines.push_back("Automation Settings " + key + ": " + value);

	return lines;
}
