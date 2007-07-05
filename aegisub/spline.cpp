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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "spline.h"


/////////////////////
// Curve constructor
SplineCurve::SplineCurve() {
	x1=x2=x3=x4=y1=y2=y3=y4=0;
	type = CURVE_INVALID;
}


//////////////////////
// Spline constructor
Spline::Spline() {
}


/////////////////
// Encode to ASS
wxString Spline::EncodeToASS() {
	wxString result;
	char lastCommand = 0;

	// At least one element?
	bool isFirst = true;

	// Insert each element
	for (std::list<SplineCurve>::iterator cur = curves.begin();cur!=curves.end();cur++) {
		// Start of spline
		if (isFirst) {
			result = wxString::Format(_T("m %i %i"),cur->x1,cur->y1);
			lastCommand = 'm';
			isFirst = false;
		}

		// Each curve
		switch (cur->type) {
			case CURVE_LINE:
				if (lastCommand != 'l') {
					result += _T("l ");
					lastCommand = 'l';
				}
				result += wxString::Format(_T("%i %i"),cur->x2,cur->y2);
				break;
			case CURVE_BICUBIC:
				if (lastCommand != 'b') {
					result += _T("b ");
					lastCommand = 'b';
				}
				result += wxString::Format(_T("%i %i %i %i"),cur->x2,cur->y2,cur->x3,cur->y3,cur->x4,cur->y4);
				break;
			default: break;
		}
	}
	return result;
}


///////////////////
// Decode from ASS
void Spline::DecodeFromASS(wxString str) {
	// Clear current
	curves.clear();
}


////////////////////////////////
// Append a curve to the spline
void Spline::AppendCurve(SplineCurve &curve) {
	curves.push_back(curve);
}


////////////////////////////////////////
// Moves a specific point in the spline
void Spline::MovePoint(int curveIndex,int point,wxPoint pos) {
	int i = 0;
	for (std::list<SplineCurve>::iterator cur = curves.begin();cur!=curves.end();cur++) {
		if (i == curveIndex) {
			switch (point) {
				case 0:
					cur->x1 = pos.x;
					cur->y1 = pos.y;
					break;
				case 1:
					cur->x2 = pos.x;
					cur->y2 = pos.y;
					break;
				case 2:
					cur->x3 = pos.x;
					cur->y3 = pos.y;
					break;
				case 3:
					cur->x4 = pos.x;
					cur->y4 = pos.y;
					break;
			}
			return;
		}
		i++;
	}
}


//////////////////////////////////////
// Gets a list of points in the curve
void Spline::GetPointList(std::vector<wxPoint> &points) {
	// Prepare
	points.clear();
	wxPoint pt;
	bool isFirst = true;

	// Generate points for each curve
	for (std::list<SplineCurve>::iterator cur = curves.begin();cur!=curves.end();cur++) {
		// First point
		if (isFirst) {
			pt.x = cur->x1;
			pt.y = cur->y1;
			points.push_back(pt);
		}

		// Line
		if (cur->type == CURVE_LINE) {
			pt.x = cur->x2;
			pt.y = cur->y2;
			points.push_back(pt);
		}

		// Bicubic
		else if (cur->type == CURVE_BICUBIC) {
			// Get the control points
			int x1 = cur->x1;
			int x2 = cur->x2;
			int x3 = cur->x3;
			int x4 = cur->x4;
			int y1 = cur->y1;
			int y2 = cur->y2;
			int y3 = cur->y3;
			int y4 = cur->y4;

			// Hardcoded at 10 steps for now
			int steps = 10;
			for (int i=0;i<steps;i++) {
				// Get t and t-1 (u)
				float t = float(i)/float(steps);
				float u = 1.0f-t;

				// Calculate the point and insert it
				pt.x = x1*u*u*u + 3*x2*t*u*u + 3*x3*t*t*u + x4*t*t*t;
				pt.y = y1*u*u*u + 3*y2*t*u*u + 3*y3*t*t*u + y4*t*t*t;
				points.push_back(pt);
			}
		}
	}
}


//////////////////////////////
// Point closest to reference
wxPoint Spline::GetClosestPoint(wxPoint reference) {
	return wxPoint(-1,-1);
}


//////////////////////////////////////
// Control point closest to reference
wxPoint Spline::GetClosestControlPoint(wxPoint reference) {
	return wxPoint(-1,-1);
}
