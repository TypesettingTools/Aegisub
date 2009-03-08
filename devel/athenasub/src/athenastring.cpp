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

#include "athenastring.h"
#include "exception.h"
#include <wx/string.h>

using namespace Athenasub;


String::String()
{
}


String::String(const char* utf8)
: std::basic_string<Character>(utf8)
{
}


String::String(const char* utf8,size_t bytes)
{
	resize(bytes);
	memcpy(GetCharPointer(0),utf8,bytes);
	*GetCharPointer(bytes) = 0;
}


String::String(const basic_string<Character>& str)
: std::basic_string<Character>(str)
{
}


String::String(const wchar_t* utf16)
{
	size_t len = GetUTF8Len(utf16);
	resize(len);
	UTF16toUTF8(utf16,GetCharPointer(0));
}


String::String(const wxString& wxstring)
{
	const wchar_t* utf16 = wxstring.c_str();
	size_t len = GetUTF8Len(utf16);
	resize(len);
	UTF16toUTF8(utf16,GetCharPointer(0));
}


String::String(char character)
{
	*this = std::string(character,1);
}


String::String(wchar_t character)
{
	wchar_t tmp[2];
	tmp[0] = character;
	tmp[1] = 0;
	*this = String(tmp);
}


String::String(int integer)
{
	*this = IntegerToString(integer);
}


String::String(float number)
{
	*this = FloatToString(number);
}


String::String(double number)
{
	*this = FloatToString(number);
}


Character* String::GetCharPointer(size_t pos)
{
	return &operator[](pos);
}


wxString String::GetWxString() const
{
	// TODO: optimize this to use UTF8ToUTF16()
	return wxString(c_str(),wxConvUTF8);
}


bool String::IsEmpty() const
{
	return size()==0;
}


void String::SetSize(size_t size)
{
	_Mysize = size;
}


void String::Truncate(size_t size)
{
	operator[](size) = 0;
	SetSize(size);
}


bool IsSpace(char chr)
{
	return (chr == ' ' || chr == '\t' || chr == '\n' || chr == '\r');
}


String& String::Trim(bool fromRight)
{
	int len = Length();
	size_t start = 0;
	size_t n = len;

	if (fromRight) {
		for (int i=len;--i>=0;) {
			if (IsSpace(operator[](i))) n = i;
			else break;
		}
	} else {
		for (int i=0;i<len;i++) {
			if (IsSpace(operator[](i))) start = i+1;
			else break;
		}
		n = len - start;
	}

	*this = String(substr(start,n));
	return *this;
}


String& String::TrimBoth()
{
	return Trim(true).Trim(false);
}


size_t String::Length() const
{
	return size();
}


size_t String::UTF8Length() const 
{
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}


bool String::Contains(const String& string) const
{
	return find(string) != npos;
}


size_t String::Find(Character c) const
{
	return find(c,0);
}


String String::Left(size_t n) const
{
	return String(substr(0,n));
}


String String::Right(size_t n) const
{
	size_t len = size();
	return String(substr(len-n,n));
}


String String::Mid(size_t start,size_t count) const
{
	return String(substr(start,count));
}


bool String::StartsWith(const String& string,bool caseSensitive) const
{
	if (caseSensitive) {
		String tmp = String(substr(0,string.size()));
		return compare(0,string.size(),string) == 0;
	} else {
		return AsciiLower().StartsWith(string.AsciiLower(),true);
	}
}


bool String::EndsWith(const String& string,bool caseSensitive) const
{
	if (caseSensitive) {
		size_t strSize = string.size();
		return compare(size() - strSize,strSize,string) == 0;
	} else {
		return AsciiLower().EndsWith(string.AsciiLower(),true);
	}
}


void String::WriteText(const Character* src,size_t len,size_t &pos)
{
	char *dst = GetCharPointer(pos);
	memcpy(dst,src,len*sizeof(Character));
	pos += len;
}


void String::WriteChar(const Character &src,size_t &pos)
{
	char *dst = GetCharPointer(pos);
	*dst = src;
	pos++;
}


void String::WriteNumber(Character *temp,int number,int pad,size_t &pos)
{
	char *dst = GetCharPointer(pos);

	// Write number backwards first
	int div, value;
	size_t len;
	for (len=0;true;len++) {
		div = number / 10;
		value = number - (div*10);
		temp[len] = (value + '0');
		if (!div) break;
		number = div;
	}
	len++;

	// Pad with zeroes
	pad -= (int)len;
	for (int i=0;i<pad;i++) {
		*dst++ = '0';
		pos++;
	}

	// Write number
	for (size_t i=0;i<len;i++) {
		*dst++ = temp[len-i-1];
		pos++;
	}
}


bool String::AsciiCompareNoCase(const Character *src) const
{
	unsigned char mask = 0xDF; // 0xDF
	unsigned char c1,c2;
	size_t len = size();
	for (size_t i=0;i<len;i++) {
		// Abort on end of string 2
		c2 = (unsigned char) operator[](i);
		if (!c2) return false;

		// Upper case both, this ONLY WORKS FOR ASCII
		c1 = ((unsigned char)src[i]) & mask;
		c2 = c2 & mask;

		// Check them
		if (c1 != c2) return false;
	}

	// Equal strings
	return true;
}


