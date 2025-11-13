// Copyright (c) 2025, Aegisub Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/// @file cinecanvas.cpp
/// @brief Unit tests for CineCanvas subtitle format color and timing conversion
/// @ingroup subtitle_io

#include <libaegisub/color.h>
#include <libaegisub/ass/time.h>
#include <libaegisub/vfr.h>

#include "../../src/subtitle_format_cinecanvas.h"

#include <main.h>

class CineCanvasColorTest : public ::testing::Test {
protected:
	CineCanvasSubtitleFormat format;
};

// Test basic RGB colors with full opacity (ASS alpha = 0)
TEST_F(CineCanvasColorTest, BasicColorsFullyOpaque) {
	// White: RGB(255, 255, 255) with ASS alpha 0 (opaque) -> FFFFFFFF (Cinema FF = opaque)
	EXPECT_EQ("FFFFFFFF", format.ConvertColorToRGBA(agi::Color(255, 255, 255), 0));

	// Black: RGB(0, 0, 0) with ASS alpha 0 (opaque) -> 000000FF (Cinema FF = opaque)
	EXPECT_EQ("000000FF", format.ConvertColorToRGBA(agi::Color(0, 0, 0), 0));

	// Red: RGB(255, 0, 0) with ASS alpha 0 (opaque) -> FF0000FF
	EXPECT_EQ("FF0000FF", format.ConvertColorToRGBA(agi::Color(255, 0, 0), 0));

	// Green: RGB(0, 255, 0) with ASS alpha 0 (opaque) -> 00FF00FF
	EXPECT_EQ("00FF00FF", format.ConvertColorToRGBA(agi::Color(0, 255, 0), 0));

	// Blue: RGB(0, 0, 255) with ASS alpha 0 (opaque) -> 0000FFFF
	EXPECT_EQ("0000FFFF", format.ConvertColorToRGBA(agi::Color(0, 0, 255), 0));
}

// Test basic RGB colors with full transparency (ASS alpha = 255)
TEST_F(CineCanvasColorTest, BasicColorsFullyTransparent) {
	// White with ASS alpha 255 (transparent) -> FFFFFF00 (Cinema 00 = transparent)
	EXPECT_EQ("FFFFFF00", format.ConvertColorToRGBA(agi::Color(255, 255, 255), 255));

	// Black with ASS alpha 255 (transparent) -> 00000000
	EXPECT_EQ("00000000", format.ConvertColorToRGBA(agi::Color(0, 0, 0), 255));

	// Red with ASS alpha 255 (transparent) -> FF000000
	EXPECT_EQ("FF000000", format.ConvertColorToRGBA(agi::Color(255, 0, 0), 255));
}

// Test alpha inversion at various levels
TEST_F(CineCanvasColorTest, AlphaInversion) {
	agi::Color white(255, 255, 255);

	// ASS alpha 0 (fully opaque) -> Cinema FF (255)
	EXPECT_EQ("FFFFFFFF", format.ConvertColorToRGBA(white, 0));

	// ASS alpha 64 (mostly opaque) -> Cinema BF (191 = 255 - 64)
	EXPECT_EQ("FFFFFFBF", format.ConvertColorToRGBA(white, 64));

	// ASS alpha 128 (50% transparent) -> Cinema 7F (127 = 255 - 128)
	EXPECT_EQ("FFFFFF7F", format.ConvertColorToRGBA(white, 128));

	// ASS alpha 191 (mostly transparent) -> Cinema 40 (64 = 255 - 191)
	EXPECT_EQ("FFFFFF40", format.ConvertColorToRGBA(white, 191));

	// ASS alpha 255 (fully transparent) -> Cinema 00 (0)
	EXPECT_EQ("FFFFFF00", format.ConvertColorToRGBA(white, 255));
}

// Test default alpha parameter (should be 255 in ASS format, meaning transparent)
TEST_F(CineCanvasColorTest, DefaultAlphaParameter) {
	// Default alpha is 255 (ASS transparent), which converts to 00 (Cinema transparent)
	EXPECT_EQ("FFFFFF00", format.ConvertColorToRGBA(agi::Color(255, 255, 255)));
	EXPECT_EQ("00000000", format.ConvertColorToRGBA(agi::Color(0, 0, 0)));
}

