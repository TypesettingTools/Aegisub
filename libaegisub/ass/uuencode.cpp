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

#include <libaegisub/ass/uuencode.h>

#include <algorithm>

// Despite being called uuencoding by ass_specs.doc, the format is actually
// somewhat different from real uuencoding.  Each 3-byte chunk is split into 4
// 6-bit pieces, then 33 is added to each piece. Lines are wrapped after 80
// characters, and files with non-multiple-of-three lengths are padded with
// zero.

namespace agi {
namespace ass {

std::string UUEncode(const char* begin, const char* end, bool insert_linebreaks) {
	size_t size = std::distance(begin, end);
	std::string ret;
	ret.reserve((size * 4 + 2) / 3 + size / 80 * 2);

	size_t written = 0;
	for(size_t pos = 0; pos < size; pos += 3) {
		unsigned char src[3] = { '\0', '\0', '\0' };
		memcpy(src, begin + pos, std::min<size_t>(3u, size - pos));

		unsigned char dst[4] = {
			static_cast<unsigned char>(src[0] >> 2),
			static_cast<unsigned char>(((src[0] & 0x3) << 4) | ((src[1] & 0xF0) >> 4)),
			static_cast<unsigned char>(((src[1] & 0xF) << 2) | ((src[2] & 0xC0) >> 6)),
			static_cast<unsigned char>(src[2] & 0x3F)
		};

		for(size_t i = 0; i < std::min<size_t>(size - pos + 1, 4u); ++i) {
			ret += dst[i] + 33;

			if(insert_linebreaks && ++written == 80 && pos + 3 < size) {
				written = 0;
				ret += "\r\n";
			}
		}
	}

	return ret;
}

std::vector<char> UUDecode(const char* begin, const char* end) {
	std::vector<char> ret;
	size_t len = end - begin;
	ret.reserve(len * 3 / 4);

	for(size_t pos = 0; pos + 1 < len;) {
		size_t bytes = 0;
		unsigned char src[4] = { '\0', '\0', '\0', '\0' };
		for(size_t i = 0; i < 4 && pos < len; ++pos) {
			char c = begin[pos];
			if(c && c != '\n' && c != '\r') {
				src[i++] = c - 33;
				++bytes;
			}
		}

		if(bytes > 1) ret.push_back((src[0] << 2) | (src[1] >> 4));
		if(bytes > 2) ret.push_back(((src[1] & 0xF) << 4) | (src[2] >> 2));
		if(bytes > 3) ret.push_back(((src[2] & 0x3) << 6) | (src[3]));
	}

	return ret;
}
} // namespace ass
} // namespace agi
