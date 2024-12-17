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
// Aegisub Project http://www.aegisub.org/

#include <libaegisub/fs.h>
#include <libaegisub/vfr.h>

#include <climits>
#include <fstream>
#include <iterator>

#include <main.h>
#include <util.h>

using namespace agi::vfr;
using namespace util;

#define EXPECT_RANGE(low, high, test) \
	EXPECT_LE(low, test); \
	EXPECT_GE(high, test)

TEST(lagi_vfr, constructors_good) {
	EXPECT_NO_THROW(Framerate(1.));
	EXPECT_NO_THROW(Framerate(Framerate(1.)));
	EXPECT_NO_THROW(Framerate({ 0, 10 }));

	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_start_equals_end.txt"));
	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_whitespace.txt"));
	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_assume_int.txt"));
	EXPECT_NO_THROW(Framerate("data/vfr/in/v1_out_of_order.txt"));
}

TEST(lagi_vfr, constructors_bad_cfr) {
	EXPECT_THROW(Framerate(-1.), InvalidFramerate);
	EXPECT_THROW(Framerate(1000.1), InvalidFramerate);
}

TEST(lagi_vfr, constructors_bad_timecodes) {
	EXPECT_THROW(Framerate(std::initializer_list<int>{}), InvalidFramerate);
	EXPECT_THROW(Framerate({0}), InvalidFramerate);
	EXPECT_THROW(Framerate({10, 0}), InvalidFramerate);
	EXPECT_THROW(Framerate({0, 0}), InvalidFramerate);
}

TEST(lagi_vfr, constructors_bad_v1) {
	EXPECT_THROW(Framerate("data/vfr/in/v1_bad_seperators.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_too_few_parts.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_too_many_parts.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_float_frame_number.txt"), MalformedLine);
	EXPECT_THROW(Framerate("data/vfr/in/v1_start_end_overlap.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v1_fully_contained.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v1_assume_over_1000.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v1_override_over_1000.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v1_override_zero.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v1_negative_start_of_range.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v1_end_less_than_start.txt"), InvalidFramerate);
}

TEST(lagi_vfr, constructors_bad_v2) {
	EXPECT_THROW(Framerate("data/vfr/in/v2_empty.txt"), InvalidFramerate);
	EXPECT_THROW(Framerate("data/vfr/in/v2_out_of_order.txt"), InvalidFramerate);

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

	ASSERT_NO_THROW(fps = Framerate(24000, 1001));
	int frames[] = {-100, -10, -1, 0, 1, 10, 100, 6820};
	for (int i : frames) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i)));
	}
}

TEST(lagi_vfr, cfr_round_trip_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, START), START));
	}

	ASSERT_NO_THROW(fps = Framerate(24000, 1001));
	int frames[] = {-100, -10, -1, 0, 1, 10, 100, 6820};
	for (int i : frames) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, START), START));
	}
}

TEST(lagi_vfr, cfr_round_trip_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(1.));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, END), END));
	}

	ASSERT_NO_THROW(fps = Framerate(24000, 1001));
	int frames[] = {-100, -10, -1, 0, 1, 10, 100, 6820};
	for (int i : frames) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, END), END));
	}
}

TEST(lagi_vfr, vfr_round_trip_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i)));
	}
}

TEST(lagi_vfr, vfr_round_trip_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, START), START));
	}
}

TEST(lagi_vfr, vfr_round_trip_end) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
	for (int i = -10; i < 11; i++) {
		EXPECT_EQ(i, fps.FrameAtTime(fps.TimeAtFrame(i, END), END));
	}
}

TEST(lagi_vfr, vfr_time_at_frame_exact) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001 }));
	EXPECT_EQ(0, fps.TimeAtFrame(0));
	EXPECT_EQ(1000, fps.TimeAtFrame(1));
	EXPECT_EQ(1500, fps.TimeAtFrame(2));
	EXPECT_EQ(2000, fps.TimeAtFrame(3));
	EXPECT_EQ(2001, fps.TimeAtFrame(4));
}

