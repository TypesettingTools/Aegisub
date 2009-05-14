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
#include "config.h"

#include <wx/tokenzr.h>
#include <wx/intl.h>
#include "ass_style.h"
#include "utils.h"
#include <ctype.h>

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
// Parse from SSA/ASS
void AssColor::Parse(const wxString value) {
	if (value.Len() > 0 && value[0] == _T('#')) {
		// HTML colour
		SetWXColor(wxColor(value));
		return;
	}

	// Prepare
	char c,ostr[12];
	unsigned long outval;
	int oindex=11;
	bool ishex=false;

	ostr[11]=0;

	for(int i=value.Len()-1;i>=0&&oindex>=0;i--) {
		c=value[i];
		if (isxdigit(c) || c=='-') {
			ostr[--oindex] = c;
			if (c>='A') ishex = true;
		}
		else if (c == 'H' || c == 'h') ishex = true;
	}
	
	outval=strtoul(ostr+oindex,0,ishex?16:10);

	r = outval		& 0xFF;
	g = (outval>>8)	& 0xFF;
	b = (outval>>16)& 0xFF;
	a = (outval>>24)& 0xFF;
}


///////////////////
// Gets a wxColour
wxColour AssColor::GetWXColor() {
	return wxColour(r,g,b,255-a);
}


//////////////////////
// Sets color from wx
void AssColor::SetWXColor(const wxColor &color) {
	r = color.Red();
	g = color.Green();
	b = color.Blue();
	//a = color.Alpha();
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
	long color = (a<<24)+(b<<16)+(g<<8)+r;
	wxString output=wxString::Format(_T("%i"),(long)color);
	return output;
}


