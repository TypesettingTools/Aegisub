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

#pragma once

#include <optional>
#include <string>
#include <variant>

namespace agi {

/// Color matrix constants matching the constants in ffmpeg
/// (specifically libavutil's AVColorSpace) and/or H.273.
enum class ycbcr_matrix : char {
	RGB = 0,
	BT709 = 1,
	Unspecified = 2,
	FCC = 4,
	BT470BG = 5,
	SMPTE170M = 6,
	SMPTE240M = 7,
	YCoCg = 8,
	BT2020_NCL = 9,
	BT2020_CL = 10,
	SMPTE2085 = 11,
	ChromacityDerivedNCL = 12,
	ChromacityDerivedCL = 13,
	ICtCp = 14,
};

/// Color matrix constants matching the constants in ffmpeg
/// (specifically libavutil's AVColorRange) and/or H.273.
enum class ycbcr_range : char {
	Unspecified = 0,
	MPEG = 1,	// TV / Limited
	JPEG = 2,	// PC / Full
};

namespace ycbcr {

#define EQOP(structname) bool operator==(const structname &) const = default

struct header_missing { EQOP(header_missing); };
struct header_invalid { EQOP(header_invalid); };
struct header_none { EQOP(header_none); };
struct header_colorspace {
	ycbcr_matrix matrix;
	ycbcr_range range;

	static header_colorspace unspecified() { return {ycbcr_matrix::Unspecified, ycbcr_range::Unspecified}; }

	EQOP(header_colorspace);
};

#undef EQOP

using header_variant = std::variant<header_missing, header_invalid, header_none, header_colorspace>;

/// @brief A value for a subtitle file's YCbCr Matrix header
///
/// A Header (when not missing or invalid) is either None or a matrix+range pair.
/// It is *not* guaranteed that said matrix+range pair has an actual corresponding
/// YCbCr Matrix header value.
struct Header : header_variant {
	Header(header_variant v);

	// Convenience overload
	Header(ycbcr_matrix cm, ycbcr_range cr) : Header(header_colorspace{cm, cr}) {}

	/// @brief Parses a YCbCr Matrix header value
	explicit Header(std::string const& matrix);

	/// @brief Returns whether this color space can be encoded to an actual YCbCr Matrix header (including a missing one)
	bool valid() const;

	/// @brief Returns the valid Header as which renderers will interpret this header value
	///
	/// to_effective should be used for determining which color matrix/range to decode a video with
	/// when given a file with this YCbCr Matrix (@see override_colorspace).
	Header to_effective() const;

	/// @brief Returns a valid Header best matching this header value
	///
	/// to_existing should be used for determining what YCbCr Matrix to set in a file
	/// when given a video with this color matrix and range, assuming ideal behavior in all implementations.
	/// That is, to_existing will also output headers like TV.FCC or PC.240M
	Header to_existing() const;

	/// @brief Returns a valid Header that's recommended to use for a video with the given color space
	///
	/// to_existing should be used for determining what YCbCr Matrix to set in a file
	/// when given a video with this color matrix and range, according to the best practices recommended
	/// by libass given the behavior of existing established renderers and players,
	/// see https://github.com/libass/libass/wiki/ASS-File-Format-Guide
	/// That is, to_best_practice will return 601/709 headers on the corresponding matrices and None otherwise.
	Header to_best_practice() const;

	/// @brief Converts the given parsed header to a string if valid and returns std::nullopt otherwise
	std::optional<std::string> to_string() const;

	std::string to_existing_string() const { return to_existing().to_string().value(); };

	std::string to_best_practice_string() const { return to_best_practice().to_string().value(); };

	/// @brief Determines the color space used for color mangling
	///
	/// If the Header is None and the input matrix or range are unspecified, they are guessed.
	void override_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, int Width, int Height) const;
};

void guess_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, int Width, int Height);

}
}
