// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

/// @file ass_time.h
/// @see ass_time.cpp
/// @ingroup subs_storage
///

#pragma once

#include <string>

#include <libaegisub/vfr.h>

class AssTime {
	/// Time in milliseconds
	int time;

public:
	AssTime(int ms = 0);
	AssTime(std::string const& text);

	/// Get millisecond, rounded to centisecond precision
	operator int() const { return time / 10 * 10; }

	int GetTimeHours() const;        ///< Get the hours portion of this time
	int GetTimeMinutes() const;      ///< Get the minutes portion of this time
	int GetTimeSeconds() const;      ///< Get the seconds portion of this time
	int GetTimeMiliseconds() const;  ///< Get the miliseconds portion of this time
	int GetTimeCentiseconds() const; ///< Get the centiseconds portion of this time

	/// Return the time as a string
	/// @param ms Use milliseconds precision, for non-ASS formats
	std::string GetAssFormated(bool ms=false) const;
};

/// @class SmpteFormatter
/// @brief Convert times to and from SMPTE timecodes
class SmpteFormatter {
	/// Frame rate to use
	agi::vfr::Framerate fps;
	/// Separator character
	std::string sep;

public:
	SmpteFormatter(agi::vfr::Framerate fps, std::string sep=":");

	/// Convert an AssTime to a SMPTE timecode
	std::string ToSMPTE(AssTime time) const;
	/// Convert a SMPTE timecode to an AssTime
	AssTime FromSMPTE(std::string const& str) const;
};
