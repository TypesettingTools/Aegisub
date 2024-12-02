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

#include "visual_tool_rotatexy.h"

#include "compat.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "selection_controller.h"

#include <libaegisub/format.h>

#include <cmath>
#include <wx/colour.h>

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

	// Load colors from options
	wxColour line_color_primary = to_wx(line_color_primary_opt->GetColor());
	wxColour line_color_secondary = to_wx(line_color_secondary_opt->GetColor());

	// Transform grid
	gl.SetOrigin(org->pos);
	gl.SetScale(100 * video_size / script_res);
	gl.SetRotation(angle_x, angle_y, angle_z, script_res.Y() / layout_res.Y());
	gl.SetScale(fsc);
	gl.SetShear(fax, fay);

	// Draw grid
	gl.SetLineColour(line_color_secondary, 0.5f, 2);
	gl.SetModeLine();
	float r = line_color_secondary.Red() / 255.f;
	float g = line_color_secondary.Green() / 255.f;
	float b = line_color_secondary.Blue() / 255.f;

	// Number of lines on each side of each axis
	static const int radius = 15;
	// Total number of lines, including center axis line
	static const int line_count = radius * 2 + 1;
	// Distance between each line in pixels
	static const int spacing = 20;
	// Length of each grid line in pixels from axis to one end
	static const int half_line_length = spacing * (radius + 1);
	static const float fade_factor = 0.9f / radius;

	std::vector<float> colors(line_count * 8 * 4);
	for (int i = 0; i < line_count * 8; ++i) {
		colors[i * 4 + 0] = r;
		colors[i * 4 + 1] = g;
		colors[i * 4 + 2] = b;
		colors[i * 4 + 3] = (i + 3) % 4 > 1 ? 0 : (1.f - abs(i / 8 - radius) * fade_factor);
	}

	std::vector<float> points(line_count * 8 * 2);
	for (int i = 0; i < line_count; ++i) {
		int pos = spacing * (i - radius);

		points[i * 16 + 0] = pos;
		points[i * 16 + 1] = half_line_length;

		points[i * 16 + 2] = pos;
		points[i * 16 + 3] = 0;

		points[i * 16 + 4] = pos;
		points[i * 16 + 5] = 0;

		points[i * 16 + 6] = pos;
		points[i * 16 + 7] = -half_line_length;

		points[i * 16 + 8] = half_line_length;
		points[i * 16 + 9] = pos;

		points[i * 16 + 10] = 0;
		points[i * 16 + 11] = pos;

		points[i * 16 + 12] = 0;
		points[i * 16 + 13] = pos;

		points[i * 16 + 14] = -half_line_length;
		points[i * 16 + 15] = pos;
	}

	gl.DrawLines(2, points, 4, colors);

	// Draw vectors
	gl.SetLineColour(line_color_primary, 1.f, 2);
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

	SetSelectedOverride("\\frx", agi::format("%.4g", angle_x));
	SetSelectedOverride("\\fry", agi::format("%.4g", angle_y));
}

void VisualToolRotateXY::UpdateDrag(Feature *feature) {
	auto org = GetLineOrigin(active_line);
	if (!org) org = GetLinePosition(active_line);
	auto d = ToScriptCoords(feature->pos) - org;

	for (auto line : c->selectionController->GetSelectedSet()) {
		org = GetLineOrigin(line);
		if (!org) org = GetLinePosition(line);
		SetOverride(line, "\\org", (d + org).PStr());
	}
}

void VisualToolRotateXY::DoRefresh() {
	if (!active_line) return;

	if (!(org->pos = GetLineOrigin(active_line)))
		org->pos = GetLinePosition(active_line);
	org->pos = FromScriptCoords(org->pos);

	GetLineRotation(active_line, angle_x, angle_y, angle_z);
	GetLineShear(active_line, fax, fay);
	GetLineScale(active_line, fsc);
}
