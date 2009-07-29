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


////////////
// Includes
#include "config.h"

#include <wx/regex.h>
#include <math.h>
#include <fstream>
#include <algorithm>
#include "ass_time.h"
#include "vfr.h"
#include "utils.h"



////////////////////// AssTime //////////////////////
// AssTime constructors
AssTime::AssTime () {
	time = 0;
}


////////////////////
// Parses from ASS
// ---------------
// Note that this function is atomic, it won't touch the values if it's invalid.
void AssTime::ParseASS (const wxString text) {
	// Prepare
	size_t pos = 0;
	size_t end = 0;
	long th=0,tm=0,tms=0;

	// Count the number of colons
	size_t len = text.Length();
	int colons = 0;
	for (pos=0;pos<len;pos++) if (text[pos] == _T(':')) colons++;
	pos = 0;
	
	// Set start so that there are only two colons at most
	if (colons > 2) {
		for (pos=0;pos<len;pos++) {
			if (text[pos] == _T(':')) {
				colons--;
				if (colons == 2) break;
			}
		}
		pos++;
		end = pos;
	}

	try {
		// Hours
		if (colons == 2) {
			while (text[end++] != _T(':'));
			th = AegiStringToInt(text,pos,end);
			pos = end;
		}

		// Minutes
		if (colons >= 1) {
			while (text[end++] != _T(':'));
			tm = AegiStringToInt(text,pos,end);
			pos = end;
		}

		// Miliseconds (includes seconds)
		end = text.Length();
		tms = AegiStringToFix(text,3,pos,end);
	}

	// Something went wrong, don't change anything
	catch (...) {
		return;
	}

	// OK, set values
	time = tms + tm*60000 + th*3600000;
}


///////////////////
// Parses from SRT
void AssTime::ParseSRT (const wxString _text) {
	// Prepare
	wxString text = _text;
	text.Trim(false);
	text.Trim(true);
	long tempv;
	wxString temp;
	int ms,s,m,h;

	// Parse
	temp = text.Mid(0,2);
	temp.ToLong(&tempv);
	h = tempv;
	temp = text.Mid(3,2);
	temp.ToLong(&tempv);
	m = tempv;
	temp = text.Mid(6,2);
	temp.ToLong(&tempv);
	s = tempv;
	temp = text.Mid(9,3);
	temp.ToLong(&tempv);
	ms = tempv;

	// Set value
	time = ms + s*1000 + m*60000 + h*3600000;
}


//////////////////////////////////////////
// AssTime conversion to/from miliseconds
int AssTime::GetMS () const {
	if (!UseMSPrecision) return time/10*10;
	else return time;
}

void AssTime::SetMS (int _ms) {
	time = _ms;
}


////////////////
// ASS Formated
wxString AssTime::GetASSFormated (bool msPrecision) {
	int h,m,s,ms;
	int _ms = time;

	// Centisecond precision
	msPrecision = msPrecision || UseMSPrecision;
	if (!msPrecision) _ms = _ms/10*10;

	// Reset
	h = m = s = ms = 0;
	if (_ms < 0) _ms = 0;

	// Hours
	while (_ms >= 3600000) {
		_ms -= 3600000;
		h++;
	}

	// Ass overflow
	if (h > 9) {
		h = 9;
		m = 59;
		s = 59;
		ms = 999;
	}

	// Minutes
	while (_ms >= 60000) {
		_ms -= 60000;
		m++;
	}

	// Seconds
	while (_ms >= 1000) {
		_ms -= 1000;
		s++;
	}
	ms = _ms;

	if (msPrecision) return wxString::Format(_T("%01i:%02i:%02i.%03i"),h,m,s,ms);
	else return wxString::Format(_T("%01i:%02i:%02i.%02i"),h,m,s,ms/10);
}


////////////////
// SRT Formated
wxString AssTime::GetSRTFormated () {
	int h,m,s,ms;
	int _ms = time;

	// Centisecond precision
	if (!UseMSPrecision) _ms = _ms/10*10;

	// Reset
	h = m = s = ms = 0;
	if (_ms < 0) _ms = 0;

	// Hours
	while (_ms >= 3600000) {
		_ms -= 3600000;
		h++;
	}

	// Ass overflow
	if (h > 9) {
		h = 9;
		m = 59;
		s = 59;
		ms = 999;
	}

	// Minutes
	while (_ms >= 60000) {
		_ms -= 60000;
		m++;
	}

	// Seconds
	while (_ms >= 1000) {
		_ms -= 1000;
		s++;
	}
	ms = _ms;

	wxString result = wxString::Format(_T("%02i:%02i:%02i,%03i"),h,m,s,ms);
	return result;
}


//////////////////////
// AssTime comparison
bool operator < (AssTime &t1, AssTime &t2) {
	return (t1.GetMS() < t2.GetMS());
}

bool operator > (AssTime &t1, AssTime &t2) {
	return (t1.GetMS() > t2.GetMS());
}

