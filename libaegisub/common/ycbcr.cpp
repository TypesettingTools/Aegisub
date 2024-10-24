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

#include <libaegisub/ycbcr.h>

#include <libaegisub/split.h>

#include <boost/algorithm/string/case_conv.hpp>

namespace agi { namespace ycbcr {

std::optional<std::string> colorspace_to_assycbcr(ycbcr_matrix CM, ycbcr_range CR) {
	std::string result;

	switch (CR) {
		case ycbcr_range::MPEG:
			result = "TV";
			break;
		case ycbcr_range::JPEG:
			result = "PC";
			break;
		default:
			return std::nullopt;
	}

	switch (CM) {
		case ycbcr_matrix::RGB:
			return "None";
		case ycbcr_matrix::BT709:
			return result + ".709";
		case ycbcr_matrix::FCC:
			return result + ".FCC";
		case ycbcr_matrix::BT470BG:
		case ycbcr_matrix::SMPTE170M:
			return result + ".601";
		case ycbcr_matrix::SMPTE240M:
			return result + ".240M";
		default:
			return std::nullopt;
	}
}

std::pair<ycbcr_matrix, ycbcr_range> assycbcr_to_colorspace(std::string const& matrix) {
	ycbcr_matrix CM = ycbcr_matrix::UNSPECIFIED;
	ycbcr_range CR = ycbcr_range::UNSPECIFIED;

	std::string lower = boost::to_lower_copy(matrix);

	std::vector<std::string> parts;
	agi::Split(parts, matrix, '.');
	if (parts.size() == 2) {
		if (parts[0] == "tv") {
			CR = ycbcr_range::MPEG;
		} else if (parts[0] == "pc") {
			CR = ycbcr_range::JPEG;
		}

		if (parts[1] == "709") {
			CM = ycbcr_matrix::BT709;
		} else if (parts[1] == "601") {
			CM = ycbcr_matrix::SMPTE170M;
		} else if (parts[1] == "fcc") {
			CM = ycbcr_matrix::FCC;
		} else if (parts[1] == "240m") {
			CM = ycbcr_matrix::SMPTE240M;
		}
	}

	if (CM == ycbcr_matrix::UNSPECIFIED || CR == ycbcr_range::UNSPECIFIED) {
		CM = ycbcr_matrix::SMPTE170M;
		CR = ycbcr_range::MPEG;
	}

	return std::make_pair(CM, CR);
}

void guess_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, int Width, int Height) {
	if (CM == ycbcr_matrix::UNSPECIFIED)
		CM = Width > 1024 || Height >= 600 ? ycbcr_matrix::BT709 : ycbcr_matrix::SMPTE170M;
	if (CR == ycbcr_range::UNSPECIFIED)
		CR = ycbcr_range::MPEG;
}

void override_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, std::string const& matrix, int Width, int Height) {
	guess_colorspace(CM, CR, Width, Height);
	auto [oCS, oCR] = assycbcr_to_colorspace(matrix);

	if (oCS == ycbcr_matrix::UNSPECIFIED || oCR != ycbcr_range::UNSPECIFIED) {
		oCS = ycbcr_matrix::SMPTE170M;
		oCR = ycbcr_range::MPEG;
	}

	CM = oCS;
	CR = oCR;
}

} }
