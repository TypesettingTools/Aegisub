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

#include "format_ass.h"
#include "tokenizer.h"
using namespace Aegilib;


////////////////
// Constructors
StyleASS::StyleASS()
{
}
StyleASS::StyleASS(String data,int version)
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


/////////
// Parse
bool StyleASS::Parse(String data,int version)
{
	try {
		// Tokenize
		wxString temp;
		Tokenizer tkn(data.Trim(false).Mid(6),_T(","));

		// Read name, font name and size
		name = tkn.GetString(true);
		font = tkn.GetString(true);
		fontSize = tkn.GetFloat();

		// Read colours
		for (int i=0;i<5;i++) {
			if ((i == 4 && version == 0) || (i == 2 && version != 0)) colour[i] = colour[i-1];
			else colour[i].Parse(tkn.GetString(),true);
		}

		// Read bold and italics
		bold = tkn.GetBool();
		italic = tkn.GetBool();

		// ASS
		if (version != 0) {
			// Read underline, strikeout, scale, spacing and angle
			underline = tkn.GetBool();
			strikeout = tkn.GetBool();
			scalex = tkn.GetFloat();
			scaley = tkn.GetFloat();
			spacing = tkn.GetFloat();
			angle = tkn.GetFloat();
		}
		
		else {
			// SSA defaults
			underline = false;
			strikeout = false;
			scalex = 100;
			scaley = 100;
			spacing = 0;
			angle = 0.0;
		}

		// Read border style and widths
		borderStyle = tkn.GetInt();
		outline_w = tkn.GetFloat();
		shadow_w = tkn.GetFloat();

		// Read alignment
		alignment = tkn.GetInt();
		if (version == 0) alignment = AlignSSAtoASS(alignment);

		// Read margins
		for (int i=0;i<4;i++) {
			if (i == 3 && version < 2) margin[i] = margin[i-1];
			else margin[i] = tkn.GetInt();
		}

		// Read and discard alpha level on SSA
		// TODO: do something with this?
		if (version == 0) tkn.GetString();

		// Read encoding
		encoding = tkn.GetInt();

		// Read relative to
		if (version == 2) relativeTo = tkn.GetInt();

		// End
		if (tkn.HasMore()) return false;
		return true;
	}

	catch (...) {
		return false;
	}
}


////////////////////////////////
// Convert SSA alignment to ASS
int StyleASS::AlignSSAtoASS(int align)
{
	switch(align) {
		case 1: return 1;
		case 2: return 2;
		case 3: return 3;
		case 5: return 7;
		case 6: return 8;
		case 7: return 9;
		case 9: return 4;
		case 10: return 5;
		case 11: return 6;
		default: return 2;
	}
}


////////////////////////////////
// Convert ASS alignment to SSA
int StyleASS::AlignASStoSSA(int assAlignment)
{
	// TODO
	return assAlignment;
}


/////////////
// Serialize
String StyleASS::ToText() const
{
	String final = L"Style";
	return final;
}
