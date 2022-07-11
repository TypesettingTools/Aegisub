// Copyright (c) 2007, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file spline_curve.h
/// @see spline_curve.cpp
/// @ingroup visual_ts
///

#include "vector2d.h"

#include <vector>

class SplineCurve {
	/// Closest t in segment p1-p2 to point p3
	float GetClosestSegmentPart(Vector2D p1, Vector2D p2, Vector2D p3) const;

	/// Closest distance between p3 and segment p1-p2
	float GetClosestSegmentDistance(Vector2D p1, Vector2D p2, Vector2D p3) const;
public:
	enum CurveType {
		POINT,
		LINE,
		BICUBIC
	};

	Vector2D p1;
	Vector2D p2;
	Vector2D p3;
	Vector2D p4;

	CurveType type;

	SplineCurve(Vector2D p1 = Vector2D(0, 0));
	SplineCurve(Vector2D p1, Vector2D p2);
	SplineCurve(Vector2D p1, Vector2D p2, Vector2D p3, Vector2D p4);

	/// @brief Split a curve in two using the de Casteljau algorithm
	/// @param t Split point from 0-1
	/// @return Curve before and after the split point
	std::pair<SplineCurve, SplineCurve> Split(float t = 0.5f);

	/// @brief Smooths the curve
	/// @note Based on http://antigrain.com/research/bezier_interpolation/index.html
	void Smooth(Vector2D prev, Vector2D next, float smooth = 1.0f);

	Vector2D GetPoint(float t) const;
	Vector2D& EndPoint();
	/// Get point on the curve closest to reference
	Vector2D GetClosestPoint(Vector2D ref) const;
	/// Get t value for the closest point to reference
	float GetClosestParam(Vector2D ref) const;
	/// Get distance from ref to the closest point on the curve
	float GetQuickDistance(Vector2D ref) const;

	/// Get the coordinates of each point on this curve
	/// @param[out] points Vector to add points to
	/// @return Number of points in the curve
	int GetPoints(std::vector<float> &points) const;
};
