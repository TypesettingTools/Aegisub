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

/// @file vector2d.h
/// @see vector2d.cpp
/// @ingroup utility visual_ts
///

#pragma once

#ifndef AGI_PRE
#include <cmath>

#include <wx/gdicmn.h>
#endif

/// DOCME
/// @class Vector2D
/// @brief DOCME
///
/// DOCME
class Vector2D {
	float x, y;

	typedef float Vector2D::*unspecified_bool_type;
public:
	float X() const { return x; }
	float Y() const { return y; }

	Vector2D() : x(0), y(0) { }
	Vector2D(float x, float y) : x(x), y(y) { }
	Vector2D(wxPoint pt) : x(pt.x), y(pt.y) { }
	Vector2D(Vector2D x, Vector2D y) : x(x.x), y(y.y) { }
	Vector2D(float x, Vector2D y) : x(x), y(y.y) { }
	Vector2D(Vector2D x, float y) : x(x.x), y(y) { }

	bool operator ==(const Vector2D r) const { return x == r.x && y == r.y; }
	bool operator !=(const Vector2D r) const { return x != r.x || y != r.y; }
	operator unspecified_bool_type() const;

	Vector2D operator -() const { return Vector2D(-x, -y); }
	Vector2D operator +(const Vector2D r) const { return Vector2D(x + r.x, y + r.y); }
	Vector2D operator -(const Vector2D r) const { return Vector2D(x - r.x, y - r.y); }
	Vector2D operator *(const Vector2D r) const { return Vector2D(x * r.x, y * r.y); }
	Vector2D operator /(const Vector2D r) const { return Vector2D(x / r.x, y / r.y); }
	Vector2D operator +(float param) const { return Vector2D(x + param, y + param); }
	Vector2D operator -(float param) const { return Vector2D(x - param, y - param); }
	Vector2D operator *(float param) const { return Vector2D(x * param, y * param); }
	Vector2D operator /(float param) const { return Vector2D(x / param, y / param); }

	Vector2D Unit() const;
	Vector2D SingleAxis() const;

	Vector2D Perpendicular() const { return Vector2D(-y, x); }

	Vector2D Max(Vector2D param) const;
	Vector2D Min(Vector2D param) const;
	Vector2D Round(float step) const;

	float Cross(const Vector2D param) const { return x * param.y - y * param.x; }
	float Dot(const Vector2D param) const { return x * param.x + y * param.y; }

	float Len() const { return sqrt(x*x + y*y); }
	float SquareLen() const { return x*x + y*y; }
	float Angle() const { return atan2(y, x); }

	/// Get as string with given separator
	wxString Str(char sep = ',') const;
	/// Get as string surrounded by parentheses with given separator
	wxString PStr(char sep = ',') const;
	/// Get as string with given separator with values rounded to ints
	wxString DStr(char sep = ',') const;

	static Vector2D FromAngle(float angle) { return Vector2D(cos(-angle), sin(-angle)); }
	static Vector2D Bad();
};

Vector2D operator * (float f, Vector2D v);
Vector2D operator / (float f, Vector2D v);
Vector2D operator + (float f, Vector2D v);
Vector2D operator - (float f, Vector2D v);
