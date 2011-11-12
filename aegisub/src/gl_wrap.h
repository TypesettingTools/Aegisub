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

/// @file gl_wrap.h
/// @see gl_wrap.cpp
/// @ingroup video_output
///

#include "vector2d.h"

#ifndef AGI_PRE
#include <vector>
#endif

class wxColour;

/// DOCME
/// @class OpenGLWrapper
/// @brief DOCME
///
/// DOCME
class OpenGLWrapper {
	float line_r, line_g, line_b, line_a;
	float fill_r, fill_g, fill_b, fill_a;

	int line_width;
	bool smooth;

	bool transform_pushed;
	void PrepareTransform();

public:
	OpenGLWrapper();

	void SetLineColour(wxColour col, float alpha = 1.0f, int width = 1);
	void SetFillColour(wxColour col, float alpha = 1.0f);
	void SetModeLine() const;
	void SetModeFill() const;

	void SetInvert();
	void ClearInvert();

	void SetScale(Vector2D scale);
	void SetOrigin(Vector2D origin);
	void SetRotation(float x, float y, float z);
	void ResetTransform();

	void DrawLine(Vector2D p1, Vector2D p2) const;
	void DrawDashedLine(Vector2D p1, Vector2D p2, float dashLen) const;
	void DrawEllipse(Vector2D center, Vector2D radius) const;
	void DrawCircle(Vector2D center, float radius) const { DrawEllipse(center, Vector2D(radius, radius)); }
	void DrawRectangle(Vector2D p1, Vector2D p2) const;
	void DrawRing(Vector2D center, float r1, float r2, float ar = 1.0f, float arcStart = 0.0f, float arcEnd = 0.0f) const;
	void DrawTriangle(Vector2D p1, Vector2D p2, Vector2D p3) const;

	void DrawLines(size_t dim, std::vector<float> const& lines);
	void DrawLines(size_t dim, std::vector<float> const& lines, size_t c_dim, std::vector<float> const& colors);
	void DrawLines(size_t dim, const float *lines, size_t n);
	void DrawLineStrip(size_t dim, std::vector<float> const& lines);

	/// Draw a multipolygon serialized into a single array
	/// @param points List of coordinates
	/// @param start Indices in points which are the start of a new polygon
	/// @param count Number of points in each polygon
	/// @param video_pos Top-left corner of the visible area
	/// @param video_size Bottom-right corner of the visible area
	/// @param invert Draw the area outside the polygons instead
	void DrawMultiPolygon(std::vector<float> const& points, std::vector<int> &start, std::vector<int> &count, Vector2D video_pos, Vector2D video_size, bool invert);

	static bool IsExtensionSupported(const char *ext);
};
