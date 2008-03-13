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
#include "utils.h"


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
			result = wxString::Format(_T("m %i %i "),(int)cur->p1.x,(int)cur->p1.y);
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
				result += wxString::Format(_T("%i %i "),(int)cur->p2.x,(int)cur->p2.y);
				break;
			case CURVE_BICUBIC:
				if (lastCommand != 'b') {
					result += _T("b ");
					lastCommand = 'b';
				}
				result += wxString::Format(_T("%i %i %i %i %i %i "),(int)cur->p2.x,(int)cur->p2.y,(int)cur->p3.x,(int)cur->p3.y,(int)cur->p4.x,(int)cur->p4.y);
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
	bool coordsSet = false;

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
				coordsSet = true;
				stack.clear();
			}

			// Line
			if (stack.size() == 2 && lastCommand == 'l') {
				SplineCurve curve;
				curve.p1.x = x;
				curve.p1.y = y;
				curve.p2.x = stack[0];
				curve.p2.y = stack[1];
				curve.type = CURVE_LINE;
				x = (int)curve.p2.x;
				y = (int)curve.p2.y;
				stack.clear();
				AppendCurve(curve);
			}

			// Bicubic
			else if (stack.size() == 6 && lastCommand == 'b') {
				SplineCurve curve;
				curve.p1.x = x;
				curve.p1.y = y;
				curve.p2.x = stack[0];
				curve.p2.y = stack[1];
				curve.p3.x = stack[2];
				curve.p3.y = stack[3];
				curve.p4.x = stack[4];
				curve.p4.y = stack[5];
				curve.type = CURVE_BICUBIC;
				x = (int)curve.p4.x;
				y = (int)curve.p4.y;
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

	// Got coordinates, but list is empty
	if (curves.size() == 0 && coordsSet) {
		SplineCurve curve;
		curve.p1.x = x;
		curve.p1.y = y;
		curve.type = CURVE_POINT;
		AppendCurve(curve);
	}
}


////////////////////////////////
// Insert a curve to the spline
void Spline::InsertCurve(SplineCurve &curve,int index) {
	if (index == -1) curves.push_back(curve);
	else {
		std::list<SplineCurve>::iterator cur;
		int i=0;
		for (cur=curves.begin();cur!=curves.end() && i < index;cur++,i++);
		curves.insert(cur,curve);
	}
}


////////////////////////
// Get a specific curve
SplineCurve *Spline::GetCurve(int index) {
	int i=0;
	for (std::list<SplineCurve>::iterator cur=curves.begin();cur!=curves.end() && i <= index;cur++,i++) {
		if (i==index) return &(*cur);
	}
	return NULL;
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
		c1->p1.x = pos.x;
		c1->p1.y = pos.y;
		if (c0) {
			if (c0->type == CURVE_BICUBIC) {
				c0->p4.x = pos.x;
				c0->p4.y = pos.y;
			}
			else {
				c0->p2.x = pos.x;
				c0->p2.y = pos.y;
			}
		}
	}
	else if (point == 1) {
		c1->p2.x = pos.x;
		c1->p2.y = pos.y;
		if (c2 && c1->type != CURVE_BICUBIC) {
			c2->p1.x = pos.x;
			c2->p1.y = pos.y;
		}
	}
	else if (point == 2) {
		c1->p3.x = pos.x;
		c1->p3.y = pos.y;
	}
	else if (point == 3) {
		c1->p4.x = pos.x;
		c1->p4.y = pos.y;
		if (c2 && c1->type == CURVE_BICUBIC) {
			c2->p1.x = pos.x;
			c2->p1.y = pos.y;
		}
	}
}


//////////////////////////////////////
// Gets a list of points in the curve
void Spline::GetPointList(std::vector<Vector2D> &points,std::vector<int> &pointCurve) {
	// Prepare
	points.clear();
	pointCurve.clear();
	Vector2D pt;
	bool isFirst = true;
	int curve = 0;

	// Generate points for each curve
	for (std::list<SplineCurve>::iterator cur = curves.begin();cur!=curves.end();cur++,curve++) {
		// First point
		if (isFirst) {
			points.push_back(cur->p1);
			pointCurve.push_back(curve);
		}

		// Line
		if (cur->type == CURVE_LINE) {
			points.push_back(cur->p2);
			pointCurve.push_back(curve);
		}

		// Bicubic
		else if (cur->type == CURVE_BICUBIC) {
			// Get the control points
			Vector2D p1 = cur->p1;
			Vector2D p2 = cur->p2;
			Vector2D p3 = cur->p3;
			Vector2D p4 = cur->p4;

			// Find number of steps
			int len = (int)((p2-p1).Len() + (p3-p2).Len() + (p4-p3).Len());
			int steps = len/8;

			// Render curve
			for (int i=1;i<=steps;i++) {
				// Get t and t-1 (u)
				float t = float(i)/float(steps);
				points.push_back(cur->GetPoint(t));
				pointCurve.push_back(curve);
			}
		}
	}

	// Insert a copy of the first point at the end
	if (points.size()) {
		points.push_back(points[0]);
		pointCurve.push_back(curve);
	}
}


///////////////////////////////////////////////////////
// t value and curve of the point closest to reference
void Spline::GetClosestParametricPoint(Vector2D reference,int &curve,float &t,Vector2D &pt) {
	// Has at least one curve?
	curve = -1;
	t = 0.0f;
	if (curves.size() == 0) return;

	// Close the shape
	SplineCurve pad;
	pad.p1 = curves.back().GetEndPoint();
	pad.p2 = curves.front().p1;
	pad.type = CURVE_LINE;
	curves.push_back(pad);

	// Prepare
	float closest = 8000000.0f;
	int i = 0;
	for (std::list<SplineCurve>::iterator cur = curves.begin();cur!=curves.end();cur++,i++) {
		float param = cur->GetClosestParam(reference);
		Vector2D p1 = cur->GetPoint(param);
		float dist = (p1-reference).Len();
		if (dist < closest) {
			closest = dist;
			t = param;
			curve = i;
			pt = p1;
		}
	}

	// Remove closing and return
	curves.pop_back();
}


//////////////////////////////
// Point closest to reference
Vector2D Spline::GetClosestPoint(Vector2D reference) {
	int curve;
	float t;
	Vector2D point;
	GetClosestParametricPoint(reference,curve,t,point);
	return point;
}


//////////////////////////////////////
// Control point closest to reference
Vector2D Spline::GetClosestControlPoint(Vector2D reference) {
	// TODO
	return Vector2D(-1,-1);
}


///////////////////////
// Smoothes the spline
void Spline::Smooth(float smooth) {
	// See if there are enough curves
	if (curves.size() < 3) return;

	// Smooth curve
	SplineCurve *curve0 = NULL;
	SplineCurve *curve1 = &curves.back();
	SplineCurve *curve2 = NULL;
	for (std::list<SplineCurve>::iterator cur=curves.begin();cur!=curves.end();) {
		// Get curves
		curve0 = curve1;
		curve1 = &(*cur);
		cur++;
		if (cur == curves.end()) curve2 = &curves.front();
		else curve2 = &(*cur);

		// Smooth curve
		curve1->Smooth(curve0->p1,curve2->p2,smooth);
	}
}
