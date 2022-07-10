// Copyright (c) 2005, Niels Martin Hansen, Rodrigo Braz Monteiro
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

/// @file colorspace.cpp
/// @brief Functions for converting colours between different representations
/// @ingroup utility
///

#include "colorspace.h"
#include "utils.h"

static inline unsigned int clip_colorval(int val) {
	return mid(0, val, 255);
}

// Algorithm from http://130.113.54.154/~monger/hsl-rgb.html
void hsl_to_rgb(int H, int S, int L, unsigned char* R, unsigned char* G, unsigned char* B) {
	if(S == 0) {
		*R = L;
		*G = L;
		*B = L;
		return;
	}

	if(L == 128 && S == 255) {
		switch(H) {
			case 0:
			case 255: // actually this is wrong, since this is more like 359 degrees... but it's
			          // what you'd expect (sadly :)
				*R = 255;
				*G = 0;
				*B = 0;
				return;
			case 43:
				*R = 255;
				*G = 255;
				*B = 0;
				return;
			case 85:
				*R = 0;
				*G = 255;
				*B = 0;
				return;
			case 128:
				*R = 0;
				*G = 255;
				*B = 255;
				return;
			case 171:
				*R = 0;
				*G = 0;
				*B = 255;
				return;
			case 213:
				*R = 255;
				*G = 0;
				*B = 255;
				return;
		}
	}

	float h, s, l, r, g, b;
	h = H / 255.f;
	s = S / 255.f;
	l = L / 255.f;

	float temp2;
	if(l < .5f) {
		temp2 = l * (1.f + s);
	} else {
		temp2 = l + s - l * s;
	}

	float temp1 = 2.f * l - temp2;

	// assume h is in range [0;1]
	float temp3[3];
	temp3[0] = h + 1.f / 3.f;
	if(temp3[0] > 1.f) temp3[0] -= 1.f;
	temp3[1] = h;
	temp3[2] = h - 1.f / 3.f;
	if(temp3[2] < 0.f) temp3[2] += 1.f;

	if(6.f * temp3[0] < 1.f)
		r = temp1 + (temp2 - temp1) * 6.f * temp3[0];
	else if(2.f * temp3[0] < 1.f)
		r = temp2;
	else if(3.f * temp3[0] < 2.f)
		r = temp1 + (temp2 - temp1) * ((2.f / 3.f) - temp3[0]) * 6.f;
	else
		r = temp1;

	if(6.f * temp3[1] < 1.f)
		g = temp1 + (temp2 - temp1) * 6.f * temp3[1];
	else if(2.f * temp3[1] < 1.f)
		g = temp2;
	else if(3.f * temp3[1] < 2.f)
		g = temp1 + (temp2 - temp1) * ((2.f / 3.f) - temp3[1]) * 6.f;
	else
		g = temp1;

	if(6.f * temp3[2] < 1.f)
		b = temp1 + (temp2 - temp1) * 6.f * temp3[2];
	else if(2.f * temp3[2] < 1.f)
		b = temp2;
	else if(3.f * temp3[2] < 2.f)
		b = temp1 + (temp2 - temp1) * ((2.f / 3.f) - temp3[2]) * 6.f;
	else
		b = temp1;

	*R = clip_colorval((int)(r * 255));
	*G = clip_colorval((int)(g * 255));
	*B = clip_colorval((int)(b * 255));
}

// Formulas taken from wikipedia: http://en.wikipedia.org/wiki/HSV_color_space
// The range for H is 0..255 instead of 0..359, so 60 degrees has been translated to 256/6 here
void hsv_to_rgb(int H, int S, int V, unsigned char* R, unsigned char* G, unsigned char* B) {
	*R = *G = *B = 0;

	// some special cases... oh yeah baby!
	if(S == 255) {
		switch(H) {
			case 0:
			case 255: // actually this is wrong, since this is more like 359 degrees... but it's
			          // what you'd expect (sadly :)
				*R = V;
				*G = 0;
				*B = 0;
				return;
			case 43:
				*R = V;
				*G = V;
				*B = 0;
				return;
			case 85:
				*R = 0;
				*G = V;
				*B = 0;
				return;
			case 128:
				*R = 0;
				*G = V;
				*B = V;
				return;
			case 171:
				*R = 0;
				*G = 0;
				*B = V;
				return;
			case 213:
				*R = V;
				*G = 0;
				*B = V;
				return;
		}
	}

	// Cap values
	unsigned int h = H * 360;
	unsigned int s = clip_colorval(S) * 256;
	unsigned int v = clip_colorval(V) * 256;

	// Saturation is zero, make grey
	if(S == 0) {
		*R = V;
		*G = V;
		*B = V;
		return;
	}

	unsigned int r, g, b;

	// Else, calculate color
	unsigned int Hi = h / 60 / 256;
	unsigned int f = h / 60 - Hi * 256;
	unsigned int p = v * (65535 - s) / 65536;
	unsigned int q = v * (65535 - (f * s) / 256) / 65536;
	unsigned int t = v * (65535 - ((255 - f) * s) / 256) / 65536;
	switch(Hi) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		case 5:
		default:
			r = v;
			g = p;
			b = q;
			break;
	}

	*R = clip_colorval(r / 256);
	*G = clip_colorval(g / 256);
	*B = clip_colorval(b / 256);
}

