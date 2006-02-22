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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


////////////
// Includes
#include "ass_time.h"
#include "vfr.h"
#include <fstream>
#include <algorithm>



////////////////////// AssTime //////////////////////
// AssTime constructors
AssTime::AssTime () {
	time = 0;
}


////////////////////
// Parses from ASS
// ---------------
// Note that this function is atomic, it won't touch the values if it's invalid.
void AssTime::ParseASS (const wxString _text) {
	// Prepare
	wxString text = _text;
	text.Trim(true);
	text.Trim(false);
	wxString temp;
	size_t pos = 0;
	size_t end = 0;
	long th,tm,ts,tms;
	double ts_raw;

	try {
		// Hours
		end = text.find(_T(":"),pos);
		temp = text.SubString(pos,end-1);
		if (!temp.ToLong(&th)) throw 0;
		pos = end+1;

		// Minutes
		end = text.find(_T(":"),pos);
		temp = text.SubString(pos,end-1);
		if (!temp.ToLong(&tm)) throw 0;
		pos = end+1;

		// Seconds
		end = text.length();
		temp = text.Mid(pos);
		if (!temp.ToDouble(&ts_raw)) throw 0;

		// Split into seconds and fraction
		ts = ts_raw;
		tms = (ts_raw-ts)*1000+0.5;
	}

	// Something went wrong, don't change anything
	catch (...) {
		return;
	}

	// OK, set values
	time = tms + ts*1000 + tm*60000 + th*3600000;
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
int AssTime::GetMS () {
	if (!UseMSPrecision) return time/10*10;
	else return time;
}

void AssTime::SetMS (int _ms) {
	time = _ms;
}


////////////////
// ASS Formated
wxString AssTime::GetASSFormated () {
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

	if (UseMSPrecision) return wxString::Format(_T("%01i:%02i:%02i.%03i"),h,m,s,ms);
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


/////////////////////////////////////////////////
// Reads value from a text control and update it
void AssTime::UpdateFromTextCtrl(wxTextCtrl *ctrl) {
	long start,end;
	wxString text = ctrl->GetValue();
	ctrl->GetSelection(&start,&end);
	if (start == end) {
		wxString nextChar = text.Mid(start,1);
		if (nextChar == _T(":") || nextChar == _T(".")) {
			wxString temp = text;
			text = temp.Left(start-1);
			text += nextChar;
			text += temp.Mid(start-1,1);
			text += temp.Mid(start+2);
			start++;
			end++;
		}
		else if (nextChar.IsEmpty()) text.Remove(start-1,1);
		else text.Remove(start,1);
	}

	// Update time
	ParseASS(text);
	ctrl->SetValue(GetASSFormated());
	ctrl->SetSelection(start,end);
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
