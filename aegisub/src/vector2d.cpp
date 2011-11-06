// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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
//
// $Id$

/// @file vector2d.cpp
/// @brief 2D mathematical vector used in visual typesetting
/// @ingroup utility visual_ts
///

#include "config.h"

#include "vector2d.h"

#ifndef AGI_PRE
#include <limits>

#include <wx/numformatter.h>
#endif

Vector2D operator *(float f, Vector2D v) {
	return Vector2D(v.X() * f, v.Y() * f);
}

Vector2D operator /(float f, Vector2D v) {
	return Vector2D(f / v.X(), f / v.Y());
}

Vector2D operator +(float f, Vector2D v) {
	return Vector2D(v.X() + f, v.Y() + f);
}

Vector2D operator -(float f, Vector2D v) {
	return Vector2D(f - v.X(), f - v.Y());
}

Vector2D Vector2D::Unit() const {
	float len = Len();
	if (len == 0)
		return Vector2D(0, 0);
	return *this / len;
}

Vector2D Vector2D::SingleAxis() const {
	if (abs(x) < abs(y))
		return Vector2D(0, y);
	else
		return Vector2D(x, 0);
}

Vector2D Vector2D::Max(Vector2D param) const {
	return Vector2D(std::max(x, param.x), std::max(y, param.y));
}

Vector2D Vector2D::Min(Vector2D param) const {
	return Vector2D(std::min(x, param.x), std::min(y, param.y));
}

Vector2D Vector2D::Round(float step) const {
	return Vector2D(floorf(x / step + .5f) * step, floorf(y / step + .5f) * step);
}

Vector2D::operator unspecified_bool_type() const {
	return *this == Bad() ? 0 : &Vector2D::x;
}

Vector2D Vector2D::Bad() {
	return Vector2D(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
}

wxString Vector2D::PStr(char sep) const {
	return "(" + Str(sep) + ")";
}

wxString Vector2D::DStr(char sep) const {
	return wxString::Format("%d%c%d", (int)x, sep, (int)y);
}

wxString Vector2D::Str(char sep) const {
	return
		wxNumberFormatter::ToString(x, 3, wxNumberFormatter::Style_NoTrailingZeroes) +
		sep +
		wxNumberFormatter::ToString(y, 3, wxNumberFormatter::Style_NoTrailingZeroes);
}
