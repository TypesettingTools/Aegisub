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

#include "format_ass_dialogue.h"
#include "tokenizer.h"
#include "utils.h"
using namespace Athenasub;


////////////////
// Constructors
DialogueASS::DialogueASS()
{
	for (int i=0;i<4;i++) margin[i] = 0;
	layer = 0;
	isComment = false;
}
DialogueASS::DialogueASS(const String &data,int version)
{
	// Try parsing with all different versions
	bool valid = false;
	for (int count=0;!valid && count < 3;count++) {
		valid = Parse(data,version);
		version++;
		if (version > 2) version = 0;
	}
	if (!valid) {
		THROW_ATHENA_EXCEPTION(Exception::Parse_Error);
	}
}


//////////////////////////////////
// Generates a string from a time
String GetTimeString(const Time& time,int ms_precision,int h_precision)
{
	// Enforce sanity
	ms_precision = Mid(0,ms_precision,3);
	h_precision = Mid(0,h_precision,2);

	// Generate values
	int _ms = time.GetMS();
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

	// Get write buffer
	String final;
	size_t size = 7+h_precision+ms_precision;
	size_t pos = 0;
	wxChar *buffer = final.GetWriteBuf(size);
	wxChar temp[16];

	// Write time
	WriteNumber(buffer,temp,h,h_precision,pos);
	WriteChar(buffer,_T(':'),pos);
	WriteNumber(buffer,temp,min,2,pos);
	WriteChar(buffer,_T(':'),pos);
	WriteNumber(buffer,temp,s,2,pos);
	WriteChar(buffer,_T('.'),pos);
	WriteNumber(buffer,temp,_ms,ms_precision,pos);

	// Write terminator
	WriteText(buffer,_T("\0"),1,pos);

	// Restore string's state and return
	final.UngetWriteBuf(pos-1);
	return final;
}


///////////////////////////////
// Parses a string into a time
Time ParseTimeString(const String &data)
{
	// Break into an array of values
	array<size_t,4> values;
	size_t last = 0;
	size_t len = data.Length();
	size_t curIndex = 0;
	wxChar cur = 0;
	for (size_t i=0;i<len;i++) {
		cur = data[i];
		if (cur == ':' || cur == '.' || cur == ',' || cur == ';') {
			values.at(curIndex++) = SubStringToInteger(data,last,i);
			last = i+1;
		}
		if (i == len-1) {
			int value = SubStringToInteger(data,last,len);
			size_t digits = len - last;
			if (digits == 2) value *= 10;
			if (digits == 1) value *= 100;
			values.at(curIndex++) = value;
		}
	}

	// Turn into milliseconds
	size_t mult[] = { 0, 1, 1000, 60000, 3600000 };
	size_t accum = 0;
	for (int i=(int)curIndex;--i>=0;) {
		accum += values[i] * mult[curIndex-i];
	}
	return Time((int)accum);
}


//////////////////
// Parse ASS Data
bool DialogueASS::Parse(wxString rawData, int version)
{
	size_t pos = 0;
	wxString temp;

	// Get type
	if (rawData.StartsWith(_T("Dialogue:"))) {
		isComment = false;
		pos = 10;
	}
	else if (rawData.StartsWith(_T("Comment:"))) {
		isComment = true;
		pos = 9;
	}
	else return false;

	try {
		Tokenizer tkn(rawData,_T(','),pos);

		// Get first token and see if it has "Marked=" in it
		temp = tkn.GetString(true);
		if (AsciiStringCompareNoCase(temp,_T("marked="))) {
			version = 0;
			layer = 0;
		}

		// Not SSA, so read layer number
		else {
			if (version == 0) version = 1;	// Only do it for SSA, not ASS2
			layer = StringToInt(temp);
		}

		// Get times
		time[0] = ParseTimeString(tkn.GetString());
		time[1] = ParseTimeString(tkn.GetString());

		// Get style and actor
		text[1] = tkn.GetString(true);
		text[2] = tkn.GetString(true);

		// Get margins
		for (int i=0;i<3;i++) margin[i] = tkn.GetInt();
		margin[3] = margin[2];

		// Read next string, which is either bottom margin or effect
		temp = tkn.GetString(true);

		// Get bottom margin
		if (version == 2) {
			if (temp.IsNumber()) {
				// Got margin
				margin[3] = StringToInt(temp);

				// Read effect
				temp = tkn.GetString(true);
			}
			else version = 1;
		}

		// Get effect
		text[3] = temp;

		// Get text
		text[0] = tkn.GetTheRest();
	}

	catch (...) {
		return false;
	}

	return true;
}


/////////////
// Serialize
String DialogueASS::ToText(int version) const
{
	// Calculate size
	size_t size = 9+9+20+12+12;			// 9 for "comment: " (+1 for dialogue below), 9 for commas,
										// 20 for times, 12 for margins, 12 just to be sure that layer fits
	if (!isComment) size++;				// Comment->Dialogue
	if (version == 0) size += 8;		// "Marked=0"
	else if (version == 2) size += 5;	// Fourth margin
	for (size_t i=0;i<4;i++) size += text[i].Length();

	// Allocate string
	wxString final;
	wxChar *buffer = final.GetWriteBuf(size);
	wxChar temp[16];

	// Write comment/dialogue
	size_t pos = 0;
	if (isComment) WriteText(buffer,_T("Comment: "),9,pos);
	else WriteText(buffer,_T("Dialogue: "),10,pos);

	// Write layer or marked
	if (version >= 1) {
		WriteNumber(buffer,temp,layer,0,pos);
		WriteChar(buffer,_T(','),pos);
	}
	else WriteText(buffer,_T("Marked=0,"),9,pos);

	// Write times
	for (size_t i=0;i<2;i++) {
		wxString tempStr = GetTimeString(time[i],2,1);
		WriteText(buffer,&tempStr[0],10,pos);
		WriteChar(buffer,_T(','),pos);
	}

	// Write style and actor
	WriteText(buffer,&text[1][0],text[1].Length(),pos);
	WriteChar(buffer,_T(','),pos);
	WriteText(buffer,&text[2][0],text[2].Length(),pos);
	WriteChar(buffer,_T(','),pos);

	// Write margins
	size_t marCount = 3;
	if (version == 2) marCount++;
	for (size_t i=0;i<marCount;i++) {
		WriteNumber(buffer,temp,margin[i],4,pos);
		WriteChar(buffer,_T(','),pos);
	}

	// Write effect and text
	WriteText(buffer,&text[3][0],text[3].Length(),pos);
	WriteChar(buffer,_T(','),pos);
	WriteText(buffer,&text[0][0],text[0].Length(),pos);

	// Write terminator
	WriteText(buffer,_T("\0"),1,pos);

	// Restore string's state
	final.UngetWriteBuf(pos-1);
	return final;
}
