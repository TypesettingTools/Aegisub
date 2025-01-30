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

#include <cstdlib>
#include <string_view>

struct LibresrcBlob {
	const unsigned char *data;
	size_t size;
	int scale;
};

#include "bitmap.h"
#include "default_config.h"

#include <wx/version.h>

class wxBitmap;
class wxBitmapBundle;
class wxIcon;
class wxIconBundle;

wxBitmap libresrc_getimage(const unsigned char *image, size_t size, int dir=0);
wxIcon libresrc_geticon(const unsigned char *image, size_t size);
#define GETIMAGE(a) libresrc_getimage(a, sizeof(a))

#define GET_DEFAULT_CONFIG(a) std::string_view(reinterpret_cast<const char *>(a), sizeof(a))

// height is the desired displayed height of the bitmap bundle in DIP.
// Having to specify this when constructing the bitmap is slightly awkward,
// but wxWidgets doesn't allow you to scale a wxBitmapBundle after creating it,
// so the scale has to be set for the individual bitmaps before constructing it.
wxBitmapBundle libresrc_getbitmapbundle(const LibresrcBlob *images, size_t count, int height, int dir=0);

wxIconBundle libresrc_geticonbundle(const LibresrcBlob *images, size_t count);

#define GETBUNDLE(a, h) libresrc_getbitmapbundle(a, sizeof(a) / sizeof(*a), h)
#define GETBUNDLEDIR(a, h, d) libresrc_getbitmapbundle(a, sizeof(a) / sizeof(*a), h, d)
#define GETICONS(a) libresrc_geticonbundle(a, sizeof(a) / sizeof(*a))
