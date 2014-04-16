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

/// @file vector2d.cpp
/// @brief 2D mathematical vector used in visual typesetting
/// @ingroup utility visual_ts
///

#include "vector2d.h"

#include "utils.h"

#include <boost/format.hpp>
#include <limits>

Vector2D::Vector2D()
: x(std::numeric_limits<float>::min())
, y(std::numeric_limits<float>::min())
{
}

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
	return *this == Vector2D() ? nullptr : &Vector2D::x;
}

std::string Vector2D::PStr(char sep) const {
	return "(" + Str(sep) + ")";
}

std::string Vector2D::DStr(char sep) const {
	return str(boost::format("%d%c%d") % (int)x % sep % (int)y);
}

std::string Vector2D::Str(char sep) const {
	return float_to_string(x) + sep + float_to_string(y);
}
