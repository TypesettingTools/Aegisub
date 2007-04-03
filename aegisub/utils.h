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


///////////////////////
// Function prototypes
#ifndef __LINUX__
__int64 abs64(__int64 input);
#endif
int CountMatches(wxString parent,wxString child);
bool Copy(wxString src,wxString dst);
bool Backup(wxString src,wxString dst);
wxString MakeRelativePath(wxString path,wxString reference);
wxString DecodeRelativePath(wxString path,wxString reference);
wxString PrettyFloat(wxString src);
wxString PrettyFloatF(float src);
wxString PrettyFloatD(double src);
wxString FloatToString(double value);
wxString IntegerToString(int value);
wxString PrettySize(int bytes);
void AppendBitmapMenuItem (wxMenu* parentMenu,int id,wxString text,wxString help,wxBitmap bmp);
int SmallestPowerOf2(int x);


//////////
// Macros
#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef MID
#define MID(a,b,c) MAX((a),MIN((b),(c)))
#endif

#ifndef FORCEINLINE
#ifdef __VISUALC__
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif
#endif


///////////
// Inlines
inline void IntSwap(int &a,int &b) {
	int c = a;
	a = b;
	b = c;
}


//////////////////////////
// Clamp integer to range
// Code taken from http://bob.allegronetwork.com/prog/tricks.html#clamp
FORCEINLINE int ClampSignedInteger32(int x,int min,int max) {
	x -= min;
	x &= (~x) >> 31;
	x += min;
	x -= max;
	x &= x >> 31;
	x += max;
	return x;
}
