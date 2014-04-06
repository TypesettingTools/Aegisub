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

#include <string>
#include <vector>

namespace agi {
	namespace ass {
		/// Encode a blob of data, using ASS's nonstandard variant
		std::string UUEncode(std::vector<char> const& data);

		/// Decode an ASS uuencoded string
		template<typename String>
		std::vector<char> UUDecode(String const& str) {
			std::vector<char> ret;
			size_t len = std::end(str) - std::begin(str);
			ret.reserve(len * 3 / 4);

			for (size_t pos = 0; pos + 1 < len; ) {
				size_t bytes = 0;
				unsigned char src[4] = { '\0', '\0', '\0', '\0' };
				for (size_t i = 0; i < 4 && pos < len; ) {
					char c = str[pos++];
					if (c && c != '\n' && c != '\r') {
						src[i++] = c - 33;
						++bytes;
					}
				}

				if (bytes > 1)
					ret.push_back((src[0] << 2) | (src[1] >> 4));
				if (bytes > 2)
					ret.push_back(((src[1] & 0xF) << 4) | (src[2] >> 2));
				if (bytes > 3)
					ret.push_back(((src[2] & 0x3) << 6) | (src[3]));
			}

			return ret;
		}
	}
}