// Test semi-transparent colors (from documentation examples)
TEST_F(CineCanvasColorTest, SemiTransparentColors) {
	// Semi-transparent white: ASS alpha 128 (0x80) -> Cinema 7F (0x7F = 127)
	EXPECT_EQ("FFFFFF7F", format.ConvertColorToRGBA(agi::Color(255, 255, 255), 128));

	// Semi-transparent red: ASS alpha 128 -> Cinema 7F
	EXPECT_EQ("FF00007F", format.ConvertColorToRGBA(agi::Color(255, 0, 0), 128));
}

// Test mixed RGB colors with various alpha values
TEST_F(CineCanvasColorTest, MixedColors) {
	// Yellow: RGB(255, 255, 0) with opaque -> FFFF00FF
	EXPECT_EQ("FFFF00FF", format.ConvertColorToRGBA(agi::Color(255, 255, 0), 0));

	// Cyan: RGB(0, 255, 255) with opaque -> 00FFFFFF
	EXPECT_EQ("00FFFFFF", format.ConvertColorToRGBA(agi::Color(0, 255, 255), 0));

	// Magenta: RGB(255, 0, 255) with opaque -> FF00FFFF
	EXPECT_EQ("FF00FFFF", format.ConvertColorToRGBA(agi::Color(255, 0, 255), 0));

	// Gray: RGB(128, 128, 128) with opaque -> 808080FF
	EXPECT_EQ("808080FF", format.ConvertColorToRGBA(agi::Color(128, 128, 128), 0));
}

// Test edge case RGB values
TEST_F(CineCanvasColorTest, EdgeCaseColors) {
	// Very dark gray: RGB(1, 1, 1) with opaque -> 010101FF
	EXPECT_EQ("010101FF", format.ConvertColorToRGBA(agi::Color(1, 1, 1), 0));

	// Very light gray: RGB(254, 254, 254) with opaque -> FEFEFEFF
	EXPECT_EQ("FEFEFEFF", format.ConvertColorToRGBA(agi::Color(254, 254, 254), 0));

	// Low RGB values: RGB(16, 32, 48) with opaque -> 102030FF
	EXPECT_EQ("102030FF", format.ConvertColorToRGBA(agi::Color(16, 32, 48), 0));
}

// Test alpha edge cases
TEST_F(CineCanvasColorTest, AlphaEdgeCases) {
	agi::Color testColor(100, 150, 200);

	// Minimum alpha (fully opaque in ASS)
	EXPECT_EQ("6496C8FF", format.ConvertColorToRGBA(testColor, 0));

	// Alpha = 1 (almost fully opaque in ASS)
	EXPECT_EQ("6496C8FE", format.ConvertColorToRGBA(testColor, 1));

	// Alpha = 254 (almost fully transparent in ASS)
	EXPECT_EQ("6496C801", format.ConvertColorToRGBA(testColor, 254));

	// Maximum alpha (fully transparent in ASS)
	EXPECT_EQ("6496C800", format.ConvertColorToRGBA(testColor, 255));
}

// Test that hex output is always uppercase and 8 characters
TEST_F(CineCanvasColorTest, OutputFormat) {
	std::string result = format.ConvertColorToRGBA(agi::Color(10, 20, 30), 5);

	// Check length is exactly 8 characters
	EXPECT_EQ(8u, result.length());

	// Verify it's uppercase hex (0A141EFA = RGB(10, 20, 30) with alpha 250 = 255-5)
	EXPECT_EQ("0A141EFA", result);

	// Check no lowercase letters
	for (char c : result) {
		if (std::isalpha(c)) {
			EXPECT_TRUE(std::isupper(c));
		}
	}
}

