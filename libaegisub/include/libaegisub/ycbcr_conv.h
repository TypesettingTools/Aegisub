// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <array>
#include <cstdint>

#include <libaegisub/color.h>

namespace agi {
enum class ycbcr_matrix {
	bt601,
	bt709,
	fcc,
	smpte_240m
};

enum class ycbcr_range {
	tv,
	pc
};

/// A converter between YCbCr colorspaces and RGB
class ycbcr_converter {
	std::array<double, 9> from_ycbcr;
	std::array<double, 9> to_ycbcr;

	std::array<double, 3> shift_from;
	std::array<double, 3> shift_to;

	void init_dst(ycbcr_matrix dst_mat, ycbcr_range dst_range);
	void init_src(ycbcr_matrix src_mat, ycbcr_range src_range);

	template<typename T>
	static std::array<double, 3> prod(std::array<double, 9> m, std::array<T, 3> v) {
		return {{
			m[0] * v[0] + m[1] * v[1] + m[2] * v[2],
			m[3] * v[0] + m[4] * v[1] + m[5] * v[2],
			m[6] * v[0] + m[7] * v[1] + m[8] * v[2],
		}};
	}

	template<typename T, typename U>
	static std::array<double, 3> add(std::array<T, 3> left, std::array<U, 3> right) {
		return {{left[0] + right[0], left[1] + right[1], left[2] + right[2]}};
	}

	static uint8_t clamp(double v) {
		auto i = static_cast<int>(v);
		i = i > 255 ? 255 : i;
		return i < 0 ? 0 : i;
	}

	static std::array<uint8_t, 3> to_uint8_t(std::array<double, 3> val) {
		return {{clamp(val[0] + .5), clamp(val[1] + .5), clamp(val[2] + .5)}};
	}

public:
	ycbcr_converter(ycbcr_matrix mat, ycbcr_range range);
	ycbcr_converter(ycbcr_matrix src_mat, ycbcr_range src_range, ycbcr_matrix dst_mat, ycbcr_range dst_range);

	/// Convert from rgb to dst_mat/dst_range
	std::array<uint8_t, 3> rgb_to_ycbcr(std::array<uint8_t, 3> input) const {
		return to_uint8_t(add(prod(to_ycbcr, input), shift_to));
	}

	/// Convert from src_mat/src_range to rgb
	std::array<uint8_t, 3> ycbcr_to_rgb(std::array<uint8_t, 3> input) const {
		return to_uint8_t(prod(from_ycbcr, add(input, shift_from)));
	}

	/// Convert rgb to ycbcr using src_mat and then back using dst_mat
	std::array<uint8_t, 3> rgb_to_rgb(std::array<uint8_t, 3> input) const {
		return to_uint8_t(prod(from_ycbcr,
			add(add(prod(to_ycbcr, input), shift_to), shift_from)));
	}

	Color rgb_to_rgb(Color c) const {
		auto arr = rgb_to_rgb(std::array<uint8_t, 3>{{c.r, c.g, c.b}});
		return Color{arr[0], arr[1], arr[2]};
	}
};
}

