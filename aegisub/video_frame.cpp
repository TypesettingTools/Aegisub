// Copyright (c) 2007, Rodrigo Braz Monteiro
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
#include "video_frame.h"


///////////////
// Constructor
AegiVideoFrame::AegiVideoFrame() {
	for (int i=0;i<4;i++) {
		data[i] = NULL;
		pitch[i] = 0;
		memSize[i] = 0;
	}
	w = 0;
	h = 0;
	format = FORMAT_RGB24;
	flipped = false;
	cppAlloc = true;
	invertChannels = true;
}


//////////////////
// Create default
AegiVideoFrame::AegiVideoFrame(int width,int height,VideoFrameFormat fmt) {
	AegiVideoFrame();
	format = fmt;
	w = width;
	h = height;
	pitch[0] = w;

	Allocate();
	for (int i=0;i<4;i++) {
		int height = h;
		if (format == FORMAT_YV12 && i > 0) height/=2;
		int size = pitch[i]*height;
		memset(data[0],0,size);
	}
}


////////////
// Allocate
void AegiVideoFrame::Allocate() {
	for (int i=0;i<4;i++) {
		// Get size
		int height = h;
		if (format == FORMAT_YV12 && i > 0) height/=2;
		unsigned int size = pitch[i]*height;

		// Reallocate, if necessary
		if (memSize[i] != size) {
			if (cppAlloc) delete[] data[i];
			else free(data[i]);
			data[i] = new unsigned char[size];
			memSize[i] = size;
		}
	}

	cppAlloc = true;
}


/////////
// Clear
void AegiVideoFrame::Clear() {
	for (int i=0;i<4;i++) {
		if (data[i]) {
			if (cppAlloc) delete[] data[i];
			else free(data[i]);
			data[i] = NULL;
		}
		pitch[i] = 0;
	}
	w = 0;
	h = 0;
	format = FORMAT_RGB24;
	flipped = false;
	cppAlloc = true;
	invertChannels = true;
}


///////////////
// Create copy
void AegiVideoFrame::CopyFrom(const AegiVideoFrame &source) {
	w = source.w;
	h = source.h;
	format = source.format;
	for (int i=0;i<4;i++) pitch[i] = source.pitch[i];
	Allocate();
	for (int i=0;i<4;i++) {
		memcpy(data[i],source.data[i],memSize[i]);
	}
	flipped = source.flipped;
	invertChannels = source.invertChannels;
}


///////////////
// Get wxImage
// ------
// This function is only used on screenshots, so it doesn't have to be fast
wxImage AegiVideoFrame::GetImage() const {
	if (format == FORMAT_RGB32 || format == FORMAT_RGB24) {
		// Create
		unsigned char *buf = (unsigned char*)malloc(w*h*3);
		const unsigned char *src = data[0];
		unsigned char *dst = buf;

		// Bytes per pixel
		int Bpp = GetBpp();

		// Convert
		for (unsigned int y=0;y<h;y++) {
			dst = buf + y*w*3;
			if (flipped) src = data[0] + (h-y-1)*pitch[0];
			else src = data[0] + y*pitch[0];
			for (unsigned int x=0;x<w;x++) {
				*dst++ = *(src+2);
				*dst++ = *(src+1);
				*dst++ = *(src);
				src += Bpp;
			}
		}

		// Make image
		wxImage img(w,h);
		img.SetData(buf);
		return img;
	}

	else {
		return wxImage(w,h);
	}
}


/////////////////////////////
// Get float luminosity data
void AegiVideoFrame::GetFloat(float *buffer) const {
	int Bpp = GetBpp();
	const unsigned char *src = data[0];
	float *dst = buffer;
	float temp;

	// Convert
	if (format == FORMAT_RGB32 || format == FORMAT_RGB24) {
		int delta = 4-Bpp;
		for (unsigned int y=0;y<h;y++) {
			dst = buffer + y*w;
			if (flipped) src = data[0] + (h-y-1)*pitch[0];	// I think that it requires flipped data - amz
			else src = data[0] + y*pitch[0];
			for (unsigned int x=0;x<w;x++) {
				temp = (*src++)*0.3 + (*src++)*0.4 + (*src++)*0.3;
				src += delta;
				*dst++ = temp;
			}
		}
	}
}


///////////////////////
// Get Bytes per Pixel
int AegiVideoFrame::GetBpp(int plane) const {
	switch (format) {
		case FORMAT_RGB32: return 4;
		case FORMAT_RGB24: return 3;
		case FORMAT_YUY2: return 2;
		case FORMAT_YV12:
			if (plane == 0) return 1;
			else return 0;
		default: return 0;
	}
}
