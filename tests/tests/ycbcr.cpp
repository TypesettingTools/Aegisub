// Copyright (c) 2026, arch1t3cht <arch1t3cht@gmail.com>
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
// Aegisub Project https://aegisub.org/

#include <libaegisub/ycbcr.h>

#include <main.h>

using agi::ycbcr_matrix;
using agi::ycbcr_range;
namespace ycbcr = agi::ycbcr;

TEST(lagi_ycbcr, parsing) {
	EXPECT_EQ(ycbcr::Header(""), ycbcr::Header(ycbcr::header_missing{}));
	EXPECT_EQ(ycbcr::Header("   "), ycbcr::Header(ycbcr::header_missing{}));
	EXPECT_EQ(ycbcr::Header("\t"), ycbcr::Header(ycbcr::header_missing{}));

	EXPECT_EQ(ycbcr::Header("."), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("foo"), ycbcr::Header(ycbcr::header_invalid{}));

	EXPECT_EQ(ycbcr::Header("none"), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header("None"), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header("\tNONE  "), ycbcr::Header(ycbcr::header_none{}));

	EXPECT_EQ(ycbcr::Header("tv.709"), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header("TV.709"), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG));

	EXPECT_EQ(ycbcr::Header("pc.709"), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header("PC.709"), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG));

	EXPECT_EQ(ycbcr::Header("tv.601"), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header("TV.601"), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header("tv.601"), ycbcr::Header(ycbcr_matrix::BT470BG, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header("TV.601"), ycbcr::Header(ycbcr_matrix::BT470BG, ycbcr_range::MPEG));

	EXPECT_EQ(ycbcr::Header("pc.601"), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header("PC.601"), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG));

	EXPECT_EQ(ycbcr::Header("tv.fcc"), ycbcr::Header(ycbcr_matrix::FCC, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header("pc.240m"), ycbcr::Header(ycbcr_matrix::SMPTE240M, ycbcr_range::JPEG));

	EXPECT_EQ(ycbcr::Header("TV"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("709"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("TV..709"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("TV.709."), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("TV709."), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("TV709"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header(".TV709"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("709.TV"), ycbcr::Header(ycbcr::header_invalid{}));

	EXPECT_EQ(ycbcr::Header("TV.2020"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("TV.P3"), ycbcr::Header(ycbcr::header_invalid{}));
	EXPECT_EQ(ycbcr::Header("TV.RGB"), ycbcr::Header(ycbcr::header_invalid{}));
}

TEST(lagi_ycbcr, to_string_roundtrip) {
	std::vector<std::string> headers({
		"",
		"None",
		"TV.709",
		"PC.709",
		"TV.601",
		"PC.601",
		"TV.FCC",
		"PC.FCC",
		"TV.240M",
		"PC.240M",
	});

	for (auto const& header : headers) {
		EXPECT_TRUE(ycbcr::Header(header).valid());
		EXPECT_EQ(ycbcr::Header(header).to_string(), header);
	}
}

TEST(lagi_ycbcr, to_effective) {
	EXPECT_EQ(ycbcr::Header(ycbcr::header_missing{}).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr::header_invalid{}).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT2020_NCL, ycbcr_range::MPEG).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::YCoCg, ycbcr_range::JPEG).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::RGB, ycbcr_range::JPEG).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::ICtCp, ycbcr_range::MPEG).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::Unspecified).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::Unspecified, ycbcr_range::MPEG).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::Unspecified, ycbcr_range::Unspecified).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));

	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG).to_effective(), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG).to_effective(), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE240M, ycbcr_range::JPEG).to_effective(), ycbcr::Header(ycbcr_matrix::SMPTE240M, ycbcr_range::JPEG));

	EXPECT_EQ(ycbcr::Header(ycbcr::header_none{}).to_effective(), ycbcr::Header(ycbcr::header_none{}));
}

