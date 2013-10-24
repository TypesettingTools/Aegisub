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

#include "../config.h"

#include "libaegisub/ass/uuencode.h"

// Despite being called uuencoding by ass_specs.doc, the format is actually
// somewhat different from real uuencoding.  Each 3-byte chunk is split into 4
// 6-bit pieces, then 33 is added to each piece. Lines are wrapped after 80
// characters, and files with non-multiple-of-three lengths are padded with
// zero.

namespace agi { namespace ass {

std::string UUEncode(std::vector<char> const& data) {
	std::string ret;
	ret.reserve((data.size() * 4 + 2) / 3 + data.size() / 80 * 2);

	size_t written = 0;
	for (size_t pos = 0; pos < data.size(); pos += 3) {
		unsigned char src[3] = { '\0', '\0', '\0' };
		memcpy(src, &data[pos], std::min<size_t>(3u, data.size() - pos));

		unsigned char dst[4] = {
			static_cast<unsigned char>(src[0] >> 2),
			static_cast<unsigned char>(((src[0] & 0x3) << 4) | ((src[1] & 0xF0) >> 4)),
			static_cast<unsigned char>(((src[1] & 0xF) << 2) | ((src[2] & 0xC0) >> 6)),
			static_cast<unsigned char>(src[2] & 0x3F)
		};

		for (size_t i = 0; i < std::min<size_t>(data.size() - pos + 1, 4u); ++i) {
			ret += dst[i] + 33;

			if (++written == 80 && pos + 3 < data.size()) {
				written = 0;
				ret += "\r\n";
			}
		}
	}

	return ret;
}

} }
