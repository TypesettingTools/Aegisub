// Copyright (c) 2026, 伤感咩吖
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/


#pragma once

#include <cstdint>
#include <string>
#include <vector>

class wxInputStream;

namespace lyrics {
struct Line {
	int64_t start_ms = 0;
	int64_t end_ms = 0;
	std::string text;
};

// The same parsing entry points are used by the readers and regression tests.
// Invalid or unsupported timestamps throw std::invalid_argument.
std::vector<Line> ParseLrc(std::vector<std::string> const& input);
// Clock/metric times and Apple Music word timing are supported. Frame/tick
// times and inherited/relative container timing are not supported.
std::vector<Line> ParseTTML(wxInputStream& input);
}
