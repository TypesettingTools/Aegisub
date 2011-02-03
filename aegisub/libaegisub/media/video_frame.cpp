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

//#include "utils.h"
#include "libaegisub/media_video_frame.h"

namespace agi {
	namespace media {

void AegiVideoFrame::Reset() {
	// Zero variables
	data = 0;
	pitch = 0;
	memSize = 0;
	w = 0;
	h = 0;

	// Set properties
	flipped = false;
	invertChannels = true;
	ownMem = true;
}

AegiVideoFrame::AegiVideoFrame() {
	Reset();
}

/// @brief Create a solid black frame of the request size and format
/// @param width  
/// @param height 
AegiVideoFrame::AegiVideoFrame(unsigned int width, unsigned int height) {
	assert(width  > 0 && width  < 10000);
	assert(height > 0 && height < 10000);

	Reset();

	// Set format
	w = width;
	h = height;
	pitch = w * GetBpp();

	Allocate();
	memset(data, 0, pitch * height);
}

void AegiVideoFrame::Allocate() {
	assert(pitch > 0 && pitch < 10000);
	assert(w     > 0 && w     < 10000);
	assert(h     > 0 && h     < 10000);

	unsigned int size = pitch * h;

	// Reallocate, if necessary
	if (memSize != size || !ownMem) {
		if (ownMem) {
			delete[] data;
		}
		data = new unsigned char[size];
		memSize = size;
	}

	ownMem = true;
}

void AegiVideoFrame::Clear() {
	if (ownMem) delete[] data;
	Reset();
}

void AegiVideoFrame::CopyFrom(const AegiVideoFrame &source) {
	w = source.w;
	h = source.h;
	pitch = source.pitch;
	Allocate();
	memcpy(data, source.data, memSize);
	flipped = source.flipped;
	invertChannels = source.invertChannels;
}

void AegiVideoFrame::SetTo(const unsigned char *source, unsigned int width, unsigned int height, unsigned int pitch) {
	assert(pitch  > 0 && pitch  < 10000);
	assert(width  > 0 && width  < 10000);
	assert(height > 0 && height < 10000);

	ownMem = false;
	w = width;
	h = height;
	// Note that despite this cast, the contents of data should still never be modified
	data = const_cast<unsigned char*>(source);
	this->pitch = pitch;
}

	} // namespace media
} // namespace agi
