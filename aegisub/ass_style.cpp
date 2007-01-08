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


////////////
// Includes
#include <wx/tokenzr.h>
#include "ass_style.h"
#include "utils.h"


///////////////////////// AssColor //////////////////////////
////////////////
// Constructors
AssColor::AssColor () {
	r=g=b=a=0;
}

AssColor::AssColor (wxColour &color) {
	SetWXColor(color);
}


//////////////////
// Parse from ASS
void AssColor::ParseASS (const wxString _value) {
	// Prepare
	wxString value = _value;
	value.Trim(false);
	value.Trim(true);
	value.UpperCase();

	// Remove leading and ending crap
	if (value.Left(1) == _T("&")) value = value.Mid(1);
	if (value.Left(1) == _T("H")) value = value.Mid(1);
	if (value.Right(1) == _T("&")) value = value.Left(value.Length()-1);

	// Read colours
	long temp[4] = { 0, 0, 0, 0 };
	bool ok;
	for (int i=0;i<4;i++) {
		if (value.Length() > 0) {
			ok = value.Right(2).ToLong(&temp[i],16);
			if (!ok) temp[i] = 0;
			value.Truncate(value.Length()-2);
		}
		else break;
	}

	// Copy
	r = temp[0];
	g = temp[1];
	b = temp[2];
	a = temp[3];
}


//////////////////
// Parse from SSA
void AssColor::ParseSSA (wxString value) {
	value.Trim(true);
	value.Trim(false);

	// Check if the moron who wrote it used ASS style in SSA
	if (value.Left(2) == _T("&H")) {
		ParseASS(value);
		return;
	}

	// Parse SSA
	long val;
	value.ToLong(&val);
	b = (val >> 16) & 0xFF;
	g = (val >> 8) & 0xFF;
	r = val & 0xFF;
	a = 0;
}


///////////////////
// Gets a wxColour
wxColour AssColor::GetWXColor() {
	return wxColour(r,g,b);
}


//////////////////////
// Sets color from wx
void AssColor::SetWXColor(const wxColor &color) {
	r = color.Red();
	g = color.Green();
	b = color.Blue();
}


///////////////////////////////
// Get formatted in ASS format
wxString AssColor::GetASSFormatted (bool alpha,bool stripped,bool isStyle) {
	wxString work;
	if (!stripped) work += _T("&H");
	if (alpha) work += wxString::Format(_T("%02X"),a);
	work += wxString::Format(_T("%02X%02X%02X"),b,g,r);
	if (!stripped && !isStyle) work += _T("&");
	return work;
}


/////////////////////////
// Get decimal formatted
wxString AssColor::GetSSAFormatted () {
	return wxString::Format(_T("%i"),(b<<16)+(g<<8)+r);
}


///////////////////////// AssStyle /////////////////////////
///////////////////////
// Default Constructor
AssStyle::AssStyle() {
	group = _T("[V4+ Styles]");
	
	name = _T("Default");
	font = _T("Arial");
	fontsize = 20;

	primary.r = 255;
	primary.g = 255;
	primary.b = 255;
	primary.a = 0;
	secondary.r = 255;
	secondary.g = 255;
	secondary.b = 0;
	secondary.a = 0;
	outline.r = 0;
	outline.g = 0;
	outline.b = 0;
	outline.a = 0;
	shadow.r = 0;
	shadow.g = 0;
	shadow.b = 0;
	shadow.a = 0;

	bold = false;
	italic = false;
	underline = false;
	strikeout = false;

	scalex = 100;
	scaley = 100;
	spacing = 0;
	angle = 0.0;
	borderstyle = 1;
	outline_w = 2.0;
	shadow_w = 2.0;
	alignment = 2;
	Margin[0] = 10;
	Margin[1] = 10;
	Margin[2] = 10;
	Margin[3] = 10;
	encoding = 0;
	relativeTo = 1;

	UpdateData();
}


///////////////
// Constructor
AssStyle::AssStyle(wxString _data,int version) {
	Valid = Parse(_data,version);
	if (!Valid) {
		throw _T("[Error] Failed parsing line.");
	}
	UpdateData();
}


//////////////
// Destructor
AssStyle::~AssStyle() {
}


