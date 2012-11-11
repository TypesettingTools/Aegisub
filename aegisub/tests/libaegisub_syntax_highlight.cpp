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
#include <libaegisub/spellchecker.h>

#include "main.h"

class MockSpellChecker : public agi::SpellChecker {
	void AddWord(std::string const&) { }
	bool CanAddWord(std::string const&) { return false; }
	std::vector<std::string> GetSuggestions(std::string const&) { return std::vector<std::string>(); }
	std::vector<std::string> GetLanguageList() { return std::vector<std::string>(); }
	bool CheckWord(std::string const& word) { return word != "incorrect"; }
};

using namespace agi::ass;
namespace dt = DialogueTokenType;
namespace ss = SyntaxStyle;

class lagi_syntax : public libagi { };

TEST(lagi_syntax, empty) {
	std::string text;
	std::vector<DialogueToken> tokens;

	EXPECT_TRUE(SyntaxHighlight(text, tokens, false, 0).empty());

	tokens.emplace_back(dt::TEXT, 0);
	auto syntax = SyntaxHighlight(text, tokens, false, 0);
	EXPECT_EQ(1u, syntax.size());
	EXPECT_EQ(ss::NORMAL, syntax[0].type);
}

#define tok_str(arg1, template_line, ...) do { \
	MockSpellChecker spellchecker; \
	std::string str = arg1; \
	std::vector<DialogueToken> tok = TokenizeDialogueBody(str); \
	SplitWords(str, tok); \
	std::vector<DialogueToken> styles = SyntaxHighlight(str, tok, template_line, &spellchecker); \
	size_t token_index = 0; \
	__VA_ARGS__ \
	EXPECT_EQ(token_index, styles.size()); \
} while(false)

#define expect_style(expected_type, expected_len) do { \
	EXPECT_LT(token_index, styles.size()); \
	if (token_index < styles.size()) { \
		EXPECT_EQ(expected_type, styles[token_index].type); \
		EXPECT_EQ(expected_len, styles[token_index].length); \
		++token_index; \
	} \
} while(false)

TEST(lagi_syntax, spellcheck) {
	tok_str("correct incorrect correct", false,
		expect_style(ss::NORMAL, 8u);
		expect_style(ss::SPELLING, 9u);
		expect_style(ss::NORMAL, 8u);
	);
}

TEST(lagi_syntax, drawing) {
	tok_str("incorrect{\\p1}m 10 10{\\p}correct", false,
		expect_style(ss::SPELLING, 9u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::TAG, 1u);
		expect_style(ss::PARAMETER, 1u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::DRAWING, 7u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::TAG, 1u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::NORMAL, 7u);
	);
}

TEST(lagi_syntax, transform) {
	tok_str("{\\t(0, 0, \\clip(0,0,10,10)}clipped text", false,
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::TAG, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::PARAMETER, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::NORMAL, 1u);
		expect_style(ss::PARAMETER, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::NORMAL, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::TAG, 4u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::PARAMETER, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::PARAMETER, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::PARAMETER, 2u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::PARAMETER, 2u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::NORMAL, 12u);
	);
}

TEST(lagi_syntax, unclosed) {
	tok_str("{\\incorrect}{\\incorrect", false,
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::TAG, 9u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::NORMAL, 2u);
		expect_style(ss::SPELLING, 9u);
	);
}

TEST(lagi_syntax, comment) {
	tok_str("abc{def}ghi", false,
		expect_style(ss::NORMAL, 3u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::COMMENT, 3u);
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::NORMAL, 3u);
	);
}

TEST(lagi_syntax, linebreak) {
	tok_str("a\\Nb\\nc\\hd\\N\\N", false,
		expect_style(ss::NORMAL, 1u);
		expect_style(ss::LINE_BREAK, 2u);
		expect_style(ss::NORMAL, 1u);
		expect_style(ss::LINE_BREAK, 2u);
		expect_style(ss::NORMAL, 1u);
		expect_style(ss::LINE_BREAK, 2u);
		expect_style(ss::NORMAL, 1u);
		expect_style(ss::LINE_BREAK, 4u);
	);
}

TEST(lagi_syntax, fn_space) {
	tok_str("{\\fnComic Sans MS}", false,
		expect_style(ss::OVERRIDE, 1u);
		expect_style(ss::PUNCTUATION, 1u);
		expect_style(ss::TAG, 2u);
		expect_style(ss::PARAMETER, 13u);
		expect_style(ss::OVERRIDE, 1u);
	);
}
