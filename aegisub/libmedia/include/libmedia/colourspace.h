// Copyright (c) 2005, Niels Martin Hansen
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

/// @file colorspace.h
/// @see colorspace.cpp
/// @ingroup utility
///


#ifndef AGI_PRE
#include <libaegisub/colour.h>
#endif

namespace media {

/// @brief DOCME
/// @param val 
///
inline unsigned int clip_colorval(int val)
{
	if (val < 0) return 0;
	if (val > 255) return 255;
	return val;
}


// Convert an YUV color to RGB; all values are expected to be in range 0..255
void yuv_to_rgb(int Y, int U, int V, unsigned char *R, unsigned char *G, unsigned char *B);

// Convert a HSL color to RGB; all values are expected to be in range 0..255
void hsl_to_rgb(int H, int S, int L, unsigned char *R, unsigned char *G, unsigned char *B);

// Convert a HSV color to RGB; all values are expected to be in range 0..255
void hsv_to_rgb(int H, int S, int V, unsigned char *R, unsigned char *G, unsigned char *B);

// Convert an RGB color to YUV; all values are expected to be in range 0..255
void rgb_to_yuv(int R, int G, int B, unsigned char *Y, unsigned char *U, unsigned char *V);

// Convert an RGB color to HSL; all values are expected to be in range 0..255
void rgb_to_hsl(int R, int G, int B, unsigned char *H, unsigned char *S, unsigned char *L);

// Convert an RGB color to HSV; all values are expected to be in range 0..255
void rgb_to_hsv(int R, int G, int B, unsigned char *H, unsigned char *S, unsigned char *V);

void hsv_to_hsl(int iH, int iS, int iV, unsigned char *oH, unsigned char *oS, unsigned char *oL);

void hsl_to_hsv(int iH, int iS, int iL, unsigned char *oH, unsigned char *oS, unsigned char *oV);


// Convert a wxColour to a HTML hex string
std::string color_to_html(agi::Colour color);

// Convert a HTML hex string to a wxColour
std::string html_to_color(agi::Colour html);

} // namespace media
