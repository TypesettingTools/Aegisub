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

#include <libaegisub/util.h>

#include <main.h>

#define IFIND(haystack, needle) \
	std::pair<size_t, size_t> pos; \
	ASSERT_NO_THROW(pos = agi::util::ifind(haystack, needle))

#define EXPECT_IFIND(haystack, needle, s, e) \
	do { \
		IFIND(haystack, needle); \
		EXPECT_EQ((size_t)s, pos.first); \
		EXPECT_EQ((size_t)e, pos.second); \
	} while(false)

#define EXPECT_NO_MATCH(haystack, needle) \
	do { \
		IFIND(haystack, needle); \
		EXPECT_EQ((size_t)-1, pos.first); \
		EXPECT_EQ((size_t)-1, pos.second); \
	} while(false)

TEST(lagi_ifind, basic_match) {
	EXPECT_IFIND(" a ", "a", 1, 2);
	EXPECT_IFIND(" a ", "A", 1, 2);
	EXPECT_NO_MATCH(" a ", "b");
}

TEST(lagi_ifind, sharp_s_matches_ss) {
	// lowercase
	EXPECT_IFIND(" \xC3\x9F ", "ss", 1, 3);
	EXPECT_IFIND(" ss ", "\xC3\x9F", 1, 3);

	// uppercase
	EXPECT_IFIND(" \xE1\xBA\x9E ", "ss", 1, 4);
	EXPECT_IFIND(" ss ", "\xE1\xBA\x9E", 1, 3);
}

TEST(lagi_ifind, no_partial_match_on_decomposed_character) {
	EXPECT_NO_MATCH("s\xEF\xAC\x86", "ss"); // LATIN SMALL LIGATURE ST
	EXPECT_NO_MATCH("\xEF\xAC\x86t", "tt");
	EXPECT_NO_MATCH(" \xE1\xBA\x9E ", "s");
	EXPECT_NO_MATCH("\xE1\xBA\x9E", "s");
	EXPECT_IFIND(" \xE1\xBA\x9E s ", "s", 5, 6);
	EXPECT_IFIND("s\xE1\xBA\x9E", "ss", 1, 4);
	EXPECT_IFIND("s\xE1\xBA\x9E", "sss", 0, 4);
	EXPECT_IFIND("\xE1\xBA\x9Es", "sss", 0, 4);
	EXPECT_IFIND("\xEF\xAC\x86", "st", 0, 3);
}

TEST(lagi_ifind, correct_index_with_expanded_character_before_match) {
	// U+0587 turns into U+0565 U+0582, all of which are two bytes in UTF-8
	EXPECT_IFIND(" \xD6\x87 a ", "a", 4, 5);
}

TEST(lagi_ifind, correct_index_with_shrunk_character_before_match) {
	// U+FB00 turns into "ff", which is one byte shorter in UTF-8
	EXPECT_IFIND(" \xEF\xAC\x80 a ", "a", 5, 6);
}
