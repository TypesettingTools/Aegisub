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

#ifndef AGI_PRE
#include <wx/image.h>
#endif

/// DOCME
/// @class AegiVideoFrame
/// @brief DOCME
///
/// DOCME
class AegiVideoFrame {
	/// Whether the object owns its buffer. If this is false, **data should never be modified
	bool ownMem;
	/// @brief Reset values to the defaults
	///
	/// Note that this function DOES NOT deallocate memory.
	/// Use Clear() for that
	void Reset();

public:
	/// @brief Allocate memory if needed
	void Allocate();

	/// The size in bytes of the frame buffer
	unsigned int memSize;

	/// Pointer to the data planes
	unsigned char *data;

	/// Width in pixels
	unsigned int w;

	/// Height in pixels
	unsigned int h;

	// Pitch, that is, the number of bytes used by each row.
	unsigned int pitch;

	/// First row is actually the bottom one
	bool flipped;

	/// Swap Red and Blue channels (controls RGB versus BGR ordering etc)
	bool invertChannels;

	AegiVideoFrame();
	AegiVideoFrame(unsigned int width, unsigned int height);

	// @brief Clear this frame, freeing its memory if nessesary
	void Clear();

	/// @brief Copy from an AegiVideoFrame
	/// @param source The frame to copy from
	void CopyFrom(const AegiVideoFrame &source);

	/// @brief Set the frame to an externally allocated block of memory
	/// @param source Target frame data
	/// @param width The frame width in pixels
	/// @param height The frame height in pixels
	/// @param pitch The frame's pitch
	/// @param format The frame's format
	void SetTo(const unsigned char *source, unsigned int width, unsigned int height, unsigned int pitch);

	/// @brief Get this frame as a wxImage
	wxImage GetImage() const;
	int GetBpp() const { return 4; };
};
