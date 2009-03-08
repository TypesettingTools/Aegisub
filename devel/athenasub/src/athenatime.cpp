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
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "athenatime.h"
#include "utils.h"
#include "exception.h"
using namespace Athenasub;


////////////////
// Constructors
Time::Time()
: ms(0)
{
}

Time::Time(int milliseconds)
{
	SetMS(milliseconds);
}

Time::Time(const String& timestamp)
{
	ParseString(timestamp);
}

Time::Time(int hours,int minutes,int seconds,int milliseconds)
{
	SetMS(hours*3600000 + minutes*60000 + seconds*1000 + milliseconds);
}


///////////////////
// Getters/setters
void Time::SetMS(int milliseconds)
{
	ms = Max(milliseconds,0);
}

int Time::GetMS() const
{
	return ms;
}


//////////////////////////////////
// Generates a string from a time
String Time::GetString(int ms_precision,int h_precision) const
{
	// Enforce sanity
	if (ms_precision < 0 || ms_precision > 3 || h_precision < 0) THROW_ATHENA_EXCEPTION(Exception::Out_Of_Range);

	// Generate values
	int _ms = GetMS();
	int h = _ms / 3600000;
	_ms -= h*3600000;
	int min = _ms / 60000;
	_ms -= min*60000;
	int s = _ms / 1000;
	_ms -= s*1000;

	// Find maximum hour value
	int maxH = 0;
	for (int i=0;i<h_precision;i++) {
		maxH = maxH * 10 + 9;
	}

	// Cap hour value
	if (h > maxH) {
		h = maxH;
		min = 59;
		s = 59;
		_ms = 999;
	}

	// Modify ms to account for precision
	if (ms_precision == 2) _ms /= 10;
	else if (ms_precision == 1) _ms /= 100;
	else if (ms_precision == 0) _ms = 0;

	// Asserts
	assert(h   >= 0 && h   <= maxH);
	assert(min >= 0 && min <= 59);
	assert(s   >= 0 && s   <= 59);
	assert(_ms >= 0 && _ms <= 999);

	// Get write buffer
	String final;
	size_t size = 5 + (h_precision > 0 ? h_precision+1 : 0) + (ms_precision > 0 ? ms_precision + 1 : 0);
	size_t pos = 0;
	//wxChar *buffer = final.GetWriteBuf(size);
	Character temp[16];
	final.resize(size);

	// Write time
	if (h_precision > 0) {
		final.WriteNumber(temp,h,h_precision,pos);
		final.WriteChar(':',pos);
	}
	final.WriteNumber(temp,min,2,pos);
	final.WriteChar(':',pos);
	final.WriteNumber(temp,s,2,pos);
	if (ms_precision > 0) {
		final.WriteChar('.',pos);
		final.WriteNumber(temp,_ms,ms_precision,pos);
	}

	// Write terminator
	//final.WriteText("\0",1,pos);

	// Restore string's state and return
	//final.SetSize(pos-1);
	return final;
}


///////////////////////////////
// Parses a string into a time
void Time::ParseString(const String &data)
{
	// Break into an array of values
	array<size_t,4> values;
	//size_t last = 0;
	size_t len = data.Length();
	size_t curIndex = 0;
	char cur = 0;
	bool gotDecimal = false;
	int curValue = 0;
	int nDigits = 0;
	for (size_t i=0;i<len;i++) {
		cur = data[i];

		// Got a separator
		bool isDecimalSeparator = (cur == '.' || cur == ',');
		if (isDecimalSeparator || cur == ':' || cur == ';') {
			if (isDecimalSeparator) {
				if (gotDecimal) break;
				gotDecimal = true;
			}
			//values.at(curIndex++) = data.SubToInteger(last,i);
			//last = i+1;
			values.at(curIndex++) = curValue;
			curValue = 0;
			nDigits = 0;
		}

		// Got a digit
		else if (cur != ' ') {
			if (cur < '0' || cur > '9') {
				THROW_ATHENA_EXCEPTION(Exception::Parse_Error);
			}
			curValue = curValue * 10 + (int)(cur-'0');
			nDigits++;

			// Check if we're already done
			if (gotDecimal && nDigits >= 3) {
				values.at(curIndex++) = curValue;
				break;
			}
		}

		// Reached end of string
		if (i == len-1) {
			//int value = data.SubToInteger(last,len);
			//size_t digits = len - last;

			// Ended in decimal, so we gotta normalize it to 3 digits
			if (gotDecimal) {
				if (nDigits == 2) curValue *= 10;
				else if (nDigits == 1) curValue *= 100;
			}
			values.at(curIndex++) = curValue;
		}
	}

	// Turn into milliseconds
	size_t mult[] = { 0, 1, 1000, 60000, 3600000 };
	size_t accum = 0;
	size_t adjust = gotDecimal ? 0 : 1;
	for (int i=(int)curIndex;--i>=0;) {
		accum += values[i] * mult[curIndex-i+adjust];
	}
	
	// Set
	SetMS((int)accum);
}


//////////////////
// Get components
int Time::GetHoursComponent() const
{
	return ms / 3600000;
}

int Time::GetMinutesComponent() const
{
	int _ms = ms;
	int h = _ms / 3600000;
	_ms -= h*3600000;
	return _ms / 60000;
}

int Time::GetSecondsComponent() const
{
	int _ms = ms;
	int h = _ms / 3600000;
	_ms -= h*3600000;
	int min = _ms / 60000;
	_ms -= min*60000;
	return _ms / 1000;
}

int Time::GetMillisecondsComponent() const
{
	int _ms = ms;
	int h = _ms / 3600000;
	_ms -= h*3600000;
	int min = _ms / 60000;
	_ms -= min*60000;
	int s = _ms / 1000;
	_ms -= s*1000;
	return _ms;
}