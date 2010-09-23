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

/// @file libaegisub_vfr.cpp
/// @brief agi::vfr::Framerate tests
/// @ingroup video_input

#include <libaegisub/vfr.h>

#include <fstream>
#include <iterator>

#include "main.h"
#include "util.h"

using namespace agi::vfr;
using namespace util;

#define EXPECT_RANGE(low, high, test) \
	EXPECT_LE(low, test); \
	EXPECT_GE(high, test)

TEST(lagi_vfr, constructors_good) {
	EXPECT_NO_THROW(Framerate(1.));
	EXPECT_NO_THROW(Framerate(Framerate(1.)));
	EXPECT_NO_THROW(Framerate(make_vector<int>(2, 0, 10)));

	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_start_equals_end.txt"));
	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_whitespace.txt"));
	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_assume_int.txt"));
	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_out_of_order.txt"));
}

TEST(lagi_vfr, constructors_bad_cfr) {
	EXPECT_THROW(Framerate(-1.), BadFPS);
	EXPECT_THROW(Framerate(1000.1), BadFPS);
}

TEST(lagi_vfr, constructors_bad_timecodes) {
	EXPECT_THROW(Framerate(make_vector<int>(0)), TooFewTimecodes);
	EXPECT_THROW(Framerate(make_vector<int>(1, 0)), TooFewTimecodes);
	EXPECT_THROW(Framerate(make_vector<int>(2, 10, 0)), UnorderedTimecodes);
}

TEST(lagi_vfr, constructors_bad_v1) {
	EXPECT_THROW(Framerate("data/vfr/in/v1_bad_seperators.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_too_few_parts.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_too_many_parts.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_float_frame_number.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_start_end_overlap.txt"), UnorderedTimecodes);
	EXPECT_THROW(Framerate("data/vfr/in/v1_fully_contained.txt"), UnorderedTimecodes);
	EXPECT_THROW(Framerate("data/vfr/in/v1_assume_over_1000.txt"), BadFPS);
	EXPECT_THROW(Framerate("data/vfr/in/v1_override_over_1000.txt"), BadFPS);
	EXPECT_THROW(Framerate("data/vfr/in/v1_override_zero.txt"), BadFPS);
	EXPECT_THROW(Framerate("data/vfr/in/v1_negative_start_of_range.txt"), UnorderedTimecodes);
	EXPECT_THROW(Framerate("data/vfr/in/v1_end_less_than_start.txt"), UnorderedTimecodes);
}

TEST(lagi_vfr, constructors_bad_v2) {
	EXPECT_THROW(Framerate("data/vfr/in/v2_empty.txt"), TooFewTimecodes);
	EXPECT_THROW(Framerate("data/vfr/in/v2_out_of_order.txt"), UnorderedTimecodes);

	EXPECT_THROW(Framerate("data/vfr/in/empty.txt"), UnknownFormat);
}

TEST(lagi_vfr, cfr_frame_at_time_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	EXPECT_EQ(0, fps.FrameAtTime(0));
	EXPECT_EQ(0, fps.FrameAtTime(999));
	EXPECT_EQ(1, fps.FrameAtTime(1000));
	EXPECT_EQ(1, fps.FrameAtTime(1999));
	EXPECT_EQ(100, fps.FrameAtTime(100000));
	EXPECT_EQ(-1, fps.FrameAtTime(-1));
	EXPECT_EQ(-1, fps.FrameAtTime(-1000));
	EXPECT_EQ(-2, fps.FrameAtTime(-1001));
}

TEST(lagi_vfr, cfr_frame_at_time_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	EXPECT_EQ(0, fps.FrameAtTime(0, START));
	EXPECT_EQ(1, fps.FrameAtTime(1, START));
	EXPECT_EQ(1, fps.FrameAtTime(1000, START));
	EXPECT_EQ(2, fps.FrameAtTime(1001, START));
	EXPECT_EQ(100, fps.FrameAtTime(100000, START));
	EXPECT_EQ(0, fps.FrameAtTime(-1, START));
	EXPECT_EQ(0, fps.FrameAtTime(-999, START));
	EXPECT_EQ(-1, fps.FrameAtTime(-1000, START));
	EXPECT_EQ(-1, fps.FrameAtTime(-1999, START));
}