TEST(lagi_ycbcr, to_recommended) {
	EXPECT_EQ(ycbcr::Header(ycbcr::header_missing{}).to_existing(), ycbcr::Header(ycbcr::header_missing()));
	EXPECT_EQ(ycbcr::Header(ycbcr::header_invalid{}).to_existing(), ycbcr::Header(ycbcr::header_missing()));

	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT2020_NCL, ycbcr_range::MPEG).to_existing(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::YCoCg, ycbcr_range::JPEG).to_existing(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::RGB, ycbcr_range::JPEG).to_existing(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::ICtCp, ycbcr_range::MPEG).to_existing(), ycbcr::Header(ycbcr::header_none{}));

	// No tests for Unspecified matrix/range as to_existing ideally shouldn't be called on that at all.

	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG).to_existing(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG).to_existing(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG).to_existing(), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG).to_existing(), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE240M, ycbcr_range::JPEG).to_existing(), ycbcr::Header(ycbcr_matrix::SMPTE240M, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::FCC, ycbcr_range::MPEG).to_existing(), ycbcr::Header(ycbcr_matrix::FCC, ycbcr_range::MPEG));

	EXPECT_EQ(ycbcr::Header(ycbcr::header_none{}).to_existing(), ycbcr::Header(ycbcr::header_none{}));
}

TEST(lagi_ycbcr, to_best_practice) {
	EXPECT_EQ(ycbcr::Header(ycbcr::header_missing{}).to_best_practice(), ycbcr::Header(ycbcr::header_missing()));
	EXPECT_EQ(ycbcr::Header(ycbcr::header_invalid{}).to_best_practice(), ycbcr::Header(ycbcr::header_missing()));

	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT2020_NCL, ycbcr_range::MPEG).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::YCoCg, ycbcr_range::JPEG).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::RGB, ycbcr_range::JPEG).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::ICtCp, ycbcr_range::MPEG).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));

	// No tests for Unspecified matrix/range here either

	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG).to_best_practice(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG).to_best_practice(), ycbcr::Header(ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG).to_best_practice(), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::MPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG).to_best_practice(), ycbcr::Header(ycbcr_matrix::BT709, ycbcr_range::JPEG));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::SMPTE240M, ycbcr_range::JPEG).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));
	EXPECT_EQ(ycbcr::Header(ycbcr_matrix::FCC, ycbcr_range::MPEG).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));

	EXPECT_EQ(ycbcr::Header(ycbcr::header_none{}).to_best_practice(), ycbcr::Header(ycbcr::header_none{}));
}