TEST(lagi_vfr, vfr_time_at_frame_start) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
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
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
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
	ASSERT_NO_THROW(fps = Framerate({ 0, 100, 200 }));
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
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
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
	ASSERT_NO_THROW(fps = Framerate({ 0, 1000, 1500, 2000, 2001, 2002, 2003 }));
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
	ASSERT_NO_THROW(fps = Framerate({ 0, 100, 200 }));
	ASSERT_NO_THROW(fps.Save("data/vfr/out/v2_nolen.txt"));

	EXPECT_TRUE(validate_save("data/vfr/in/v2_nolen.txt", "data/vfr/out/v2_nolen.txt"));
}

TEST(lagi_vfr, save_vfr_len) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 100, 200 }));
	ASSERT_NO_THROW(fps.Save("data/vfr/out/v2_len_3_10.txt", 10));

	EXPECT_TRUE(validate_save("data/vfr/in/v2_len_3_10.txt", "data/vfr/out/v2_len_3_10.txt", 3));
}

TEST(lagi_vfr, load_v2) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v2_1fps.txt"));
	for (int i = 0; i < 30; i++) {
		EXPECT_EQ(i * 1000, fps.TimeAtFrame(i));
	}
}

TEST(lagi_vfr, load_v2_comments) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v2_comments.txt"));
	for (int i = 0; i < 30; i++) {
		EXPECT_EQ(i * 1000, fps.TimeAtFrame(i));
	}
}

TEST(lagi_vfr, load_v2_number_in_comment) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate("data/vfr/in/v2_number_in_comment.txt"));
	for (int i = 0; i < 30; i++) {
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

	ASSERT_NO_THROW(fps = Framerate({ 10, 20, 30, 40, 50 }));
	EXPECT_EQ(0, fps.TimeAtFrame(0, EXACT));
	EXPECT_EQ(10, fps.TimeAtFrame(1, EXACT));
	EXPECT_EQ(20, fps.TimeAtFrame(2, EXACT));
	EXPECT_EQ(30, fps.TimeAtFrame(3, EXACT));
	EXPECT_EQ(40, fps.TimeAtFrame(4, EXACT));

	ASSERT_NO_THROW(fps = Framerate({ -10, 20, 30, 40, 50 }));
	EXPECT_EQ(0, fps.TimeAtFrame(0, EXACT));
	EXPECT_EQ(30, fps.TimeAtFrame(1, EXACT));
	EXPECT_EQ(40, fps.TimeAtFrame(2, EXACT));
	EXPECT_EQ(50, fps.TimeAtFrame(3, EXACT));
	EXPECT_EQ(60, fps.TimeAtFrame(4, EXACT));
}

TEST(lagi_vfr, rational_timebase) {
	Framerate fps;

	ASSERT_NO_THROW(fps = Framerate(30000, 1001));
	for (int i = 0; i < 100000; ++i) {
		EXPECT_EQ(i * 1001, fps.TimeAtFrame(i * 30, EXACT));
		EXPECT_EQ(i * 30, fps.FrameAtTime(i * 1001, EXACT));
	}

	ASSERT_NO_THROW(fps = Framerate(24000, 1001));
	for (int i = 0; i < 100000; ++i) {
		EXPECT_EQ(i * 1001, fps.TimeAtFrame(i * 24, EXACT));
		EXPECT_EQ(i * 24, fps.FrameAtTime(i * 1001, EXACT));
	}
}

TEST(lagi_vfr, no_intermediate_overflow) {
	Framerate fps;

	ASSERT_NO_THROW(fps = Framerate(1.0));
	int last_frame = INT_MAX / 1000;
	EXPECT_EQ(last_frame * 1000, fps.TimeAtFrame(last_frame, EXACT));
	EXPECT_EQ(last_frame, fps.FrameAtTime(last_frame * 1000, EXACT));
}

