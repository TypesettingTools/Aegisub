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

/// @file vfr.cpp
/// @brief Framerate handling of all sorts
/// @ingroup libaegisub video_input

#include "../config.h"

#include "libaegisub/vfr.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iterator>
#include <list>
#include <numeric>

#include "libaegisub/charset.h"
#include "libaegisub/io.h"
#include "libaegisub/line_iterator.h"
#include "libaegisub/scoped_ptr.h"

#include <boost/range/algorithm.hpp>

namespace {

static const int64_t default_denominator = 1000000000;
using agi::line_iterator;
using namespace agi::vfr;

/// @brief Verify that timecodes monotonically increase
/// @param timecodes List of timecodes to check
void validate_timecodes(std::vector<int> const& timecodes) {
	if (timecodes.size() <= 1)
		throw TooFewTimecodes("Must have at least two timecodes to do anything useful");
	if (!is_sorted(timecodes.begin(), timecodes.end()))
		throw UnorderedTimecodes("Timecodes are out of order");
}

/// @brief Shift timecodes so that frame 0 starts at time 0
/// @param timecodes List of timecodes to normalize
void normalize_timecodes(std::vector<int> &timecodes) {
	if (int front = timecodes.front())
		boost::for_each(timecodes, [=](int &tc) { tc -= front; });
}

// A "start,end,fps" line in a v1 timecode file
struct TimecodeRange {
	int start;
	int end;
	double fps;
	bool operator<(TimecodeRange const& cmp) const { return start < cmp.start; }
	TimecodeRange(int start=0, int end=0, double fps=0.)
	: start(start), end(end), fps(fps) { }
};

/// @brief Parse a single line of a v1 timecode file
/// @param str Line to parse
/// @return The line in TimecodeRange form, or TimecodeRange() if it's a comment
TimecodeRange v1_parse_line(std::string const& str) {
	if (str.empty() || str[0] == '#') return TimecodeRange();

	std::istringstream ss(str);
	TimecodeRange range;
	char comma1 = 0, comma2 = 0;
	ss >> range.start >> comma1 >> range.end >> comma2 >> range.fps;
	if (ss.fail() || comma1 != ',' || comma2 != ',' || !ss.eof())
		throw MalformedLine(str);
	if (range.start < 0 || range.end < 0)
		throw UnorderedTimecodes("Cannot specify frame rate for negative frames.");
	if (range.end < range.start)
		throw UnorderedTimecodes("End frame must be greater than or equal to start frame");
	if (range.fps <= 0.)
		throw BadFPS("FPS must be greater than zero");
	if (range.fps > 1000.)
		// This is our limitation, not mkvmerge's
		// mkvmerge uses nanoseconds internally
		throw BadFPS("FPS must be at most 1000");
	return range;
}

/// @brief Generate override ranges for all frames with assumed fpses
/// @param ranges List with ranges which is mutated
/// @param fps    Assumed fps to use for gaps
void v1_fill_range_gaps(std::list<TimecodeRange> &ranges, double fps) {
	// Range for frames between start and first override
	if (ranges.empty() || ranges.front().start > 0)
		ranges.emplace_front(0, ranges.empty() ? 0 : ranges.front().start - 1, fps);

	for (auto cur = ++begin(ranges), prev = begin(ranges); cur != end(ranges); ++cur, ++prev) {
		if (prev->end >= cur->start)
			// mkvmerge allows overlapping timecode ranges, but does completely
			// broken things with them
			throw UnorderedTimecodes("Override ranges must not overlap");
		if (prev->end + 1 < cur->start) {
			ranges.emplace(cur, prev->end + 1, cur->start -1, fps);
			++prev;
		}
	}
}

/// @brief Parse a v1 timecode file
/// @param      file      Iterator of lines in the file
/// @param      line      Header of file with assumed fps
/// @param[out] timecodes Vector filled with frame start times
/// @param[out] last      Unrounded time of the last frame
/// @return Assumed fps times one million
int64_t v1_parse(line_iterator<std::string> file, std::string line, std::vector<int> &timecodes, int64_t &last) {
	double fps = atof(line.substr(7).c_str());
	if (fps <= 0.) throw BadFPS("Assumed FPS must be greater than zero");
	if (fps > 1000.) throw BadFPS("Assumed FPS must not be greater than 1000");

	std::list<TimecodeRange> ranges;
	transform(file, end(file), back_inserter(ranges), v1_parse_line);
	ranges.erase(boost::remove_if(ranges, [](TimecodeRange const& r) { return r.fps == 0; }), ranges.end());

	ranges.sort();
	v1_fill_range_gaps(ranges, fps);
	timecodes.reserve(ranges.back().end);

	double time = 0.;
	for (auto const& range : ranges) {
		for (int frame = range.start; frame <= range.end; ++frame) {
			timecodes.push_back(int(time + .5));
			time += 1000. / range.fps;
		}
	}
	timecodes.push_back(int(time + .5));
	last = int64_t(time * fps * default_denominator);
	return int64_t(fps * default_denominator);
}

}