//////////////////////////////
// Parses value from ASS data
bool AssStyle::Parse(wxString rawData,int version) {
	// Tokenize
	wxString temp;
	long templ;
	wxStringTokenizer tkn(rawData.Mid(6),_T(","),wxTOKEN_RET_EMPTY_ALL);

	// Read name
	if (!tkn.HasMoreTokens()) return false;
	name = tkn.GetNextToken();
	name.Trim(true);
	name.Trim(false);

	// Read font name
	if (!tkn.HasMoreTokens()) return false;
	font = tkn.GetNextToken();
	font.Trim(true);
	font.Trim(false);

	// Read font size
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToLong(&templ);
	fontsize = templ;

	if (version != 0) {
		// Read primary color
		if (!tkn.HasMoreTokens()) return false;
		primary.ParseASS(tkn.GetNextToken());

		// Read secondary color
		if (!tkn.HasMoreTokens()) return false;
		secondary.ParseASS(tkn.GetNextToken());

		// Read outline color
		if (!tkn.HasMoreTokens()) return false;
		outline.ParseASS(tkn.GetNextToken());

		// Read shadow color
		if (!tkn.HasMoreTokens()) return false;
		shadow.ParseASS(tkn.GetNextToken());
	}

	else {
		// Read primary color
		if (!tkn.HasMoreTokens()) return false;
		primary.ParseSSA(tkn.GetNextToken());

		// Read secondary color
		if (!tkn.HasMoreTokens()) return false;
		secondary.ParseSSA(tkn.GetNextToken());

		// Read and discard tertiary color
		if (!tkn.HasMoreTokens()) return false;
		tkn.GetNextToken();

		// Read shadow/outline color
		if (!tkn.HasMoreTokens()) return false;
		outline.ParseSSA(tkn.GetNextToken());
		shadow = outline;
	}

	// Read bold
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToLong(&templ);
	bold = true;
	if (templ == 0) bold = false;

	// Read italics
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();	temp.ToLong(&templ);
	italic = true;
	if (templ == 0) italic = false;

	if (version != 0) {
		// Read underline
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToLong(&templ);
		underline = true;
		if (templ == 0) underline = false;

		// Read strikeout
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToLong(&templ);
		strikeout = true;
		if (templ == 0) strikeout = false;

		// Read scale x
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToDouble(&scalex);
		//scalex = templ;

		// Read scale y
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToDouble(&scaley);
		//scaley = templ;

		// Read spacing
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToDouble(&spacing);
		//spacing = templ;

		// Read angle
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToDouble(&angle);
	}
	
	else {
		// SSA defaults
		shadow.a = 128;
		underline = false;
		strikeout = false;

		scalex = 100;
		scaley = 100;
		spacing = 0;
		angle = 0.0;
	}

	// Read border style
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToLong(&templ);
	borderstyle = templ;

	// Read outline width
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToDouble(&outline_w);

	// Read shadow width
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToDouble(&shadow_w);

	// Read alignment
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToLong(&templ);
	if (version == 0) {
		switch(templ) {
			case 1: alignment = 1; break;
			case 2: alignment = 2; break;
			case 3: alignment = 3; break;
			case 5: alignment = 7; break;
			case 6: alignment = 8; break;
			case 7: alignment = 9; break;
			case 9: alignment = 4; break;
			case 10: alignment = 5; break;
			case 11: alignment = 6; break;
			default: alignment = 2; break;
		}
	}
	else alignment = templ;

	// Read left margin
	if (!tkn.HasMoreTokens()) return false;
	SetMarginString(tkn.GetNextToken(),0);

	// Read right margin
	if (!tkn.HasMoreTokens()) return false;
	SetMarginString(tkn.GetNextToken(),1);

	// Read top margin
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	SetMarginString(temp,2);

	// Read bottom margin
	if (version == 2) {
		if (!tkn.HasMoreTokens()) return false;
		SetMarginString(tkn.GetNextToken(),3);
	}
	else SetMarginString(temp,3);

	// Read alpha level
	if (version == 0) {
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
	}

	// Read encoding
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToLong(&templ);
	encoding = templ;

	// Read relative to
	if (version == 2) {
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToLong(&templ);
		relativeTo = templ;
	}

	// End
	if (tkn.HasMoreTokens()) return false;
	return true;
}


