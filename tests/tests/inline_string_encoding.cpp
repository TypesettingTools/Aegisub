// Copyright (c) 2026, Aegisub contributors
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
// Aegisub Project https://aegisub.org/

#include <libaegisub/ass/string_codec.h>

#include <main.h>

using namespace agi::ass;

static const char *test_cases[][2] = {
	{"", ""},
	{"abc", "abc"},
	{"\1", "#01"},
	{"ab\2cd", "ab#02cd"},
	{"#,:|", "#23#2C#3A#7C"},
	{"č あ", "č あ"},
};

TEST(lagi_inline_string_encoding, encode) {
	for (auto [data, encoded] : test_cases) {
		EXPECT_EQ(encoded, inline_string_encode(data));
	}
}

TEST(lagi_inline_string_encoding, decode) {
	for (auto [data, encoded] : test_cases) {
		EXPECT_EQ(data, inline_string_decode(encoded));
	}
}

TEST(lagi_inline_string_encoding, decode_extra) {
	// incomplete escape
	EXPECT_EQ("#", inline_string_decode("#"));
	EXPECT_EQ("#0", inline_string_decode("#0"));

	// unnecessary escape
	EXPECT_EQ("abc", inline_string_decode("a#62c"));
	EXPECT_EQ("č あ", inline_string_decode("#C4#8D #E3#81#82"));
}

TEST(lagi_inline_string_encoding, random_blobs_roundtrip) {
	std::string data;

	for (size_t len = 1; len < 200; ++len) {
		data.push_back(rand());
		auto encoded = inline_string_encode(data);
		EXPECT_EQ(data, inline_string_decode(encoded));
	}
}
