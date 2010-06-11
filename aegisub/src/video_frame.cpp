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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_frame.cpp
/// @brief Wrapper around a frame of video data
/// @ingroup video
///

#include "config.h"

#include "utils.h"
#include "video_frame.h"


/// @brief Reset values to the defaults
///
/// Note that this function DOES NOT unallocate memory.
/// Use Clear() for that
void AegiVideoFrame::Reset() {
	// Zero variables
	for (int i=0;i<4;i++) {
		data[i] = NULL;
		pitch[i] = 0;
	}
	memSize = 0;
	w = 0;
	h = 0;

	// Set properties
	format = FORMAT_NONE;
	flipped = false;
	invertChannels = true;
	ownMem = true;
}

/// @brief Constructor 
AegiVideoFrame::AegiVideoFrame() {
	Reset();
}

/// @brief Create a solid black frame of the request size and format
/// @param width  
/// @param height 
/// @param fmt    
AegiVideoFrame::AegiVideoFrame(int width,int height,VideoFrameFormat fmt) {
	Reset();

	// Set format
	format = fmt;
	w = width;
	h = height;
	pitch[0] = w * GetBpp();
	if (fmt == FORMAT_YV12) {
		pitch[1] = w/2;
		pitch[2] = w/2;
	}

	Allocate();

	// Clear data
	int size = pitch[0]*height + (pitch[1]+pitch[2])*height/2;
	memset(data[0],0,size);
}

/// @brief Allocate memory if needed
void AegiVideoFrame::Allocate() {
	// Check for sanity
	wxASSERT(pitch[0] > 0 && pitch[0] < 10000);
	wxASSERT(w > 0 && w < 10000);
	wxASSERT(h > 0 && h < 10000);
	wxASSERT(format != FORMAT_NONE);

	// Get size
	int height = h;
	unsigned int size;
	if (format == FORMAT_YV12) {
		wxASSERT(pitch[1] > 0 && pitch[1] < 10000);
		wxASSERT(pitch[2] > 0 && pitch[2] < 10000);
		size = pitch[0]*height + (pitch[1]+pitch[2])*height/2;
	}
	else size = pitch[0] * height;

	// Reallocate, if necessary
	if (memSize != size || !ownMem) {
		if (data[0] && ownMem) {
			delete[] data[0];
		}
		data[0] = new unsigned char[size];
		for (int i=1;i<4;i++) data[i] = NULL;
		memSize = size;

		// Planar
		if (format == FORMAT_YV12) {
			data[1] = data[0] + (pitch[0]*height);
			data[2] = data[0] + (pitch[0]*height+pitch[1]*height/2);
		}
	}

	ownMem = true;
}

/// @brief Clear 
void AegiVideoFrame::Clear() {
	if (ownMem) delete[] data[0];

	Reset();
}

/// @brief Copy from an AegiVideoFrame
/// @param source The frame to copy from
void AegiVideoFrame::CopyFrom(const AegiVideoFrame &source) {
	w = source.w;
	h = source.h;
	format = source.format;
	for (int i=0;i<4;i++) pitch[i] = source.pitch[i];
	Allocate();
	memcpy(data[0],source.data[0],memSize);
	flipped = source.flipped;
	invertChannels = source.invertChannels;
}

/// @brief Set the frame to an externally allocated block of memory
/// @param source Target frame data
/// @param width The frame width in pixels
/// @param height The frame height in pixels
/// @param pitch The frame's pitch
/// @param format The fram'e format
void AegiVideoFrame::SetTo(const unsigned char *const source[], int width, int height, const int pitch[4], VideoFrameFormat format) {
	wxASSERT(pitch[0] > 0 && pitch[0] < 10000);
	wxASSERT(width    > 0 && width    < 10000);
	wxASSERT(height   > 0 && height   < 10000);
	wxASSERT(format != FORMAT_NONE);

	ownMem = false;
	w = width;
	h = height;
	this->format = format;

	for (int i = 0; i < 4; i++) {
		// Note that despite this cast, the contents of data should still never be modified
		data[i] = const_cast<unsigned char*>(source[i]);
		this->pitch[i] = pitch[i];
	}
	
}

/// @brief Get wxImage 
/// @return 
wxImage AegiVideoFrame::GetImage() const {
	if (format == FORMAT_RGB32 || format == FORMAT_RGB24) {
		// Create
		unsigned char *buf = (unsigned char*)malloc(w*h*3);
		if (!buf) throw std::bad_alloc();
		const unsigned char *src = data[0];
		unsigned char *dst = buf;

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
	else if (format == FORMAT_YV12) {
		AegiVideoFrame temp;
		temp.ConvertFrom(*this);
		return temp.GetImage();
	}
	else {
		return wxImage(w,h);
	}
}

/// @brief Get bytes per pixel for the current frame format
/// @param plane 
/// @return 
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

/// @brief Convert from another frame 
/// @param source The frame to convert from
/// @param newFormat The format to convert to
void AegiVideoFrame::ConvertFrom(const AegiVideoFrame &source, VideoFrameFormat newFormat) {
	if (newFormat     != FORMAT_RGB32) throw _T("AegiVideoFrame::ConvertFrom: Unsupported destination format.");
	if (source.format != FORMAT_YV12)  throw _T("AegiVideoFrame::ConvertFrom: Unsupported source format.");

	w = source.w;
	h = source.h;
	format = newFormat;
	pitch[0] = w * 4;

	Allocate();

	// Set up pointers
	const unsigned char *src_y = source.data[0];
	const unsigned char *src_u = source.data[1];
	const unsigned char *src_v = source.data[2];
	unsigned char *dst = data[0];

	// Set up pitches
	const int src_delta1 = source.pitch[0]-w;
	const int src_delta2 = source.pitch[1]-w/2;
	const int src_delta3 = source.pitch[2]-w/2;
	const int dst_delta = pitch[0]-w*4;
	register int y,u,v;

	// Loop
	for (unsigned int py=0;py<h;py++) {
		for (unsigned int px=0;px<w/2;px++) {
			u = *src_u++ - 128;
			v = *src_v++ - 128;
			for (unsigned int i=0;i<2;i++) {
				y = (*src_y++ - 16) * 298;

				// Assign
				*dst++ = ClampSignedInteger32((y + 516 * u + 128) >> 8,0,255);				// Blue
				*dst++ = ClampSignedInteger32((y - 100 * u - 208 * v + 128) >> 8,0,255);	// Green
				*dst++ = ClampSignedInteger32((y + 409 * v + 128) >> 8,0,255);				// Red
				*dst++ = 0;
			}
		}

		// Increase pointers
		src_y += src_delta1;
		src_u += src_delta2;
		src_v += src_delta3;
		dst += dst_delta;

		// Roll back u/v on even lines
		if (!(py & 1)) {
			src_u -= source.pitch[1];
			src_v -= source.pitch[2];
		}
	}
}
