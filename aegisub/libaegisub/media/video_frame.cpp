// Copyright (c) 2007, Rodrigo Braz Monteiro <amz@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file video_frame.cpp
/// @brief Wrapper around a frame of video data
/// @ingroup video
///

#include "config.h"

#include "libaegisub/media_video_frame.h"

#ifndef LAGI_PRE
#include <assert.h>
#include <string.h>
#endif

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
