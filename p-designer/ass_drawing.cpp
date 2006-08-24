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

#include "ass_drawing.h"
#include <ctype.h>


// Point Four on the roadmap: code the ToString function

wxString AssDrawing::ToString()
{
	return wxString(_T(""));
}


// Point Two on the roadmap: code the FromString function
// "Steal" some ideas from RTS.cpp :P

// Attempt to read a point from string src.
// On success: Add point to target, "eat" point from src, increase where by chars eaten, return true.
// Assume first char is a non-space
static bool ReadPoint(wxString &src, int &where, std::vector<AssDrawingPoint> &target)
{
	AssDrawingPoint p;

	int i = 0;
	bool isneg = false;

	// Check for negativity
	if (src[i] == '+') isneg = false, i++;
	if (src[i] == '-') isneg = true, i++;

	// Faster exit for non-digit things
	if (!isdigit(src[i])) return false;

	// This loop should terminate even if it hits end of string, since isdigit('\0')==false
	while (isdigit(src[i])) i++;
	p.x = src.Mid(0, i).ToLong();
	if (isneg) p.x = -p.x;

	// Error checking
	if (!isspace(src[i])) return false;

	// Find start of next number
	while (isspace(src[i])) i++;

	// Get second number
	int j = i;
	isneg = false;
	if (src[j] == '+') isneg = false, j++;
	if (src[j] == '-') isneg = true, j++;
	while (isdigit(src[j])) j++;
	p.y = src.Mid(j, j-i).ToLong();
	if (isneg) p.y = -p.y;
	if (!isspace(src[j])) return false;
	while (isspace(src[j])) j++;

	// Finish off
	src = src.Mid(j);
	target.push_back(p);
	where += j;
	return true;
}

int AssDrawing::FromString(wxString src)
{
	data.clear();

	// Counter for number of chars eaten
	int where = 0;

	src.Trim(false);

	while (src.Len() > 0) {
		AssDrawingSegment seg;
		int i = 0;

		// Next character should be a letter, read it
		switch (src[i]) {
			case 'm':
				seg.type = SEGMENT_MOVE;
				break;
			case 'n':
				seg.type = SEGMENT_MOVE_NOCLOSE;
				break;
			case 'l':
				seg.type = SEGMENT_LINE;
				break;
			case 'b':
				seg.type = SEGMENT_BEZIER;
				break;
			case 's':
				seg.type = SEGMENT_BSPLINE;
				break;
			case 'p':
				seg.type = SEGMENT_BSPLINE_EXTEND;
				break;
			case 'c':
				seg.type = SEGMENT_BSPLINE_CLOSE;
				goto skippoints;
			default:
				// something unknown, stop parsing
				return where+1;
		}
		where++, i++;
		while (isspace(src[i])) where++, i++;
		src = src.Mid(i);
		while (ReadPoint(src, where, seg.points);
skippoints:
		data.push_back(seg);
	}

	return 0;
}


// These might be a bit redundant... save them for last

void AssDrawing::Collapse()
{
	// TODO
}

void AssDrawing::Expand()
{
	// TODO
}


// I suppose Rescale is just a matter of calculating the new scale, calling Stretch and setting the scale field

void AssDrawing::Rescale(int new_scale)
{
	// TODO
}

void AssDrawing::Stretch(float factor)
{
	// TODO
}


// A note on the following functions:
// According to VSFilter (last time I checked), the width and height of a drawing are calculated just by looking
// at the min and max X and Y values that appear in the drawing string. This is technically wrong (because of
// splines, those do funny things) but better do as the Romans do.

AssDrawingPoint AssDrawing::GetMinXY()
{
	// TODO
	return AssDrawingPoint(0, 0);
}

AssDrawingPoint AssDrawing::GetMaxXY()
{
	// TODO
	return AssDrawingPoint(0, 0);
}

AssDrawingPoint AssDrawing::GetExtents()
{
	// TODO
	return AssDrawingPoint(0, 0);
}

AssDrawingPoint AssDrawing::GetCenterXY()
{
	// TODO
	return AssDrawingPoint(0, 0);
}
