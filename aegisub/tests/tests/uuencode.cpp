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

#include "main.h"

#include <boost/algorithm/string/replace.hpp>

using namespace agi::ass;

TEST(lagi_uuencode, short_blobs) {
	std::vector<char> data;

	data.push_back(120);
	EXPECT_STREQ("?!", UUEncode(data).c_str());
	data.push_back(121);
	EXPECT_STREQ("?(E", UUEncode(data).c_str());
	data.push_back(122);
	EXPECT_STREQ("?(F[", UUEncode(data).c_str());
}

TEST(lagi_uuencode, short_strings) {
	std::vector<char> data;

	data.push_back(120);
	EXPECT_EQ(data, UUDecode("?!"));
	data.push_back(121);
	EXPECT_EQ(data, UUDecode("?(E"));
	data.push_back(122);
	EXPECT_EQ(data, UUDecode("?(F["));
}

TEST(lagi_uuencode, random_blobs_roundtrip) {
	std::vector<char> data;

	for (size_t len = 0; len < 200; ++len) {
		EXPECT_EQ(data, UUDecode(boost::replace_all_copy(UUEncode(data), "\r\n", "")));
		data.push_back(rand());
	}
}
