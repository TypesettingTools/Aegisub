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
#include <wx/tokenzr.h>
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
	for (std::list<SplineCurve>::iterator cur=curves.begin();cur!=curves.end();cur++) {
		// Start of spline
		if (isFirst) {
			result = wxString::Format(_T("m %i %i "),cur->x1,cur->y1);
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
				result += wxString::Format(_T("%i %i "),cur->x2,cur->y2);
				break;
			case CURVE_BICUBIC:
				if (lastCommand != 'b') {
					result += _T("b ");
					lastCommand = 'b';
				}
				result += wxString::Format(_T("%i %i %i %i %i %i "),cur->x2,cur->y2,cur->x3,cur->y3,cur->x4,cur->y4);
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
	std::vector<int> stack;

	// Prepare
	char lastCommand = 'm';
	int x = 0;
	int y = 0;

	// Tokenize the string
	wxStringTokenizer tkn(str,_T(" "));
	while (tkn.HasMoreTokens()) {
		wxString token = tkn.GetNextToken();

		// Got a number
		if (token.IsNumber()) {
			long n;
			token.ToLong(&n);
			stack.push_back(n);

			// Move
			if (stack.size() == 2 && lastCommand == 'm') {
				x = stack[0];
				y = stack[1];
				stack.clear();
			}

			// Line
			if (stack.size() == 2 && lastCommand == 'l') {
				SplineCurve curve;
				curve.x1 = x;
				curve.y1 = y;
				curve.x2 = stack[0];
				curve.y2 = stack[1];
				curve.type = CURVE_LINE;
				x = curve.x2;
				y = curve.y2;
				stack.clear();
				AppendCurve(curve);
			}

			// Bicubic
			else if (stack.size() == 6 && lastCommand == 'b') {
				SplineCurve curve;
				curve.x1 = x;
				curve.y1 = y;
				curve.x2 = stack[0];
				curve.y2 = stack[1];
				curve.x3 = stack[2];
				curve.y3 = stack[3];
				curve.x4 = stack[4];
				curve.y4 = stack[5];
				curve.type = CURVE_BICUBIC;
				x = curve.x4;
				y = curve.y4;
				stack.clear();
				AppendCurve(curve);
			}

			// Close
			else if (lastCommand == 'c') {
				stack.clear();
			}
		}

		// Got something else
		else {
			if (token == _T("m")) lastCommand = 'm';
			else if (token == _T("l")) lastCommand = 'l';
			else if (token == _T("b")) lastCommand = 'b';
			else if (token == _T("n")) lastCommand = 'n';
			else if (token == _T("s")) lastCommand = 's';
			else if (token == _T("c")) lastCommand = 'c';
		}
	}
}


////////////////////////////////
// Append a curve to the spline
void Spline::AppendCurve(SplineCurve &curve) {
	curves.push_back(curve);
}


////////////////////////////////////////
// Moves a specific point in the spline
void Spline::MovePoint(int curveIndex,int point,wxPoint pos) {
	// Curves
	int i = 0;
	SplineCurve *c0 = NULL;
	SplineCurve *c1 = NULL;
	SplineCurve *c2 = NULL;

	// Indices
	//int size = curves.size();
	int i0 = curveIndex-1;
	int i1 = curveIndex;
	int i2 = curveIndex+1;
	//if (i0 < 0) i0 = size-1;
	//if (i2 >= size) i2 = 0;

	// Get the curves
	for (std::list<SplineCurve>::iterator cur = curves.begin();cur!=curves.end();cur++) {
		if (i == i0) c0 = &(*cur);
		if (i == i1) c1 = &(*cur);
		if (i == i2) c2 = &(*cur);
		i++;
	}

	// Modify
	if (point == 0) {
		c1->x1 = pos.x;
		c1->y1 = pos.y;
		if (c0) {
			if (c0->type == CURVE_BICUBIC) {
				c0->x4 = pos.x;
				c0->y4 = pos.y;
			}
			else {
				c0->x2 = pos.x;
				c0->y2 = pos.y;
			}
		}
	}
	else if (point == 1) {
		c1->x2 = pos.x;
		c1->y2 = pos.y;
		if (c2 && c1->type != CURVE_BICUBIC) {
			c2->x1 = pos.x;
			c2->y1 = pos.y;
		}
	}
	else if (point == 2) {
		c1->x3 = pos.x;
		c1->y3 = pos.y;
	}
	else if (point == 3) {
		c1->x4 = pos.x;
		c1->y4 = pos.y;
		if (c2 && c1->type == CURVE_BICUBIC) {
			c2->x1 = pos.x;
			c2->y1 = pos.y;
		}
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

			// Find number of steps
			int len = sqrt(double((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))) + sqrt(double((x2-x3)*(x2-x3)+(y2-y3)*(y2-y3))) + sqrt(double((x3-x4)*(x3-x4)+(y3-y4)*(y3-y4)));
			int steps = len/8;

			// Render curve
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

	// Insert a copy of the first point at the end
	if (points.size()) {
		points.push_back(points[0]);
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
