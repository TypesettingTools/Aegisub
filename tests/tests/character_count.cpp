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

#include <libaegisub/character_count.h>

TEST(lagi_character_count, basic) {
	EXPECT_EQ(5, agi::CharacterCount("hello", agi::IGNORE_NONE));
}

TEST(lagi_character_count, japanese) {
	EXPECT_EQ(4, agi::CharacterCount("ドングズ", agi::IGNORE_NONE));
}

TEST(lagi_character_count, zalgo) {
	EXPECT_EQ(5, agi::CharacterCount("\xe1\xb8\xa9\x65\xcc\x94\xcc\x8b\xcd\xad\xcc\x80\xcd\x86\xcd\x97\xcc\x84\x6c\xcc\xb6\xcc\x88\xcc\x81\x6c\xcc\xab\xcc\x9c\xcd\x94\xcc\xac\xcc\x96\xcc\x9f\xcc\xb2\xcd\xa8\xcd\xae\xcc\x8b\xcc\x93\x6f\xcc\xad\xcd\x88\xcc\x9f\xcc\x9c\xcd\x94\xcc\xab\xcc\xb0\xcd\x8a\xcd\x97", agi::IGNORE_NONE));
}

TEST(lagi_character_count, ignore_blocks) {
	EXPECT_EQ(11, agi::CharacterCount("{asdf}hello", agi::IGNORE_NONE));
	EXPECT_EQ(10, agi::CharacterCount("{asdfhello", agi::IGNORE_NONE));
	EXPECT_EQ(5, agi::CharacterCount("{asdf}hello", agi::IGNORE_BLOCKS));
}

TEST(lagi_character_count, ignore_punctuation) {
	EXPECT_EQ(6, agi::CharacterCount("hello.", agi::IGNORE_NONE));
	EXPECT_EQ(5, agi::CharacterCount("hello.", agi::IGNORE_PUNCTUATION));
}

TEST(lagi_character_count, ignore_whitespace) {
	EXPECT_EQ(10, agi::CharacterCount("h e l l o ", agi::IGNORE_NONE));
	EXPECT_EQ(5, agi::CharacterCount("h e l l o ", agi::IGNORE_WHITESPACE));
}

TEST(lagi_character_count, ignore_blocks_and_punctuation) {
	EXPECT_EQ(5, agi::CharacterCount("{asdf}hello.", agi::IGNORE_PUNCTUATION | agi::IGNORE_BLOCKS));
}

TEST(lagi_character_count, line_length) {
	EXPECT_EQ(5, agi::MaxLineLength("hello", agi::IGNORE_NONE));
	EXPECT_EQ(5, agi::MaxLineLength("hello\\Nasdf", agi::IGNORE_NONE));
	EXPECT_EQ(5, agi::MaxLineLength("hello\\nasdf", agi::IGNORE_NONE));
	EXPECT_EQ(5, agi::MaxLineLength("asdf\\nhello", agi::IGNORE_NONE));

	EXPECT_EQ(10, agi::MaxLineLength("hello\\hasdf", agi::IGNORE_NONE));
	EXPECT_EQ(9, agi::MaxLineLength("hello\\hasdf", agi::IGNORE_WHITESPACE));
	EXPECT_EQ(9, agi::MaxLineLength("hello asdf", agi::IGNORE_WHITESPACE));
}

TEST(lagi_character_count, line_length_ignores_drawings) {
	EXPECT_EQ(0, agi::MaxLineLength("{\\p1}m 10 10", agi::IGNORE_NONE));
}

TEST(lagi_character_count, character_index) {
	EXPECT_EQ(0, agi::IndexOfCharacter("", 0));
	EXPECT_EQ(0, agi::IndexOfCharacter("", 1));

	EXPECT_EQ(0, agi::IndexOfCharacter("abc", 0));
	EXPECT_EQ(1, agi::IndexOfCharacter("abc", 1));
	EXPECT_EQ(2, agi::IndexOfCharacter("abc", 2));
	EXPECT_EQ(3, agi::IndexOfCharacter("abc", 3));
	EXPECT_EQ(3, agi::IndexOfCharacter("abc", 4));

	EXPECT_EQ(3, agi::IndexOfCharacter("ドングズ", 1));
	EXPECT_EQ(6, agi::IndexOfCharacter("ドングズ", 2));
	EXPECT_EQ(9, agi::IndexOfCharacter("ドングズ", 3));
}


