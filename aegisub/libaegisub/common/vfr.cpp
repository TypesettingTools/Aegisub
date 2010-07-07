// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file vfr.cpp
/// @brief Framerate handling of all sorts
/// @ingroup libaegisub video_input

#include "libaegisub/vfr.h"

#ifndef LAGI_PRE
#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#endif

#include "libaegisub/charset.h"
#include "libaegisub/io.h"
#include "libaegisub/line_iterator.h"

namespace std {
	template<> void swap(agi::vfr::Framerate &lft, agi::vfr::Framerate &rgt) throw() {
		lft.swap(rgt);
	}
}

namespace agi {
namespace vfr {

static int is_increasing(int prev, int cur) {
	if (prev >= cur) {
		throw UnorderedTimecodes("Timecodes are out of order or too close together");
	}
	return cur;
}

/// @brief Verify that timecodes monotonically increase
/// @param timecodes List of timecodes to check
static void validate_timecodes(std::vector<int> const& timecodes) {
	if (timecodes.size() <= 1) {
		throw TooFewTimecodes("Must have at least two timecodes to do anything useful");
	}
	std::accumulate(timecodes.begin()+1, timecodes.end(), timecodes.front(), is_increasing);
}

// A "start,end,fps" line in a v1 timecode file
struct TimecodeRange {
	int start;
	int end;
	double fps;
	double time;
	bool operator<(TimecodeRange cmp) {
		return start < cmp.start;
	}
	TimecodeRange() : fps(0.) { }
};

/// @brief Parse a single line of a v1 timecode file
/// @param str Line to parse
/// @return The line in TimecodeRange form, or TimecodeRange() if it's a comment
static TimecodeRange v1_parse_line(std::string const& str) {
	if (str.empty() || str[0] == '#') return TimecodeRange();

	std::istringstream ss(str);
	TimecodeRange range;
	char comma1, comma2;
	ss >> range.start >> comma1 >> range.end >> comma2 >> range.fps;
	if (ss.fail() || comma1 != ',' || comma2 != ',' || !ss.eof()) {
		throw MalformedLine(str);
	}
	if (range.start < 0 || range.end < 0) {
		throw UnorderedTimecodes("Cannot specify frame rate for negative frames.");
	}
	if (range.end < range.start) {
		throw UnorderedTimecodes("End frame must be greater than or equal to start frame");
	}
	if (range.fps <= 0.) {
		throw BadFPS("FPS must be greater than zero");
	}
	if (range.fps > 1000.) {
		// This is our limitation, not mkvmerge's
		// mkvmerge uses nanoseconds internally
		throw BadFPS("FPS must be at most 1000");
	}
	return range;
}

/// @brief Is the timecode range a comment line?
static bool v1_invalid_timecode(TimecodeRange const& range) {
	return range.fps == 0.;
}

/// @brief Generate override ranges for all frames with assumed fpses
/// @param ranges List with ranges which is mutated
/// @param fps    Assumed fps to use for gaps
static void v1_fill_range_gaps(std::list<TimecodeRange> &ranges, double fps) {
	// Range for frames between start and first override
	if (ranges.empty() || ranges.front().start > 0) {
		TimecodeRange range;
		range.fps = fps;
		range.start = 0;
		range.end = ranges.empty() ? 0 : ranges.front().start - 1;
		ranges.push_front(range);
	}
	std::list<TimecodeRange>::iterator cur = ++ranges.begin();
	std::list<TimecodeRange>::iterator prev = ranges.begin();
	for (; cur != ranges.end(); ++cur, ++prev) {
		if (prev->end >= cur->start) {
			// mkvmerge allows overlapping timecode ranges, but does completely
			// broken things with them
			throw UnorderedTimecodes("Override ranges must not overlap");
		}
		if (prev->end + 1 < cur->start) {
			TimecodeRange range;
			range.fps = fps;
			range.start = prev->end + 1;
			range.end = cur->start - 1;
			ranges.insert(cur, range);
			++prev;
		}
	}
}

/// @brief Parse a v1 timecode file
/// @param      file      Iterator of lines in the file
/// @param      line      Header of file with assumed fps
/// @param[out] timecodes Vector filled with frame start times
/// @param[out] time      Unrounded time of the last frame
/// @return Assumed fps
static double v1_parse(line_iterator<std::string> file, std::string line, std::vector<int> &timecodes, double &time) {
	using namespace std;
	double fps = atof(line.substr(7).c_str());
	if (fps <= 0.) throw BadFPS("Assumed FPS must be greater than zero");
	if (fps > 1000.) throw BadFPS("Assumed FPS must not be greater than 1000");

	list<TimecodeRange> ranges;
	transform(file, line_iterator<string>(), back_inserter(ranges), v1_parse_line);
	ranges.erase(remove_if(ranges.begin(), ranges.end(), v1_invalid_timecode), ranges.end());

	ranges.sort();
	v1_fill_range_gaps(ranges, fps);
	timecodes.reserve(ranges.back().end);

	time = 0.;
	for (list<TimecodeRange>::iterator cur = ranges.begin(); cur != ranges.end(); ++cur) {
		for (int frame = cur->start; frame <= cur->end; frame++) {
			timecodes.push_back(int(time + .5));
			time += 1000. / cur->fps;
		}
	}
	timecodes.push_back(int(time + .5));
	return fps;
}

Framerate::Framerate(Framerate const& that)
: fps(that.fps)
, last(that.last)
, timecodes(that.timecodes)
{
}

Framerate::Framerate(double fps) : fps(fps), last(0.) {
	if (fps < 0.) throw BadFPS("FPS must be greater than zero");
	if (fps > 1000.) throw BadFPS("FPS must not be greater than 1000");
}

Framerate::Framerate(std::vector<int> const& timecodes)
: timecodes(timecodes)
{
	validate_timecodes(timecodes);
	fps = timecodes.size() / (timecodes.back() / 1000.);
	last = timecodes.back();
}

Framerate::~Framerate() {
}

void Framerate::swap(Framerate &right) throw() {
	std::swap(fps, right.fps);
	std::swap(last, right.last);
	std::swap(timecodes, right.timecodes);
}

Framerate &Framerate::operator=(Framerate right) {
	std::swap(*this, right);
	return *this;
}
Framerate &Framerate::operator=(double fps) {
	return *this = Framerate(fps);
}

bool Framerate::operator==(Framerate const& right) const {
	return fps == right.fps && timecodes == right.timecodes;
}

Framerate::Framerate(std::string const& filename) : fps(0.) {
	using namespace std;
	auto_ptr<ifstream> file(agi::io::Open(filename));
	string encoding = agi::charset::Detect(filename);
	string line = *line_iterator<string>(*file, encoding);
	if (line == "# timecode format v2") {
		copy(line_iterator<int>(*file, encoding), line_iterator<int>(), back_inserter(timecodes));
		validate_timecodes(timecodes);
		fps = timecodes.size() / (timecodes.back() / 1000.);
		return;
	}
	if (line == "# timecode format v1" || line.substr(0, 7) == "Assume ") {
		if (line[0] == '#') {
			line = *line_iterator<string>(*file, encoding);
		}
		fps = v1_parse(line_iterator<string>(*file, encoding), line, timecodes, last);
		return;
	}

	throw UnknownFormat(line);
}

void Framerate::Save(std::string const& filename, int length) const {
	agi::io::Save file(filename);
	std::ofstream &out = file.Get();

	out << "# timecode format v2\n";
	std::copy(timecodes.begin(), timecodes.end(), std::ostream_iterator<int>(out, "\n"));
	for (int written = timecodes.size(); written < length; ++written) {
		out << TimeAtFrame(written) << std::endl;
	}
}

static int round(double value) {
	return int(value + .5);
}

int Framerate::FrameAtTime(int ms, Time type) const {
	// With X ms per frame, this should return 0 for:
	// EXACT: [0, X]
	// START: [-X, 0]
	// END:   [1, X + 1]

	// There are two properties we take advantage of here:
	//  1. START and END's ranges are adjacent, meaning doing the calculations
	//     for END and adding one gives us START
	//  2. END is EXACT plus one ms, meaning we can subtract one ms to get EXACT

	// Combining these allows us to easily calculate START and END in terms of
	// EXACT

	if (type == START) {
		return FrameAtTime(ms - 1) + 1;
	}
	if (type == END) {
		return FrameAtTime(ms - 1);
	}

	if (timecodes.empty()) {
		return (int)floor(ms * fps / 1000.);
	}
	if (ms < timecodes.front()) {
		return (int)floor((ms - timecodes.front()) * fps / 1000.);
	}
	if (ms > timecodes.back()) {
		return round((ms - timecodes.back()) * fps / 1000.) + timecodes.size() - 1;
	}

	return std::distance(std::lower_bound(timecodes.rbegin(), timecodes.rend(), ms, std::greater<int>()), timecodes.rend()) - 1;
}

int Framerate::TimeAtFrame(int frame, Time type) const {
	if (type == START) {
		int prev = TimeAtFrame(frame - 1);
		int cur = TimeAtFrame(frame);
		// + 1 as these need to round up for the case of two frames 1 ms apart
		return prev + (cur - prev + 1) / 2;
	}
	if (type == END) {
		int cur = TimeAtFrame(frame);
		int next = TimeAtFrame(frame + 1);
		return cur + (next - cur + 1) / 2;
	}

	if (timecodes.empty()) {
		return (int)ceil(frame / fps * 1000.);
	}

	if (frame < 0) {
		return (int)ceil(frame / fps * 1000.) + timecodes.front();
	}
	if (frame >= (signed)timecodes.size()) {
		return round((frame - timecodes.size() + 1) * 1000. / fps + last);
	}
	return timecodes[frame];
}

}
}