/////////////
// Operators
bool AssColor::operator==(AssColor &col) const {
	return r==col.r && g==col.g && b==col.b && a==col.a;
}
bool AssColor::operator!=(AssColor &col) const {
	return r!=col.r || g!=col.g || b!=col.b || a!=col.a;
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
	secondary.g = 0;
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
	encoding = 1;
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
	wxStringTokenizer tkn(rawData.Trim(false).Mid(6),_T(","),wxTOKEN_RET_EMPTY_ALL);

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
		primary.Parse(tkn.GetNextToken());

		// Read secondary color
		if (!tkn.HasMoreTokens()) return false;
		secondary.Parse(tkn.GetNextToken());

		// Read outline color
		if (!tkn.HasMoreTokens()) return false;
		outline.Parse(tkn.GetNextToken());

		// Read shadow color
		if (!tkn.HasMoreTokens()) return false;
		shadow.Parse(tkn.GetNextToken());
	}

	else {
		// Read primary color
		if (!tkn.HasMoreTokens()) return false;
		primary.Parse(tkn.GetNextToken());

		// Read secondary color
		if (!tkn.HasMoreTokens()) return false;
		secondary.Parse(tkn.GetNextToken());

		// Read and discard tertiary color
		if (!tkn.HasMoreTokens()) return false;
		tkn.GetNextToken();

		// Read shadow/outline color
		if (!tkn.HasMoreTokens()) return false;
		outline.Parse(tkn.GetNextToken());
		shadow = outline;
	}

	// Read bold
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();
	temp.ToLong(&templ);
	bold = (templ==0)?false:true;

	// Read italics
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken();	temp.ToLong(&templ);
	italic = (templ==0)?false:true;

	if (version != 0) {
		// Read underline
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToLong(&templ);
		underline = (templ==0)?false:true;

		// Read strikeout
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
		temp.ToLong(&templ);
		strikeout = (templ==0)?false:true;
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
		//shadow.a = 128; //Parsed
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


///////////////////////////////////////
// Writes data back to ASS (v4+) format
void AssStyle::UpdateData() {
	wxString final;

	name.Replace(_T(","),_T(";"));
	font.Replace(_T(","),_T(";"));


	final = wxString::Format(_T("Style: %s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%s,%s,%s,%s,%d,%s,%s,%i,%i,%i,%i,%i"),
					  name.c_str(), font.c_str(), PrettyFloatD(fontsize).c_str(),
					  primary.GetASSFormatted(true,false,true).c_str(),
					  secondary.GetASSFormatted(true,false,true).c_str(),
					  outline.GetASSFormatted(true,false,true).c_str(),
					  shadow.GetASSFormatted(true,false,true).c_str(),
					  (bold? -1 : 0), (italic ? -1 : 0),
					  (underline?-1:0),(strikeout?-1:0),
					  PrettyFloatD(scalex).c_str(),PrettyFloatD(scaley).c_str(),PrettyFloatD(spacing).c_str(),PrettyFloatD(angle).c_str(),
					  borderstyle,PrettyFloatD(outline_w).c_str(),PrettyFloatD(shadow_w).c_str(),alignment,
					  Margin[0],Margin[1],Margin[2],encoding);

	SetEntryData(final);
}


/////////////////////////////
// Sets margin from a string
void AssStyle::SetMarginString(const wxString str,int which) {
	if (which < 0 || which >= 4) throw _T("Invalid margin id");
	if (!str.IsNumber()) throw _T("Invalid margin value");
	long value;
	str.ToLong(&value);
	if (value < 0) value = 0;
	else if (value > 9999) value = 9999;
	
	Margin[which] = value;
}


//////////////////////////
// Gets string for margin
wxString AssStyle::GetMarginString(int which) {
	if (which < 0 || which >= 4) throw _T("Invalid margin id");
	wxString result = wxString::Format(_T("%04i"),Margin[which]);
	return result;
}


///////////////////////////////
// Convert style to SSA string
wxString AssStyle::GetSSAText() {
	wxString output;
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
	name.Replace(_T(","),_T(";"));
	font.Replace(_T(","),_T(";"));

	output = wxString::Format(_T("Style: %s,%s,%s,%s,%s,0,%s,%d,%d,%d,%s,%s,%d,%d,%d,%d,0,%i"),
				  name.c_str(), font.c_str(), PrettyFloatD(fontsize).c_str(),
				  primary.GetSSAFormatted().c_str(),
				  secondary.GetSSAFormatted().c_str(),
				  shadow.GetSSAFormatted().c_str(),
				  (bold? -1 : 0), (italic ? -1 : 0),
				  borderstyle,PrettyFloatD(outline_w).c_str(),PrettyFloatD(shadow_w).c_str(),align,
				  Margin[0],Margin[1],Margin[2],encoding);

	return output;
}


/////////
// Clone
AssEntry *AssStyle::Clone() {
	// Create clone
	AssStyle *final = new AssStyle();

	// Copy data
	final->group = group;
	final->SetStartMS(GetStartMS());
	final->Valid = Valid;
	final->alignment = alignment;
	final->angle = angle;
	final->bold = bold;
	final->borderstyle = borderstyle;
	final->encoding = encoding;
	final->font = font;
	final->fontsize = fontsize;
	final->italic = italic;
	final->Margin[0] = Margin[0];
	final->Margin[1] = Margin[1];
	final->Margin[2] = Margin[2];
	final->Margin[3] = Margin[3];
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


///////////////////////////
// Equal to another style?
bool AssStyle::IsEqualTo(AssStyle *style) {
	// memcmp won't work because strings won't match
	if (style->alignment != alignment || 
		style->angle != angle ||
		style->bold != bold ||
		style->borderstyle != borderstyle ||
		style->encoding != encoding ||
		style->font != font ||
		style->fontsize != fontsize ||
		style->italic != italic ||
		style->Margin[0] != Margin[0] ||
		style->Margin[1] != Margin[1] ||
		style->Margin[2] != Margin[2] ||
		style->Margin[3] != Margin[3] ||
		style->name != name ||
		style->outline != outline ||
		style->outline_w != outline_w ||
		style->primary != primary ||
		style->scalex != scalex ||
		style->scaley != scaley ||
		style->secondary != secondary ||
		style->shadow != shadow ||
		style->shadow_w != shadow_w ||
		style->spacing != spacing ||
		style->strikeout != strikeout ||
		style->underline != underline ||
		style->relativeTo != relativeTo)
		return false;

	else return true;
}


/////////////////////////////////////
// Get a list of valid ASS encodings
void AssStyle::GetEncodings(wxArrayString &encodingStrings) {
	encodingStrings.Clear();
	encodingStrings.Add(wxString(_T("0 - ")) + _("ANSI"));
	encodingStrings.Add(wxString(_T("1 - ")) + _("Default"));
	encodingStrings.Add(wxString(_T("2 - ")) + _("Symbol"));
	encodingStrings.Add(wxString(_T("77 - ")) + _("Mac"));
	encodingStrings.Add(wxString(_T("128 - ")) + _("Shift_JIS"));
	encodingStrings.Add(wxString(_T("129 - ")) + _("Hangeul"));
	encodingStrings.Add(wxString(_T("130 - ")) + _("Johab"));
	encodingStrings.Add(wxString(_T("134 - ")) + _("GB2312"));
	encodingStrings.Add(wxString(_T("136 - ")) + _("Chinese BIG5"));
	encodingStrings.Add(wxString(_T("161 - ")) + _("Greek"));
	encodingStrings.Add(wxString(_T("162 - ")) + _("Turkish"));
	encodingStrings.Add(wxString(_T("163 - ")) + _("Vietnamese"));
	encodingStrings.Add(wxString(_T("177 - ")) + _("Hebrew"));
	encodingStrings.Add(wxString(_T("178 - ")) + _("Arabic"));
	encodingStrings.Add(wxString(_T("186 - ")) + _("Baltic"));
	encodingStrings.Add(wxString(_T("204 - ")) + _("Russian"));
	encodingStrings.Add(wxString(_T("222 - ")) + _("Thai"));
	encodingStrings.Add(wxString(_T("238 - ")) + _("East European"));
	encodingStrings.Add(wxString(_T("255 - ")) + _("OEM"));
}
