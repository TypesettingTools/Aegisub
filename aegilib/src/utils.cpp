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
// AEGISUB/GORGONSUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//


#include "utils.h"
using namespace Gorgonsub;


//////////////////////////////////
// Convert a string to an integer
int Gorgonsub::StringToInt(const String &str)
{
	// TODO: optimize
	if (!str.IsNumber()) return 0;
	long temp;
	str.ToLong(&temp);
	return (int) temp;
}


//////////////////////////////////////
// Converts a substring to an integer
int Gorgonsub::SubStringToInteger(const String &str,size_t start,size_t end)
{
	int value = 0;
	int chr;
	for (size_t i=start;i<end;i++) {
		chr = (int)str[i]-(int)'0';
		if (chr >= 0 && chr <= 9) value = 10*value+chr;
	}
	return value;
}


////////////////
// Pretty float
String Gorgonsub::PrettyFloat(String src) {
	if (src.Contains(_T("."))) {
		size_t len = src.Length();
		while (src.Right(1) == _T("0")) {
			len--;
			src.Truncate(len);
		}
		if (src.Right(1) == _T(".")) {
			len--;
			src.Truncate(len);
		}
	}
	return src;
}

String Gorgonsub::PrettyFloatF(float src) { return Gorgonsub::PrettyFloat(wxString::Format(_T("%f"),src)); }
String Gorgonsub::PrettyFloatD(double src) { return Gorgonsub::PrettyFloat(wxString::Format(_T("%f"),src)); }


///////////////////
// Float to string
String Gorgonsub::FloatToString(double value) {
	return PrettyFloat(wxString::Format(_T("%f"),value));
}


/////////////////
// Int to string
String Gorgonsub::IntegerToString(int value) {
	return wxString::Format(_T("%i"),value);
}


////////////////////////////
// Fast writing to a string
void Gorgonsub::WriteNumber(wxChar *&dst,wxChar *temp,int number,int pad,size_t &pos) {
	// Write number backwards first
	int div, value;
	size_t len;
	for (len=0;true;len++) {
		div = number / 10;
		value = number - (div*10);
		temp[len] = (wxChar) (value + '0');
		if (!div) break;
		number = div;
	}
	len++;

	// Pad with zeroes
	pad -= (int)len;
	for (int i=0;i<pad;i++) {
		*dst++ = (wxChar) '0';
		pos++;
	}

	// Write number
	for (size_t i=0;i<len;i++) {
		*dst++ = temp[len-i-1];
		pos++;
	}
}


/////////////////
// Trim a string
const wxChar *Gorgonsub::StringTrim(wxString &str,size_t startPos)
{
	size_t len = str.Length();
	size_t start = startPos;
	size_t end = len;
	bool isStart = true;
	bool isEnd = false;
	wxChar cur;
	for (size_t i=start;i<len;i++) {
		cur = str[i];
		if (isStart)
			if (cur == ' ') start++;
			else isStart = false;
		if (isEnd)
			if (cur != ' ') isEnd = false;
		else {
			if (cur == ' ') {
				isEnd = true;
				end = i;
			}
		}
	}
	startPos = start;
	if (isEnd) str[end] = 0;
	return str.c_str() + startPos;
}
