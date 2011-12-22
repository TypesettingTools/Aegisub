// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file ass_time.h
/// @see ass_time.cpp
/// @ingroup subs_storage
///

#pragma once

#ifndef AGI_PRE
#include <wx/string.h>
#endif

#include <libaegisub/vfr.h>

/// DOCME
/// @class AssTime
/// @brief DOCME
///
/// DOCME
class AssTime {
	/// Time in milliseconds
	int time;

public:
	AssTime(int ms = 0);

	/// Get millisecond, rounded to centisecond precision
	operator int() const { return time / 10 * 10; }

	int GetTimeHours() const;        ///< Get the hours portion of this time
	int GetTimeMinutes() const;      ///< Get the minutes portion of this time
	int GetTimeSeconds() const;      ///< Get the seconds portion of this time
	int GetTimeMiliseconds() const;  ///< Get the miliseconds portion of this time
	int GetTimeCentiseconds() const; ///< Get the centiseconds portion of this time

	/// Parse an ASS time string, leaving the time unchanged if the string is malformed
	void ParseASS(wxString const& text);
	/// Return the time as a string
	/// @param ms Use milliseconds precision, for non-ASS formats
	wxString GetASSFormated(bool ms=false) const;
};

/// DOCME
/// @class FractionalTime
/// @brief DOCME
///
/// DOCME
class FractionalTime {
	agi::vfr::Framerate fps;
	bool drop; ///< Enable SMPTE dropframe handling

	/// How often to drop frames when enabled
	static const int frames_per_period = 17982;

public:
	FractionalTime(agi::vfr::Framerate fps, bool dropframe = false);

	bool IsDrop() const { return drop; }
	agi::vfr::Framerate const& FPS() const { return fps; }

	/// Convert an AssTime to a SMPTE timecode
	wxString ToSMPTE(AssTime time, char sep=':');
};
