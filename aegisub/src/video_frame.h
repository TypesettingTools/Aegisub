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

/// @file video_frame.h
/// @see video_frame.cpp
/// @ingroup video
///


#pragma once
#include <wx/image.h>


//////////////////////
// Video Frame format
// All formats use 8 bits per sample.
enum VideoFrameFormat {
	FORMAT_NONE		= 0x0000,
	FORMAT_RGB24	= 0x0001, // RGB, interleaved
	FORMAT_RGB32	= 0x0002, // RGBA, interleaved
	FORMAT_YUY2		= 0x0004, // YCbCr 4:2:2, planar
	FORMAT_YV12		= 0x0008, // YCbCr 4:2:0, planar
	FORMAT_YUV444	= 0x0010, // YCbCr 4:4:4, planar
	FORMAT_YUV444A	= 0x0020, // YCbCr 4:4:4 plus alpha, planar
	FORMAT_YUVMONO	= 0x0040, // Y only (greyscale)
};


/////////////////////
// Video Frame class
class AegiVideoFrame {
private:
	unsigned int memSize;
	void Reset();

public:
	unsigned char *data[4];		// Pointers to the data planes. Interleaved formats only use data[0]
	VideoFrameFormat format;	// Data format
	unsigned int w;				// Width in pixels
	unsigned int h;				// Height in pixels
	unsigned int pitch[4];		// Pitch, that is, the number of bytes used by each row.

	bool flipped;				// First row is actually the bottom one
	bool invertChannels;		// Swap Red and Blue channels or U and V planes (controls RGB versus BGR ordering etc)
	bool cppAlloc;				// Allocated with C++'s "new" operator, instead of "malloc"

	AegiVideoFrame();
	AegiVideoFrame(int width,int height,VideoFrameFormat format=FORMAT_RGB32);

	void Allocate();
	void Clear();
	void CopyFrom(const AegiVideoFrame &source);
	void ConvertFrom(const AegiVideoFrame &source);

	wxImage GetImage() const;
	void GetFloat(float *buffer) const;
	int GetBpp(int plane=0) const;
};

