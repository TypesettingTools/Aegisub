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

#include <libaegisub/ass/dialogue_parser.h>

#include <main.h>
#include <util.h>

class lagi_dialogue_lexer : public libagi {
};

using namespace agi::ass;

TEST(lagi_dialogue_lexer, empty) {
	ASSERT_TRUE(TokenizeDialogueBody("").empty());
}

#define tok_str(arg1, ktemplate, ...) do { \
	std::string str = arg1; \
	std::vector<DialogueToken> tok = TokenizeDialogueBody(str, ktemplate); \
	size_t token_index = 0; \
	__VA_ARGS__ \
	EXPECT_EQ(token_index, tok.size()); \
} while(false)

#define expect_tok(expected_type, expected_len) do { \
	EXPECT_LT(token_index, tok.size()); \
	if (token_index < tok.size()) { \
		EXPECT_EQ(DialogueTokenType::expected_type, tok[token_index].type); \
		EXPECT_EQ(expected_len, tok[token_index].length); \
		++token_index; \
	} \
} while(false)

TEST(lagi_dialogue_lexer, plain_text) {
	tok_str("hello there", false,
		expect_tok(TEXT, 11);
	);

	tok_str("hello\\Nthere", false,
		expect_tok(TEXT, 5);
		expect_tok(LINE_BREAK, 2);
		expect_tok(TEXT, 5);
	);

	tok_str("hello\\n\\h\\kthere", false,
		expect_tok(TEXT, 5);
		expect_tok(LINE_BREAK, 4);
		expect_tok(TEXT, 7);
	);
}

TEST(lagi_dialogue_lexer, basic_override_tags) {
	tok_str("{\\b1}bold text{\\b0}", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(ARG, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 9);
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(ARG, 1);
		expect_tok(OVR_END, 1);
	);

	tok_str("{\\fnComic Sans MS}text", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 2);
		expect_tok(ARG, 5);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 4);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 2);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 4);
	);

	tok_str("{\\pos(0,0)}a", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 3);
		expect_tok(OPEN_PAREN, 1);
		expect_tok(ARG, 1);
		expect_tok(ARG_SEP, 1);
		expect_tok(ARG, 1);
		expect_tok(CLOSE_PAREN, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);

	tok_str("{\\pos( 0 , 0 )}a", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 3);
		expect_tok(OPEN_PAREN, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG_SEP, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(CLOSE_PAREN, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);

	tok_str("{\\c&HFFFFFF&\\2c&H0000FF&\\3c&H000000&}a", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(ARG, 9);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 2);
		expect_tok(ARG, 9);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 2);
		expect_tok(ARG, 9);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);

	tok_str("{\\t(0,100,\\clip(1, m 0 0 l 10 10 10 20))}a", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(OPEN_PAREN, 1);
		expect_tok(ARG, 1);
		expect_tok(ARG_SEP, 1);
		expect_tok(ARG, 3);
		expect_tok(ARG_SEP, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 4);
		expect_tok(OPEN_PAREN, 1);
		expect_tok(ARG, 1);
		expect_tok(ARG_SEP, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 2);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 2);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 2);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 2);
		expect_tok(CLOSE_PAREN, 2);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);
}

TEST(lagi_dialogue_lexer, merging) {
	tok_str("{\\b\\b", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
	);
}

TEST(lagi_dialogue_lexer, whitespace) {
	tok_str("{ \\ fn Comic Sans MS }asd", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(TAG_START, 1);
		expect_tok(WHITESPACE, 1);
		expect_tok(TAG_NAME, 2);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 5);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 4);
		expect_tok(WHITESPACE, 1);
		expect_tok(ARG, 2);
		expect_tok(WHITESPACE, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 3);
	);
}

