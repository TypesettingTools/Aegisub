// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once

///////////
// Headers
#include <wx/colour.h>
#include "ass_entry.h"


/////////////////////////
// Class to store colors
class AssColor {
public:
	int r;	// Red component
	int g;	// Green component
	int b;	// Blue component
	int a;	// Alpha component

	AssColor();
	AssColor(wxColour &color);

	bool operator==(AssColor &col) const;
	bool operator!=(AssColor &col) const;

	wxColor GetWXColor();					// Return as a wxColor
	void SetWXColor(const wxColor &color);	// Sets from a wxColor
	void Parse(const wxString value);		// Parse SSA or ASS-style color
	wxString GetASSFormatted(bool alpha,bool stripped=false,bool isStyle=false);	// Gets color formated in ASS format
	wxString GetSSAFormatted();
};


/////////////////////////
// Class to store styles
class AssStyle : public AssEntry {
public:
	wxString name;
	wxString font;
	double fontsize;

	AssColor primary;
	AssColor secondary;
	AssColor outline;
	AssColor shadow;

	bool bold;
	bool italic;
	bool underline;
	bool strikeout;

	double scalex;
	double scaley;
	double spacing;
	double angle;
	int borderstyle;
	double outline_w;
	double shadow_w;
	int alignment;
	int Margin[4];
	int encoding;
	int relativeTo;

	ASS_EntryType GetType() { return ENTRY_STYLE; }

	bool Parse(wxString data,int version=1);	// Parses raw ASS/SSA data into everything else
	void UpdateData();				// Updates raw data
	wxString GetSSAText();			// Retrieves SSA-formatted style

	wxString GetMarginString(int which);					// Returns the margin value as a string (0 = left, 1 = right, 2 = vertical/top, 3 = bottom)
	void SetMarginString(const wxString value,int which);	// Sets margin value from a string (0 = left, 1 = right, 2 = vertical/top, 3 = bottom)
	static void GetEncodings(wxArrayString &encodingStrings);

	AssEntry *Clone() const;
	bool IsEqualTo(AssStyle *style);

	AssStyle();
	AssStyle(wxString data,int version=1);
	~AssStyle();
};
