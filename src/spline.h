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

#include "spline_curve.h"

#include <string>
#include <vector>

class VisualToolBase;

class Spline final : private std::vector<SplineCurve> {
	/// Visual tool to do the conversion between script and video pixels
	const VisualToolBase& coord_translator;
	/// Spline scale
	int scale = 0;
	int raw_scale = 0;

	/// Video coordinates -> Script coordinates
	Vector2D ToScript(Vector2D vec) const;

	/// Script coordinates -> Video coordinates
	Vector2D FromScript(Vector2D vec) const;

  public:
	Spline(const VisualToolBase& scale);

	/// Encode to an ASS vector drawing
	std::string EncodeToAss() const;

	/// Decode an ASS vector drawing
	void DecodeFromAss(std::string const& str);

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
	void Smooth(float smooth = 1.0f);

	/// Gets a list of points in the curve
	std::vector<float> GetPointList(std::vector<int>& first, std::vector<int>& count);
	/// Gets a list of points in the curve
	std::vector<float> GetPointList(iterator curve);

	/// Get t value and curve of the point closest to reference
	void GetClosestParametricPoint(Vector2D reference, iterator& curve, float& t, Vector2D& point);
	/// Get closest point on the curve to reference
	Vector2D GetClosestPoint(Vector2D reference);
	Vector2D GetClosestControlPoint(Vector2D reference);

	using std::vector<SplineCurve>::value_type;
	using std::vector<SplineCurve>::pointer;
	using std::vector<SplineCurve>::reference;
	using std::vector<SplineCurve>::const_reference;
	using std::vector<SplineCurve>::size_type;
	using std::vector<SplineCurve>::difference_type;
	using std::vector<SplineCurve>::iterator;
	using std::vector<SplineCurve>::const_iterator;
	using std::vector<SplineCurve>::reverse_iterator;
	using std::vector<SplineCurve>::const_reverse_iterator;

	using std::vector<SplineCurve>::back;
	using std::vector<SplineCurve>::begin;
	using std::vector<SplineCurve>::clear;
	using std::vector<SplineCurve>::emplace_back;
	using std::vector<SplineCurve>::empty;
	using std::vector<SplineCurve>::end;
	using std::vector<SplineCurve>::erase;
	using std::vector<SplineCurve>::front;
	using std::vector<SplineCurve>::insert;
	using std::vector<SplineCurve>::operator[];
	using std::vector<SplineCurve>::pop_back;
	using std::vector<SplineCurve>::push_back;
	using std::vector<SplineCurve>::rbegin;
	using std::vector<SplineCurve>::rend;
	using std::vector<SplineCurve>::size;
};
