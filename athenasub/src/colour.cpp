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


#include "Athenasub.h"
#include "colour.h"
#include "utils.h"
using namespace Athenasub;


////////////////
// Constructors
Colour::Colour ()
{
	r = g = b = a = 0;
}
Colour::Colour (unsigned char red,unsigned char green,unsigned char blue,unsigned char alpha)
{
	r = red;
	g = green;
	b = blue;
	a = alpha;
}
Colour::Colour (int red,int green,int blue,int alpha)
{
	SetRed(red);
	SetGreen(green);
	SetBlue(blue);
	SetAlpha(alpha);
}


////////////////////////
// Set colour component
void Colour::SetRed(int red) { r = (unsigned char) Mid(0,red,255); }
void Colour::SetGreen(int green) { g = (unsigned char) Mid(0,green,255); }
void Colour::SetBlue(int blue) { b = (unsigned char) Mid(0,blue,255); }
void Colour::SetAlpha(int alpha) { a = (unsigned char) Mid(0,alpha,255); }


//////////////
// Parse text
void Colour::Parse(String value,bool reverse)
{
	// Prepare
	unsigned char c;
	char ostr[12];
	ostr[11]=0;
	unsigned long outval;
	int oindex=11;
	bool ishex=false;

	// Determines whether it is hexadecimal or decimal
	for(int i=(int)value.Length()-1;i>=0&&oindex>=0;i--) {
		c=(char) value[i];
		if (isxdigit(c) || c=='-') {
			ostr[--oindex] = c;
			if (c>='A') ishex = true;
		}
		else if (c == 'H' || c == 'h') ishex = true;
	}

	// Convert to decimal
	outval=strtoul(ostr+oindex,0,ishex?16:10);

	// Split components
	b = (unsigned char)(outval		& 0xFF);
	g = (unsigned char)((outval>>8)	& 0xFF);
	r = (unsigned char)((outval>>16)& 0xFF);
	a = (unsigned char)((outval>>24)& 0xFF);

	// Reverse
	if (reverse) {
		unsigned char aux = r;
		r = b;
		b = aux;
	}
}


/////////////////////////////
// Generate Visual Basic hex
String Colour::GetVBHex(bool withAlpha,bool withHeader,bool withFooter) const
{
	String work;
	if (withHeader) work += "&H";
	if (withAlpha) work += wxString::Format(_T("%02X"),a);
	work += wxString::Format(_T("%02X%02X%02X"),b,g,r);
	if (withFooter) work += "&";
	return work;
}
