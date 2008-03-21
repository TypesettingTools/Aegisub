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


#include "utils.h"
using namespace Athenasub;


//////////////////////////////////
// Convert a string to an integer
int Athenasub::StringToInt(const String &str)
{
	size_t len = str.Length();
	int value = 0;
	int chr;
	for (size_t i=0;i<len;i++) {
		chr = (int)str[i]-(int)'0';
		if (chr >= 0 && chr <= 9) value = 10*value+chr;
	}
	return value;
}


//////////////////////////////////////
// Converts a substring to an integer
int Athenasub::SubStringToInteger(const String &str,size_t start,size_t end)
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
String Athenasub::PrettyFloat(String src) {
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

String Athenasub::PrettyFloatF(float src) { return Athenasub::PrettyFloat(wxString::Format(_T("%f"),src)); }
String Athenasub::PrettyFloatD(double src) { return Athenasub::PrettyFloat(wxString::Format(_T("%f"),src)); }


///////////////////
// Float to string
String Athenasub::FloatToString(double value) {
	return PrettyFloat(wxString::Format(_T("%f"),value));
}


/////////////////
// Int to string
String Athenasub::IntegerToString(int value) {
	return wxString::Format(_T("%i"),value);
}


////////////////////////////
// Fast writing to a string
void Athenasub::WriteNumber(wxChar *&dst,wxChar *temp,int number,int pad,size_t &pos) {
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
const wxChar *Athenasub::StringPtrTrim(wxChar *chr,size_t len,size_t startPos)
{
	// String metrics
	wxChar *read = chr;
	size_t start = startPos;
	size_t end = len;
	bool isStart = true;
	bool isEnd = false;
	wxChar cur;

	// Search for spaces
	for (size_t i=start;i<len;i++) {
		cur = read[i];
		bool isSpace = (cur == ' ');
		if (isStart) {
			if (isSpace) start++;
			else isStart = false;
		}
		if (isEnd) {
			if (!isSpace) isEnd = false;
		}
		else {
			if (isSpace) {
				isEnd = true;
				end = i;
			}
		}
	}

	// Apply changes to pointer
	if (isEnd) chr[end] = 0;
	return chr + start;
}

const wxChar *Athenasub::StringTrim(wxString &str,size_t startPos)
{
	// Get a pointer to the string data
	wxChar *chr = const_cast<wxChar*> (str.c_str());
	return StringPtrTrim(chr,str.Length(),startPos);
}


//////////////////////////////////////////////////
// Compares a string to a constant, ignoring case
bool Athenasub::AsciiStringCompareNoCase(const wxString &str1,const wxChar *str2)
{
	const wxChar *src = str1.c_str();
	wxChar c1,c2;
	wxChar mask = 0xFFDF;
	size_t len = str1.Length();
	for (size_t i=0;i<len;i++) {
		// Abort on end of string 2
		c2 = str2[i];
		if (!c2) return false;

		// Upper case both, this ONLY WORKS FOR ASCII
		c1 = src[i] & mask;
		c2 = c2 & mask;

		// Check them
		if (c1 != c2) return false;
	}

	// Equal strings
	return true;
}


///////////////////////////////////////////////
// Get the UTF-8 length out of a UTF-16 string
size_t Athenasub::GetUTF8Len(const wchar_t *utf16)
{
	size_t len = 0;
	wchar_t curChar = utf16[0];
	for (size_t i=0;curChar;i++) {
		// 1 byte
		if ((curChar & 0xFF80) == 0) len++;

		// Surrogate pair UTF-16, 4 bytes
		else if ((curChar & 0xFC00) == 0xD800) {
			len += 4;
			i++;
		}
		
		// 3 bytes
		else if (curChar & 0xF800) len += 3;
		
		// 2 bytes
		else if (curChar & 0xFF80) len += 2;

		// Get next
		curChar = utf16[i];
	}

	return len;
}


///////////////////////////
// Convert UTF-16 to UTF-8
size_t Athenasub::UTF16toUTF8(const wchar_t *utf16,char *utf8)
{
	wchar_t curChar = utf16[0];
	size_t value;
	size_t written = 0;
	for (size_t i=0;;i++) {
		// 1 byte
		if ((curChar & 0xFF80) == 0) {
			utf8[written] = char(curChar);
			if (curChar == 0) break;
			written++;
		}

		// 2 bytes
		else if ((curChar & 0xF800) == 0) {
			utf8[written] = char(((curChar & 0x07C0) >> 6)  | 0xC0);
			utf8[written+1] = char((curChar & 0x003F)       | 0x80);
			written += 2;
		}

		// Surrogate pair UTF-16
		else if ((curChar & 0xFC00) == 0xD800) {
			// Read
			value = (curChar - 0xD800) << 10;
			value |= utf16[i+1] & 0x3FF;
			i++;

			// Write
			utf8[written] = char(((value & 0x1C0000) >> 18)	  | 0xF0);
			utf8[written+1] = char(((value & 0x03F000) >> 12) | 0x80);
			utf8[written+2] = char(((value & 0x000FC0) >> 6)  | 0x80);
			utf8[written+3] = char((value & 0x00003F)         | 0x80);
			written += 4;
		}

		// 3 bytes
		else if (curChar & 0xF800) {
			utf8[written] = char(((curChar & 0xF000) >> 12)   | 0xE0);
			utf8[written+1] = char(((curChar & 0x0FC0) >> 6)  | 0x80);
			utf8[written+2] = char((curChar & 0x003F)         | 0x80);
			written += 3;
		}

		// Get next
		curChar = utf16[i+1];
	}
	return written;
}

