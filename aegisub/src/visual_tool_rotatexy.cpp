// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

/// @file visual_tool_rotatexy.cpp
/// @brief 3D rotation in X/Y axes visual typesetting tool
/// @ingroup visual_ts

#include "config.h"

#include "visual_tool_rotatexy.h"

#include <boost/format.hpp>
#include <cmath>

VisualToolRotateXY::VisualToolRotateXY(VideoDisplay *parent, agi::Context *context)
: VisualTool<VisualDraggableFeature>(parent, context)
{
	org = new Feature;
	org->type = DRAG_BIG_TRIANGLE;
	features.push_back(*org);
}

void VisualToolRotateXY::Draw() {
	if (!active_line) return;

	DrawAllFeatures();

	// Transform grid
	gl.SetOrigin(org->pos);
	gl.SetRotation(angle_x, angle_y, angle_z);

	// Draw grid
	gl.SetLineColour(colour[0], 0.5f, 2);
	gl.SetModeLine();
	float r = colour[0].Red() / 255.f;
	float g = colour[0].Green() / 255.f;
	float b = colour[0].Blue() / 255.f;

	std::vector<float> colors(11 * 8 * 4);
	for (int i = 0; i < 88; ++i) {
		colors[i * 4 + 0] = r;
		colors[i * 4 + 1] = g;
		colors[i * 4 + 2] = b;
		colors[i * 4 + 3] = (i + 3) % 4 > 1 ? 0 : (1.f - abs(i / 8 - 5) * 0.18f);
	}

	std::vector<float> points(11 * 8 * 2);
	for (int i = 0; i < 11; ++i) {
		int pos = 20 * (i - 5);

		points[i * 16 + 0] = pos;
		points[i * 16 + 1] = 120;

		points[i * 16 + 2] = pos;
		points[i * 16 + 3] = 0;

		points[i * 16 + 4] = pos;
		points[i * 16 + 5] = 0;

		points[i * 16 + 6] = pos;
		points[i * 16 + 7] = -120;

		points[i * 16 + 8] = 120;
		points[i * 16 + 9] = pos;

		points[i * 16 + 10] = 0;
		points[i * 16 + 11] = pos;

		points[i * 16 + 12] = 0;
		points[i * 16 + 13] = pos;

		points[i * 16 + 14] = -120;
		points[i * 16 + 15] = pos;
	}

	gl.DrawLines(2, points, 4, colors);

	// Draw vectors
	gl.SetLineColour(colour[3], 1.f, 2);
	float vectors[] = {
		0.f, 0.f, 0.f,
		50.f, 0.f, 0.f,
		0.f, 0.f, 0.f,
		0.f, 50.f, 0.f,
		0.f, 0.f, 0.f,
		0.f, 0.f, 50.f,
	};
	gl.DrawLines(3, vectors, 6);

	// Draw arrow tops
	float arrows[] = {
		60.f,  0.f,  0.f,
		50.f, -3.f, -3.f,
		50.f,  3.f, -3.f,
		50.f,  3.f,  3.f,
		50.f, -3.f,  3.f,
		50.f, -3.f, -3.f,

		 0.f, 60.f,  0.f,
		-3.f, 50.f, -3.f,
		 3.f, 50.f, -3.f,
		 3.f, 50.f,  3.f,
		-3.f, 50.f,  3.f,
		-3.f, 50.f, -3.f,

		 0.f,  0.f, 60.f,
		-3.f, -3.f, 50.f,
		 3.f, -3.f, 50.f,
		 3.f,  3.f, 50.f,
		-3.f,  3.f, 50.f,
		-3.f, -3.f, 50.f,
	};

	gl.DrawLines(3, arrows, 18);

	gl.ResetTransform();
}

bool VisualToolRotateXY::InitializeHold() {
	orig_x = angle_x;
	orig_y = angle_y;

	return true;
}

void VisualToolRotateXY::UpdateHold() {
	Vector2D delta = (mouse_pos - drag_start) * 2;
	if (shift_down)
		delta = delta.SingleAxis();

	angle_x = orig_x - delta.Y();
	angle_y = orig_y + delta.X();

	if (ctrl_down) {
		angle_x = floorf(angle_x / 30.f + .5f) * 30.f;
		angle_y = floorf(angle_y / 30.f + .5f) * 30.f;
	}

	angle_x = fmodf(angle_x + 360.f, 360.f);
	angle_y = fmodf(angle_y + 360.f, 360.f);

	SetSelectedOverride("\\frx", str(boost::format("%.4g") % angle_x));
	SetSelectedOverride("\\fry", str(boost::format("%.4g") % angle_y));
}

void VisualToolRotateXY::UpdateDrag(Feature *feature) {
	SetOverride(active_line, "\\org", ToScriptCoords(feature->pos).PStr());
}

void VisualToolRotateXY::DoRefresh() {
	if (!active_line) return;

	if (!(org->pos = GetLineOrigin(active_line)))
		org->pos = GetLinePosition(active_line);
	org->pos = FromScriptCoords(org->pos);

	GetLineRotation(active_line, angle_x, angle_y, angle_z);
}
