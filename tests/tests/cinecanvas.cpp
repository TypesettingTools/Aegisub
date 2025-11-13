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
/// @brief Unit tests for CineCanvas subtitle format color conversion
/// @ingroup subtitle_io

#include <libaegisub/color.h>

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
