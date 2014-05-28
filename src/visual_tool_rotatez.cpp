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

/// @file visual_tool_rotatez.cpp
/// @brief 2D rotation in Z axis visual typesetting tool
/// @ingroup visual_ts

#include "visual_tool_rotatez.h"

#include <libaegisub/format.h>

#include <cmath>
#include <wx/colour.h>

static const float deg2rad = 3.1415926536f / 180.f;
static const float rad2deg = 180.f / 3.1415926536f;

VisualToolRotateZ::VisualToolRotateZ(VideoDisplay *parent, agi::Context *context)
: VisualTool<VisualDraggableFeature>(parent, context)
, org(new Feature)
{
	features.push_back(*org);
	org->type = DRAG_BIG_TRIANGLE;
}

void VisualToolRotateZ::Draw() {
	if (!active_line) return;

	DrawAllFeatures();

	float radius = (pos - org->pos).Len();
	float oRadius = radius;
	if (radius < 50)
		radius = 50;

	// Set up the projection
	gl.SetOrigin(org->pos);
	gl.SetRotation(rotation_x, rotation_y, 0);
	gl.SetScale(scale);

	// Draw the circle
	gl.SetLineColour(colour[0]);
	gl.SetFillColour(colour[1], 0.3f);
	gl.DrawRing(Vector2D(0, 0), radius + 4, radius - 4);

	// Draw markers around circle
	int markers = 6;
	float markStart = -90.f / markers;
	float markEnd = markStart + (180.f / markers);
	for (int i = 0; i < markers; ++i) {
		float angle = i * (360.f / markers);
		gl.DrawRing(Vector2D(0, 0), radius+30, radius+12, 1.0, angle+markStart, angle+markEnd);
	}

	// Draw the baseline through the origin showing current rotation
	Vector2D angle_vec(Vector2D::FromAngle(angle * deg2rad));
	gl.SetLineColour(colour[3], 1, 2);
	gl.DrawLine(angle_vec * -radius, angle_vec * radius);

	if (org->pos != pos) {
		Vector2D rotated_pos = Vector2D::FromAngle(angle * deg2rad - (pos - org->pos).Angle()) * oRadius;

		// Draw the line from origin to rotated position
		gl.DrawLine(Vector2D(), rotated_pos);

		// Draw the line under the text
		gl.DrawLine(rotated_pos - angle_vec * 20, rotated_pos + angle_vec * 20);
	}

	// Draw the fake features on the ring
	gl.SetLineColour(colour[0], 1.f, 1);
	gl.SetFillColour(colour[1], 0.3f);
	gl.DrawCircle(angle_vec * radius, 4);
	gl.DrawCircle(angle_vec * -radius, 4);

	// Clear the projection
	gl.ResetTransform();

	// Draw line to mouse if it isn't over the origin feature
	if (mouse_pos && (mouse_pos - org->pos).SquareLen() > 100) {
		gl.SetLineColour(colour[0]);
		gl.DrawLine(org->pos, mouse_pos);
	}
}

bool VisualToolRotateZ::InitializeHold() {
	orig_angle = angle + (org->pos - mouse_pos).Angle() * rad2deg;
	return true;
}

void VisualToolRotateZ::UpdateHold() {
	angle = orig_angle - (org->pos - mouse_pos).Angle() * rad2deg;

	if (ctrl_down)
		angle = floorf(angle / 30.f + .5f) * 30.f;

	angle = fmodf(angle + 360.f, 360.f);

	SetSelectedOverride("\\frz", agi::format("%.4g", angle));
}

void VisualToolRotateZ::UpdateDrag(Feature *feature) {
	SetOverride(active_line, "\\org", ToScriptCoords(feature->pos).PStr());
}

void VisualToolRotateZ::DoRefresh() {
	if (!active_line) return;

	pos = FromScriptCoords(GetLinePosition(active_line));
	if (!(org->pos = GetLineOrigin(active_line)))
		org->pos = pos;
	else
		org->pos = FromScriptCoords(org->pos);

	GetLineRotation(active_line, rotation_x, rotation_y, angle);
	GetLineScale(active_line, scale);
}
