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

#include "format_ass.h"
#include "tokenizer.h"
#include "utils.h"
using namespace Gorgonsub;


////////////////
// Constructors
DialogueASS::DialogueASS()
{
	for (int i=0;i<4;i++) margin[i] = 0;
	layer = 0;
	isComment = false;
}
DialogueASS::DialogueASS(String data,int version)
{
	// Try parsing with all different versions
	bool valid = false;
	for (int count=0;!valid && count < 3;count++) {
		valid = Parse(data,version);
		version++;
		if (version > 2) version = 0;
	}
	if (!valid) throw Exception(Exception::Parse_Error);
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
		Tokenizer tkn(rawData.Mid(pos),_T(","));

		// Get first token and see if it has "Marked=" in it
		temp = tkn.GetString(true);
		if (temp.Lower().StartsWith(_T("marked="))) {
			version = 0;
			layer = 0;
		}

		// Not SSA, so read layer number
		else {
			if (version == 0) version = 1;	// Only do it for SSA, not ASS2
			layer = StringToInt(temp);
		}

		// Get times
		start.Parse(tkn.GetString());
		end.Parse(tkn.GetString());

		// Get style and actor
		style = tkn.GetString(true);
		actor = tkn.GetString(true);

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
		effect = temp;

		// Get text
		text = rawData.Mid(pos+tkn.GetPosition());
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
	// Old, slow code
	if (false) {
		// Prepare
		wxString final;

		// Write comment or dialogue
		if (isComment) final = _T("Comment: ");
		else final = _T("Dialogue: ");

		// Write layer or marked
		if (version >= 1) final += wxString::Format(_T("%01i,"),layer);
		else final += _T("Marked=0,");

		// Write times, style and actor
		final += start.GetString(2,1) + _T(",") + end.GetString(2,1) + _T(",") + style + _T(",") + actor + _T(",");

		// Write margins
		if (version <= 1) final += wxString::Format(_T("%04i,%04i,%04i,"),margin[0],margin[1],margin[2]);
		else final += wxString::Format(_T("%04i,%04i,%04i,%04i,"),margin[0],margin[1],margin[2],margin[3]);

		// Write effect and text
		final += effect + _T(",") + text;

		// Return final
		return final;
	}

	// New, faster code
	else {
		// Calculate size
		size_t size = 9+9+20+12+12;			// 9 for "comment: " (+1 for dialogue below), 9 for commas,
											// 20 for times, 12 for margins, 12 just to be sure that layer fits
		if (!isComment) size++;				// Comment->Dialogue
		if (version == 0) size += 8;		// "Marked=0"
		else if (version == 2) size += 5;	// Fourth margin
		size += style.Length() + actor.Length() + effect.Length() + text.Length();

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
		wxString tempStr = start.GetString(2,1);
		WriteText(buffer,&tempStr[0],10,pos);
		WriteChar(buffer,_T(','),pos);
		tempStr = end.GetString(2,1);
		WriteText(buffer,&tempStr[0],10,pos);
		WriteChar(buffer,_T(','),pos);

		// Write style and actor
		WriteText(buffer,&style[0],style.Length(),pos);
		WriteChar(buffer,_T(','),pos);
		WriteText(buffer,&actor[0],actor.Length(),pos);
		WriteChar(buffer,_T(','),pos);

		// Write margins
		size_t marCount = 3;
		if (version == 2) marCount++;
		for (size_t i=0;i<marCount;i++) {
			WriteNumber(buffer,temp,margin[i],4,pos);
			WriteChar(buffer,_T(','),pos);
		}

		// Write effect and text
		WriteText(buffer,&effect[0],effect.Length(),pos);
		WriteChar(buffer,_T(','),pos);
		WriteText(buffer,&text[0],text.Length(),pos);

		// Write terminator
		WriteText(buffer,_T("\0"),1,pos);

		// Restore string's state
		final.UngetWriteBuf(pos-1);
		return final;
	}
}
