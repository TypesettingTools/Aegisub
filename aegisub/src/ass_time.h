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
#include <stdint.h>

#include <wx/string.h>
#endif

/// DOCME
/// @class AssTime
/// @brief DOCME
///
/// DOCME
class AssTime {
	/// Time in miliseconds
	int time;

public:
	/// DOCME
	static bool UseMSPrecision;

	AssTime();
	AssTime(int ms);

	int GetTimeHours();
	int GetTimeMinutes();
	int GetTimeSeconds();
	int GetTimeMiliseconds();
	int GetTimeCentiseconds();

	int GetMS() const;  ///< Returns milliseconds
	void SetMS(int ms); ///< Sets values to milliseconds with bounds-checking
	void ParseASS(const wxString text); ///< Sets value to text-form time, in ASS format
	wxString GetASSFormated(bool ms=false) const; ///< Returns the ASS representation of time
};

// Comparison operators
bool operator == (const AssTime &t1, const AssTime &t2);
bool operator != (const AssTime &t1, const AssTime &t2);
bool operator <  (const AssTime &t1, const AssTime &t2);
bool operator >  (const AssTime &t1, const AssTime &t2);
bool operator <= (const AssTime &t1, const AssTime &t2);
bool operator >= (const AssTime &t1, const AssTime &t2);
// Arithmetic operators
AssTime operator + (const AssTime &t1, const AssTime &t2);
AssTime operator - (const AssTime &t1, const AssTime &t2);

/// DOCME
/// @class FractionalTime
/// @brief DOCME
///
/// DOCME
class FractionalTime {
	int time; ///< Time in miliseconds
	int num;  ///< Numerator
	int den;  ///< Denominator
	bool drop; ///< Enable SMPTE dropframe handling
	char sep;  ///< Timecode component separator

	/// How often to drop frames when enabled
	static const int frames_per_period = 17982;

public:
	FractionalTime(int numerator=30, int denominator=1, bool dropframe=false, char sep=':');

	/// Parse a SMPTE timecode, returning an AssTime
	AssTime ToAssTime(wxString fractime);
	/// Parse a SMPTE timecode, returning milliseconds
	int ToMillisecs(wxString fractime);

	/// Convert an AssTime to a SMPTE timecode
	wxString FromAssTime(AssTime time);
	/// Convert milliseconds to a SMPTE timecode
	wxString FromMillisecs(int64_t msec);
};
