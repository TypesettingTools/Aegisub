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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file ass_style.h
/// @see ass_style.cpp
/// @ingroup subs_storage
///

#ifndef AGI_PRE
#include <wx/colour.h>
#endif

#include "ass_entry.h"

/// DOCME
/// @class AssColor
/// @brief DOCME
///
/// DOCME
struct AssColor {
	int r;	///< Red component
	int g;	///< Green component
	int b;	///< Blue component
	int a;	///< Alpha component

	AssColor();
	AssColor(int r, int g, int b, int a = 0);
	AssColor(const wxColour &color);

	bool operator==(const AssColor &col) const;
	bool operator!=(const AssColor &col) const;

	wxColor GetWXColor();					// Return as a wxColor
	void SetWXColor(const wxColor &color);	// Sets from a wxColor
	void Parse(const wxString value);		// Parse SSA or ASS-style color
	wxString GetASSFormatted(bool alpha,bool stripped=false,bool isStyle=false) const;	// Gets color formated in ASS format
	wxString GetSSAFormatted() const;
};

/// DOCME
/// @class AssStyle
/// @brief DOCME
///
/// DOCME
class AssStyle : public AssEntry {
public:
	/// DOCME
	wxString name;

	/// DOCME
	wxString font;

	/// DOCME
	double fontsize;


	/// DOCME
	AssColor primary;

	/// DOCME
	AssColor secondary;

	/// DOCME
	AssColor outline;

	/// DOCME
	AssColor shadow;


	/// DOCME
	bool bold;

	/// DOCME
	bool italic;

	/// DOCME
	bool underline;

	/// DOCME
	bool strikeout;


	/// DOCME
	double scalex;

	/// DOCME
	double scaley;

	/// DOCME
	double spacing;

	/// DOCME
	double angle;

	/// DOCME
	int borderstyle;

	/// DOCME
	double outline_w;

	/// DOCME
	double shadow_w;

	/// DOCME
	int alignment;

	/// DOCME
	int Margin[4];

	/// DOCME
	int encoding;

	/// DOCME
	int relativeTo;


	/// @brief DOCME
	///
	ASS_EntryType GetType() const { return ENTRY_STYLE; }

	bool Parse(wxString data,int version=1);	// Parses raw ASS/SSA data into everything else
	void UpdateData();				// Updates raw data
	wxString GetSSAText() const;			// Retrieves SSA-formatted style

	wxString GetMarginString(int which) const;					// Returns the margin value as a string (0 = left, 1 = right, 2 = vertical/top, 3 = bottom)
	void SetMarginString(const wxString value,int which);	// Sets margin value from a string (0 = left, 1 = right, 2 = vertical/top, 3 = bottom)
	static void GetEncodings(wxArrayString &encodingStrings);

	AssEntry *Clone() const;
	bool IsEqualTo(AssStyle *style) const;

	AssStyle();
	AssStyle(AssStyle const&);
	AssStyle(wxString data,int version=1);
	~AssStyle();
};


