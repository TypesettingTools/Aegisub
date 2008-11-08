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

#include "tokenizer.h"
#include "exception.h"
#include "utils.h"
#include <wx/tokenzr.h>
using namespace Athenasub;

///////////////
// Constructor
Tokenizer::Tokenizer(String &_string,Character _token,size_t start)
: string(_string), pos(start), token(_token)
{
	str = const_cast<Character*> (string.c_str());
}
Tokenizer::~Tokenizer()
{
}


////////////
// Has more
bool Tokenizer::HasMoreTokens()
{
	return pos < string.Length();
}


////////////////
// Get position
int Tokenizer::GetPosition()
{
	return (int) pos;
}


/////////////
// Get token
String Tokenizer::GetString(bool trim)
{
	// Has any more?
	if (!HasMoreTokens()) THROW_ATHENA_EXCEPTION(Exception::Invalid_Token);

	// Find token
	size_t len = string.Length();
	size_t oldPos = pos;
	bool found = false;
	for (size_t i=pos;i<len;i++) {
		if (str[i] == token) {
			found = true;
			str[i] = 0;
			pos = i+1;
			break;
		}
	}
	if (!found) pos = len;

	// Trimmed
	if (trim) {
		return String::StringPtrTrim(str+oldPos,pos-oldPos-1,0);
	}

	// Untrimmed
	return String(str+oldPos,pos-oldPos-1);
}


//////////////////
// Get as integer
int Tokenizer::GetInt()
{
	return GetString().ToInteger();
}


////////////////
// Get as float
float Tokenizer::GetFloat()
{
	double value;
	String temp = GetString();
	//temp.ToDouble(&value);
	value << temp;
	return (float) value;
}


//////////////////
// Get as boolean
bool Tokenizer::GetBool()
{
	return GetInt() != 0;
}


//////////////////////////////////
// Gets all remaining string data
String Tokenizer::GetTheRest()
{
	// Make string
	size_t size = string.Length()-pos;
	size_t oldPos = pos;
	pos += size;
	return String(str+oldPos,size);
}
