// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <main.h>
#include <util.h>

#include <libaegisub/split.h>

TEST(lagi_split, delim_not_present) {
	std::string str("hello");
	for (auto tok : agi::Split(str, ','))
		EXPECT_EQ("hello", agi::str(tok));
}

TEST(lagi_split, delim_present) {
	std::string str("a,b,c,");
	std::string expected[] = {"a", "b", "c", ""};
	size_t i = 0;
	for (auto tok : agi::Split(str, ','))
		EXPECT_EQ(expected[i++], agi::str(tok));
}

TEST(lagi_split, does_not_copy_input) {
	std::string str("hello");
	for (auto tok : agi::Split(str, ',')) {
		EXPECT_EQ(str.begin(), tok.begin());
		EXPECT_EQ(str.end(), tok.end());
	}

	auto rng = agi::Split(str, 'e');
	EXPECT_EQ(str.begin(), begin(*rng));
	EXPECT_EQ(str.end(), end(*std::next(rng)));
}

