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
//
// $Id$

/// @file libaegisub_line_iterator.cpp
/// @brief agi::line_iterator tests
/// @ingroup 

#include <libaegisub/line_iterator.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "main.h"
#include "util.h"

using agi::line_iterator;

using namespace util;

template<typename T>
struct varg_type {
	typedef T type;
};
template<> struct varg_type<std::string> {
	typedef const char * type;
};

template<typename T, int N>
void expect_eq(const char (&str)[N], const char *charset, int num, ...) {
	std::string string(str, N - 1);
	agi::charset::IconvWrapper conv("utf-8", charset);
	string = conv.Convert(string);
	std::stringstream ss(string);
	line_iterator<T> iter;
	EXPECT_NO_THROW(iter = line_iterator<T>(ss, charset));
	va_list argp;
	va_start(argp, num);
	for (; num > 0; --num) {
		EXPECT_FALSE(iter == line_iterator<T>());
		EXPECT_EQ(*iter, va_arg(argp, typename varg_type<T>::type));
		EXPECT_NO_THROW(++iter);
	}
	va_end(argp);
	EXPECT_TRUE(iter == line_iterator<T>());
}

TEST(lagi_line, int) {
	std::vector<std::string> charsets = agi::charset::GetEncodingsList<std::vector<std::string> >();
	for (std::vector<std::string>::iterator cur = charsets.begin(); cur != charsets.end(); ++cur) {
		expect_eq<int>("1\n2\n3\n4", cur->c_str(), 4, 1, 2, 3, 4);
		expect_eq<int>("1\n2\n3\n4\n", cur->c_str(), 4, 1, 2, 3, 4);
		expect_eq<int>("1\n2\nb\n3\n4", cur->c_str(), 4, 1, 2, 3, 4);
		expect_eq<int>("1.0\n2.0\n3.0\n4.0", cur->c_str(), 4, 1, 2, 3, 4);
		expect_eq<int>(" 0x16 \n 09 \n -2", cur->c_str(), 3, 0, 9, -2);
	}
}
TEST(lagi_line, double) {
	std::vector<std::string> charsets = agi::charset::GetEncodingsList<std::vector<std::string> >();
	for (std::vector<std::string>::iterator cur = charsets.begin(); cur != charsets.end(); ++cur) {
		expect_eq<double>("1.0\n2.0", cur->c_str(), 2, 1.0, 2.0);
		expect_eq<double>("#1.0\n\t2.5", cur->c_str(), 1, 2.5);
	}
}
TEST(lagi_line, string) {
	std::vector<std::string> charsets = agi::charset::GetEncodingsList<std::vector<std::string> >();
	for (std::vector<std::string>::iterator cur = charsets.begin(); cur != charsets.end(); ++cur) {
		expect_eq<std::string>("line 1\nline 2\nline 3", cur->c_str(), 3, "line 1", "line 2", "line 3");
		expect_eq<std::string>(" white space ", cur->c_str(), 1, " white space ");
		expect_eq<std::string>("blank\n\nlines\n", cur->c_str(), 4, "blank", "", "lines", "");
	}
}
