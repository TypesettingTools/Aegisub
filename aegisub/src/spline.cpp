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

/// @file spline.cpp
/// @brief Manage vector drawings for visual typesetting tools
/// @ingroup visual_ts
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/tokenzr.h>
#endif

#include <limits>

#include "spline.h"
#include "utils.h"
#include "video_display.h"

/// @brief Spline constructor 
Spline::Spline(const VideoDisplay &scale) : scale(scale) {
}

/// @brief Encode to ASS 
wxString Spline::EncodeToASS() {
	wxString result;
	char lastCommand = 0;

	// Insert each element
	for (iterator cur=begin();cur!=end();cur++) {
		// Each curve
		switch (cur->type) {
			case CURVE_POINT: {
				if (lastCommand != 'm') {
					result += L"m ";
					lastCommand = 'm';
				}
				int x = cur->p1.x;
				int y = cur->p1.y;
				scale.ToScriptCoords(&x, &y);
				result += wxString::Format(L"%i %i ", x, y);
				break;
			}
			case CURVE_LINE: {
				if (lastCommand != 'l') {
					result += L"l ";
					lastCommand = 'l';
				}
				int x = cur->p2.x;
				int y = cur->p2.y;
				scale.ToScriptCoords(&x, &y);
				result += wxString::Format(L"%i %i ", x, y);
				break;
			}
			case CURVE_BICUBIC: {
				if (lastCommand != 'b') {
					result += L"b ";
					lastCommand = 'b';
				}
				int x2 = cur->p2.x;
				int y2 = cur->p2.y;
				int x3 = cur->p3.x;
				int y3 = cur->p3.y;
				int x4 = cur->p4.x;
				int y4 = cur->p4.y;
				scale.ToScriptCoords(&x2, &y2);
				scale.ToScriptCoords(&x3, &y3);
				scale.ToScriptCoords(&x4, &y4);
				result += wxString::Format(L"%i %i %i %i %i %i ", x2, y2, x3, y3, x4, y4);
				break;
			}
			default: break;
		}
	}
	return result;
}

/// @brief Decode from ASS 
/// @param str 
void Spline::DecodeFromASS(wxString str) {
	// Clear current
	clear();
	std::vector<int> stack;

	// Prepare
	char lastCommand = 'm';
	int x = 0;
	int y = 0;

	// Tokenize the string
	wxStringTokenizer tkn(str,L" ");
	while (tkn.HasMoreTokens()) {
		wxString token = tkn.GetNextToken();

		// Got a number
		if (token.IsNumber()) {
			long n;
			token.ToLong(&n);
			stack.push_back(n);

			// Move
			if (stack.size() == 2 && lastCommand == 'm') {
				scale.FromScriptCoords(&stack[0], &stack[1]);
				SplineCurve curve;
				x = curve.p1.x = stack[0];
				y = curve.p1.y = stack[1];
				curve.type = CURVE_POINT;
				stack.clear();
				push_back(curve);
			}

			// Line
			if (stack.size() == 2 && lastCommand == 'l') {
				scale.FromScriptCoords(&stack[0], &stack[1]);
				SplineCurve curve;
				curve.p1.x = x;
				curve.p1.y = y;
				x = curve.p2.x = stack[0];
				y = curve.p2.y = stack[1];
				curve.type = CURVE_LINE;
				stack.clear();
				push_back(curve);
			}

			// Bicubic
			else if (stack.size() == 6 && lastCommand == 'b') {
				scale.FromScriptCoords(&stack[0], &stack[1]);
				scale.FromScriptCoords(&stack[2], &stack[3]);
				scale.FromScriptCoords(&stack[4], &stack[5]);
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
				x = curve.p4.x;
				y = curve.p4.y;
				stack.clear();
				push_back(curve);
			}

			// Close
			else if (lastCommand == 'c') {
				stack.clear();
			}
		}

		// Got something else
		else {
			if (token == L"m") lastCommand = 'm';
			else if (token == L"l") lastCommand = 'l';
			else if (token == L"b") lastCommand = 'b';
			else if (token == L"n") lastCommand = 'n';
			else if (token == L"s") lastCommand = 's';
			else if (token == L"c") lastCommand = 'c';
		}
	}
}