bool operator <= (AssTime &t1, AssTime &t2) {
	return (t1.GetMS() <= t2.GetMS());
}

bool operator >= (AssTime &t1, AssTime &t2) {
	return (t1.GetMS() >= t2.GetMS());
}

bool operator == (AssTime &t1, AssTime &t2) {
	return (t1.GetMS() == t2.GetMS());
}

bool operator != (AssTime &t1, AssTime &t2) {
	return (t1.GetMS() != t2.GetMS());
}


/////////////////
// Static option
bool AssTime::UseMSPrecision = false;


///////
// Get
int AssTime::GetTimeHours() { return time / 3600000; }
int AssTime::GetTimeMinutes() { return (time % 3600000)/60000; }
int AssTime::GetTimeSeconds() { return (time % 60000)/1000; }
int AssTime::GetTimeMiliseconds() { return (time % 1000); }
int AssTime::GetTimeCentiseconds() { return (time % 1000)/10; }




///////
// Constructor
FractionalTime::FractionalTime (wxString separator, int numerator, int denominator, bool dropframe) {
	drop = dropframe;
	if (drop) {
		// no dropframe for any other framerates
		num = 30000;
		den = 1001;
	} else {
		num = numerator;
		den = denominator;
	}
	sep = separator;

	// fractions < 1 are not welcome here
	if ((num <= 0 || den <= 0) || (num < den))
		throw _T("FractionalTime: nonsensical enumerator or denominator");
	if (sep.IsEmpty())
		throw _T("FractionalTime: no separator specified");
}

///////
// Destructor
FractionalTime::~FractionalTime () {
	sep.Clear();
}

///////
// SMPTE text string to milliseconds conversion
int FractionalTime::ToMillisecs (wxString _text) {
	wxString text = _text;
	wxString re_str = _T("");
	text.Trim(false);
	text.Trim(true);
	long h=0,m=0,s=0,f=0;

	//           hour                   minute                 second                 fraction
	re_str << _T("(\\d+)") << sep << _T("(\\d+)") << sep << _T("(\\d+)") << sep << _T("(\\d+)");

	wxRegEx re(re_str, wxRE_ADVANCED);
	if (!re.IsValid())
		throw _T("FractionalTime: regex failure");
	if (!re.Matches(text))
		return 0; // FIXME: throw here too?
	
	re.GetMatch(text,1).ToLong(&h);
	re.GetMatch(text,2).ToLong(&m);
	re.GetMatch(text,3).ToLong(&s);
	re.GetMatch(text,4).ToLong(&f);

	int msecs_f = 0;
	int fn = 0;
	// dropframe? do silly things
	if (drop) {
		fn += h * frames_per_period * 6;
		fn += (m % 10) * frames_per_period;
		
		if (m > 0) {
			fn += 1800;
			m--;

			fn += m * 1798; // two timestamps dropped per minute after the first
			fn += s * 30 + f - 2;
		}
		else {				// minute is evenly divisible by 10, keep first two timestamps
			fn += s * 30;
			fn += f;
		}

		msecs_f = (fn * num) / den;
	}
	// no dropframe, may or may not sync with wallclock time
	// (see comment in FromMillisecs for an explanation of why it's done like this)
	else {
		int fps_approx = floor((double(num)/double(den))+0.5);
		fn += h * 3600 * fps_approx;
		fn += m * 60 * fps_approx;
		fn += s * fps_approx;
		fn += f;

		msecs_f = (fn * num) / den;
	}

	return msecs_f;
}

///////
// SMPTE text string to AssTime conversion
AssTime FractionalTime::ToAssTime (wxString _text) {
	AssTime time;
	time.SetMS((int)ToMillisecs(_text));
	return time;
}

///////
// AssTime to SMPTE text string conversion
wxString FractionalTime::FromAssTime(AssTime time) {
	return FromMillisecs(time.GetMS());
}

///////
// Milliseconds to SMPTE text string conversion
wxString FractionalTime::FromMillisecs(int64_t msec) {
	int h=0, m=0, s=0, f=0; // hours, minutes, seconds, fractions
	int fn = (msec*(int64_t)num) / (1000*den); // frame number

	// return 00:00:00:00
	if (msec <= 0)
		goto RETURN;

	// dropframe?
	if (drop) {
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
		int fps_approx = floor((double(num)/double(den))+0.5);
		int frames_per_h = 3600*fps_approx;
		int frames_per_m = 60*fps_approx;
		int frames_per_s = fps_approx;
		while (fn >= frames_per_h) {
			h++; fn -= frames_per_h;
		}
		while (fn >= frames_per_m) {
			m++; fn -= frames_per_m;
		}
		while (fn >= frames_per_s) {
			s++; fn -= frames_per_s;
		}
		f = fn;
	}

RETURN:
	return wxString::Format(_T("%02i") + sep + _T("%02i") + sep + _T("%02i") + sep + _T("%02i"),h,m,s,f);
}

