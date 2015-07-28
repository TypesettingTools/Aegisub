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

#include <libaegisub/calltip_provider.h>

#include <libaegisub/ass/dialogue_parser.h>

#include <main.h>

using agi::Calltip;

static void expect_tip(const char *line, size_t pos, agi::Calltip tip) {
	auto tokenized_line = agi::ass::TokenizeDialogueBody(line, false);
	auto actual = agi::GetCalltip(tokenized_line, line, pos);
	if (!tip.text) {
		EXPECT_EQ(nullptr, actual.text);
	}
	else {
		ASSERT_TRUE(actual.text);
		EXPECT_STREQ(tip.text, actual.text);
		EXPECT_EQ(tip.tag_position, actual.tag_position);
		EXPECT_EQ(tip.highlight_start, actual.highlight_start);
		EXPECT_EQ(tip.highlight_end, actual.highlight_end);
	}
}

const auto bad_tip = Calltip{nullptr, 0, 0, 0};

TEST(lagi_calltip, empty_line) {
	expect_tip("", 0, bad_tip);
}

TEST(lagi_calltip, no_override_blocks) {
	expect_tip("hello", 0, bad_tip);
}

TEST(lagi_calltip, cursor_outside_of_block) {
	expect_tip("{\\b1}hello", 6, bad_tip);
}

TEST(lagi_calltip, basic_cursor_on_tag) {
	expect_tip("{\\b1}hello", 3, Calltip{"\\bWeight", 2, 8, 2});
}

TEST(lagi_calltip, basic_two_arg) {
	expect_tip("{\\pos(100,100)}hello", 3, Calltip{"\\pos(X,Y)", 5, 6, 2});
	expect_tip("{\\pos(100,100)}hello", 9, Calltip{"\\pos(X,Y)", 5, 6, 2});
	expect_tip("{\\pos(100,100)}hello", 10, Calltip{"\\pos(X,Y)", 7, 8, 2});
	expect_tip("{\\pos(100,100)}hello", 14, Calltip{"\\pos(X,Y)", 7, 8, 2});
	expect_tip("{\\pos(100,100)}hello", 15, bad_tip);
}

TEST(lagi_calltip, overloads) {
	expect_tip("{\\clip(m)}", 3, Calltip{"\\clip(Command)", 6, 13, 2});
	expect_tip("{\\clip(1, m)}", 3, Calltip{"\\clip(Scale,Command)", 6, 11, 2});
	expect_tip("{\\clip(1, m)}", 10, Calltip{"\\clip(Scale,Command)", 12, 19, 2});
}

TEST(lagi_calltip, too_many_args) {
	expect_tip("{\\pos(100,100,100)}hello", 15, bad_tip);
	expect_tip("{\\pos(100,100,100)}hello", 3, Calltip{"\\pos(X,Y)", 5, 6, 2});
}

TEST(lagi_calltip, unknown_tag) {
	expect_tip("{\\foo(100,100,100)}hello", 3, bad_tip);
	expect_tip("{\\toolong(100,100,100)}hello", 3, bad_tip);
}
