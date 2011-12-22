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

/// @file ass_style.cpp
/// @brief Class for style definitions in subtitles
/// @ingroup subs_storage
///

#include "config.h"

#ifndef AGI_PRE
#include <ctype.h>

#include <wx/intl.h>
#include <wx/tokenzr.h>
#endif

#include "ass_style.h"
#include "utils.h"

AssColor::AssColor () {
	r=g=b=a=0;
}
AssColor::AssColor(int r, int g, int b, int a)
: r(r)
, g(g)
, b(b)
, a(a)
{
}

/// @brief DOCME
/// @param color 
///
AssColor::AssColor (const wxColour &color) {
	SetWXColor(color);
}

/// @brief Parse from SSA/ASS 
/// @param value 
void AssColor::Parse(const wxString value) {
	if (value.Len() > 0 && value[0] == '#') {
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

	for(unsigned char i = value.Len(); i > 0 && oindex >= 0; i--) {
		c=value[i - 1];
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

/// @brief Gets a wxColour 
/// @return 
wxColour AssColor::GetWXColor() {
	return wxColour(r,g,b,255-a);
}

/// @brief Sets color from wx 
/// @param color 
void AssColor::SetWXColor(const wxColor &color) {
	r = color.Red();
	g = color.Green();
	b = color.Blue();
	//a = color.Alpha();
}

/// @brief Get formatted in ASS format 
/// @param alpha    
/// @param stripped 
/// @param isStyle  
/// @return 
wxString AssColor::GetASSFormatted(bool alpha,bool stripped,bool isStyle) const {
	wxString work;
	if (!stripped) work += "&H";
	if (alpha) work += wxString::Format("%02X",a);
	work += wxString::Format("%02X%02X%02X",b,g,r);
	if (!stripped && !isStyle) work += "&";
	return work;
}

/// @brief Get decimal formatted 
/// @return 
wxString AssColor::GetSSAFormatted() const {
	long color = (a<<24)+(b<<16)+(g<<8)+r;
	wxString output=wxString::Format("%i",(long)color);
	return output;
}

bool AssColor::operator==(const AssColor &col) const {
	return r==col.r && g==col.g && b==col.b && a==col.a;
}

bool AssColor::operator!=(const AssColor &col) const {
	return !(*this == col);
}

AssStyle::AssStyle()
: name("Default")
, font("Arial")
, fontsize(20.)
, primary(255, 255, 255)
, secondary(255, 0, 0)
, outline(0, 0, 0)
, shadow(0, 0, 0)
, bold(false)
, italic(false)
, underline(false)
, strikeout(false)
, scalex(100.)
, scaley(100.)
, spacing(0.)
, angle(0.)
, borderstyle(1)
, outline_w(2.)
, shadow_w(2.)
, alignment(2)
, encoding(1)
, relativeTo(1)
{
	group = "[V4+ Styles]";
	for (int i = 0; i < 4; i++)
		Margin[i] = 10;

	UpdateData();
}

AssStyle::AssStyle(const AssStyle& s)
: AssEntry(s)
, name(s.name)
, font(s.font)
, fontsize(s.fontsize)
, primary(s.primary)
, secondary(s.secondary)
, outline(s.outline)
, shadow(s.shadow)
, bold(s.bold)
, italic(s.italic)
, underline(s.underline)
, strikeout(s.strikeout)
, scalex(s.scalex)
, scaley(s.scaley)
, spacing(s.spacing)
, angle(s.angle)
, borderstyle(s.borderstyle)
, outline_w(s.outline_w)
, shadow_w(s.shadow_w)
, alignment(s.alignment)
, encoding(s.encoding)
, relativeTo(s.relativeTo)
{
	group = "[V4+ Styles]";
	memcpy(Margin, s.Margin, sizeof(Margin));
	SetEntryData(s.GetEntryData());
}

AssStyle::AssStyle(wxString _data,int version) {
	if (!Parse(_data,version))
		throw "[Error] Failed parsing line.";
	UpdateData();
}

/// @brief Parses value from ASS data 
/// @param rawData 
/// @param version 
/// @return 
///
bool AssStyle::Parse(wxString rawData,int version) {
	// Tokenize
	wxString temp;
	long templ;
	wxStringTokenizer tkn(rawData.Trim(false).Mid(6),",",wxTOKEN_RET_EMPTY_ALL);

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

/// @brief Writes data back to ASS (v4+) format 
///
void AssStyle::UpdateData() {
	wxString final;

	name.Replace(",",";");
	font.Replace(",",";");


	final = wxString::Format("Style: %s,%s,%g,%s,%s,%s,%s,%d,%d,%d,%d,%g,%g,%g,%g,%d,%g,%g,%i,%i,%i,%i,%i",
					  name, font, fontsize,
					  primary.GetASSFormatted(true,false,true),
					  secondary.GetASSFormatted(true,false,true),
					  outline.GetASSFormatted(true,false,true),
					  shadow.GetASSFormatted(true,false,true),
					  (bold? -1 : 0), (italic ? -1 : 0),
					  (underline?-1:0),(strikeout?-1:0),
					  scalex,scaley,spacing,angle,
					  borderstyle,outline_w,shadow_w,alignment,
					  Margin[0],Margin[1],Margin[2],encoding);

	SetEntryData(final);
}

/// @brief Sets margin from a string 
/// @param str   
/// @param which 
///
void AssStyle::SetMarginString(const wxString str,int which) {
	if (which < 0 || which >= 4) throw Aegisub::InvalidMarginIdError();
	if (!str.IsNumber()) throw "Invalid margin value";
	long value;
	str.ToLong(&value);
	if (value < 0) value = 0;
	else if (value > 9999) value = 9999;
	
	Margin[which] = value;
}

/// @brief Gets string for margin 
/// @param which 
/// @return 
///
wxString AssStyle::GetMarginString(int which) const {
	if (which < 0 || which >= 4) throw Aegisub::InvalidMarginIdError();
	wxString result = wxString::Format("%04i",Margin[which]);
	return result;
}

/// @brief Convert style to SSA string 
/// @return 
///
wxString AssStyle::GetSSAText() const {
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
	wxString n = name;
	n.Replace(",", ";");
	wxString f = font;
	f.Replace(",", ";");

	output = wxString::Format("Style: %s,%s,%g,%s,%s,0,%s,%d,%d,%d,%g,%g,%d,%d,%d,%d,0,%i",
				  n, f, fontsize,
				  primary.GetSSAFormatted(),
				  secondary.GetSSAFormatted(),
				  shadow.GetSSAFormatted(),
				  (bold? -1 : 0), (italic ? -1 : 0),
				  borderstyle,outline_w,shadow_w,align,
				  Margin[0],Margin[1],Margin[2],encoding);

	return output;
}

/// @brief Clone 
/// @return 
///
AssEntry *AssStyle::Clone() const {
	return new AssStyle(*this);
}



/// @brief Equal to another style? 
/// @param style 
/// @return 
///
bool AssStyle::IsEqualTo(AssStyle *style) const {
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



/// @brief Get a list of valid ASS encodings 
/// @param encodingStrings 
///
void AssStyle::GetEncodings(wxArrayString &encodingStrings) {
	encodingStrings.Clear();
	encodingStrings.Add(wxString("0 - ") + _("ANSI"));
	encodingStrings.Add(wxString("1 - ") + _("Default"));
	encodingStrings.Add(wxString("2 - ") + _("Symbol"));
	encodingStrings.Add(wxString("77 - ") + _("Mac"));
	encodingStrings.Add(wxString("128 - ") + _("Shift_JIS"));
	encodingStrings.Add(wxString("129 - ") + _("Hangeul"));
	encodingStrings.Add(wxString("130 - ") + _("Johab"));
	encodingStrings.Add(wxString("134 - ") + _("GB2312"));
	encodingStrings.Add(wxString("136 - ") + _("Chinese BIG5"));
	encodingStrings.Add(wxString("161 - ") + _("Greek"));
	encodingStrings.Add(wxString("162 - ") + _("Turkish"));
	encodingStrings.Add(wxString("163 - ") + _("Vietnamese"));
	encodingStrings.Add(wxString("177 - ") + _("Hebrew"));
	encodingStrings.Add(wxString("178 - ") + _("Arabic"));
	encodingStrings.Add(wxString("186 - ") + _("Baltic"));
	encodingStrings.Add(wxString("204 - ") + _("Russian"));
	encodingStrings.Add(wxString("222 - ") + _("Thai"));
	encodingStrings.Add(wxString("238 - ") + _("East European"));
	encodingStrings.Add(wxString("255 - ") + _("OEM"));
}