TEST(lagi_vfr, duplicate_timestamps) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate({ 0, 0, 1, 2, 2, 3 }));

	EXPECT_EQ(1, fps.FrameAtTime(0, EXACT));
	EXPECT_EQ(2, fps.FrameAtTime(1, EXACT));
	EXPECT_EQ(4, fps.FrameAtTime(2, EXACT));
	EXPECT_EQ(5, fps.FrameAtTime(3, EXACT));

	EXPECT_EQ(0, fps.TimeAtFrame(0, EXACT));
	EXPECT_EQ(0, fps.TimeAtFrame(1, EXACT));
	EXPECT_EQ(1, fps.TimeAtFrame(2, EXACT));
	EXPECT_EQ(2, fps.TimeAtFrame(3, EXACT));
	EXPECT_EQ(2, fps.TimeAtFrame(4, EXACT));
	EXPECT_EQ(3, fps.TimeAtFrame(5, EXACT));

	ASSERT_NO_THROW(fps = Framerate({ 0, 100, 100, 200, 300 }));

	EXPECT_EQ(0, fps.FrameAtTime(0, EXACT));
	EXPECT_EQ(0, fps.FrameAtTime(99, EXACT));
	EXPECT_EQ(2, fps.FrameAtTime(100, EXACT));
	EXPECT_EQ(2, fps.FrameAtTime(199, EXACT));
	EXPECT_EQ(3, fps.FrameAtTime(200, EXACT));
}

#define EXPECT_SMPTE(eh, em, es, ef) \
	EXPECT_EQ(eh, h); \
	EXPECT_EQ(em, m); \
	EXPECT_EQ(es, s); \
	EXPECT_EQ(ef, f)

