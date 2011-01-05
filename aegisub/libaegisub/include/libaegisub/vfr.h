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

/// @file vfr.h
/// @brief Framerate handling of all sorts
/// @ingroup libaegisub video_input

#pragma once

#if !defined(AGI_PRE) && !defined(LAGI_PRE)
#include <string>
#include <vector>
#endif

#include <libaegisub/exception.h>

namespace agi {
	/// Framerate handling.
	namespace vfr {

enum Time {
	/// Use the actual frame times
	/// With 1 FPS video, frame 0 is [0, 999] ms
	EXACT,
	/// Calculate based on the rules for start times of lines
	/// Lines are first visible on the first frame with start time less than
	/// or equal to the start time; thus with 1.0 FPS video, frame 0 is
	/// [-999, 0] ms
	START,
	/// Calculate based on the rules for end times of lines
	/// Lines are last visible on the last frame with start time less than the
	/// end time; thus with 1.0 FPS video, frame 0 is [1, 1000] ms
	END
};

DEFINE_BASE_EXCEPTION_NOINNER(Error, Exception)
/// FPS specified is not a valid frame rate
DEFINE_SIMPLE_EXCEPTION_NOINNER(BadFPS, Error, "vfr/badfps")
/// Unknown timecode file format
DEFINE_SIMPLE_EXCEPTION_NOINNER(UnknownFormat, Error, "vfr/timecodes/unknownformat")
/// Invalid line encountered in a timecode file
DEFINE_SIMPLE_EXCEPTION_NOINNER(MalformedLine, Error, "vfr/timecodes/malformed")
/// Timecode file or vector has too few timecodes to be usable
DEFINE_SIMPLE_EXCEPTION_NOINNER(TooFewTimecodes, Error, "vfr/timecodes/toofew")
/// Timecode file or vector has timecodes that are not monotonically increasing
DEFINE_SIMPLE_EXCEPTION_NOINNER(UnorderedTimecodes, Error, "vfr/timecodes/order")

/// @class Framerate
/// @brief Class for managing everything related to converting frames to times
///        or vice versa
class Framerate {
	/// Average FPS for v2, assumed FPS for v1, fps for CFR
	double fps;
	/// Unrounded time of the last frame in a v1 override range. Needed to
	/// match mkvmerge's rounding
	double last;
	/// Start time in milliseconds of each frame
	std::vector<int> timecodes;
public:
	/// Copy constructor
	Framerate(Framerate const&);
	/// @brief VFR from timecodes file
	/// @param filename File with v1 or v2 timecodes
	///
	/// Note that loading a v1 timecode file with Assume X and no overrides is
	/// not the same thing as CFR X. When timecodes are loaded from a file,
	/// mkvmerge-style rounding is applied, while setting a constant frame rate
	/// uses truncation.
	Framerate(std::string const& filename);
	/// @brief CFR constructor
	/// @param fps Frames per second or 0 for unloaded
	Framerate(double fps = 0.);
	/// @brief VFR from frame times
	/// @param timecodes Vector of frame start times in milliseconds
	Framerate(std::vector<int> const& timecodes);
	~Framerate();
	/// Atomic assignment operator
	Framerate &operator=(Framerate);
	/// Atomic CFR assignment operator
	Framerate &operator=(double);
	/// Helper function for the std::swap specialization
	void swap(Framerate &right) throw();

	/// @brief Get the frame visible at a given time
	/// @param ms Time in milliseconds
	/// @param type Time mode
	///
	/// When type is EXACT, the frame returned is the frame visible at the given
	/// time; when it is START or END it is the frame on which a line with that
	/// start/end time would first/last be visible
	int FrameAtTime(int ms, Time type = EXACT) const;

	/// @brief Get the time at a given frame
	/// @param frame Frame number
	/// @param type Time mode
	///
	/// When type is EXACT, the frame's exact start time is returned; START and
	/// END give a time somewhere within the range that will result in the line
	/// starting/ending on that frame
	///
	/// With v2 timecodes, frames outside the defined range are not an error
	/// and are guaranteed to be monotonically increasing/decreasing values
	/// which when passed to FrameAtTime will return the original frame; they
	/// are not guaranteed to be sensible or useful for any other purpose
	///
	/// v1 timecodes and CFR do not have a defined range, and will give sensible
	/// results for all frame numbers
	int TimeAtFrame(int frame, Time type = EXACT) const;

	/// @brief Save the current time codes to a file as v2 timecodes
	/// @param file File name
	/// @param length Minimum number of frames to output
	///
	/// The length parameter is only particularly useful for v1 timecodes (and
	/// CFR, but saving CFR timecodes is a bit silly). Extra timecodes generated
	/// to hit length with v2 timecodes will monotonically increase but may not
	/// be otherwise sensible.
	void Save(std::string const& file, int length = -1) const;

	bool IsVFR() const {return !timecodes.empty(); }
	bool IsLoaded() const { return !timecodes.empty() || fps; };
	double FPS() const { return fps; }
};

	} // namespace vfr
} // namespace agi
