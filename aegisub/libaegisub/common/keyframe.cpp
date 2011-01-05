// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file keyframe.cpp
/// @see keyframe.h
/// @ingroup libaegisub
///


#include "../config.h"

#ifndef LAGI_PRE
#include <algorithm>
#include <fstream>
#endif

#include "libaegisub/io.h"
#include "libaegisub/line_iterator.h"
#include "libaegisub/keyframe.h"
#include "libaegisub/vfr.h"

static std::pair<std::vector<int>, double> agi_keyframes(std::istream &file) {
	double fps;
	std::string fps_str;
	file >> fps_str;
	file >> fps;

	if (!file.good() || fps_str != "fps")
		throw agi::keyframe::Error("FPS not found");

	std::vector<int> ret;
	std::copy(std::istream_iterator<int>(file), std::istream_iterator<int>(), std::back_inserter(ret));
	return make_pair(ret, fps);
}

static std::pair<std::vector<int>, double> other_keyframes(std::istream &file, char (*func)(std::string const&)) {
	int count = 0;
	std::vector<int> ret;
	agi::line_iterator<std::string> end;
	for (agi::line_iterator<std::string> iter(file); iter != end; ++iter) {
		char c = tolower(func(*iter));
		if (c == 'i') {
			ret.push_back(count++);
		}
		else if (c == 'p' || c == 'b') {
			++count;
		}
	}
	return std::make_pair(ret, 0);
}

char xvid(std::string const& line) {
	return line.empty() ? 0 : line[0];
}

char divx(std::string const& line) {
	char chrs[] = "IPB";
	for (int i = 0; i < 3; ++i) {
		std::string::size_type pos = line.find(chrs[i]);
		if (pos != line.npos)
			return line[pos];
	}
	return 0;
}

char x264(std::string const& line) {
	std::string::size_type pos = line.find("type:");
	if (pos == line.npos || pos + 5 >= line.size()) return 0;
	return line[pos + 5];
}

template<int N>
static bool starts_with(std::string const& str, const char (&test)[N]) {
	if (str.size() < N) return false;
	return std::mismatch(str.begin(), str.begin() + N - 1, test).first == str.begin() + N - 1;
}


namespace agi { namespace keyframe {

	void Save(std::string const& filename, std::vector<int> const& keyframes, vfr::Framerate const& fps) {
	io::Save file(filename);
	std::ofstream& of = file.Get();
	of << "# keyframe format v1" << std::endl;
	of << "fps " << fps.FPS() << std::endl;
	std::copy(keyframes.begin(), keyframes.end(), std::ostream_iterator<int>(of, "\n"));
}

std::pair<std::vector<int>, double> Load(std::string const& filename) {
	std::auto_ptr<std::ifstream> file(io::Open(filename));
	std::istream &is(*file.get());

	std::string header;
	std::getline(is, header);

	if (header == "# keyframe format v1") return agi_keyframes(is);
	if (starts_with(header, "# XviD 2pass stat file")) return other_keyframes(is, xvid);
	if (starts_with(header, "##map version")) return other_keyframes(is, divx);
	if (starts_with(header, "#options:")) return other_keyframes(is, x264);

	throw Error("Unknown keyframe format");
}

} }
