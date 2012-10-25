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

/// @file ass_time.cpp
/// @brief Class for managing timestamps in subtitles
/// @ingroup subs_storage
///

#include "config.h"

#include "ass_time.h"

#ifndef AGI_PRE
#include <algorithm>
#include <cmath>

#include <wx/tokenzr.h>
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

wxString AssTime::GetAssFormated(bool msPrecision) const {
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

SmpteFormatter::SmpteFormatter(agi::vfr::Framerate fps, char sep)
: fps(fps)
, sep(sep)
{
}

wxString SmpteFormatter::ToSMPTE(AssTime time) const {
	int h=0, m=0, s=0, f=0;
	fps.SmpteAtTime(time, &h, &m, &s, &f);
	return wxString::Format("%02d%c%02d%c%02d%c%02d", h, sep, m, sep, s, sep, f);
}

AssTime SmpteFormatter::FromSMPTE(wxString const& str) const {
	long h, m, s, f;
	wxArrayString toks = wxStringTokenize(str, sep);
	if (toks.size() != 4) return 0;
	toks[0].ToLong(&h);
	toks[1].ToLong(&m);
	toks[2].ToLong(&s);
	toks[3].ToLong(&f);
	return fps.TimeAtSmpte(h, m, s, f);
}
