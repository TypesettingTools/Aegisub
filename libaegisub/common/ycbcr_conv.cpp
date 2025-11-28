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

#include "libaegisub/ycbcr_conv.h"

#include "libaegisub/exception.h"

namespace {
const std::array<double, 3> matrix_coefficients_bt601 = {.299, .587, .114};
const std::array<double, 3> matrix_coefficients_bt709 = {.2126, .7152, .0722};
const std::array<double, 3> matrix_coefficients_fcc = {.3, .59, .11};
const std::array<double, 3> matrix_coefficients_smpte240m = {.212, .701, .087};

const std::array<double, 3> &get_coefficients(agi::ycbcr_matrix mat) {
	switch (mat) {
        case agi::ycbcr_matrix::BT709:
			return matrix_coefficients_bt709;
        case agi::ycbcr_matrix::FCC:
			return matrix_coefficients_fcc;
        case agi::ycbcr_matrix::BT470BG:
        case agi::ycbcr_matrix::SMPTE170M:
			return matrix_coefficients_bt601;
        case agi::ycbcr_matrix::SMPTE240M:
			return matrix_coefficients_smpte240m;
		default:
			throw agi::InternalError("Unsupported colorspace conversion");
	}
}

void row_mult(std::array<double, 9>& arr, std::array<double, 3> values) {
	size_t i = 0;
	for (auto v : values) {
		arr[i++] *= v;
		arr[i++] *= v;
		arr[i++] *= v;
	}
}

void col_mult(std::array<double, 9>& m, std::array<double, 3> v) {
	m = {{
		m[0] * v[0], m[1] * v[1], m[2] * v[2],
		m[3] * v[0], m[4] * v[1], m[5] * v[2],
		m[6] * v[0], m[7] * v[1], m[8] * v[2],
	}};
}
}

namespace agi {
void ycbcr_converter::init_src(ycbcr_matrix src_mat, ycbcr_range src_range) {
	auto const& coeff = get_coefficients(src_mat);
	double Kr = coeff[0];
	double Kg = coeff[1];
	double Kb = coeff[2];
	to_ycbcr = {{
		Kr,         Kg,         Kb,
		-Kr/(1-Kb), -Kg/(1-Kb), 1,
		1,          -Kg/(1-Kr), -Kb/(1-Kr),
	}};

	if (src_range == ycbcr_range::JPEG) {
		row_mult(to_ycbcr, {{1., .5, .5}});
		shift_to = {{0, 128., 128.}};
	}
	else {
		row_mult(to_ycbcr, {{219./255., 112./255., 112./255.}});
		shift_to = {{16., 128., 128.}};
	}
}

void ycbcr_converter::init_dst(ycbcr_matrix dst_mat, ycbcr_range dst_range) {
	auto const& coeff = get_coefficients(dst_mat);
	double Kr = coeff[0];
	double Kg = coeff[1];
	double Kb = coeff[2];
	from_ycbcr = {{
		1,  0,             (1-Kr),
		1, -(1-Kb)*Kb/Kg, -(1-Kr)*Kr/Kg,
		1,  (1-Kb),        0,
	}};

	if (dst_range == ycbcr_range::JPEG) {
		col_mult(from_ycbcr, {{1., 2., 2.}});
		shift_from = {{0, -128., -128.}};
	}
	else {
		col_mult(from_ycbcr, {{255./219., 255./112., 255./112.}});
		shift_from = {{-16., -128., -128.}};
	}
}

ycbcr_converter::ycbcr_converter(ycbcr_matrix mat, ycbcr_range range) {
	init_src(mat, range);
	init_dst(mat, range);
}

ycbcr_converter::ycbcr_converter(ycbcr_matrix src_mat, ycbcr_range src_range, ycbcr_matrix dst_mat, ycbcr_range dst_range) {
	init_src(src_mat, src_range);
	init_dst(dst_mat, dst_range);
}
}

