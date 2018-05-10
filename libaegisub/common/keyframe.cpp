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

/// @file keyframe.cpp
/// @see keyframe.h
/// @ingroup libaegisub
///

#include "libaegisub/keyframe.h"

#include <sstream>

#include "libaegisub/io.h"
#include "libaegisub/line_iterator.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace {
std::vector<int> agi_keyframes(std::istream &file) {
	double fps;
	std::string fps_str;
	file >> fps_str;
	file >> fps;

	return std::vector<int>(agi::line_iterator<int>(file), agi::line_iterator<int>());
}

std::vector<int> enumerated_keyframes(std::istream &file, char (*func)(std::string const&)) {
	int count = 0;
	std::vector<int> ret;
	for (auto line : agi::line_iterator<std::string>(file)) {
		char c = tolower(func(line));
		if (c == 'i')
			ret.push_back(count++);
		else if (c == 'p' || c == 'b')
			++count;
	}
	return ret;
}

std::vector<int> indexed_keyframes(std::istream &file, int (*func)(std::string const&)) {
	std::vector<int> ret;
	for (auto line : agi::line_iterator<std::string>(file)) {
		int frame_no = func(line);
		if (frame_no >= 0)
			ret.push_back(frame_no);
	}
	return ret;
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

int wwxd(std::string const& line) {
	if (line.empty() || line[0] == '#')
		return -1;
	std::istringstream ss(line);
	int frame_no;
	char frame_type;
	ss >> frame_no >> frame_type;
	if (ss.fail())
		throw agi::keyframe::KeyframeFormatParseError("WWXD keyframe file not in qpfile format");
	if (frame_type == 'I')
		return frame_no;
	return -1;
}
}

namespace agi { namespace keyframe {
void Save(agi::fs::path const& filename, std::vector<int> const& keyframes) {
	io::Save file(filename);
	std::ostream& of = file.Get();
	of << "# keyframe format v1" << std::endl;
	of << "fps " << 0 << std::endl;
	boost::copy(keyframes, std::ostream_iterator<int>(of, "\n"));
}

std::vector<int> Load(agi::fs::path const& filename) {
	auto file = io::Open(filename);
	std::istream &is(*file);

	std::string header;
	getline(is, header);

	if (header == "# keyframe format v1") return agi_keyframes(is);
	if (boost::starts_with(header, "# XviD 2pass stat file")) return enumerated_keyframes(is, xvid);
	if (boost::starts_with(header, "# ffmpeg 2-pass log file, using xvid codec")) return enumerated_keyframes(is, xvid);
	if (boost::starts_with(header, "# avconv 2-pass log file, using xvid codec")) return enumerated_keyframes(is, xvid);
	if (boost::starts_with(header, "##map version")) return enumerated_keyframes(is, divx);
	if (boost::starts_with(header, "#options:")) return enumerated_keyframes(is, x264);
	if (boost::starts_with(header, "# WWXD log file, using qpfile format")) return indexed_keyframes(is, wwxd);

	throw UnknownKeyframeFormatError("File header does not match any known formats");
}

} }
