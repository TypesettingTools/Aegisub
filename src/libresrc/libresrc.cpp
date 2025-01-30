// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
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

#include "libresrc.h"

#include <map>

#include <wx/bitmap.h>
#include <wx/bmpbndl.h>
#include <wx/icon.h>
#include <wx/iconbndl.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/mstream.h>

wxBitmap libresrc_getimage(const unsigned char *buff, size_t size, int dir) {
	wxMemoryInputStream mem(buff, size);
	if (dir != wxLayout_RightToLeft)
		return wxBitmap(wxImage(mem));
	return wxBitmap(wxImage(mem).Mirror());
}

wxIcon libresrc_geticon(const unsigned char *buff, size_t size) {
	wxMemoryInputStream mem(buff, size);
	wxIcon icon;
	icon.CopyFromBitmap(wxBitmap(wxImage(mem)));
	return icon;
}

wxBitmapBundle libresrc_getbitmapbundle(const LibresrcBlob *images, size_t count, int height, int dir) {
	// This function should only ever be called on the GUI thread but declaring this thread_local is the safe way
	thread_local std::map<std::tuple<const LibresrcBlob *, int, int>, wxBitmapBundle> cache;
	auto key = std::make_tuple(images, height, dir);

	if (auto cached = cache.find(key); cached != cache.end()) {
		return cached->second;
	}

	wxVector<wxBitmap> bitmaps;
	bitmaps.reserve(count);
	for (size_t i = 0; i < count; i++) {
		bitmaps.push_back(libresrc_getimage(images[i].data, images[i].size, dir));
		bitmaps.back().SetScaleFactor(double(images[i].scale) / height);
	}

	auto bundle = wxBitmapBundle::FromBitmaps(bitmaps);
	cache[key] = bundle;

	return bundle;
}

wxIconBundle libresrc_geticonbundle(const LibresrcBlob *images, size_t count) {
	thread_local std::map<const LibresrcBlob *, wxIconBundle> cache;

	if (auto cached = cache.find(images); cached != cache.end()) {
		return cached->second;
	}

	wxIconBundle bundle;
	for (size_t i = 0; i < count; i++) {
		bundle.AddIcon(libresrc_geticon(images[i].data, images[i].size));
	}

	cache[images] = bundle;

	return bundle;
}
