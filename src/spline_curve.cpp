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

/// @file spline_curve.cpp
/// @brief Handle bicubic splines (Bezier curves) in vector drawings
/// @ingroup visual_ts
///

#include "spline_curve.h"
#include "utils.h"

#include <limits>

SplineCurve::SplineCurve(Vector2D p1) : p1(p1), type(POINT) { }
SplineCurve::SplineCurve(Vector2D p1, Vector2D p2) : p1(p1), p2(p2), type(LINE) { }
SplineCurve::SplineCurve(Vector2D p1, Vector2D p2, Vector2D p3, Vector2D p4)
: p1(p1), p2(p2), p3(p3), p4(p4), type(BICUBIC)
{
}

std::pair<SplineCurve, SplineCurve> SplineCurve::Split(float t) {
	if (type == LINE) {
		Vector2D m = p1 * (1 - t) + p2 * t;
		return std::make_pair(
			SplineCurve(p1, m),
			SplineCurve(m, p2));
	}
	else if (type == BICUBIC) {
		float u = 1 - t;
		Vector2D p12   = p1   * u + p2   * t;
		Vector2D p23   = p2   * u + p3   * t;
		Vector2D p34   = p3   * u + p4   * t;
		Vector2D p123  = p12  * u + p23  * t;
		Vector2D p234  = p23  * u + p34  * t;
		Vector2D p1234 = p123 * u + p234 * t;

		return std::make_pair(
			SplineCurve(p1, p12, p123, p1234),
			SplineCurve(p1234, p234, p34, p4));
	}
	return std::make_pair(SplineCurve(p1), SplineCurve(p1));
}

void SplineCurve::Smooth(Vector2D p0, Vector2D p5, float smooth) {
	if (type != LINE || p1 == p2) return;
	smooth = mid(0.f, smooth, 1.f);

	// Calculate intermediate points
	Vector2D c1 = (p0 + p1) / 2.f;
	Vector2D c2 = (p1 + p2) / 2.f;
	Vector2D c3 = (p2 + p5) / 2.f;

	float len1 = (p1 - p0).Len();
	float len2 = (p2 - p1).Len();
	float len3 = (p5 - p2).Len();

	float k1 = len1 / (len1 + len2);
	float k2 = len2 / (len2 + len3);

	Vector2D m1 = c1 + (c2 - c1) * k1;
	Vector2D m2 = c2 + (c3 - c2) * k2;

	// Set curve points
	p4 = p2;
	p3 = m2 + (c2 - m2) * smooth + p2 - m2;
	p2 = m1 + (c2 - m1) * smooth + p1 - m1;
	type = BICUBIC;
}

Vector2D SplineCurve::GetPoint(float t) const {
	float u = 1.f - t;

	if (type == POINT)
		return p1;
	if (type == LINE)
		return p1 * u + p2 * t;

	return p1*u*u*u + 3*p2*t*u*u + 3*p3*t*t*u + p4*t*t*t;
}

Vector2D& SplineCurve::EndPoint() {
	switch (type) {
		case POINT:   return p1;
		case LINE:    return p2;
		case BICUBIC: return p4;
		default:      return p1;
	}
}

Vector2D SplineCurve::GetClosestPoint(Vector2D ref) const {
	return GetPoint(GetClosestParam(ref));
}

float SplineCurve::GetClosestParam(Vector2D ref) const {
	if (type == LINE)
		return GetClosestSegmentPart(p1, p2, ref);

	if (type == BICUBIC) {
		int steps = 100;
		float bestDist = std::numeric_limits<float>::max();
		float bestT = 0.f;
		for (int i = 0; i <= steps; ++i) {
			float t = i / float(steps);
			float dist = (GetPoint(t) - ref).SquareLen();
			if (dist < bestDist) {
				bestDist = dist;
				bestT = t;
			}
		}
		return bestT;
	}

	return 0.f;
}

float SplineCurve::GetQuickDistance(Vector2D ref) const {
	if (type == BICUBIC) {
		float lens[] = {
			GetClosestSegmentDistance(p1, p2, ref),
			GetClosestSegmentDistance(p2, p3, ref),
			GetClosestSegmentDistance(p3, p4, ref),
			GetClosestSegmentDistance(p4, p1, ref),
			GetClosestSegmentDistance(p1, p3, ref),
			GetClosestSegmentDistance(p2, p4, ref)
		};
		return *std::min_element(lens, lens + 6);
	}
	return (GetClosestPoint(ref) - ref).Len();
}

float SplineCurve::GetClosestSegmentPart(Vector2D pt1, Vector2D pt2, Vector2D pt3) const {
	return mid(0.f, (pt3 - pt1).Dot(pt2 - pt1) / (pt2 - pt1).SquareLen(), 1.f);
}

float SplineCurve::GetClosestSegmentDistance(Vector2D pt1, Vector2D pt2, Vector2D pt3) const {
	float t = GetClosestSegmentPart(pt1, pt2, pt3);
	return (pt1 * (1.f - t) + pt2 * t - pt3).Len();
}

int SplineCurve::GetPoints(std::vector<float> &points) const {
	switch (type) {
		case POINT:
			points.push_back(p1.X());
			points.push_back(p1.Y());
			return 1;

		case LINE:
			points.push_back(p2.X());
			points.push_back(p2.Y());
			return 1;

		case BICUBIC: {
			int len = int(
				(p2 - p1).Len() +
				(p3 - p2).Len() +
				(p4 - p3).Len());
			int steps = len/8;

			for (int i = 0; i <= steps; ++i) {
				// Get t and t-1 (u)
				float t = i / float(steps);
				Vector2D p = GetPoint(t);
				points.push_back(p.X());
				points.push_back(p.Y());
			}

			return steps + 1;
		}

		default:
			return 0;
	}
}
