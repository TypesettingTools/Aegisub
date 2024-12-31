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

#pragma once

#include <cstdint>
#include <vector>

#include <libaegisub/exception.h>
#include <libaegisub/fs.h>

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

DEFINE_EXCEPTION(Error, Exception);
/// FPS specified is not a valid frame rate
DEFINE_EXCEPTION(InvalidFramerate, Error);
/// Unknown timecode file format
DEFINE_EXCEPTION(UnknownFormat, Error);
/// Invalid line encountered in a timecode file
DEFINE_EXCEPTION(MalformedLine, Error);

/// @class Framerate
/// @brief Class for managing everything related to converting frames to times
///        or vice versa
class Framerate {
	/// Denominator of the FPS
	///
	/// For v1 VFR, the assumed FPS is used, for v2 the average FPS
	int64_t denominator = 0;
	/// Numerator of the FPS
	///
	/// For v1 VFR, the assumed FPS is used, for v2 the average FPS
	int64_t numerator = 0;

	/// Unrounded frame-seconds of the final frame in timecodes. For CFR and v2,
	/// this is simply frame count * denominator, but for v1 it's the
	/// "unrounded" frame count, since override ranges generally don't exactly
	/// cover timebase-unit ranges of time. This is needed to match mkvmerge's
	/// rounding past the end of the final override range.
	int64_t last = 0;

	/// Start time in milliseconds of each frame
	std::vector<int> timecodes;

	/// Does this frame rate need drop frames and have them enabled?
	bool drop = false;

	/// Set FPS properties from the timecodes vector
	void SetFromTimecodes();
public:
	Framerate(Framerate const&) = default;
	Framerate& operator=(Framerate const&) = default;

#ifndef _MSC_VER
	Framerate(Framerate&&) = default;
	Framerate& operator=(Framerate&&) = default;
#endif

	/// @brief VFR from timecodes file
	/// @param filename File with v1 or v2 timecodes
	///
	/// Note that loading a v1 timecode file with Assume X and no overrides is
	/// not the same thing as CFR X. When timecodes are loaded from a file,
	/// mkvmerge-style rounding is applied, while setting a constant frame rate
	/// uses truncation.
	Framerate(agi::fs::path const& filename);

	/// @brief CFR constructor
	/// @param fps Frames per second or 0 for unloaded
	Framerate(double fps = 0.);

	/// @brief CFR constructor with rational timebase
	/// @param numerator Timebase numerator
	/// @param denominator Timebase denominator
	/// @param drop Enable drop frames if the FPS requires it
	Framerate(int64_t numerator, int64_t denominator, bool drop=true);

	/// @brief VFR from frame times
	/// @param timecodes Vector of frame start times in milliseconds
	Framerate(std::vector<int> timecodes);
	Framerate(std::initializer_list<int> timecodes);

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

	/// @brief Get the components of the SMPTE timecode for the given time
	/// @param[out] h Hours component
	/// @param[out] m Minutes component
	/// @param[out] s Seconds component
	/// @param[out] f Frames component
	///
	/// For NTSC (30000/1001), this generates proper SMPTE timecodes with drop
	/// frames handled. For multiples of NTSC, this multiplies the number of
	/// dropped frames. For other non-integral frame rates, it drops frames in
	/// an undefined manner which results in no more than half a second error
	/// from wall clock time.
	///
	/// For integral frame rates, no frame dropping occurs.
	void SmpteAtTime(int ms, int *h, int *m, int *s, int *f) const;

	/// @brief Get the components of the SMPTE timecode for the given frame
	/// @param[out] h Hours component
	/// @param[out] m Minutes component
	/// @param[out] s Seconds component
	/// @param[out] f Frames component
	///
	/// For NTSC (30000/1001), this generates proper SMPTE timecodes with drop
	/// frames handled. For multiples of NTSC, this multiplies the number of
	/// dropped frames. For other non-integral frame rates, it drops frames in
	/// an undefined manner which results in no more than half a second error
	/// from wall clock time.
	///
	/// For integral frame rates, no frame dropping occurs.
	void SmpteAtFrame(int frame, int *h, int *m, int *s, int *f) const;

	/// @brief Get the frame indicated by the SMPTE timecode components
	/// @param h Hours component
	/// @param m Minutes component
	/// @param s Seconds component
	/// @param f Frames component
	/// @return Frame number
	/// @see SmpteAtFrame
	int FrameAtSmpte(int h, int m, int s, int f) const;

	/// @brief Get the time indicated by the SMPTE timecode components
	/// @param h Hours component
	/// @param m Minutes component
	/// @param s Seconds component
	/// @param f Frames component
	/// @return Time in milliseconds
	/// @see SmpteAtTime
	int TimeAtSmpte(int h, int m, int s, int f) const;

	/// @brief Save the current time codes to a file as v2 timecodes
	/// @param file File name
	/// @param length Minimum number of frames to output
	///
	/// The length parameter is only particularly useful for v1 timecodes (and
	/// CFR, but saving CFR timecodes is a bit silly). Extra timecodes generated
	/// to hit length with v2 timecodes will monotonically increase but may not
	/// be otherwise sensible.
	void Save(agi::fs::path const& file, int length = -1) const;

	/// Is this frame rate possibly variable?
	bool IsVFR() const {return timecodes.size() > 1; }

	/// Does this represent a valid frame rate?
	bool IsLoaded() const { return numerator > 0; }

	/// Get average FPS of this frame rate
	double FPS() const { return double(numerator) / denominator; }

	/// Does this frame rate need drop frames for SMPTE timeish frame numbers?
	bool NeedsDropFrames() const { return drop; }
};

	} // namespace vfr
} // namespace agi
