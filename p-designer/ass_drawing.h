// Copyright (c) 2006, Niels Martin Hansen
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

#pragma once

#ifndef _ASS_DRAWING_H
#define _ASS_DRAWING_H

#include <vector>
#include <pair>
#include <wx/string.h>

enum AssDrawingSegmentType {
	SEGMENT_MOVE,          // m <x> <y>
	SEGMENT_MOVE_NOCLOSE,  // n <x> <y>
	SEGMENT_LINE,          // l <x> <y>
	SEGMENT_BEZIER,        // b <x1> <y1> <x2> <y2> <x3> <y3>
	SEGMENT_SPLINE,        // s <x1> <y2> <x2> <y2> <x3> <y3> [... <xN> <yN>]
	SEGMENT_SPLINE_EXTEND, // p <x> <y>
	SEGMENT_SPLINE_CLOSE,  // c
	                       // thought: maybe join the three last into one?
};

struct AssDrawingPoint {
	int x, y;
};

struct AssDrawingSegment {
	AssDrawingSegmentType type;
	std::vector<AssDrawingPoint> points;
};

struct AssDrawing {
	int scale;
	std::vector<AssDrawingSegment> data;

	wxString ToString(); // stupid conversion to string, doesn't try to shorten things out
	int FromString(wxString &src); // returns 0 on success, index+1 of offending character on fail

	void Collapse(); // try to join as many segments together as possible, without affecting the actual shape (eg. join "l 1 2 l 3 4" into "l 1 2 3 4")
	void Expand(); // split into as many segments as possible, without creating any meaningless ones, such as null-moves (ie. expand "l 1 2 3 4" into "l 1 2 l 3 4")

	void Rescale(int new_scale); // change the scale value, stretching the drawing to match the new one
	void Stretch(float factor); // up/down scale the drawing by a factor
};

#endif
