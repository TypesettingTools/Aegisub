// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "main.h"

#include <libaegisub/ass/dialogue_parser.h>

class lagi_word_split : public libagi { };

using namespace agi::ass;
namespace dt = DialogueTokenType;

TEST(lagi_word_split, empty) {
	std::string text;
	std::vector<DialogueToken> tokens;

	SplitWords(text, tokens);
	EXPECT_TRUE(tokens.empty());

	tokens.emplace_back(0, 0);
	SplitWords(text, tokens);
	EXPECT_EQ(1u, tokens.size());
}

TEST(lagi_word_split, one_word) {
	std::string text = "abc";
	std::vector<DialogueToken> tokens = {{dt::TEXT, 3}};

	SplitWords(text, tokens);
	ASSERT_EQ(1u, tokens.size());
	EXPECT_EQ(dt::WORD, tokens[0].type);
}

TEST(lagi_word_split, two_words_space) {
	std::string text = "abc def";
	std::vector<DialogueToken> tokens = {{dt::TEXT, 7}};

	SplitWords(text, tokens);
	ASSERT_EQ(3u, tokens.size());
	EXPECT_EQ(dt::WORD, tokens[0].type);
	EXPECT_EQ(3u, tokens[0].length);
	EXPECT_EQ(dt::TEXT, tokens[1].type);
	EXPECT_EQ(1u, tokens[1].length);
	EXPECT_EQ(dt::WORD, tokens[2].type);
	EXPECT_EQ(3u, tokens[2].length);
}

TEST(lagi_word_split, two_words_newline) {
	std::string text = "abc\\Ndef";
	std::vector<DialogueToken> tokens = {
		{dt::TEXT, 3},
		{dt::LINE_BREAK, 2},
		{dt::TEXT, 3}
	};

	SplitWords(text, tokens);
	ASSERT_EQ(3u, tokens.size());
	EXPECT_EQ(dt::WORD, tokens[0].type);
	EXPECT_EQ(3u, tokens[0].length);
	EXPECT_EQ(dt::LINE_BREAK, tokens[1].type);
	EXPECT_EQ(2u, tokens[1].length);
	EXPECT_EQ(dt::WORD, tokens[2].type);
	EXPECT_EQ(3u, tokens[2].length);
}

TEST(lagi_word_split, two_words_unicode) {
	std::string text = u8"abc\u300adef";
	std::vector<DialogueToken> tokens = {{dt::TEXT, 9}};

	SplitWords(text, tokens);
	ASSERT_EQ(3u, tokens.size());
	EXPECT_EQ(dt::WORD, tokens[0].type);
	EXPECT_EQ(3u, tokens[0].length);
	EXPECT_EQ(dt::TEXT, tokens[1].type);
	EXPECT_EQ(3u, tokens[1].length);
	EXPECT_EQ(dt::WORD, tokens[2].type);
	EXPECT_EQ(3u, tokens[2].length);
}

TEST(lagi_word_split, drawing) {
	std::string text = "a b{\\p1}m 10{\\p0}c";
	std::vector<DialogueToken> tokens = {
		{dt::TEXT, 3},
		{dt::OVR_BEGIN, 1},
		{dt::TAG_START, 1},
		{dt::TAG_NAME, 1},
		{dt::ARG, 1},
		{dt::OVR_END, 1},
		{dt::TEXT, 4},
		{dt::OVR_BEGIN, 1},
		{dt::TAG_START, 1},
		{dt::TAG_NAME, 1},
		{dt::ARG, 1},
		{dt::OVR_END, 1},
		{dt::TEXT, 1}
	};

	SplitWords(text, tokens);

	ASSERT_EQ(15u, tokens.size());
	EXPECT_EQ(dt::WORD, tokens[0].type);
	EXPECT_EQ(dt::WORD, tokens[2].type);
	EXPECT_EQ(dt::WORD, tokens[14].type);

	EXPECT_EQ(dt::DRAWING, tokens[8].type);
}

TEST(lagi_word_split, unclosed_ovr) {
	std::string text = "a{\\b";
	std::vector<DialogueToken> tokens = {
		{dt::TEXT, 1},
		{dt::OVR_BEGIN, 1},
		{dt::TAG_START, 1},
		{dt::TAG_NAME, 1}
	};

	SplitWords(text, tokens);
	ASSERT_EQ(3u, tokens.size());
	EXPECT_EQ(dt::WORD, tokens[0].type);
	EXPECT_EQ(dt::TEXT, tokens[1].type);
	EXPECT_EQ(dt::WORD, tokens[2].type);

	text = "{";
	tokens.clear();
	tokens.emplace_back(dt::OVR_BEGIN, 1);

	SplitWords(text, tokens);
	ASSERT_EQ(1u, tokens.size());
	EXPECT_EQ(dt::TEXT, tokens[0].type);
}