TEST(lagi_ycbcr, override_colorspace) {
	// Matrices other than None completely overrride the matrix
	std::vector<std::pair<ycbcr_matrix, ycbcr_range>> colorspaces({
		{ycbcr_matrix::Unspecified, ycbcr_range::Unspecified},
		{ycbcr_matrix::Unspecified, ycbcr_range::JPEG},
		{ycbcr_matrix::SMPTE170M, ycbcr_range::Unspecified},
		{ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG},
		{ycbcr_matrix::BT709, ycbcr_range::MPEG},
		{ycbcr_matrix::BT2020_NCL, ycbcr_range::MPEG},
		{ycbcr_matrix::ICtCp, ycbcr_range::JPEG},
		{ycbcr_matrix::YCoCg, ycbcr_range::JPEG},
	});

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::SMPTE170M);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("foo").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::SMPTE170M);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("TV.601").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::SMPTE170M);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("TV.709").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::BT709);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("PC.709").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::BT709);
		EXPECT_EQ(CR, ycbcr_range::JPEG);
	}

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("PC.FCC").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::FCC);
		EXPECT_EQ(CR, ycbcr_range::JPEG);
	}

	for (auto [CM, CR] : colorspaces) {
		ycbcr::Header("TV.240M").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::SMPTE240M);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	// None will preserve color spaces
	std::vector<std::pair<ycbcr_matrix, ycbcr_range>> fullcolorspaces({
		{ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG},
		{ycbcr_matrix::BT709, ycbcr_range::MPEG},
		{ycbcr_matrix::BT2020_NCL, ycbcr_range::MPEG},
		{ycbcr_matrix::ICtCp, ycbcr_range::JPEG},
		{ycbcr_matrix::YCoCg, ycbcr_range::JPEG},
	});

	for (auto [m, r] : fullcolorspaces) {
		auto CM = m;
		auto CR = r;
		ycbcr::Header("None").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, m);
		EXPECT_EQ(CR, r);
	}

	// None will guess unspecified color space parameters
	{
		ycbcr_matrix CM = ycbcr_matrix::Unspecified;
		ycbcr_range CR = ycbcr_range::Unspecified;
		ycbcr::Header("None").override_colorspace(CM, CR, 1920, 1080);
		EXPECT_EQ(CM, ycbcr_matrix::BT709);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	{
		ycbcr_matrix CM = ycbcr_matrix::Unspecified;
		ycbcr_range CR = ycbcr_range::Unspecified;
		ycbcr::Header("None").override_colorspace(CM, CR, 720, 480);
		EXPECT_EQ(CM, ycbcr_matrix::SMPTE170M);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	{
		ycbcr_matrix CM = ycbcr_matrix::BT2020_NCL;
		ycbcr_range CR = ycbcr_range::Unspecified;
		ycbcr::Header("None").override_colorspace(CM, CR, 720, 480);
		EXPECT_EQ(CM, ycbcr_matrix::BT2020_NCL);
		EXPECT_EQ(CR, ycbcr_range::MPEG);
	}

	{
		ycbcr_matrix CM = ycbcr_matrix::Unspecified;
		ycbcr_range CR = ycbcr_range::JPEG;
		ycbcr::Header("None").override_colorspace(CM, CR, 720, 480);
		EXPECT_EQ(CM, ycbcr_matrix::SMPTE170M);
		EXPECT_EQ(CR, ycbcr_range::JPEG);
	}
}

TEST(lagi_ycbcr, guess_colorspace) {
	std::vector<std::pair<int, int>> resolutions601({
		{640, 360},
		{720, 480},
		{768, 576},
	});

	std::vector<std::pair<int, int>> resolutions709({
		{1280, 720},
		{1920, 1080},
		{1080, 1920},
		{3840, 2160},
	});

	// Guessing does not change tagged resolutions
	std::vector<std::pair<ycbcr_matrix, ycbcr_range>> fullcolorspaces({
		{ycbcr_matrix::SMPTE170M, ycbcr_range::JPEG},
		{ycbcr_matrix::BT709, ycbcr_range::MPEG},
		{ycbcr_matrix::BT2020_NCL, ycbcr_range::MPEG},
		{ycbcr_matrix::ICtCp, ycbcr_range::JPEG},
		{ycbcr_matrix::YCoCg, ycbcr_range::JPEG},
	});

	for (auto [m, r] : fullcolorspaces) {
		for (auto [w, h] : resolutions601) {
			auto CM = m;
			auto CR = r;
			agi::ycbcr::guess_colorspace(CM, CR, w, h);
			EXPECT_EQ(CM, m);
			EXPECT_EQ(CR, r);
		}

		for (auto [w, h] : resolutions709) {
			auto CM = m;
			auto CR = r;
			agi::ycbcr::guess_colorspace(CM, CR, w, h);
			EXPECT_EQ(CM, m);
			EXPECT_EQ(CR, r);
		}
	}

	// Range is always guessed as MPEG
	std::vector<ycbcr_matrix> matrices({
		ycbcr_matrix::SMPTE170M,
		ycbcr_matrix::BT709,
		ycbcr_matrix::BT2020_NCL,
		ycbcr_matrix::ICtCp,
		ycbcr_matrix::YCoCg,
	});

	for (auto matrix : matrices) {
		for (auto [w, h] : resolutions601) {
			auto CM = matrix;
			auto CR = ycbcr_range::Unspecified;
			agi::ycbcr::guess_colorspace(CM, CR, w, h);
			EXPECT_EQ(CM, matrix);
			EXPECT_EQ(CR, ycbcr_range::MPEG);
		}

		for (auto [w, h] : resolutions709) {
			auto CM = matrix;
			auto CR = ycbcr_range::Unspecified;
			agi::ycbcr::guess_colorspace(CM, CR, w, h);
			EXPECT_EQ(CM, matrix);
			EXPECT_EQ(CR, ycbcr_range::MPEG);
		}
	}

	// Color matrix is guessed as 601 or 709
	std::vector<ycbcr_range> ranges({
		ycbcr_range::Unspecified,
		ycbcr_range::MPEG,
		ycbcr_range::JPEG,
	});

	for (auto range : ranges) {
		for (auto [w, h] : resolutions601) {
			auto CM = ycbcr_matrix::Unspecified;
			auto CR = range;
			agi::ycbcr::guess_colorspace(CM, CR, w, h);
			EXPECT_EQ(CM, ycbcr_matrix::SMPTE170M);
			EXPECT_EQ(CR, range == ycbcr_range::Unspecified ? ycbcr_range::MPEG : range);
		}

		for (auto [w, h] : resolutions709) {
			auto CM = ycbcr_matrix::Unspecified;
			auto CR = range;
			agi::ycbcr::guess_colorspace(CM, CR, w, h);
			EXPECT_EQ(CM, ycbcr_matrix::BT709);
			EXPECT_EQ(CR, range == ycbcr_range::Unspecified ? ycbcr_range::MPEG : range);
		}
	}
}
