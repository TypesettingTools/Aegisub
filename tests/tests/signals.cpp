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

#include <libaegisub/signal.h>

#include "main.h"

using namespace agi::signal;

TEST(lagi_signal, basic) {
	Signal<> s;
	int x = 0;
	Connection c = s.Connect([&] { ++x; });

	EXPECT_EQ(0, x);
	s();
	EXPECT_EQ(1, x);
}

TEST(lagi_signal, multiple) {
	Signal<> s;
	int x = 0;
	Connection c1 = s.Connect([&] { ++x; });
	Connection c2 = s.Connect([&] { ++x; });

	EXPECT_EQ(0, x);
	s();
	EXPECT_EQ(2, x);
}
TEST(lagi_signal, manual_disconnect) {
	Signal<> s;
	int x = 0;
	Connection c1 = s.Connect([&] { ++x; });
	EXPECT_EQ(0, x);
	s();
	EXPECT_EQ(1, x);
	c1.Disconnect();
	s();
	EXPECT_EQ(1, x);
}

TEST(lagi_signal, auto_disconnect) {
	Signal<> s;
	int x = 0;

	EXPECT_EQ(0, x);
	{
		Connection c = s.Connect([&] { ++x; });
		s();
		EXPECT_EQ(1, x);
	}
	s();
	EXPECT_EQ(1, x);
}

TEST(lagi_signal, connection_outlives_slot) {
	int x = 0;
	Connection c;

	EXPECT_EQ(0, x);
	{
		Signal<> s;
		c = s.Connect([&] { ++x; });
		s();
		EXPECT_EQ(1, x);
	}
	c.Disconnect();
}

TEST(lagi_signal, one_arg) {
	Signal<int> s;
	int x = 0;

	Connection c = s.Connect([&](int v) { x += v; });
	s(0);
	EXPECT_EQ(0, x);
	s(10);
	EXPECT_EQ(10, x);
	s(20);
	EXPECT_EQ(30, x);
}