TEST(lagi_dialogue_lexer, comment) {
	tok_str("{a}b", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(COMMENT, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);

	tok_str("{a\\b}c", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(COMMENT, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);
}

TEST(lagi_dialogue_lexer, malformed) {
	tok_str("}", false,
		expect_tok(TEXT, 1);
	);

	tok_str("{{", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(ERROR, 1);
	);

	tok_str("{\\pos(0,0}a", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 3);
		expect_tok(OPEN_PAREN, 1);
		expect_tok(ARG, 1);
		expect_tok(ARG_SEP, 1);
		expect_tok(ARG, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 1);
	);

	tok_str("{\\b1\\}asdf", false,
		expect_tok(OVR_BEGIN, 1);
		expect_tok(TAG_START, 1);
		expect_tok(TAG_NAME, 1);
		expect_tok(ARG, 1);
		expect_tok(TAG_START, 1);
		expect_tok(OVR_END, 1);
		expect_tok(TEXT, 4);
	);
}

TEST(lagi_dialogue_lexer, templater_variable_nontmpl) {
	tok_str("{\\pos($x, $y)\\fs!10 + 10!}abc", false,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 3u);
		expect_tok(OPEN_PAREN, 1);
		expect_tok(ARG, 2u);
		expect_tok(ARG_SEP, 1u);
		expect_tok(WHITESPACE, 1u);
		expect_tok(ARG, 2u);
		expect_tok(CLOSE_PAREN, 1);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 2u);
		expect_tok(ARG, 3u);
		expect_tok(WHITESPACE, 1u);
		expect_tok(ARG, 1u);
		expect_tok(WHITESPACE, 1u);
		expect_tok(ARG, 3u);
		expect_tok(OVR_END, 1u);
		expect_tok(TEXT, 3u);
	);

	tok_str("{\\b1!'}'!a", false,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 1u);
		expect_tok(ARG, 3u);
		expect_tok(OVR_END, 1u);
		expect_tok(TEXT, 3u);
	);
}

TEST(lagi_dialogue_lexer, templater_variable) {
	tok_str("$a", true,
		expect_tok(KARAOKE_VARIABLE, 2u);
	);

	tok_str("{\\pos($x,$y)}a", true,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 3u);
		expect_tok(OPEN_PAREN, 1u);
		expect_tok(KARAOKE_VARIABLE, 2u);
		expect_tok(ARG_SEP, 1u);
		expect_tok(KARAOKE_VARIABLE, 2u);
		expect_tok(CLOSE_PAREN, 1u);
		expect_tok(OVR_END, 1u);
		expect_tok(TEXT, 1u);
	);

	tok_str("{\\fn$fn}a", true,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 2u);
		expect_tok(KARAOKE_VARIABLE, 3u);
		expect_tok(OVR_END, 1u);
		expect_tok(TEXT, 1u);
	);

	tok_str("{foo$bar}", true,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(COMMENT, 3u);
		expect_tok(KARAOKE_VARIABLE, 4u);
		expect_tok(OVR_END, 1u);
	);

	tok_str("{foo$bar", true,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(COMMENT, 3u);
		expect_tok(KARAOKE_VARIABLE, 4u);
	);
}

TEST(lagi_dialogue_lexer, templater_expression) {
	tok_str("!5!", true,
		expect_tok(KARAOKE_TEMPLATE, 3u);
	);

	tok_str("!5", true,
		expect_tok(TEXT, 2u);
	);

	tok_str("!x * 10!", true,
		expect_tok(KARAOKE_TEMPLATE, 8u);
	);

	tok_str("{\\pos(!x + 1!, $y)}a", true,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 3u);
		expect_tok(OPEN_PAREN, 1u);
		expect_tok(KARAOKE_TEMPLATE, 7u);
		expect_tok(ARG_SEP, 1u);
		expect_tok(WHITESPACE, 1u);
		expect_tok(KARAOKE_VARIABLE, 2u);
		expect_tok(CLOSE_PAREN, 1u);
		expect_tok(OVR_END, 1u);
		expect_tok(TEXT, 1u);
	);

	tok_str("{\\b1!'}'!a", true,
		expect_tok(OVR_BEGIN, 1u);
		expect_tok(TAG_START, 1u);
		expect_tok(TAG_NAME, 1u);
		expect_tok(ARG, 1u);
		expect_tok(KARAOKE_TEMPLATE, 5u);
		expect_tok(ARG, 1u);
	);
}
