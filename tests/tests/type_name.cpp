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

#include <libaegisub/type_name.h>

TEST(lagi_type_name, type_name) {
	EXPECT_STREQ("bool", agi::type_name<bool>::name());
	EXPECT_EQ("char const", agi::type_name<char const>::name());
	EXPECT_EQ("void*", agi::type_name<void *>::name());
	EXPECT_EQ("float const*", agi::type_name<float const*>::name());
	EXPECT_EQ("int const&", agi::type_name<int const&>::name());
	EXPECT_EQ("double&", agi::type_name<double &>::name());
	EXPECT_EQ("void (*)()", agi::type_name<void (*)()>::name());
	EXPECT_EQ("int (*)(bool)", agi::type_name<int (*)(bool)>::name());
	EXPECT_EQ("char const* (*)(int, bool, int)", agi::type_name<char const*(*)(int, bool, int)>::name());
	EXPECT_EQ("char* (*)(int, int, void (*)())", agi::type_name<char *(*)(int, int, void (*)())>::name());
}

