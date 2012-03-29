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
	AssTime(wxString const& text);

	/// Get millisecond, rounded to centisecond precision
	operator int() const { return time / 10 * 10; }

	int GetTimeHours() const;        ///< Get the hours portion of this time
	int GetTimeMinutes() const;      ///< Get the minutes portion of this time
	int GetTimeSeconds() const;      ///< Get the seconds portion of this time
	int GetTimeMiliseconds() const;  ///< Get the miliseconds portion of this time
	int GetTimeCentiseconds() const; ///< Get the centiseconds portion of this time

	/// Return the time as a string
	/// @param ms Use milliseconds precision, for non-ASS formats
	wxString GetASSFormated(bool ms=false) const;
};

/// @class SmpteFormatter
/// @brief Convert times to and from SMPTE timecodes
class SmpteFormatter {
	/// Frame rate to use
	agi::vfr::Framerate fps;
	/// Separator character
	char sep;

public:
	SmpteFormatter(agi::vfr::Framerate fps, char sep=':');

	/// Convert an AssTime to a SMPTE timecode
	wxString ToSMPTE(AssTime time) const;
	/// Convert a SMPTE timecode to an AssTime
	AssTime FromSMPTE(wxString const& str) const;
};
