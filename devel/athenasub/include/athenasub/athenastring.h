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

#pragma once

#include "api.h"
#include <vector>

class wxString;

namespace Athenasub {

	typedef char Character;

	// String class
	class ATHENA_API String : public std::basic_string<Character> {
	private:
		Character* GetCharPointer(size_t pos);

	public:

		String();
		String(const char* utf8);
		String(const char* utf8,size_t bytes);
		String(const basic_string<Character>& str);
		explicit String(const wchar_t* utf16);
		explicit String(const wxString& wxstring);
		explicit String(char character);
		explicit String(wchar_t character);
		explicit String(int integer);
		explicit String(float number);
		explicit String(double number);

		wxString GetWxString() const;

		bool IsEmpty() const;
		size_t Length() const;
		size_t UTF8Length() const;
		
		void SetSize(size_t size);
		void Truncate(size_t size);

		String& Trim(bool fromRight);
		String& TrimBoth();

		bool Contains(const String& string) const;
		size_t Find(Character c) const;

		String Left(size_t n) const;
		String Right(size_t n) const;
		String Mid(size_t start,size_t count=npos) const;

		bool StartsWith(const String& string,bool caseSensitive=true) const;
		bool EndsWith(const String& string,bool caseSensitive=true) const;

		void WriteText(const Character* src,size_t len,size_t &pos);
		void WriteChar(const Character &src,size_t &pos);
		void WriteNumber(Character *temp,int number,int pad,size_t &pos);

		bool IsNumber() const;

		//String Lower() const;
		//String Upper() const;
		//void MakeUpper();
		//void MakeLower();

		String AsciiLower() const;
		String AsciiUpper() const;
		void AsciiMakeUpper();
		void AsciiMakeLower();
		bool AsciiCompareNoCase(const Character *src) const;

		// Convert a string to an integer
		int ToInteger() const;
		int SubToInteger(size_t start,size_t end) const;

		//

		static const Character* StringPtrTrim(Character *chr,size_t len,size_t startPos);
		static const Character* StringTrim(String &str,size_t startPos);

		// Number to string functions
		static String PrettyFloat(String src);
		static String FloatToString(double value);
		static String FloatToString(float value);
		static String IntegerToString(int value);
		static String PrettySize(int bytes);

		// Unicode routines
		static size_t GetUTF8Len(const wchar_t *utf16);
		static size_t UTF16toUTF8(const wchar_t *utf16,char *utf8);
		static size_t UTF8toUTF16(const char *utf8,wchar_t *utf16);

		//////////

		String operator += (const String &p);
		String operator += (const wxString &p);
		String operator += (const char* p);
		String operator += (const wchar_t* p);
		String operator += (const double &p);
		String operator += (const int &p);
		String operator += (const Character &p);
	};

	void operator <<(double &p1,String &p2);

	//typedef std::basic_string<wchar_t> String;
	typedef std::vector<String> StringArray;

}
