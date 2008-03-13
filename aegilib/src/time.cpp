// Copyright (c) 2008, Rodrigo Braz Monteiro
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
// AEGISUB/AEGILIB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "aegilib.h"
using namespace Aegilib;


//////////////////////
// Generates a string
String Time::GetString(int ms_precision,int h_precision) const
{
	// Enforce sanity
	ms_precision = Mid(0,ms_precision,3);
	h_precision = Mid(0,h_precision,2);

	// Generate values
	int _ms = ms;
	int h = _ms / 3600000;
	_ms -= h*3600000;
	int min = _ms / 60000;
	_ms -= min*60000;
	int s = _ms / 1000;
	_ms -= s*1000;

	// Cap hour value
	if (h > 9 && h_precision == 1) {
		h = 9;
		min = 59;
		s = 59;
		_ms = 999;
	}

	// Modify ms to account for precision
	if (ms_precision == 2) _ms /= 10;
	else if (ms_precision == 1) _ms /= 100;
	else if (ms_precision == 0) _ms = 0;

	// Generate mask string
	wxString mask = wxString::Format(_T("%%0%ii:%%0%ii:%%0%ii.%%0%ii"),h_precision,2,2,ms_precision);

	// Generate final string
	wxString final = wxString::Format(mask,h,min,s,_ms);

	// Done
	return final;
}


///////////////////
// Parses a string
void Time::Parse(String data)
{
	// Break into an array of values
	std::vector<double> values;
	values.reserve(4);
	size_t last = 0;
	data += _T(":");
	size_t len = data.Length();
	for (size_t i=0;i<len;i++) {
		if (data[i] == ':' || data[i] == ';') {
			wxString temp = data.Mid(last,i-last);
			last = i+1;
			double tempd;
			temp.ToDouble(&tempd);
			values.push_back(tempd);
		}
	}

	// Turn into milliseconds
	ms = 0;
	double mult = 1000.0;
	int len2 = (int) values.size();
	for (int i=len2;--i>=0;) {
		ms += (int)(values[i] * mult);
		mult *= 60.0;
	}
}