TEST(lagi_vfr, cfr_frame_at_time_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	EXPECT_EQ(-1, fps.FrameAtTime(0, END));
	EXPECT_EQ(0, fps.FrameAtTime(1, END));
	EXPECT_EQ(0, fps.FrameAtTime(1000, END));
	EXPECT_EQ(1, fps.FrameAtTime(1001, END));
	EXPECT_EQ(99, fps.FrameAtTime(100000, END));
	EXPECT_EQ(-1, fps.FrameAtTime(-1, END));
	EXPECT_EQ(-1, fps.FrameAtTime(-999, END));
	EXPECT_EQ(-2, fps.FrameAtTime(-1000, END));
	EXPECT_EQ(-2, fps.FrameAtTime(-1999, END));
}

TEST(lagi_vfr, cfr_time_at_frame_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	EXPECT_EQ(    0, fps.TimeAtFrame(0));
	EXPECT_EQ( 1000, fps.TimeAtFrame(1));
	EXPECT_EQ( 2000, fps.TimeAtFrame(2));
	EXPECT_EQ(-1000, fps.TimeAtFrame(-1));
}

TEST(lagi_vfr, cfr_time_at_frame_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	EXPECT_RANGE( -999,     0, fps.TimeAtFrame( 0, START));
	EXPECT_RANGE(    1,  1000, fps.TimeAtFrame( 1, START));
	EXPECT_RANGE( 1001,  2000, fps.TimeAtFrame( 2, START));
	EXPECT_RANGE(-1999, -1000, fps.TimeAtFrame(-1, START));
}

TEST(lagi_vfr, cfr_time_at_frame_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	EXPECT_RANGE(   1, 1000, fps.TimeAtFrame( 0, END));
	EXPECT_RANGE(1001, 2000, fps.TimeAtFrame( 1, END));
	EXPECT_RANGE(2001, 3000, fps.TimeAtFrame( 2, END));
	EXPECT_RANGE(-999,    0, fps.TimeAtFrame(-1, END));
}

TEST(lagi_vfr, cfr_round_trip_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i)));
	}
}

TEST(lagi_vfr, cfr_round_trip_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, START), START));
	}
}

TEST(lagi_vfr, cfr_round_trip_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, END), END));
	}
}

TEST(lagi_vfr, vfr_round_trip_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i)));
	}
}

TEST(lagi_vfr, vfr_round_trip_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, START), START));
	}
}

TEST(lagi_vfr, vfr_round_trip_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, END), END));
	}
}

TEST(lagi_vfr, vfr_time_at_frame_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(5, 0, 1000, 1500, 2000, 2001)));
	EXPECT_EQ(0, fps.TimeAtFrame(0));
	EXPECT_EQ(1000, fps.TimeAtFrame(1));
	EXPECT_EQ(1500, fps.TimeAtFrame(2));
	EXPECT_EQ(2000, fps.TimeAtFrame(3));
	EXPECT_EQ(2001, fps.TimeAtFrame(4));
}

TEST(lagi_vfr, vfr_time_at_frame_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	EXPECT_GE(0, fps.TimeAtFrame(0, START));
	EXPECT_RANGE(1, 1000, fps.TimeAtFrame(1, START));
	EXPECT_RANGE(1001, 1500, fps.TimeAtFrame(2, START));
	EXPECT_RANGE(1501, 2000, fps.TimeAtFrame(3, START));
	EXPECT_EQ(2001, fps.TimeAtFrame(4, START));
	EXPECT_EQ(2002, fps.TimeAtFrame(5, START));
	EXPECT_LE(2003, fps.TimeAtFrame(6, START));
}

TEST(lagi_vfr, vfr_time_at_frame_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	EXPECT_RANGE(1, 1000, fps.TimeAtFrame(0, END));
	EXPECT_RANGE(1001, 1500, fps.TimeAtFrame(1, END));
	EXPECT_RANGE(1501, 2000, fps.TimeAtFrame(2, END));
	EXPECT_EQ(2001, fps.TimeAtFrame(3, END));
	EXPECT_EQ(2002, fps.TimeAtFrame(4, END));
	EXPECT_EQ(2003, fps.TimeAtFrame(5, END));
	EXPECT_LE(2004, fps.TimeAtFrame(6, END));
}