TEST(lagi_vfr, to_smpte_ntsc) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(30000, 1001));

	EXPECT_TRUE(fps.NeedsDropFrames());

	int h = -1, m = -1, s = -1, f = -1;

	ASSERT_NO_THROW(fps.SmpteAtFrame(0, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 1);

	ASSERT_NO_THROW(fps.SmpteAtFrame(29, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 29);

	ASSERT_NO_THROW(fps.SmpteAtFrame(30, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 1, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1799, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 59, 29);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1800, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 1, 0, 2);

	ASSERT_NO_THROW(fps.SmpteAtFrame(3597, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 1, 59, 29);

	ASSERT_NO_THROW(fps.SmpteAtFrame(3598, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 2, 0, 2);

	ASSERT_NO_THROW(fps.SmpteAtFrame(5396, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 3, 0, 2);

	ASSERT_NO_THROW(fps.SmpteAtFrame(7194, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 4, 0, 2);

	ASSERT_NO_THROW(fps.SmpteAtFrame(107892, &h, &m, &s, &f));
	EXPECT_SMPTE(1, 0, 0, 0);

	for (int i = 0; i < 60 * 60 * 10; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtTime(i * 1000, &h, &m, &s, &f));
		ASSERT_NEAR(i, h * 3600 + m * 60 + s, 1);
	}
}

TEST(lagi_vfr, to_smpte_double_ntsc) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(60000, 1001));

	EXPECT_TRUE(fps.NeedsDropFrames());

	int h = -1, m = -1, s = -1, f = -1;

	ASSERT_NO_THROW(fps.SmpteAtFrame(0, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 1);

	ASSERT_NO_THROW(fps.SmpteAtFrame(59, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 59);

	ASSERT_NO_THROW(fps.SmpteAtFrame(60, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 1, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(3599, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 59, 59);

	ASSERT_NO_THROW(fps.SmpteAtFrame(3600, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 1, 0, 4);

	ASSERT_NO_THROW(fps.SmpteAtFrame(7195, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 1, 59, 59);

	ASSERT_NO_THROW(fps.SmpteAtFrame(7196, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 2, 0, 4);

	ASSERT_NO_THROW(fps.SmpteAtFrame(215784, &h, &m, &s, &f));
	EXPECT_SMPTE(1, 0, 0, 0);

	for (int i = 0; i < 60 * 60 * 10; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtTime(i * 1000, &h, &m, &s, &f));
		ASSERT_NEAR(i, h * 3600 + m * 60 + s, 1);
	}
}

TEST(lagi_vfr, to_smpte_pal) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(25, 1));

	EXPECT_FALSE(fps.NeedsDropFrames());

	int h = -1, m = -1, s = -1, f = -1;

	ASSERT_NO_THROW(fps.SmpteAtFrame(0, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 1);

	ASSERT_NO_THROW(fps.SmpteAtFrame(24, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 24);

	ASSERT_NO_THROW(fps.SmpteAtFrame(25, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 1, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1499, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 59, 24);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1500, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 1, 0, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(25 * 60 * 60, &h, &m, &s, &f));
	EXPECT_SMPTE(1, 0, 0, 0);

	for (int i = 0; i < 60 * 60 * 10; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtTime(i * 1000, &h, &m, &s, &f));
		ASSERT_EQ(i, h * 3600 + m * 60 + s);
	}
}

// this test is different from the above due to that the exact frames which are
// skipped are undefined, so instead it tests that the error never exceeds the
// limit
TEST(lagi_vfr, to_smpte_decimated) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(24000, 1001));

	EXPECT_TRUE(fps.NeedsDropFrames());

	int h = -1, m = -1, s = -1, f = -1;

	ASSERT_NO_THROW(fps.SmpteAtFrame(0, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 1);

	for (int frame = 0; frame < 100000; ++frame) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(frame, &h, &m, &s, &f));
		int expected_time = fps.TimeAtFrame(frame);
		int real_time = int((h * 3600 + m * 60 + s + f / 24.0) * 1000.0);
		ASSERT_NEAR(expected_time, real_time, 600.0 / fps.FPS());
	}
}

TEST(lagi_vfr, to_smpte_manydrop) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(24, 11));

	EXPECT_TRUE(fps.NeedsDropFrames());

	int h = -1, m = -1, s = -1, f = -1;

	ASSERT_NO_THROW(fps.SmpteAtFrame(0, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 0);

	ASSERT_NO_THROW(fps.SmpteAtFrame(1, &h, &m, &s, &f));
	EXPECT_SMPTE(0, 0, 0, 1);

	for (int frame = 0; frame < 1000; ++frame) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(frame, &h, &m, &s, &f));
		int expected_time = fps.TimeAtFrame(frame);
		int real_time = int((h * 3600 + m * 60 + s + f / 3.0) * 1000.0);
		ASSERT_NEAR(expected_time, real_time, 600.0 / fps.FPS());
	}
}

TEST(lagi_vfr, from_smpte_ntsc) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(30000, 1001));

	EXPECT_EQ(0, fps.FrameAtSmpte(0, 0, 0, 0));
	EXPECT_EQ(1, fps.FrameAtSmpte(0, 0, 0, 1));
	EXPECT_EQ(29, fps.FrameAtSmpte(0, 0, 0, 29));
	EXPECT_EQ(30, fps.FrameAtSmpte(0, 0, 1, 0));
	EXPECT_EQ(1799, fps.FrameAtSmpte(0, 0, 59, 29));
	EXPECT_EQ(1800, fps.FrameAtSmpte(0, 1, 0, 0));
	EXPECT_EQ(1800, fps.FrameAtSmpte(0, 1, 0, 1));
	EXPECT_EQ(1800, fps.FrameAtSmpte(0, 1, 0, 2));
	EXPECT_EQ(3597, fps.FrameAtSmpte(0, 1, 59, 29));
	EXPECT_EQ(3598, fps.FrameAtSmpte(0, 2, 0, 0));
	EXPECT_EQ(3598, fps.FrameAtSmpte(0, 2, 0, 1));
	EXPECT_EQ(3598, fps.FrameAtSmpte(0, 2, 0, 2));
	EXPECT_EQ(5396, fps.FrameAtSmpte(0, 3, 0, 2));
	EXPECT_EQ(7194, fps.FrameAtSmpte(0, 4, 0, 2));
	EXPECT_EQ(107892, fps.FrameAtSmpte(1, 0, 0, 0));
}