//////////////////////////////////
// Writes data back to ASS format
void AssStyle::UpdateData() {
	// Prepare
	wxString final = _T("Style: ");

	// Write all final
	name.Replace(_T(","),_T(";"));
	font.Replace(_T(","),_T(";"));
	final += name + _T(",");
	final += font + _T(",");
	final += FloatToString(fontsize) + _T(",");

	final += primary.GetASSFormatted(true,false,true) + _T(",");
	final += secondary.GetASSFormatted(true,false,true) + _T(",");
	final += outline.GetASSFormatted(true,false,true) + _T(",");
	final += shadow.GetASSFormatted(true,false,true) + _T(",");

	final += IntegerToString(bold?-1:0) + _T(",");
	final += IntegerToString(italic?-1:0) + _T(",");
	final += IntegerToString(underline?-1:0) + _T(",");
	final += IntegerToString(strikeout?-1:0) + _T(",");

	final += FloatToString(scalex) + _T(",");
	final += FloatToString(scaley) + _T(",");
	final += FloatToString(spacing) + _T(",");

	final += FloatToString(angle) + _T(",");
	final += IntegerToString(borderstyle) + _T(",");
	final += FloatToString(outline_w) + _T(",");
	final += FloatToString(shadow_w) + _T(",");

	final += IntegerToString(alignment) + _T(",");
	final += IntegerToString(Margin[0]) + _T(",");
	final += IntegerToString(Margin[1]) + _T(",");
	final += IntegerToString(Margin[2]) + _T(",");
	final += IntegerToString(encoding);
	SetEntryData(final);
}


/////////////////////////////
// Sets margin from a string
void AssStyle::SetMarginString(const wxString str,int which) {
	wxString work = str;
	work.Trim(false);
	work.Trim(true);
	if (!work.IsNumber()) throw _T("Invalid margin value");
	long value;
	work.ToLong(&value);
	if (value < 0) value = 0;
	if (value > 9999) value = 9999;
	if (which < 0 || which >= 4) throw _T("Invalid margin");
	Margin[which] = value;
}


//////////////////////////
// Gets string for margin
wxString AssStyle::GetMarginString(int which) {
	int value;
	if (which < 0 || which >= 4) throw _T("Invalid margin");
	value = Margin[which];
	wxString result = wxString::Format(_T("%04i"),value);
	return result;
}


///////////////////////////////
// Convert style to SSA string
wxString AssStyle::GetSSAText() {
	// Prepare
	wxString output = _T("Style: ");

	// Write all data
	name.Replace(_T(","),_T(";"));
	font.Replace(_T(","),_T(";"));
	output += name + _T(",");
	output += font + _T(",");
	output += FloatToString(fontsize) + _T(",");

	output += primary.GetSSAFormatted() + _T(",");
	output += secondary.GetSSAFormatted() + _T(",");
	output += _T("0,");
	output += shadow.GetSSAFormatted() + _T(",");

	output += IntegerToString(bold?-1:0) + _T(",");
	output += IntegerToString(italic?-1:0) + _T(",");

	output += IntegerToString(borderstyle) + _T(",");
	output += FloatToString(outline_w) + _T(",");
	output += FloatToString(shadow_w) + _T(",");

	int align = 0;
	switch (alignment) {
		case 1: align = 1; break;
		case 2: align = 2; break;
		case 3: align = 3; break;
		case 4: align = 9; break;
		case 5: align = 10; break;
		case 6: align = 11; break;
		case 7: align = 5; break;
		case 8: align = 6; break;
		case 9: align = 7; break;
	}
	output += IntegerToString(align) + _T(",");

	output += IntegerToString(Margin[0]) + _T(",");
	output += IntegerToString(Margin[1]) + _T(",");
	output += IntegerToString(Margin[2]) + _T(",");
	output += _T("0,");
	output += IntegerToString(encoding);

	return output;
}


/////////
// Clone
AssEntry *AssStyle::Clone() {
	// Create clone
	AssStyle *final = new AssStyle();

	// Copy data
	final->group = group;
	final->StartMS = StartMS;
	final->Valid = Valid;
	final->alignment = alignment;
	final->angle = angle;
	final->bold = bold;
	final->borderstyle = borderstyle;
	final->encoding = encoding;
	final->font = font;
	final->fontsize = fontsize;
	final->italic = italic;
	for (int i=0;i<4;i++) final->Margin[i] = Margin[i];
	final->name = name;
	final->outline = outline;
	final->outline_w = outline_w;
	final->primary = primary;
	final->scalex = scalex;
	final->scaley = scaley;
	final->secondary = secondary;
	final->shadow = shadow;
	final->shadow_w = shadow_w;
	final->spacing = spacing;
	final->strikeout = strikeout;
	final->underline = underline;
	final->relativeTo = relativeTo;
	final->SetEntryData(GetEntryData());

	// Return
	return final;
}
