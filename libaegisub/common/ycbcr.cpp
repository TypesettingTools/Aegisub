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

#include <libaegisub/split.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace agi::ycbcr {

namespace {

header_variant parse_ycbcr_header(std::string const& matrix) {
	ycbcr_matrix CM = ycbcr_matrix::Unspecified;
	ycbcr_range CR = ycbcr_range::Unspecified;

	std::string lower = boost::to_lower_copy(matrix);
	boost::trim(lower);

	if (lower.empty())
		return header_missing{};

	if (lower == "none")
		return header_none{};

	std::vector<std::string> parts;
	agi::Split(parts, lower, '.');
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

	if (CM == ycbcr_matrix::Unspecified || CR == ycbcr_range::Unspecified)
		return header_invalid{};

	return header_colorspace(CM, CR);
}

}

Header::Header(header_variant v) : header_variant(v) {
	if (auto *cs = std::get_if<header_colorspace>(this)) {
		if (cs->matrix == ycbcr_matrix::BT470BG)
			cs->matrix = ycbcr_matrix::SMPTE170M;	// Unify these two values such that the default operator== works
	}
}

Header::Header(std::string const& matrix) : Header(parse_ycbcr_header(matrix)) {}

bool Header::valid() const {
	return to_string().has_value();
}

std::optional<std::string> Header::to_string() const {
	if (auto *cs = std::get_if<header_colorspace>(this)) {
		std::string result;

		switch (cs->range) {
			using enum ycbcr_range;
			case MPEG:
				result = "TV";
				break;
			case JPEG:
				result = "PC";
				break;
			default:
				return std::nullopt;
		}

		switch (cs->matrix) {
			using enum ycbcr_matrix;
			case BT709:
				return result + ".709";
			case FCC:
				return result + ".FCC";
			case BT470BG:
			case SMPTE170M:
				return result + ".601";
			case SMPTE240M:
				return result + ".240M";
			default:
				return std::nullopt;
		}
	} else if (std::get_if<header_none>(this)) {
		return "None";
	} else if (std::get_if<header_missing>(this)) {
		return "";
	}

	return std::nullopt;
}

Header Header::to_effective() const {
	return valid() && !std::holds_alternative<header_missing>(*this) ? *this : Header(ycbcr_matrix::SMPTE170M, ycbcr_range::MPEG);
}

Header Header::to_existing() const {
	if (valid())
		return *this;

	if (std::holds_alternative<header_colorspace>(*this))
		return Header(header_none{});

	return Header(header_missing{});
}

Header Header::to_best_practice() const {
	if (auto *cs = std::get_if<header_colorspace>(this)) {
		switch (cs->matrix) {
			case ycbcr_matrix::BT709:
			case ycbcr_matrix::SMPTE170M:
			case ycbcr_matrix::BT470BG:
				if (cs->range != ycbcr_range::Unspecified)
					return *this;

				break;

			default:
				break;
		}
	} else if (std::get_if<header_missing>(this) || std::get_if<header_invalid>(this)) {
		return Header(header_missing{});
	}

	return Header(header_none{});
}

void Header::override_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, int Width, int Height) const {
	guess_colorspace(CM, CR, Width, Height);
	Header effective = to_effective();

	if (auto *cs = std::get_if<header_colorspace>(&effective)) {
		CM = cs->matrix;
		CR = cs->range;
	}
}

void guess_colorspace(ycbcr_matrix &CM, ycbcr_range &CR, int Width, int Height) {
	if (CM == ycbcr_matrix::Unspecified)
		CM = Width > 1024 || Height >= 600 ? ycbcr_matrix::BT709 : ycbcr_matrix::SMPTE170M;
	if (CR == ycbcr_range::Unspecified)
		CR = ycbcr_range::MPEG;
}

}