namespace agi {
namespace vfr {

Framerate::Framerate(double fps)
: denominator(default_denominator)
, numerator(int64_t(fps * denominator))
, last(0)
, drop(false)
{
	if (fps < 0.) throw BadFPS("FPS must be greater than zero");
	if (fps > 1000.) throw BadFPS("FPS must not be greater than 1000");
	timecodes.push_back(0);
}

Framerate::Framerate(int64_t numerator, int64_t denominator, bool drop)
: denominator(denominator)
, numerator(numerator)
, last(0)
, drop(drop && numerator % denominator != 0)
{
	if (numerator <= 0 || denominator <= 0)
		throw BadFPS("Numerator and denominator must both be greater than zero");
	if (numerator / denominator > 1000) throw BadFPS("FPS must not be greater than 1000");
	timecodes.push_back(0);
}

void Framerate::SetFromTimecodes() {
	validate_timecodes(timecodes);
	normalize_timecodes(timecodes);
	denominator = default_denominator;
	numerator = (timecodes.size() - 1) * denominator * 1000 / timecodes.back();
	last = (timecodes.size() - 1) * denominator * 1000;
}

Framerate::Framerate(std::vector<int> timecodes)
: timecodes(std::move(timecodes))
, drop(false)
{
	SetFromTimecodes();
}

Framerate::Framerate(std::initializer_list<int> timecodes)
: timecodes(timecodes)
, drop(false)
{
	SetFromTimecodes();
}

void Framerate::swap(Framerate &right) throw() {
	using std::swap;
	swap(numerator, right.numerator);
	swap(denominator, right.denominator);
	swap(last, right.last);
	swap(timecodes, right.timecodes);
}

Framerate::Framerate(fs::path const& filename)
: denominator(default_denominator)
, numerator(0)
, drop(false)
{
	auto file = agi::io::Open(filename);
	auto encoding = agi::charset::Detect(filename);
	auto line = *line_iterator<std::string>(*file, encoding);
	if (line == "# timecode format v2") {
		copy(line_iterator<int>(*file, encoding), line_iterator<int>(), back_inserter(timecodes));
		SetFromTimecodes();
		return;
	}
	if (line == "# timecode format v1" || line.substr(0, 7) == "Assume ") {
		if (line[0] == '#')
			line = *line_iterator<std::string>(*file, encoding);
		numerator = v1_parse(line_iterator<std::string>(*file, encoding), line, timecodes, last);
		return;
	}

	throw UnknownFormat(line);
}

void Framerate::Save(fs::path const& filename, int length) const {
	agi::io::Save file(filename);
	auto &out = file.Get();

	out << "# timecode format v2\n";
	boost::copy(timecodes, std::ostream_iterator<int>(out, "\n"));
	for (int written = (int)timecodes.size(); written < length; ++written)
		out << TimeAtFrame(written) << std::endl;
}

int Framerate::FrameAtTime(int ms, Time type) const {
	// With X ms per frame, this should return 0 for:
	// EXACT: [0, X - 1]
	// START: [1 - X , 0]
	// END:   [1, X]

	// There are two properties we take advantage of here:
	//  1. START and END's ranges are adjacent, meaning doing the calculations
	//     for END and adding one gives us START
	//  2. END is EXACT plus one ms, meaning we can subtract one ms to get EXACT

	// Combining these allows us to easily calculate START and END in terms of
	// EXACT

	if (type == START)
		return FrameAtTime(ms - 1) + 1;
	if (type == END)
		return FrameAtTime(ms - 1);

	if (ms < 0)
		return int((ms * numerator / denominator - 999) / 1000);

	if (ms > timecodes.back())
		return int((ms * numerator - last + denominator - 1) / denominator / 1000) + (int)timecodes.size() - 1;

	return (int)distance(lower_bound(timecodes.rbegin(), timecodes.rend(), ms, std::greater<int>()), timecodes.rend()) - 1;
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

	if (frame < 0)
		return (int)(frame * denominator * 1000 / numerator);

	if (frame >= (signed)timecodes.size()) {
		int64_t frames_past_end = frame - (int)timecodes.size() + 1;
		return int((frames_past_end * 1000 * denominator + last + numerator / 2) / numerator);
	}

	return timecodes[frame];
}

void Framerate::SmpteAtFrame(int frame, int *h, int *m, int *s, int *f) const {
	frame = std::max(frame, 0);
	int ifps = (int)ceil(FPS());

	if (drop && denominator == 1001 && numerator % 30000 == 0) {
		// NTSC skips the first two frames of every minute except for multiples
		// of ten. For multiples of NTSC, simply multiplying the number of
		// frames skips seems like the most sensible option.
		const int drop_factor = int(numerator / 30000);
		const int one_minute = 60 * 30 * drop_factor - drop_factor * 2;
		const int ten_minutes = 60 * 10 * 30 * drop_factor - drop_factor * 18;
		const int ten_minute_groups = frame / ten_minutes;
		const int last_ten_minutes  = frame % ten_minutes;

		frame += ten_minute_groups * 18 * drop_factor;
		frame += (last_ten_minutes - 2 * drop_factor) / one_minute * 2 * drop_factor;
	}

	// Non-integral frame rates other than NTSC aren't supported by SMPTE
	// timecodes, but the user has asked for it so just give something that
	// resembles a valid timecode which is no more than half a frame off
	// wallclock time
	else if (drop && ifps != FPS()) {
		frame = int(frame / FPS() * ifps + 0.5);
	}

	*h = frame / (ifps * 60 * 60);
	*m = (frame / (ifps * 60)) % 60;
	*s = (frame / ifps) % 60;
	*f = frame % ifps;
}

void Framerate::SmpteAtTime(int ms, int *h, int *m, int *s, int *f) const {
	SmpteAtFrame(FrameAtTime(ms), h, m, s, f);
}

int Framerate::FrameAtSmpte(int h, int m, int s, int f) const {
	int ifps = (int)ceil(FPS());

	if (drop && denominator == 1001 && numerator % 30000 == 0) {
		const int drop_factor = int(numerator / 30000);
		const int one_minute = 60 * 30 * drop_factor - drop_factor * 2;
		const int ten_minutes = 60 * 10 * 30 * drop_factor - drop_factor * 18;

		const int ten_m = m / 10;
		m = m % 10;

		// The specified frame doesn't actually exist so skip forward to the
		// next frame that does
		if (m != 0 && s == 0 && f < 2 * drop_factor)
			f = 2 * drop_factor;

		return h * ten_minutes * 6 + ten_m * ten_minutes + m * one_minute + s * ifps + f;
	}
	else if (drop && ifps != FPS()) {
		int frame = (h * 60 * 60 + m * 60 + s) * ifps + f;
		return int((double)frame / ifps * FPS() + 0.5);
	}

	return (h * 60 * 60 + m * 60 + s) * ifps + f;
}

int Framerate::TimeAtSmpte(int h, int m, int s, int f) const {
	return TimeAtFrame(FrameAtSmpte(h, m, s, f));
}

}
}
