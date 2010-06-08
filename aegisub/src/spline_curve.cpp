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
//
// $Id$

/// @file spline_curve.cpp
/// @brief Handle bicubic splines (Bezier curves) in vector drawings
/// @ingroup visual_ts
///

///////////
// Headers
#include "config.h"

#include "spline_curve.h"
#include "utils.h"

/// @brief Curve constructor 
///
SplineCurve::SplineCurve() {
	type = CURVE_INVALID;
}

/// @brief Split a curve in two using the de Casteljau algorithm 
/// @param c1 
/// @param c2 
/// @param t  
///
void SplineCurve::Split(SplineCurve &c1,SplineCurve &c2,float t) {
	// Split a line
	if (type == CURVE_LINE) {
		c1.type = CURVE_LINE;
		c2.type = CURVE_LINE;
		c1.p1 = p1;
		c2.p2 = p2;
		c1.p2 = p1*(1-t)+p2*t;
		c2.p1 = c1.p2;
	}

	// Split a bicubic
	else if (type == CURVE_BICUBIC) {
		c1.type = CURVE_BICUBIC;
		c2.type = CURVE_BICUBIC;

		// Sub-divisions
		float u = 1-t;
		Vector2D p12 = p1*u+p2*t;
		Vector2D p23 = p2*u+p3*t;
		Vector2D p34 = p3*u+p4*t;
		Vector2D p123 = p12*u+p23*t;
		Vector2D p234 = p23*u+p34*t;
		Vector2D p1234 = p123*u+p234*t;

		// Set points
		c1.p1 = p1;
		c2.p4 = p4;
		c1.p2 = p12;
		c1.p3 = p123;
		c1.p4 = p1234;
		c2.p1 = p1234;
		c2.p2 = p234;
		c2.p3 = p34;
	}
}

/// @brief Based on http://antigrain.com/research/bezier_interpolation/index.html Smoothes the curve 
/// @param P0     
/// @param P3     
/// @param smooth 
/// @return 
///
void SplineCurve::Smooth(Vector2D const& P0,Vector2D const& P3,float smooth) {
	// Validate
	if (type != CURVE_LINE) return;
	if (p1 == p2) return;
	smooth = MID(0.f,smooth,1.f);

	// Get points
	Vector2D P1 = p1;
	Vector2D P2 = p2;

	// Calculate intermediate points
	Vector2D c1 = (P0+P1)/2.f;
	Vector2D c2 = (P1+P2)/2.f;
	Vector2D c3 = (P2+P3)/2.f;
	float len1 = (P1-P0).Len();
	float len2 = (P2-P1).Len();
	float len3 = (P3-P2).Len();
	float k1 = len1/(len1+len2);
	float k2 = len2/(len2+len3);
	Vector2D m1 = c1+(c2-c1)*k1;
	Vector2D m2 = c2+(c3-c2)*k2;

	// Set curve points
	p4 = p2;
	p2 = m1+(c2-m1)*smooth + P1 - m1;
	p3 = m2+(c2-m2)*smooth + P2 - m2;
	type = CURVE_BICUBIC;
}

/// @brief Get a point 
/// @param t 
/// @return 
///
Vector2D SplineCurve::GetPoint(float t) const {
	if (type == CURVE_POINT) return p1;
	if (type == CURVE_LINE) {
		return p1*(1.f-t) + p2*t;
	}
	if (type == CURVE_BICUBIC) {
		float u = 1.f-t;
		return p1*u*u*u + 3*p2*t*u*u + 3*p3*t*t*u + p4*t*t*t;
	}

	return Vector2D(0,0);
}

Vector2D& SplineCurve::EndPoint() {
	switch (type) {
		case CURVE_POINT: return p1;
		case CURVE_LINE: return p2;
		case CURVE_BICUBIC: return p4;
		default: return p1;
	}
}

/// @brief Get point closest to reference 
/// @param ref 
/// @return 
///
Vector2D SplineCurve::GetClosestPoint(Vector2D const& ref) const {
	return GetPoint(GetClosestParam(ref));
}

/// @brief Get value of parameter closest to point 
/// @param ref 
/// @return 
///
float SplineCurve::GetClosestParam(Vector2D const& ref) const {
	if (type == CURVE_LINE) {
		return GetClosestSegmentPart(p1,p2,ref);
	}
	if (type == CURVE_BICUBIC) {
		int steps = 100;
		float bestDist = 80000000.f;
		float bestT = 0.f;
		for (int i=0;i<=steps;i++) {
			float t = float(i)/float(steps);
			float dist = (GetPoint(t)-ref).Len();
			if (dist < bestDist) {
				bestDist = dist;
				bestT = t;
			}
		}
		return bestT;
	}
	return 0.f;
}

/// @brief Quick distance 
/// @param ref 
/// @return 
///
float SplineCurve::GetQuickDistance(Vector2D const& ref) const {
	using std::min;
	if (type == CURVE_BICUBIC) {
		float len1 = GetClosestSegmentDistance(p1,p2,ref);
		float len2 = GetClosestSegmentDistance(p2,p3,ref);
		float len3 = GetClosestSegmentDistance(p3,p4,ref);
		float len4 = GetClosestSegmentDistance(p4,p1,ref);
		float len5 = GetClosestSegmentDistance(p1,p3,ref);
		float len6 = GetClosestSegmentDistance(p2,p4,ref);
		return min(min(min(len1,len2),min(len3,len4)),min(len5,len6));
	}

	// Something else
	else return (GetClosestPoint(ref)-ref).Len();
}

/// @brief Closest t in segment p1-p2 to point p3 
/// @param pt1 
/// @param pt2 
/// @param pt3 
/// @return 
///
float SplineCurve::GetClosestSegmentPart(Vector2D const& pt1,Vector2D const& pt2,Vector2D const& pt3) const {
	return MID(0.f,(pt3-pt1).Dot(pt2-pt1)/(pt2-pt1).SquareLen(),1.f);
}

/// @brief Closest distance between p3 and segment p1-p2 
/// @param pt1 
/// @param pt2 
/// @param pt3 
///
float SplineCurve::GetClosestSegmentDistance(Vector2D const& pt1,Vector2D const& pt2,Vector2D const& pt3) const {
	float t = GetClosestSegmentPart(pt1,pt2,pt3);
	return (pt1*(1.f-t)+pt2*t-pt3).Len();
}
