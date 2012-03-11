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

/// @file ass_time.cpp
/// @brief Class for managing timestamps in subtitles
/// @ingroup subs_storage
///

#include "config.h"

#include "ass_time.h"

#ifndef AGI_PRE
#include <algorithm>
#include <cmath>
#endif

#include "utils.h"

AssTime::AssTime(int time) : time(mid(0, time, 10 * 60 * 60 * 1000 - 1)) { }

AssTime::AssTime(wxString const& text)
: time(0)
{
	size_t pos = 0, end = 0;

	int colons = text.Freq(':');
	
	// Set start so that there are only two colons at most
	for (; colons > 2; --colons) pos = text.find(':', pos) + 1;

	// Hours
	if (colons == 2) {
		while (text[end++] != ':') { }
		time += AegiStringToInt(text, pos, end) * 60 * 60 * 1000;
		pos = end;
	}

	// Minutes
	if (colons >= 1) {
		while (text[end++] != ':') { }
		time += AegiStringToInt(text, pos, end) * 60 * 1000;
	}

	// Milliseconds (includes seconds)
	time += AegiStringToFix(text, 3, end, text.size());

	// Limit to the valid range
	time = mid(0, time, 10 * 60 * 60 * 1000 - 1);
}

wxString AssTime::GetASSFormated(bool msPrecision) const {
	wxChar ret[] = {
		'0' + GetTimeHours(),
		':',
		'0' + (time % (60 * 60 * 1000)) / (60 * 1000 * 10),
		'0' + (time % (10 * 60 * 1000)) / (60 * 1000),
		':',
		'0' + (time % (60 * 1000)) / (1000 * 10),
		'0' + (time % (10 * 1000)) / 1000,
		'.',
		'0' + (time % 1000) / 100,
		'0' + (time % 100) / 10,
		'0' + time % 10
	};

	return wxString(ret, msPrecision ? 11 : 10);
}

int AssTime::GetTimeHours() const { return time / 3600000; }
int AssTime::GetTimeMinutes() const { return (time % 3600000) / 60000; }
int AssTime::GetTimeSeconds() const { return (time % 60000) / 1000; }
int AssTime::GetTimeMiliseconds() const { return (time % 1000); }
int AssTime::GetTimeCentiseconds() const { return (time % 1000) / 10; }

FractionalTime::FractionalTime(agi::vfr::Framerate fps, bool dropframe)
: fps(fps)
, drop(dropframe)
{
}

wxString FractionalTime::ToSMPTE(AssTime time, char sep) {
	int h=0, m=0, s=0, f=0; // hours, minutes, seconds, fractions
	int fn = fps.FrameAtTime(time);

	// return 00:00:00:00
	if (time <= 0) {
	}
	// dropframe?
	else if (drop) {
		fn += 2 * (fn / (30 * 60)) - 2 * (fn / (30 * 60 * 10));
		h = fn / (30 * 60 * 60);
		m = (fn / (30 * 60)) % 60;
		s = (fn / 30) % 60;
		f = fn % 30;
	}
	// no dropframe; h/m/s may or may not sync to wallclock time
	else {
		/*
		This is truly the dumbest shit. What we're trying to ensure here
		is that non-integer framerates are desynced from the wallclock
		time by a correct amount of time. For example, in the
		NTSC-without-dropframe case, 3600*num/den would be 107892
		(when truncated to int), which is quite a good approximation of
		how a long an hour is when counted in 30000/1001 frames per second.
		Unfortunately, that's not what we want, since frame numbers will
		still range from 00 to 29, meaning that we're really getting _30_
		frames per second and not 29.97 and the full hour will be off by
		almost 4 seconds (108000 frames versus 107892).

		DEATH TO SMPTE
		*/ 
		int fps_approx = floor(fps.FPS() + 0.5);
		int frames_per_h = 3600*fps_approx;
		int frames_per_m = 60*fps_approx;
		int frames_per_s = fps_approx;

		h = fn / frames_per_h;
		fn = fn % frames_per_h;

		m = fn / frames_per_m;
		fn = fn % frames_per_m;

		s = fn / frames_per_s;
		fn = fn % frames_per_s;

		f = fn;
	}

	return wxString::Format("%02i%c%02%c%02i%c%02i", h, sep, m, sep, s, sep, f);
}