bool String::IsNumber() const
{
	for (const char *chr = c_str();*chr;chr++) {
		char cur = *chr;
		if (cur < '0' || cur > '9') {
			if (cur != '.' && cur != ',' && cur != '+' && cur != '-') {
				return false;
			}
		}
	}
	return true;
}


//

const Character* String::StringPtrTrim(Character *chr,size_t len,size_t startPos)
{
	// String metrics
	Character *read = chr;
	size_t start = startPos;
	size_t end = len;
	bool isStart = true;
	bool isEnd = false;
	Character cur;

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

const Character* String::StringTrim(String &str,size_t startPos)
{
	// Get a pointer to the string data
	Character *chr = const_cast<Character*> (str.c_str());
	return StringPtrTrim(chr,str.Length(),startPos);
}

/*
String String::Lower() const
{
	String tmp(*this);
	tmp.MakeLower();
	return tmp;
}

String String::Upper() const {
	String tmp(*this);
	tmp.MakeUpper();
	return tmp;
}

void String::MakeUpper()
{
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}

void String::MakeLower() 
{
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}
*/

String String::AsciiLower() const
{
	String tmp(*this);
	tmp.AsciiMakeLower();
	return tmp;
}

String String::AsciiUpper() const {
	String tmp(*this);
	tmp.AsciiMakeUpper();
	return tmp;
}

void String::AsciiMakeUpper()
{
	char* str = GetCharPointer(0);
	for (int i=0; str[i]; str++) {
		char cur = str[i];
		if (cur >= 'a' && cur <= 'z') str[i] -= 32;
	}
}

void String::AsciiMakeLower() 
{
	char* str = GetCharPointer(0);
	for (int i=0; str[i]; str++) {
		char cur = str[i];
		if (cur >= 'A' && cur <= 'Z') str[i] += 32;
	}
}


///////////////

String String::operator += (const String &p)
{
	append(p);
	return *this;
}

String String::operator += (const wxString &p)
{
	append(String(p));
	return *this;
}

String String::operator += (const char* p)
{
	append(p);
	return *this;
}

String String::operator += (const wchar_t* p)
{
	append(String(p));
	return *this;
}

String String::operator += (const double &p)
{
	append(FloatToString(p));
	return *this;
}

String String::operator += (const int &p)
{
	append(IntegerToString(p));
	return *this;
}

String String::operator += (const Character &p)
{
	append(1,p);
	return *this;
}

void Athenasub::operator <<(double &p1,String &p2)
{
	p1 = atof(p2.c_str());
}



//////////////////////////////////
// Convert a string to an integer
int String::ToInteger() const
{
	size_t len = Length();
	int value = 0;
	int mult = 1;
	int chr;
	bool firstChar = true;
	const char *data = c_str();
	for (size_t i=0;i<len;i++) {
		if (data[i] == ' ') continue;
		chr = (int)(data[i])-(int)'0';
		if (chr >= 0 && chr <= 9) value = 10*value+chr;
		else if (firstChar && data[i] == '-') mult = -1;
		else if (firstChar && data[i] == '+') {
			firstChar = false;
			continue;
		}
		else THROW_ATHENA_EXCEPTION(Exception::Out_Of_Range);
		firstChar = false;
	}
	return value * mult;
}


//////////////////////////////////////
// Converts a substring to an integer
int String::SubToInteger(size_t start,size_t end) const
{
	int value = 0;
	int chr;
	const char *data = c_str();
	for (size_t i=start;i<end;i++) {
		chr = (int)(data[i])-(int)'0';
		if (chr >= 0 && chr <= 9) value = 10*value+chr;
	}
	return value;
}


////////////////
// Pretty float
String String::PrettyFloat(String src)
{
	if (src.Contains(".")) {
		size_t len = src.Length();
		while (src.EndsWith("0")) {
			len--;
			src.Truncate(len);
		}
		if (src.EndsWith(".")) {
			len--;
			src.Truncate(len);
		}
	}
	return src;
}


///////////////////
// Float to string
String String::FloatToString(float src)
{
	return PrettyFloat(String(wxString::Format(_T("%f"),src)));
}

String String::FloatToString(double src)
{
	return PrettyFloat(String(wxString::Format(_T("%f"),src)));
}


/////////////////
// Int to string
String String::IntegerToString(int value)
{
	return String(wxString::Format(_T("%i"),value));
}


///////////////////////////////////////////////
// Get the UTF-8 length out of a UTF-16 string
size_t String::GetUTF8Len(const wchar_t *utf16)
{
	size_t len = 0;
	wchar_t curChar = utf16[0];
	for (size_t i=0;curChar;) {
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
		curChar = utf16[++i];
	}

	return len;
}


///////////////////////////
// Convert UTF-16 to UTF-8
size_t String::UTF16toUTF8(const wchar_t *utf16,char *utf8)
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

size_t String::UTF8toUTF16(const char *utf8,wchar_t *utf16)
{
	(void) utf8;
	(void) utf16;
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}
