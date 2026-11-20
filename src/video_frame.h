// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

class wxImage;

/// Memory holding a frame's pixels
class FrameStorage {
public:
	virtual ~FrameStorage() = default;
	virtual unsigned char *Data() const = 0;
	virtual size_t Capacity() const = 0;
};

/// Allocates storage of at least the given size, or returns null to use
/// ordinary memory instead
using FrameAllocator = std::function<std::unique_ptr<FrameStorage>(size_t)>;

/// A frame's pixels. This works like a std::vector<unsigned char>, but the
/// memory can come from an allocator, which is used to share frames with
/// sandboxed subtitle renderers so that they don't have to be copied. The
/// allocator stays with the buffer it was set on, and isn't copied or moved
/// along with the contents.
class FrameBuffer {
	std::unique_ptr<FrameStorage> storage;
	size_t used = 0;
	FrameAllocator allocator;

	std::unique_ptr<FrameStorage> NewStorage(size_t size) const;

public:
	FrameBuffer() = default;
	FrameBuffer(FrameBuffer const& other);
	FrameBuffer(FrameBuffer&& other) noexcept;
	FrameBuffer& operator=(FrameBuffer const& other);
	FrameBuffer& operator=(FrameBuffer&& other) noexcept;

	/// Use allocator for any memory allocated from now on
	void SetAllocator(FrameAllocator allocator) { this->allocator = std::move(allocator); }
	/// The memory holding the contents, if any
	FrameStorage *Storage() const { return storage.get(); }

	unsigned char *data() { return storage ? storage->Data() : nullptr; }
	const unsigned char *data() const { return storage ? storage->Data() : nullptr; }
	size_t size() const { return used; }
	bool empty() const { return used == 0; }
	unsigned char& operator[](size_t i) { return data()[i]; }
	const unsigned char& operator[](size_t i) const { return data()[i]; }

	void assign(const unsigned char *first, const unsigned char *last);
	/// Resize, keeping the existing contents and setting any new bytes to value
	void resize(size_t size, unsigned char value = 0);
};

struct VideoFrame {
	FrameBuffer data;
	int width;
	int height;
	int pitch;
	bool flipped;
};

wxImage GetImage(VideoFrame const& frame);
wxImage GetImageWithAlpha(VideoFrame const& frame);
