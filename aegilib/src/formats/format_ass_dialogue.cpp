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
		temp = tkn.GetString().Trim(false).Trim(true);
		if (temp.Lower().StartsWith(_T("marked="))) version = 0;
		else if (version == 0) version = 1;

		// Get layer number
		if (version == 0) layer = 0;
		else {
			long templ;
			temp.ToLong(&templ);
			layer = templ;
		}

		// Get times
		start.Parse(tkn.GetString());
		end.Parse(tkn.GetString());

		// Get style
		style = tkn.GetString();
		style.Trim(true);
		style.Trim(false);

		// Get actor
		actor = tkn.GetString();
		actor.Trim(true);
		actor.Trim(false);

		// Get margins
		for (int i=0;i<3;i++) margin[i] = tkn.GetInt();

		// Get bottom margin
		if (version == 1) margin[3] = margin[2];
		bool rollBack = false;
		if (version == 2) {
			wxString oldTemp = temp;
			temp = tkn.GetString().Trim(false).Trim(true);
			if (!temp.IsNumber()) {
				version = 1;
				rollBack = true;
			}
			else {
				long templ;
				temp.ToLong(&templ);
				margin[3] = templ;
			}
		}

		// Get effect
		if (!rollBack) temp = tkn.GetString();
		effect = temp;
		effect.Trim(true);
		effect.Trim(false);

		// Get text
		text = rawData.Mid(pos+tkn.GetPosition());
	}

	catch (...) {
		return false;
	}

	return true;
}