/// @brief Moves a specific point in the spline 
/// @param curveIndex 
/// @param point      
/// @param pos        
void Spline::MovePoint(iterator curve,int point,Vector2D const& pos) {
	iterator prev = curve;
	if (curve != begin()) --prev;
	iterator next = curve;
	++next;
	if (next != end() && next->type == CURVE_POINT) next = end();

	// Modify
	if (point == 0) {
		curve->p1 = pos;
		if (curve != begin() && curve->type != CURVE_POINT) prev->EndPoint() = pos;
		if (next != end() && curve->type == CURVE_POINT) next->p1 = pos;
	}
	else if (point == 1) {
		curve->p2 = pos;
		if (next != end() && curve->type == CURVE_LINE) next->p1 = pos;
	}
	else if (point == 2) {
		curve->p3 = pos;
	}
	else if (point == 3) {
		curve->p4 = pos;
		if (next != end()) next->p1 = pos;
	}
}

/// @brief Gets a list of points in the curve 
/// @param points     
/// @param pointCurve 
void Spline::GetPointList(std::vector<float>& points, std::vector<int>& first, std::vector<int>& count) {
	points.clear();
	first.clear();
	count.clear();

	points.reserve((size() + 1) * 2);
	int curCount = 0;

	// Generate points for each curve
	for (iterator cur = begin();cur!=end();cur++) {
		switch (cur->type) {
			case CURVE_POINT:
				if (curCount > 0) {
					count.push_back(curCount);
				}

				// start new path
				first.push_back(points.size() / 2);
				points.push_back(cur->p1.x);
				points.push_back(cur->p1.y);
				curCount = 1;
				break;
			case CURVE_LINE:
				points.push_back(cur->p2.x);
				points.push_back(cur->p2.y);
				curCount++;
				break;
			case CURVE_BICUBIC: {
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
					Vector2D p = cur->GetPoint(t);
					points.push_back(p.x);
					points.push_back(p.y);
				}
				curCount += steps;
				break;
			}
			default: break;
		}
	}

	count.push_back(curCount);
}
void Spline::GetPointList(std::vector<float> &points, iterator curve) {
	points.clear();
	if (curve == end()) return;
	switch (curve->type) {
		case CURVE_LINE:
			points.push_back(curve->p1.x);
			points.push_back(curve->p1.y);
			points.push_back(curve->p2.x);
			points.push_back(curve->p2.y);
			break;
		case CURVE_BICUBIC: {
			// Get the control points
			Vector2D p1 = curve->p1;
			Vector2D p2 = curve->p2;
			Vector2D p3 = curve->p3;
			Vector2D p4 = curve->p4;

			// Find number of steps
			int len = (int)((p2-p1).Len() + (p3-p2).Len() + (p4-p3).Len());
			int steps = len/8;

			// Render curve
			for (int i=0;i<=steps;i++) {
				// Get t and t-1 (u)
				float t = float(i)/float(steps);
				Vector2D p = curve->GetPoint(t);
				points.push_back(p.x);
				points.push_back(p.y);
			}
			break;
		}
		default: break;
	}
}

/// @brief t value and curve of the point closest to reference 
/// @param reference 
/// @param curve     
/// @param t         
/// @param pt        
void Spline::GetClosestParametricPoint(Vector2D const& reference,iterator &curve,float &t,Vector2D &pt) {
	curve = end();
	t = 0.f;
	if (empty()) return;

	// Close the shape
	SplineCurve pad;
	pad.p1 = back().EndPoint();
	pad.p2 = front().p1;
	pad.type = CURVE_LINE;
	push_back(pad);

	float closest = std::numeric_limits<float>::infinity();
	for (iterator cur = begin();cur!=end();cur++) {
		float param = cur->GetClosestParam(reference);
		Vector2D p1 = cur->GetPoint(param);
		float dist = (p1-reference).SquareLen();
		if (dist < closest) {
			closest = dist;
			t = param;
			curve = cur;
			pt = p1;
		}
	}

	if (&*curve == &back()) {
		curve = end();
	}

	// Remove closing and return
	pop_back();
}

/// @brief Point closest to reference 
/// @param reference 
/// @return 
Vector2D Spline::GetClosestPoint(Vector2D const& reference) {
	iterator curve;
	float t;
	Vector2D point;
	GetClosestParametricPoint(reference,curve,t,point);
	return point;
}

/// @brief Smoothes the spline 
/// @param smooth 
void Spline::Smooth(float smooth) {
	// See if there are enough curves
	if (size() < 3) return;

	// Smooth curve
	iterator curve1 = end();
	--curve1;
	for (iterator cur = begin(); cur != end();) {
		iterator curve0 = curve1;
		curve1 = cur;
		cur++;
		iterator curve2 = cur == end() ? begin() : cur;

		// Smooth curve
		curve1->Smooth(curve0->p1,curve2->p2,smooth);
	}
}