TEST(lagi_vfr, vfr_time_at_frame_outside_range) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(3, 0, 100, 200)));
	EXPECT_GT(0, fps.TimeAtFrame(-1));
	EXPECT_EQ(0, fps.TimeAtFrame(0));
	EXPECT_EQ(100, fps.TimeAtFrame(1));
	EXPECT_EQ(200, fps.TimeAtFrame(2));
	EXPECT_LT(200, fps.TimeAtFrame(3));

	int prev = fps.TimeAtFrame(3);
	for (int i = 4; i < 10; i++) {
		EXPECT_LT(prev, fps.TimeAtFrame(i));
		prev = fps.TimeAtFrame(i);
	}
}

TEST(lagi_vfr, vfr_frame_at_time_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	EXPECT_GT(0, fps.FrameAtTime(-1));
	EXPECT_EQ(0, fps.FrameAtTime(0));
	EXPECT_EQ(0, fps.FrameAtTime(999));
	EXPECT_EQ(1, fps.FrameAtTime(1000));
	EXPECT_EQ(1, fps.FrameAtTime(1499));
	EXPECT_EQ(2, fps.FrameAtTime(1500));
	EXPECT_EQ(2, fps.FrameAtTime(1999));
	EXPECT_EQ(3, fps.FrameAtTime(2000));
	EXPECT_EQ(4, fps.FrameAtTime(2001));
	EXPECT_EQ(5, fps.FrameAtTime(2002));
	EXPECT_EQ(6, fps.FrameAtTime(2003));
	EXPECT_LE(6, fps.FrameAtTime(2004));
}
TEST(lagi_vfr, vfr_frame_at_time_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(7, 0, 1000, 1500, 2000, 2001, 2002, 2003)));
	EXPECT_GE(0, fps.FrameAtTime(-1, START));
	EXPECT_EQ(0, fps.FrameAtTime(0, START));
	EXPECT_EQ(1, fps.FrameAtTime(1, START));
	EXPECT_EQ(1, fps.FrameAtTime(1000, START));
	EXPECT_EQ(2, fps.FrameAtTime(1001, START));
	EXPECT_EQ(2, fps.FrameAtTime(1500, START));
	EXPECT_EQ(3, fps.FrameAtTime(1501, START));
	EXPECT_EQ(3, fps.FrameAtTime(2000, START));
	EXPECT_EQ(4, fps.FrameAtTime(2001, START));
	EXPECT_EQ(5, fps.FrameAtTime(2002, START));
	EXPECT_EQ(6, fps.FrameAtTime(2003, START));
	EXPECT_LE(6, fps.FrameAtTime(2004, START));
}

bool validate_save(std::string const& goodFilename, std::string const& checkFilename, int v2Lines = -1, bool allowLonger = false) {
	std::ifstream good(goodFilename.c_str());
	std::ifstream check(checkFilename.c_str());

	EXPECT_TRUE(good.good());
	EXPECT_TRUE(check.good());

	std::string good_header;
	std::string check_header;

	std::getline(good, good_header);
	std::getline(check, check_header);

	// istream_iterator rather than line_reader because we never write comments
	// or empty lines in timecode files
	std::istream_iterator<double> good_iter(good);
	std::istream_iterator<double> check_iter(check);
	std::istream_iterator<double> end;

	int line = 0;
	for (; good_iter != end; ++good_iter, ++check_iter, ++line) {
		if (check_iter == end) return false;
		if (v2Lines < 0 || line < v2Lines) {
			if (*good_iter != *check_iter) return false;
		}
	}
	// v1 timecodes with the end of a range past the end of the video are valid,
	// and when saving those there will be too many timecodes in the v2 file
	if (!allowLonger && check_iter != end) return false;
	return true;
}