/// Algorithm from http://130.113.54.154/~monger/hsl-rgb.html
void rgb_to_hsl(int R, int G, int B, unsigned char* H, unsigned char* S, unsigned char* L) {
	float r = R / 255.f, g = G / 255.f, b = B / 255.f;
	float h, s, l;

	float maxrgb = std::max(r, std::max(g, b)), minrgb = std::min(r, std::min(g, b));

	l = (minrgb + maxrgb) / 2;

	if(minrgb == maxrgb) {
		h = 0;
		s = 0;
	} else {
		if(l < .5f) {
			s = (maxrgb - minrgb) / (maxrgb + minrgb);
		} else {
			s = (maxrgb - minrgb) / (2.f - maxrgb - minrgb);
		}
		if(r == maxrgb) {
			h = (g - b) / (maxrgb - minrgb) + 0;
		} else if(g == maxrgb) {
			h = (b - r) / (maxrgb - minrgb) + 2;
		} else { // if b == maxrgb
			h = (r - g) / (maxrgb - minrgb) + 4;
		}
	}

	if(h < 0) h += 6;
	if(h >= 6) h -= 6;

	*H = clip_colorval(int(h * 256 / 6));
	*S = clip_colorval(int(s * 255));
	*L = clip_colorval(int(l * 255));
}

/// Formulas from http://en.wikipedia.org/wiki/HSV_color_space
void rgb_to_hsv(int R, int G, int B, unsigned char* H, unsigned char* S, unsigned char* V) {
	float r = R / 255.f, g = G / 255.f, b = B / 255.f;
	float h, s, v;

	float maxrgb = std::max(r, std::max(g, b)), minrgb = std::min(r, std::min(g, b));

	v = maxrgb;

	if(maxrgb < .001f) {
		s = 1;
	} else {
		s = (maxrgb - minrgb) / maxrgb;
	}

	if(minrgb == maxrgb) {
		h = 0;
	} else if(maxrgb == r) {
		h = (g - b) / (maxrgb - minrgb) + 0;
	} else if(maxrgb == g) {
		h = (b - r) / (maxrgb - minrgb) + 2;
	} else { // if maxrgb == b
		h = (r - g) / (maxrgb - minrgb) + 4;
	}

	if(h < 0) h += 6;
	if(h >= 6) h -= 6;

	*H = clip_colorval(int(h * 256 / 6));
	*S = clip_colorval(int(s * 255));
	*V = clip_colorval(int(v * 255));
}

void hsv_to_hsl(int iH, int iS, int iV, unsigned char* oH, unsigned char* oS, unsigned char* oL) {
	int p = iV * (255 - iS);
	*oH = iH;
	*oL = clip_colorval((p + iV * 255) / 255 / 2);
	if(*oL == 0) {
		*oS = iS; // oS is actually undefined, so any value should work ;)
	} else if(*oL <= 128) {
		*oS = clip_colorval((iV * 255 - p) / (2 * *oL));
	} else {
		*oS = clip_colorval((iV * 255 - p) / (511 - 2 * *oL));
	}
}

void hsl_to_hsv(int iH, int iS, int iL, unsigned char* oH, unsigned char* oS, unsigned char* oV) {
	*oH = iH;

	if(iS == 0) {
		*oS = 0;
		*oV = iL;
		return;
	}

	if(iL < 128) {
		*oV = iL * (255 + iS) / 255;
		*oS = 2 * 255 * iS / (255 + iS);
	} else {
		*oV = (iL * 255 + iS * 255 - iL * iS) / 255;
		*oS = 2 * 255 * iS * (255 - iL) / (iL * 255 + iS * 255 - iL * iS);
	}
}
