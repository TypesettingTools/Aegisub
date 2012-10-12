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

/// @file spline.h
/// @see spline.cpp
/// @ingroup visual_ts
///

#ifndef AGI_PRE
#include <list>
#include <vector>

#include <wx/gdicmn.h>
#endif

#include "spline_curve.h"

class VisualToolBase;

/// DOCME
/// @class Spline
/// @brief DOCME
class Spline : private std::list<SplineCurve> {
	/// Visual tool to do the conversion between script and video pixels
	const VisualToolBase &coord_translator;
	/// Spline scale
	int scale;
	int raw_scale;

	/// Video coordinates -> Script coordinates
	Vector2D ToScript(Vector2D vec) const;

	/// Script coordinates -> Video coordinates
	Vector2D FromScript(Vector2D vec) const;
public:
	Spline(const VisualToolBase &scale);

	/// Encode to an ASS vector drawing
	wxString EncodeToAss() const;

	/// Decode an ASS vector drawing
	void DecodeFromAss(wxString str);

	/// Set the scale
	/// @param new_scale Power-of-two to scale coordinates by
	void SetScale(int new_scale);
	/// Get the current scale
	int GetScale() const { return raw_scale; }

	/// @brief Moves a specific point in the spline
	/// @param curve Curve which the point is in
	/// @param point Index in the curve
	/// @param pos New position
	void MovePoint(iterator curve, int point, Vector2D pos);

	/// Smooth the spline
	void Smooth(float smooth=1.0f);

	/// Gets a list of points in the curve
	void GetPointList(std::vector<float>& points, std::vector<int>& first, std::vector<int>& count);
	/// Gets a list of points in the curve
	void GetPointList(std::vector<float> &points, iterator curve);

	/// Get t value and curve of the point closest to reference
	void GetClosestParametricPoint(Vector2D reference, iterator& curve, float &t, Vector2D &point);
	/// Get closest point on the curve to reference
	Vector2D GetClosestPoint(Vector2D reference);
	Vector2D GetClosestControlPoint(Vector2D reference);

	// This list intentionally excludes things specific to std::list
	using std::list<SplineCurve>::value_type;
	using std::list<SplineCurve>::pointer;
	using std::list<SplineCurve>::reference;
	using std::list<SplineCurve>::const_reference;
	using std::list<SplineCurve>::size_type;
	using std::list<SplineCurve>::difference_type;
	using std::list<SplineCurve>::iterator;
	using std::list<SplineCurve>::const_iterator;
	using std::list<SplineCurve>::reverse_iterator;
	using std::list<SplineCurve>::const_reverse_iterator;

	using std::list<SplineCurve>::begin;
	using std::list<SplineCurve>::end;
	using std::list<SplineCurve>::rbegin;
	using std::list<SplineCurve>::rend;
	using std::list<SplineCurve>::size;
	using std::list<SplineCurve>::empty;
	using std::list<SplineCurve>::front;
	using std::list<SplineCurve>::back;
	using std::list<SplineCurve>::push_front;
	using std::list<SplineCurve>::push_back;
	using std::list<SplineCurve>::pop_front;
	using std::list<SplineCurve>::pop_back;
	using std::list<SplineCurve>::insert;
	using std::list<SplineCurve>::erase;
	using std::list<SplineCurve>::clear;
};