TEST(lagi_vfr, validate_save) {
	EXPECT_TRUE(validate_save("data/vfr/in/validate_base.txt", "data/vfr/in/validate_base.txt"));
	EXPECT_FALSE(validate_save("data/vfr/in/validate_base.txt", "data/vfr/in/validate_different.txt"));
	EXPECT_FALSE(validate_save("data/vfr/in/validate_base.txt", "data/vfr/in/validate_shorter.txt"));
	EXPECT_FALSE(validate_save("data/vfr/in/validate_base.txt", "data/vfr/in/validate_longer.txt"));
	EXPECT_TRUE(validate_save("data/vfr/in/validate_base.txt", "data/vfr/in/validate_longer.txt", -1, true));
}

TEST(lagi_vfr, save_vfr_nolen) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(3, 0, 100, 200)));
	ASSERT_NO_THROW(fps.Save("data/vfr/out/v2_nolen.txt"));

	EXPECT_TRUE(validate_save("data/vfr/in/v2_nolen.txt", "data/vfr/out/v2_nolen.txt"));
}

TEST(lagi_vfr, save_vfr_len) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(3, 0, 100, 200)));
	ASSERT_NO_THROW(fps.Save("data/vfr/out/v2_len_3_10.txt", 10));

	EXPECT_TRUE(validate_save("data/vfr/in/v2_len_3_10.txt", "data/vfr/out/v2_len_3_10.txt", 3));
}

TEST(lagi_vfr, load_v2) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v2_1fps.txt"));
	for (int i = 0; i < 9; i++) {
		EXPECT_EQ(i * 1000, fps.TimeAtFrame(i));
	}
}

TEST(lagi_vfr, load_v2_comments) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v2_comments.txt"));
	for (int i = 0; i < 9; i++) {
		EXPECT_EQ(i * 1000, fps.TimeAtFrame(i));
	}
}

TEST(lagi_vfr, load_v2_number_in_comment) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v2_number_in_comment.txt"));
	for (int i = 0; i < 9; i++) {
		EXPECT_EQ(i * 1000, fps.TimeAtFrame(i));
	}
}

TEST(lagi_vfr, load_v1_save_v2) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v1_mode5.txt"));
	EXPECT_NO_THROW(fps.Save("data/vfr/out/v2_mode5.txt"));

	EXPECT_TRUE(validate_save("data/vfr/in/v2_mode5.txt", "data/vfr/out/v2_mode5.txt", -1, true));
}

TEST(lagi_vfr, load_v1_save_v2_len) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v1_assume_30.txt"));
	ASSERT_NO_THROW(fps.Save("data/vfr/out/v2_100_frames_30_fps.txt", 100));
	EXPECT_TRUE(validate_save("data/vfr/in/v2_100_frames_30_fps.txt", "data/vfr/out/v2_100_frames_30_fps.txt"));
}

TEST(lagi_vfr, load_v1_save_v2_ovr) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v1_assume_30_with_override.txt"));
	ASSERT_NO_THROW(fps.Save("data/vfr/out/v2_100_frames_30_with_override.txt", 100));
	EXPECT_TRUE(validate_save("data/vfr/in/v2_100_frames_30_with_override.txt", "data/vfr/out/v2_100_frames_30_with_override.txt"));
}

TEST(lagi_vfr, nonzero_start_time) {
	Framerate fps;

	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(5, 10, 20, 30, 40, 50)));
	EXPECT_EQ(0, fps.TimeAtFrame(0, EXACT));
	EXPECT_EQ(10, fps.TimeAtFrame(1, EXACT));
	EXPECT_EQ(20, fps.TimeAtFrame(2, EXACT));
	EXPECT_EQ(30, fps.TimeAtFrame(3, EXACT));
	EXPECT_EQ(40, fps.TimeAtFrame(4, EXACT));

	ASSERT_NO_THROW(fps = Framerate(make_vector<int>(5, -10, 20, 30, 40, 50)));
	EXPECT_EQ(0, fps.TimeAtFrame(0, EXACT));
	EXPECT_EQ(30, fps.TimeAtFrame(1, EXACT));
	EXPECT_EQ(40, fps.TimeAtFrame(2, EXACT));
	EXPECT_EQ(50, fps.TimeAtFrame(3, EXACT));
	EXPECT_EQ(60, fps.TimeAtFrame(4, EXACT));
}
