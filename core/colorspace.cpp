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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#include "colorspace.h"
#include "utils.h"


// matrix from http://forum.doom9.org/showthread.php?p=684080#post684080
void yuv_to_rgb(int Y, int U, int V, unsigned char *R, unsigned char *G, unsigned char *B)
{
	U = U - 128;
	V = V - 128;
	*R = clip_colorval((Y*65536                        + int(1.140*65536) * V) / 65536);
	*G = clip_colorval((Y*65536 - int(0.395*65536) * U - int(0.581*65536) * V) / 65536);
	*B = clip_colorval((Y*65536 + int(2.032*65536) * U                       ) / 65536);
}


// algorithm from http://130.113.54.154/~monger/hsl-rgb.html
// making every value into 0..255 range though
void hsl_to_rgb(int H, int S, int L, unsigned char *R, unsigned char *G, unsigned char *B)
{
	if (S == 0) {
		*R = L;
		*G = L;
		*B = L;
		return;
	}

	int r, g, b;

	int temp2;
	if (L < 128) {
		temp2 = L * (256 + S) / 256;
	} else {
		temp2 = L + S - L * S / 256;
	}

	int temp1 = 2*L - temp2;

	int temp3[3];
	temp3[0] = H + 256/3;
	if (temp3[0] < 0) temp3[0] += 256;
	if (temp3[0] > 256) temp3[0] -= 256;
	temp3[1] = H;
	temp3[2] = H - 256/3;
	if (temp3[2] < 0) temp3[2] += 256;
	if (temp3[2] > 256) temp3[2] -= 256;

	if (6 * temp3[0] < 255)
		r = temp1 + (temp2 - temp1) * 6 * temp3[0] / 256;
	else if (2 * temp3[0] < 255)
		r = temp2;
	else if (3 * temp3[0] < 511)
		r = temp1 + (temp2 - temp1) * ((512/3) - temp3[0]) * 6 / 256;
	else
		r = temp1;

	if (6 * temp3[1] < 255)
		g = temp1 + (temp2 - temp1) * 6 * temp3[1] / 256;
	else if (2 * temp3[1] < 255)
		g = temp2;
	else if (3 * temp3[1] < 511)
		g = temp1 + (temp2 - temp1) * ((512/3) - temp3[1]) * 6 / 256;
	else
		g = temp1;

	if (6 * temp3[2] < 255)
		b = temp1 + (temp2 - temp1) * 6 * temp3[2] / 256;
	else if (2 * temp3[2] < 255)
		b = temp2;
	else if (3 * temp3[2] < 511)
		b = temp1 + (temp2 - temp1) * ((512/3) - temp3[2]) * 6 / 256;
	else
		b = temp1;

	*R = clip_colorval(r);
	*G = clip_colorval(g);
	*B = clip_colorval(b);
}


// formulas taken from wikipedia: http://en.wikipedia.org/wiki/HSV_color_space
// the range for H is 0..255 instead of 0..359, so 60 degrees has been translated to 256/6 here
void hsv_to_rgb(int H, int S, int V, unsigned char *R, unsigned char *G, unsigned char *B)
{
	*R = *G = *B = 0;

	// Cap values
	unsigned int h = H * 360;
	unsigned int s = clip_colorval(S)*256;
	unsigned int v = clip_colorval(V)*256;

	// Saturation is zero, make grey
	if (S == 0) {
		*R = V;
		*G = V;
		*B = V;
		return;
	}

	unsigned int r, g, b;

	// Else, calculate color
	unsigned int Hi = h / 60 / 256;
	unsigned int f  = h / 60 - Hi * 256;
	unsigned int p  = v * (65535 - s) / 65536;
	unsigned int q  = v * (65535 - (f * s)/256) / 65536;
	unsigned int t  = v * (65535 - ((255 - f) * s)/256) / 65536;
	switch (Hi) {
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
			r = v;
			g = p;
			b = q;
			break;
	}

	*R = clip_colorval(r/256);
	*G = clip_colorval(g/256);
	*B = clip_colorval(b/256);
}


// matrix from http://forum.doom9.org/showthread.php?p=684080#post684080
void rgb_to_yuv(int R, int G, int B, unsigned char *Y, unsigned char *U, unsigned char *V)
{
	*Y = clip_colorval(( int(0.299*65536) * R + int(0.587*65536) * G + int(0.114*65536) * B) / 65536);
	*U = clip_colorval((-int(0.147*65536) * R - int(0.289*65536) * G + int(0.436*65536) * B) / 65536 + 128);
	*V = clip_colorval(( int(0.615*65536) * R - int(0.515*65536) * G - int(0.100*65536) * B) / 65536 + 128);
}


