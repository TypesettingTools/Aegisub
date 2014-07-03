// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/line_iterator.h>

#include <main.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

template<typename T>
void test_values(agi::line_iterator<T>& iter) {
	EXPECT_EQ(iter, end(iter));
}

template<typename T, typename First, typename... Values>
void test_values(agi::line_iterator<T>& iter, First first, Values... values) {
	EXPECT_FALSE(iter == end(iter));
	EXPECT_EQ(*iter, first);
	EXPECT_NO_THROW(++iter);
	test_values(iter, values...);
}

template<typename T, typename... Values>
void test(std::string const& str, const char *encoding, Values... values) {
	std::stringstream ss(str);
	agi::line_iterator<T> iter;
	EXPECT_NO_THROW(iter = agi::line_iterator<T>(ss, encoding));
	test_values(iter, values...);
}

template<typename T, typename... Values>
void expect_eq(const char *str, Values... values) {
	std::string utf8(str);
	test<T>(utf8, "utf-8", values...);

	agi::charset::IconvWrapper conv("utf-8", "utf-16");
	auto utf16 = conv.Convert(utf8);
	test<T>(utf16, "utf-16", values...);
}

TEST(lagi_line, int) {
	expect_eq<int>("1\n2\n3\n4", 1, 2, 3, 4);
	expect_eq<int>("1\n2\n3\n4\n", 1, 2, 3, 4);
	expect_eq<int>("1\n2\nb\n3\n4", 1, 2, 3, 4);
	expect_eq<int>("1.0\n2.0\n3.0\n4.0", 1, 2, 3, 4);
	expect_eq<int>(" 0x16 \n 09 \n -2", 0, 9, -2);
}
TEST(lagi_line, double) {
	expect_eq<double>("1.0\n2.0", 1.0, 2.0);
	expect_eq<double>("#1.0\n\t2.5", 2.5);
}
TEST(lagi_line, string) {
	expect_eq<std::string>("line 1\nline 2\nline 3", "line 1", "line 2", "line 3");
	expect_eq<std::string>(" white space ", " white space ");
	expect_eq<std::string>("blank\n\nlines\n", "blank", "", "lines", "");
}
