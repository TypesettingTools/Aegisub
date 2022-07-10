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
#include <utility>

#include "bitmap.h"
#include "default_config.h"

class wxBitmap;
class wxIcon;

wxBitmap libresrc_getimage(const unsigned char* image, size_t size, double scale = 1.0,
                           int dir = 0);
wxIcon libresrc_geticon(const unsigned char* image, size_t size);
#define GETIMAGE(a) libresrc_getimage(a, sizeof(a))
#define GETIMAGEDIR(a, s, d) libresrc_getimage(a, sizeof(a), s, d)
#define GETICON(a) libresrc_geticon(a, sizeof(a))

#define GET_DEFAULT_CONFIG(a) std::make_pair(reinterpret_cast<const char*>(a), sizeof(a))
