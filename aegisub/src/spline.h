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

class VideoDisplay;


/// DOCME
/// @class Spline
/// @brief DOCME
class Spline {
private:
	const VideoDisplay &scale;
public:

	/// DOCME
	std::list<SplineCurve> curves;

	Spline(const VideoDisplay &scale);

	wxString EncodeToASS();
	void DecodeFromASS(wxString str);


	/// @brief DOCME
	/// @param curve 
	///
	void AppendCurve(SplineCurve &curve) { InsertCurve(curve,-1); }
	void InsertCurve(SplineCurve &curve,int index);
	void MovePoint(int curveIndex,int point,wxPoint pos);
	void Smooth(float smooth=1.0f);

	void GetPointList(std::vector<Vector2D> &points,std::vector<int> &pointCurve);
	SplineCurve *GetCurve(int index);

	void GetClosestParametricPoint(Vector2D reference,int &curve,float &t,Vector2D &point);
	Vector2D GetClosestPoint(Vector2D reference);
	Vector2D GetClosestControlPoint(Vector2D reference);
};
