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

#include <libaegisub/line_wrap.h>

#include <main.h>
#include <util.h>

using namespace agi;
using namespace util;

TEST(lagi_wrap, no_wrapping_needed) {
	for (int i = Wrap_Balanced_FirstLonger; i <= Wrap_Balanced_LastLonger; ++i)
		ASSERT_NO_THROW(get_wrap_points(std::vector<int>{}, 100, (agi::WrapMode)i));

	for (int i = Wrap_Balanced_FirstLonger; i <= Wrap_Balanced_LastLonger; ++i)
		ASSERT_NO_THROW(get_wrap_points(std::vector<int>{ 10 }, 100, (agi::WrapMode)i));

	for (int i = Wrap_Balanced_FirstLonger; i <= Wrap_Balanced_LastLonger; ++i)
		EXPECT_TRUE(get_wrap_points(std::vector<int>{ 99 }, 100, (agi::WrapMode)i).empty());

	for (int i = Wrap_Balanced_FirstLonger; i <= Wrap_Balanced_LastLonger; ++i)
		EXPECT_TRUE(get_wrap_points(std::vector<int>{ 25, 25, 25, 24 }, 100, (agi::WrapMode)i).empty());

	for (int i = Wrap_Balanced_FirstLonger; i <= Wrap_Balanced_LastLonger; ++i)
		EXPECT_TRUE(get_wrap_points(std::vector<int>{ 101 }, 100, (agi::WrapMode)i).empty());
}

TEST(lagi_wrap, greedy) {
	std::vector<size_t> ret;

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 20, Wrap_Greedy));
	EXPECT_EQ(0, ret.size());

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 19, Wrap_Greedy));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 9, Wrap_Greedy));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 5, 1 }, 15, Wrap_Greedy));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(3, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>(10, 3), 7, Wrap_Greedy));
	ASSERT_EQ(4, ret.size());
	EXPECT_EQ(2, ret[0]);
	EXPECT_EQ(4, ret[1]);
	EXPECT_EQ(6, ret[2]);
	EXPECT_EQ(8, ret[3]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 6, 7, 6, 8, 10, 10, 3, 4, 10 }, 20, Wrap_Greedy));
	ASSERT_EQ(3, ret.size());
	EXPECT_EQ(3, ret[0]);
	EXPECT_EQ(5, ret[1]);
	EXPECT_EQ(8, ret[2]);
}

TEST(lagi_wrap, first_longer) {
	std::vector<size_t> ret;

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 20, Wrap_Balanced_FirstLonger));
	EXPECT_EQ(0, ret.size());

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 19, Wrap_Balanced_FirstLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 9, Wrap_Balanced_FirstLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 1 }, 10, Wrap_Balanced_FirstLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(2, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 5, 1 }, 15, Wrap_Balanced_FirstLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(2, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 6, 5, 5 }, 10, Wrap_Balanced_FirstLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);
}

TEST(lagi_wrap, last_longer) {
	std::vector<size_t> ret;

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 20, Wrap_Balanced_LastLonger));
	EXPECT_EQ(0, ret.size());

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 19, Wrap_Balanced_LastLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 9, Wrap_Balanced_LastLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 1 }, 10, Wrap_Balanced_LastLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 5, 1 }, 15, Wrap_Balanced_LastLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 6 }, 10, Wrap_Balanced_LastLonger));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(2, ret[0]);
}

TEST(lagi_wrap, balanced) {
	std::vector<size_t> ret;

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 20, Wrap_Balanced));
	EXPECT_EQ(0, ret.size());

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 19, Wrap_Balanced));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 10, 10 }, 9, Wrap_Balanced));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 1 }, 10, Wrap_Balanced));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(1, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 5, 5, 5, 1 }, 15, Wrap_Balanced));
	ASSERT_EQ(1, ret.size());
	EXPECT_EQ(2, ret[0]);

	ASSERT_NO_THROW(ret = get_wrap_points(std::vector<int>{ 6, 7, 6, 8, 10, 10, 3, 4, 10 }, 20, Wrap_Balanced));
	ASSERT_EQ(3, ret.size());
	EXPECT_EQ(3, ret[0]);
	EXPECT_EQ(5, ret[1]);
	EXPECT_EQ(7, ret[2]);
}
