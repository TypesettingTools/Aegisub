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

#include "main.h"
#include "util.h"

#include <libaegisub/format.h>

TEST(lagi_format, s) {
	EXPECT_EQ("hello", agi::format("%s", "hello"));
	EXPECT_EQ("he", agi::format("%.2s", "hello"));
	EXPECT_EQ("hello", agi::format("%.8s", "hello"));
	EXPECT_EQ("he", agi::format("%.*s", 2, "hello"));
	EXPECT_EQ("hello", agi::format("%", "hello"));
	EXPECT_EQ("hello", agi::format("%s", std::string("hello")));
	EXPECT_EQ("he", agi::format("%.2s", std::string("hello")));

	EXPECT_EQ("true", agi::format("%s", true));
	EXPECT_EQ("false", agi::format("%s", false));
}

TEST(lagi_format, c) {
	EXPECT_EQ("a", agi::format("%c", 'a'));
	EXPECT_EQ("a", agi::format("%c", (int)'a'));
}

TEST(lagi_format, d) {
	EXPECT_EQ("10", agi::format("%i", 10));
	EXPECT_EQ("10", agi::format("%d", 10));
	EXPECT_EQ("-10", agi::format("%d", -10));
	EXPECT_EQ("1", agi::format("%d", 1.1));
	EXPECT_EQ("97", agi::format("%d", 'a'));
	EXPECT_EQ("97", agi::format("%d", (int)'a'));
	EXPECT_EQ("1", agi::format("%d", true));
}

TEST(lagi_format, o) {
	EXPECT_EQ("15", agi::format("%o", 015));
}

TEST(lagi_format, u) {
	EXPECT_EQ("10", agi::format("%u", 10));
	EXPECT_EQ(std::to_string(static_cast<uintmax_t>(-10)), agi::format("%u", -10));
}

TEST(lagi_format, x) {
	EXPECT_EQ("deadbeef", agi::format("%x", 0xdeadbeef));
	EXPECT_EQ("DEADBEEF", agi::format("%X", 0xDEADBEEF));
}

TEST(lagi_format, e) {
	EXPECT_EQ("1.234560e+10", agi::format("%e", 1.23456e10));
	EXPECT_EQ("-1.234560E+10", agi::format("%E", -1.23456E10));
}

TEST(lagi_format, f) {
	EXPECT_EQ("-9.876500", agi::format("%f", -9.8765));
	EXPECT_EQ("9.876500", agi::format("%F", 9.8765));
}

TEST(lagi_format, g) {
	EXPECT_EQ("10", agi::format("%g", 10));
	EXPECT_EQ("100", agi::format("%G", 100));
}

TEST(lagi_format, escape) {
	EXPECT_EQ("%d", agi::format("%%d"));
	EXPECT_EQ("%d", agi::format("%%d", 5));
}

TEST(lagi_format, length_modifiers) {
	EXPECT_EQ("10", agi::format("%hd",  10));
	EXPECT_EQ("10", agi::format("%ld",  10));
	EXPECT_EQ("10", agi::format("%lld", 10));
	EXPECT_EQ("10", agi::format("%zd",  10));
	EXPECT_EQ("10", agi::format("%td",  10));
	EXPECT_EQ("10", agi::format("%jd",  10));
}

TEST(lagi_format, precision_width) {
	EXPECT_EQ("05", agi::format("%02X", 5));
	EXPECT_EQ("       -10", agi::format("%10d", -10));
	EXPECT_EQ("0010", agi::format("%04d", 10));
	EXPECT_EQ(" 1234.1235", agi::format("%10.4f", 1234.1234567890));
	EXPECT_EQ("10", agi::format("%.f", 10.1));
	EXPECT_EQ("0000000100", agi::format("%010d", 100));
	EXPECT_EQ("-000000010", agi::format("%010d", -10));
	EXPECT_EQ("0X0000BEEF", agi::format("%#010X", 0xBEEF));
}

TEST(lagi_format, flags) {
	EXPECT_EQ("0x271828", agi::format("%#x", 0x271828));
	EXPECT_EQ("011614050", agi::format("%#o", 0x271828));
	EXPECT_EQ("3.000000", agi::format("%#f", 3.0));
	EXPECT_EQ("+3", agi::format("%+d", 3));
	EXPECT_EQ("+0", agi::format("%+d", 0));
	EXPECT_EQ("-3", agi::format("%+d", -3));

	EXPECT_EQ("10        ", agi::format("%-010d", 10));
	EXPECT_EQ("10        ", agi::format("%0-10d", 10));
}

TEST(lagi_format, bad_cast) {
	EXPECT_THROW(agi::format("%d", "hello"), std::bad_cast);
	EXPECT_THROW(agi::format("%.*s", "hello", "str"), std::bad_cast);
}