TEST(lagi_vfr, from_smpte_double_ntsc) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(60000, 1001));

	EXPECT_TRUE(fps.NeedsDropFrames());

	EXPECT_EQ(0, fps.FrameAtSmpte(0, 0, 0, 0));
	EXPECT_EQ(1, fps.FrameAtSmpte(0, 0, 0, 1));
	EXPECT_EQ(59, fps.FrameAtSmpte(0, 0, 0, 59));
	EXPECT_EQ(60, fps.FrameAtSmpte(0, 0, 1, 0));
	EXPECT_EQ(3599, fps.FrameAtSmpte(0, 0, 59, 59));
	EXPECT_EQ(3600, fps.FrameAtSmpte(0, 1, 0, 4));
	EXPECT_EQ(7195, fps.FrameAtSmpte(0, 1, 59, 59));
	EXPECT_EQ(7196, fps.FrameAtSmpte(0, 2, 0, 4));
	EXPECT_EQ(10792, fps.FrameAtSmpte(0, 3, 0, 0));
	EXPECT_EQ(10792, fps.FrameAtSmpte(0, 3, 0, 1));
	EXPECT_EQ(10792, fps.FrameAtSmpte(0, 3, 0, 2));
	EXPECT_EQ(10792, fps.FrameAtSmpte(0, 3, 0, 3));
	EXPECT_EQ(10792, fps.FrameAtSmpte(0, 3, 0, 4));
	EXPECT_EQ(10793, fps.FrameAtSmpte(0, 3, 0, 5));
	EXPECT_EQ(215784, fps.FrameAtSmpte(1, 0, 0, 0));
}

TEST(lagi_vfr, from_smpte_pal) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(25, 1));

	EXPECT_FALSE(fps.NeedsDropFrames());

	EXPECT_EQ(0, fps.FrameAtSmpte(0, 0, 0, 0));
	EXPECT_EQ(1, fps.FrameAtSmpte(0, 0, 0, 1));
	EXPECT_EQ(24, fps.FrameAtSmpte(0, 0, 0, 24));
	EXPECT_EQ(25, fps.FrameAtSmpte(0, 0, 1, 0));
	EXPECT_EQ(1499, fps.FrameAtSmpte(0, 0, 59, 24));
	EXPECT_EQ(1500, fps.FrameAtSmpte(0, 1, 0, 0));
	EXPECT_EQ(25 * 60 * 60, fps.FrameAtSmpte(1, 0, 0, 0));
}

TEST(lagi_vfr, roundtrip_smpte_ntsc) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(30000, 1001));

	int h = -1, m = -1, s = -1, f = -1;

	for (int i = 0; i < 100000; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(i, &h, &m, &s, &f));
		ASSERT_EQ(i, fps.FrameAtSmpte(h, m, s, f));
	}
}

TEST(lagi_vfr, roundtrip_smpte_pal) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(25, 1));

	int h = -1, m = -1, s = -1, f = -1;

	for (int i = 0; i < 100000; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(i, &h, &m, &s, &f));
		ASSERT_EQ(i, fps.FrameAtSmpte(h, m, s, f));
	}
}

TEST(lagi_vfr, roundtrip_smpte_manydrop) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(20, 11));

	int h = -1, m = -1, s = -1, f = -1;

	for (int i = 0; i < 10000; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(i, &h, &m, &s, &f));
		ASSERT_EQ(i, fps.FrameAtSmpte(h, m, s, f));
	}
}

TEST(lagi_vfr, roundtrip_smpte_decimated) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(24000, 1001));

	int h = -1, m = -1, s = -1, f = -1;

	for (int i = 0; i < 100000; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(i, &h, &m, &s, &f));
		ASSERT_EQ(i, fps.FrameAtSmpte(h, m, s, f));
	}
}

TEST(lagi_vfr, to_smpte_ntsc_nodrop) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(30000, 1001, false));

	int h = -1, m = -1, s = -1, f = -1;

	for (int i = 0; i < 100000; ++i) {
		ASSERT_NO_THROW(fps.SmpteAtFrame(i, &h, &m, &s, &f));
		ASSERT_EQ(i, h * 60 * 60 * 30 + m * 60 * 30 + s * 30 + f);
	}
}

TEST(lagi_vfr, from_smpte_ntsc_nodrop) {
	Framerate fps;
	ASSERT_NO_THROW(fps = Framerate(30000, 1001, false));

	int h = 0, m = 0, s = 0, f = 0;

	int i = 0;
	while (h < 10) {
		if (f >= 30) {
			f = 0;
			++s;
		}

		if (s >= 60) {
			s = 0;
			++m;
		}

		if (m >= 60) {
			m = 0;
			++h;
		}

		ASSERT_EQ(i, fps.FrameAtSmpte(h, m, s, f));
		++i;
		++f;
	}
}
