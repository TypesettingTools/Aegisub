// Copyright (c) 2025, arch1t3cht <arch1t3cht@gmail.com>
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

#pragma once

#include <optional>
#include <string>

namespace agi {

/// Color matrix constants matching the constants in ffmpeg
/// (specifically libavutil's AVColorSpace) and/or H.273.
enum class ycbcr_matrix {
	RGB = 0,
	BT709 = 1,
	UNSPECIFIED = 2,
	FCC = 4,
	BT470BG = 5,
	SMPTE170M = 6,
	SMPTE240M = 7,
	YCOCG = 8,
	BT2020_NCL = 9,
	BT2020_CL = 10,
	SMPTE2085 = 11,
	CHROMATICITY_DERIVED_NCL = 12,
	CHROMATICITY_DERIVED_CL = 13,
	ICTCP = 14,
};

/// Color matrix constants matching the constants in ffmpeg
/// (specifically libavutil's AVColorRange) and/or H.273.
enum class ycbcr_range {
	UNSPECIFIED = 0,
	MPEG = 1,	// TV / Limited
	JPEG = 2,	// PC / Full
};

namespace ycbcr {

std::optional<std::string> colorspace_to_assycbcr(ycbcr_matrix CM, ycbcr_range CR);

std::pair<ycbcr_matrix, ycbcr_range> assycbcr_to_colorspace(std::string const& matrix);

void guess_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, int Width, int Height);

void override_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, std::string const& matrix, int Width, int Height);

}
}