// Test examples from ASS_to_CineCanvas_Mapping.md documentation
TEST_F(CineCanvasColorTest, DocumentationExamples) {
	// From the mapping table in docs:
	// White: &H00FFFFFF& (ASS) -> FFFFFFFF (CineCanvas)
	EXPECT_EQ("FFFFFFFF", format.ConvertColorToRGBA(agi::Color(255, 255, 255), 0x00));

	// Black: &H00000000& (ASS) -> 000000FF (CineCanvas)
	EXPECT_EQ("000000FF", format.ConvertColorToRGBA(agi::Color(0, 0, 0), 0x00));

	// Red: &H000000FF& (ASS, BGR format) -> FF0000FF (CineCanvas)
	// Note: agi::Color is stored as RGB, so R=255
	EXPECT_EQ("FF0000FF", format.ConvertColorToRGBA(agi::Color(255, 0, 0), 0x00));

	// Green: &H0000FF00& (ASS, BGR format) -> 00FF00FF (CineCanvas)
	EXPECT_EQ("00FF00FF", format.ConvertColorToRGBA(agi::Color(0, 255, 0), 0x00));

	// Blue: &H00FF0000& (ASS, BGR format) -> 0000FFFF (CineCanvas)
	EXPECT_EQ("0000FFFF", format.ConvertColorToRGBA(agi::Color(0, 0, 255), 0x00));

	// Semi-transparent white: &H80FFFFFF& (ASS) -> FFFFFF7F (CineCanvas)
	EXPECT_EQ("FFFFFF7F", format.ConvertColorToRGBA(agi::Color(255, 255, 255), 0x80));
}

// Test conversion formula: cinema_alpha = 255 - ass_alpha
TEST_F(CineCanvasColorTest, AlphaConversionFormula) {
	agi::Color testColor(100, 100, 100);

	// Test the formula at various points
	for (int ass_alpha = 0; ass_alpha <= 255; ass_alpha += 17) {
		int expected_cinema_alpha = 255 - ass_alpha;
		std::string result = format.ConvertColorToRGBA(testColor, ass_alpha);

		// Extract alpha from result (last 2 characters)
		std::string alpha_hex = result.substr(6, 2);
		int actual_cinema_alpha = std::stoi(alpha_hex, nullptr, 16);

		EXPECT_EQ(expected_cinema_alpha, actual_cinema_alpha)
			<< "Failed for ASS alpha=" << ass_alpha;
	}
}

// ==================== Timing Conversion Tests ====================

class CineCanvasTimingTest : public ::testing::Test {
protected:
	CineCanvasSubtitleFormat format;
};

// Test basic time conversion at 24fps
TEST_F(CineCanvasTimingTest, BasicTimeConversion24fps) {
	agi::vfr::Framerate fps(24.0);

	// Test midnight / zero time
	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));

	// Test 1 second
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));

	// Test 1 minute
	EXPECT_EQ("00:01:00:000", format.ConvertTimeToCineCanvas(agi::Time(60000), fps));

	// Test 1 hour
	EXPECT_EQ("01:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(3600000), fps));
}

// Test basic time conversion at 25fps
TEST_F(CineCanvasTimingTest, BasicTimeConversion25fps) {
	agi::vfr::Framerate fps(25.0);

	// Test midnight / zero time
	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));

	// Test 1 second
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));

	// Test 1 minute
	EXPECT_EQ("00:01:00:000", format.ConvertTimeToCineCanvas(agi::Time(60000), fps));
}

// Test basic time conversion at 30fps
TEST_F(CineCanvasTimingTest, BasicTimeConversion30fps) {
	agi::vfr::Framerate fps(30.0);

	// Test midnight / zero time
	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));

	// Test 1 second
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));

	// Test 1 minute
	EXPECT_EQ("00:01:00:000", format.ConvertTimeToCineCanvas(agi::Time(60000), fps));
}

// Test millisecond precision
TEST_F(CineCanvasTimingTest, MillisecondPrecision) {
	agi::vfr::Framerate fps(24.0);

	// Test various millisecond values
	EXPECT_EQ("00:00:00:100", format.ConvertTimeToCineCanvas(agi::Time(100), fps));
	EXPECT_EQ("00:00:00:500", format.ConvertTimeToCineCanvas(agi::Time(500), fps));
	EXPECT_EQ("00:00:00:999", format.ConvertTimeToCineCanvas(agi::Time(999), fps));

	// Test mixed time components
	EXPECT_EQ("00:00:05:250", format.ConvertTimeToCineCanvas(agi::Time(5250), fps));
	EXPECT_EQ("00:01:23:456", format.ConvertTimeToCineCanvas(agi::Time(83456), fps));
}

