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

/// @file media_video_frame.h
/// @see media_video_frame.cpp
/// @ingroup video
///

#ifndef LAGI_PRE
#endif

namespace media {

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

	int GetBpp() const { return 4; };
};

} // namespace media