// also from http://130.113.54.154/~monger/hsl-rgb.html
// still keeping everything integer
void rgb_to_hsl(int R, int G, int B, unsigned char *H, unsigned char *S, unsigned char *L)
{
	/*
	int maxcolor = MAX(R, MAX(G, B));
	int mincolor = MIN(R, MIN(G, B));

	*L = (maxcolor + mincolor) / 2;

	if (mincolor == maxcolor) {
		*S = 0;
		*H = 0;
		return;
	}

	if (*L < 128)
		*S = (maxcolor - mincolor) * 256 / (maxcolor + mincolor);
	else
		*S = (maxcolor - mincolor) * 256 / (512 - maxcolor - mincolor);

	int h;
	if (R == maxcolor)
		h = (G - B) * 256 / (maxcolor - mincolor);
	else if (G == maxcolor)
		h = 512 + (B - R) / (maxcolor - mincolor);
	else // if (B == maxcolor)
		h = 1024 + (R - G) / (maxcolor - mincolor);

	h /= 6;
	if (h < 0)
		h += 256;
	*H = h;
	*/

	float r = R/255.f, g = G/255.f, b = B/255.f;
	float h, s, l;

	float maxrgb = MAX(r, MAX(g, b)), minrgb = MIN(r, MIN(g, b));

	l = (minrgb + maxrgb) / 2;

	if (minrgb == maxrgb) {
		h = 0;
		s = 0;
	} else {
		if (l < 0.5) {
			s = (maxrgb - minrgb) / (maxrgb + minrgb);
		} else {
			s = (maxrgb - minrgb) / (2.f - maxrgb - minrgb);
		}
		if (r == maxrgb) {
			h = (g-b) / (maxrgb-minrgb) + 0;
		} else if (g == maxrgb) {
			h = (b-r) / (maxrgb-minrgb) + 2;
		} else { // if b == maxrgb
			h = (r-g) / (maxrgb-minrgb) + 4;
		}
	}

	if (h < 0) h += 6;
	if (h >= 6) h -= 6;

	*H = clip_colorval(int(h*256/6));
	*S = clip_colorval(int(s*255));
	*L = clip_colorval(int(l*255));
}


// formulas from http://en.wikipedia.org/wiki/HSV_color_space
void rgb_to_hsv(int R, int G, int B, unsigned char *H, unsigned char *S, unsigned char *V)
{
	/*
	int maxrgb = MAX(R, MAX(R, B)), minrgb = MIN(R, MIN(G, B));

	*V = clip_colorval(maxrgb);

	if (maxrgb == 0) {
		*S = 0;
	} else {
		*S = clip_colorval((maxrgb - minrgb) * 256 / maxrgb);
	}

	int h;
	if (minrgb == maxrgb) {
		h = 0;
	} else if (R == maxrgb) {
		h = (G - B) * 256 / (maxrgb - minrgb) * 256 / 6;
	} else if (G == maxrgb) {
		h = (B - R) * 256 / (maxrgb - minrgb) * 256 / 6 + 256/3;
	} else { // if B == maxrgb
		h = (R - G) * 256 / (maxrgb - minrgb) * 256 / 6 + 256*2/3;
	}

	if (h < 0) h += 256;
	if (h > 255) h -= 256;
	*H = clip_colorval(h);
	*/

	float r = R/255.f, g = G/255.f, b = B/255.f;
	float h, s, v;

	float maxrgb = MAX(r, MAX(g, b)), minrgb = MIN(r, MIN(g, b));

	v = maxrgb;

	if (maxrgb < .001f) {
		s = 1;
	} else {
		s = (maxrgb - minrgb) / maxrgb;
	}

	if (minrgb == maxrgb) {
		h = 0;
	} else if (maxrgb == r) {
		h = (g-b) / (maxrgb-minrgb) + 0;
	} else if (maxrgb == g) {
		h = (b-r) / (maxrgb-minrgb) + 2;
	} else { // if maxrgb == b
		h = (r-g) / (maxrgb-minrgb) + 4;
	}

	if (h < 0) h += 6;
	if (h >= 6) h -= 6;

	*H = clip_colorval(int(h*256/6));
	*S = clip_colorval(int(s*255));
	*V = clip_colorval(int(v*255));
}



wxString color_to_html(wxColour color)
{
	return wxString::Format(_T("#%02X%02X%02X"), color.Red(), color.Green(), color.Blue());
}


wxColour html_to_color(wxString html)
{
	html.Trim(true);
	html.Trim(false);
	if (html.StartsWith(_T("#"))) {
		html.Remove(0, 1);
	}
	if (html.size() == 6) {
		// 8 bit per channel
		long r, g, b;
		wxString sr, sg, sb;
		sr = html.Mid(0, 2);
		sg = html.Mid(2, 2);
		sb = html.Mid(4, 2);
		if (sr.ToLong(&r, 16) && sg.ToLong(&g, 16) && sb.ToLong(&b, 16)) {
			return wxColour(r, g, b);
		} else {
			return wxColour(*wxBLACK);
		}
	} else if (html.size() == 3) {
		// 4 bit per channel
		long r, g, b;
		wxString sr, sg, sb;
		sr = html.Mid(0, 1);
		sg = html.Mid(1, 1);
		sb = html.Mid(2, 1);
		if (sr.ToLong(&r, 16) && sg.ToLong(&g, 16) && sb.ToLong(&b, 16)) {
			return wxColour(r*16+r, g*16+g, b*16+b);
		} else {
			return wxColour(*wxBLACK);
		}
	} else {
		// only care about valid colors
		return wxColour(*wxBLACK);
	}
}