// Test frame-accurate timing at 24fps
TEST_F(CineCanvasTimingTest, FrameAccurateTiming24fps) {
	agi::vfr::Framerate fps(24.0);

	// At 24fps, each frame is ~41.667ms
	// Frame 0 = 0ms
	// Frame 1 = 41.667ms -> should round to frame boundary
	// Frame 24 = 1000ms (exactly 1 second)

	// Test frame 0
	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));

	// Test frame 1 (should be frame-accurate, not necessarily 41ms)
	std::string frame1 = format.ConvertTimeToCineCanvas(agi::Time(42), fps);
	// Should be consistent with frame timing
	EXPECT_TRUE(frame1.find("00:00:00:") == 0);

	// Test 1 second (frame 24)
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));

	// Test 10 seconds (frame 240)
	EXPECT_EQ("00:00:10:000", format.ConvertTimeToCineCanvas(agi::Time(10000), fps));
}

// Test frame-accurate timing at 25fps
TEST_F(CineCanvasTimingTest, FrameAccurateTiming25fps) {
	agi::vfr::Framerate fps(25.0);

	// At 25fps, each frame is exactly 40ms
	// Frame 0 = 0ms
	// Frame 1 = 40ms
	// Frame 25 = 1000ms

	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));

	// Frame 1 at 25fps = 40ms
	std::string frame1 = format.ConvertTimeToCineCanvas(agi::Time(40), fps);
	EXPECT_TRUE(frame1.find("00:00:00:") == 0);

	// 1 second
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));
}

// Test frame-accurate timing at 30fps
TEST_F(CineCanvasTimingTest, FrameAccurateTiming30fps) {
	agi::vfr::Framerate fps(30.0);

	// At 30fps, each frame is ~33.333ms
	// Frame 0 = 0ms
	// Frame 30 = 1000ms

	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));
}

// Test long duration times (no drift over time)
TEST_F(CineCanvasTimingTest, LongDurationNoDrift) {
	agi::vfr::Framerate fps(24.0);

	// Test 1 hour
	EXPECT_EQ("01:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(3600000), fps));

	// Test 2 hours
	EXPECT_EQ("02:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(7200000), fps));

	// Test 10 hours (typical feature film length)
	EXPECT_EQ("10:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(36000000), fps));

	// Test 99 hours (max 2-digit hours)
	EXPECT_EQ("99:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(356400000), fps));
}

// Test timing accuracy over long durations with fractional times
TEST_F(CineCanvasTimingTest, LongDurationWithMilliseconds) {
	agi::vfr::Framerate fps(24.0);

	// Test 1 hour, 23 minutes, 45 seconds, 678 milliseconds
	// = 3600000 + 1380000 + 45000 + 678 = 5025678 ms
	int ms = 1 * 3600000 + 23 * 60000 + 45 * 1000 + 678;
	std::string result = format.ConvertTimeToCineCanvas(agi::Time(ms), fps);

	// Should be close to 01:23:45:678 (may vary slightly due to frame rounding)
	EXPECT_TRUE(result.find("01:23:45:") == 0);
}

// Test that timing is consistent across different frame rates for exact seconds
TEST_F(CineCanvasTimingTest, ConsistentTimingAcrossFramerates) {
	agi::vfr::Framerate fps24(24.0);
	agi::vfr::Framerate fps25(25.0);
	agi::vfr::Framerate fps30(30.0);

	// Exact second boundaries should be the same across all frame rates
	EXPECT_EQ("00:00:05:000", format.ConvertTimeToCineCanvas(agi::Time(5000), fps24));
	EXPECT_EQ("00:00:05:000", format.ConvertTimeToCineCanvas(agi::Time(5000), fps25));
	EXPECT_EQ("00:00:05:000", format.ConvertTimeToCineCanvas(agi::Time(5000), fps30));

	EXPECT_EQ("00:01:00:000", format.ConvertTimeToCineCanvas(agi::Time(60000), fps24));
	EXPECT_EQ("00:01:00:000", format.ConvertTimeToCineCanvas(agi::Time(60000), fps25));
	EXPECT_EQ("00:01:00:000", format.ConvertTimeToCineCanvas(agi::Time(60000), fps30));
}

// Test formatting consistency
TEST_F(CineCanvasTimingTest, FormattingConsistency) {
	agi::vfr::Framerate fps(24.0);

	// All components should be zero-padded to correct width
	std::string result = format.ConvertTimeToCineCanvas(agi::Time(3661001), fps);

	// Should be 01:01:01:001 (1 hour, 1 minute, 1 second, 1 millisecond)
	EXPECT_EQ(12u, result.length()); // HH:MM:SS:mmm = 12 characters
	EXPECT_EQ(':', result[2]);
	EXPECT_EQ(':', result[5]);
	EXPECT_EQ(':', result[8]);

	// Check zero-padding
	std::string smallTime = format.ConvertTimeToCineCanvas(agi::Time(1001), fps);
	EXPECT_TRUE(smallTime.find("00:00:01:") == 0);
}

// Test NTSC frame rate (23.976 fps)
TEST_F(CineCanvasTimingTest, NTSCFramerate) {
	agi::vfr::Framerate fps(24000.0 / 1001.0); // 23.976 fps

	// Test basic conversions
	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));

	// 1 second should still be close to 00:00:01:000
	std::string oneSecond = format.ConvertTimeToCineCanvas(agi::Time(1000), fps);
	EXPECT_TRUE(oneSecond.find("00:00:01:") == 0);
}

// Test PAL frame rate (24.975 fps)
TEST_F(CineCanvasTimingTest, PALFramerate) {
	agi::vfr::Framerate fps(25000.0 / 1000.0); // 25 fps (PAL)

	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));
}

