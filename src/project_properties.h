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

#pragma once

#include <map>
#include <string>

struct ProjectProperties {
	std::string automation_scripts;
	std::string export_filters;
	std::string export_encoding;
	std::string style_storage;
	std::string audio_file;
	std::string video_file;
	std::string timecodes_file;
	std::string keyframes_file;
	std::map<std::string, std::string> automation_settings;

	// UI State
	double video_zoom = 0.;
	double ar_value = 0.;
	int scroll_position = 0;
	int active_row = 0;
	int ar_mode = 0;
	int video_position = 0;
};
