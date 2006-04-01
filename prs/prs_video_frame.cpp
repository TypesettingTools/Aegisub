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


///////////
// Headers
#include "prs_video_frame.h"


//////////
// Macros
#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef MID
#define MID(a,b,c) MAX(a,MIN(b,c))
#endif


///////////////
// Constructor
PRSVideoFrame::PRSVideoFrame () {
	for (int i=0;i<4;i++) data[i] = 0;
	w = 0;
	h = 0;
	ownData = false;
}


//////////////
// Destructor
PRSVideoFrame::~PRSVideoFrame () {
	if (ownData) {
		for (int i=0;i<4;i++) delete [] data[i];
	}
}


///////////////////////////////////
// Overlay frame on top of another
void PRSVideoFrame::Overlay(PRSVideoFrame *dstFrame,int x,int y,unsigned char alpha,unsigned char blend) {
	// TODO: Colorspace conversion, for now, the function assumes RGB32 on RGB32!

	// Get pointers
	const unsigned char *src;
	unsigned char *dst;

	// Get boundaries
	int srcBpp = 4;
	int dstBpp = 4;
	int srcRowLen = w * srcBpp;
	int dstRowLen = dstFrame->w * dstBpp;
	int dstStarty = MAX(0,y);
	int dstEndy = MIN(y+h,dstFrame->h);
	int height = dstEndy - dstStarty;
	int rowLen = MID(0,w,dstFrame->w - x);

	// Values
	char sc1,sc2,sc3,a,ia;
	char dc1,dc2,dc3,da;

	// Draw each row
	for (int j=0;j<height;j++) {
		src = (const unsigned char *) data[0] + j*srcRowLen;
		dst = (unsigned char *) dstFrame->data[0] + (j+dstStarty)*dstRowLen + x*dstBpp;

		// Draw the row
		for (int i=0;i<rowLen;i++) {
			// Read alpha
			a = *src++;
			da = *dst;
			ia = 255-a;

			// Read colors
			sc1 = *src++;
			dc1 = *(dst+1);
			sc2 = *src++;
			dc2 = *(dst+2);
			sc3 = *src++;
			dc3 = *(dst+3);

			// Write colors
			*dst++ = da;
			*dst++ = (sc1*a + dc1*ia)/255;
			*dst++ = (sc2*a + dc2*ia)/255;
			*dst++ = (sc3*a + dc3*ia)/255;
		}
	}
}