// Test without FPS (should still work, just without frame-accurate snapping)
TEST_F(CineCanvasTimingTest, NoFramerateProvided) {
	agi::vfr::Framerate fps; // Uninitialized/unloaded FPS

	// Should still convert correctly, just without frame snapping
	EXPECT_EQ("00:00:00:000", format.ConvertTimeToCineCanvas(agi::Time(0), fps));
	EXPECT_EQ("00:00:01:000", format.ConvertTimeToCineCanvas(agi::Time(1000), fps));
	EXPECT_EQ("00:00:01:234", format.ConvertTimeToCineCanvas(agi::Time(1234), fps));
}

// Test typical subtitle times from real usage
TEST_F(CineCanvasTimingTest, TypicalSubtitleTimes) {
	agi::vfr::Framerate fps(24.0);

	// Opening credits at 30 seconds
	std::string opening = format.ConvertTimeToCineCanvas(agi::Time(30000), fps);
	EXPECT_EQ("00:00:30:000", opening);

	// Mid-film at 45 minutes, 30 seconds
	int midFilm = 45 * 60000 + 30 * 1000;
	std::string midResult = format.ConvertTimeToCineCanvas(agi::Time(midFilm), fps);
	EXPECT_EQ("00:45:30:000", midResult);

	// End credits at 2 hours
	std::string endCredits = format.ConvertTimeToCineCanvas(agi::Time(7200000), fps);
	EXPECT_EQ("02:00:00:000", endCredits);
}

// Test timing drift over 1000 subtitles
TEST_F(CineCanvasTimingTest, NoDriftOver1000Subtitles) {
	agi::vfr::Framerate fps(24.0);

	// Simulate 1000 subtitles, each 3 seconds apart
	// After 1000 subtitles, we should be at 3000 seconds = 50 minutes
	for (int i = 0; i < 1000; i++) {
		int timeMs = i * 3000;
		std::string result = format.ConvertTimeToCineCanvas(agi::Time(timeMs), fps);

		// Verify format is correct
		EXPECT_EQ(12u, result.length());
		EXPECT_EQ(':', result[2]);
		EXPECT_EQ(':', result[5]);
		EXPECT_EQ(':', result[8]);
	}

	// Check the 1000th subtitle (at 2997 seconds = 49 minutes 57 seconds)
	std::string result1000 = format.ConvertTimeToCineCanvas(agi::Time(999 * 3000), fps);
	EXPECT_EQ("00:49:57:000", result1000);
}
