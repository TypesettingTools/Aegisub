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

/// @file visual_tool_scale.cpp
/// @brief X/Y scaling visual typesetting tool
/// @ingroup visual_ts

#include "config.h"

#include <cmath>

#include "visual_tool_scale.h"

#include "utils.h"

VisualToolScale::VisualToolScale(VideoDisplay *parent, agi::Context *context)
: VisualTool<VisualDraggableFeature>(parent, context)
, rx(0)
, ry(0)
, rz(0)
{
}

void VisualToolScale::Draw() {
	if (!active_line) return;

	// The length in pixels of the 100% zoom
	static const int base_len = 160;
	// The width of the y scale guide/height of the x scale guide
	static const int guide_size = 10;

	// Ensure that the scaling UI is comfortably visible on screen
	Vector2D base_point = pos
		.Max(Vector2D(base_len / 2 + guide_size, base_len / 2 + guide_size))
		.Min(video_res - base_len / 2 - guide_size * 3);

	// Set the origin to the base point and apply the line's rotation
	gl.SetOrigin(base_point);
	gl.SetRotation(rx, ry, rz);

	Vector2D scale_half_length = scale * base_len / 200;
	float minor_dim_offset = base_len / 2 + guide_size * 1.5f;

	// The ends of the current scale amount lines
	Vector2D x_p1(minor_dim_offset, -scale_half_length.Y());
	Vector2D x_p2(minor_dim_offset, scale_half_length.Y());
	Vector2D y_p1(-scale_half_length.X(), minor_dim_offset);
	Vector2D y_p2(scale_half_length.X(), minor_dim_offset);

	// Current scale amount lines
	gl.SetLineColour(colour[3], 1.f, 2);
	gl.DrawLine(x_p1, x_p2);
	gl.DrawLine(y_p1, y_p2);

	// Fake features at the end of the lines
	gl.SetLineColour(colour[0], 1.f, 1);
	gl.SetFillColour(colour[1], 0.3f);
	gl.DrawCircle(x_p1, 4);
	gl.DrawCircle(x_p2, 4);
	gl.DrawCircle(y_p1, 4);
	gl.DrawCircle(y_p2, 4);

	// Draw the guides
	int half_len = base_len / 2;
	gl.SetLineColour(colour[0], 1.f, 1);
	gl.DrawRectangle(Vector2D(half_len, -half_len), Vector2D(half_len + guide_size, half_len));
	gl.DrawRectangle(Vector2D(-half_len, half_len), Vector2D(half_len, half_len + guide_size));

	// Draw the feet
	gl.SetLineColour(colour[0], 1.f, 2);
	gl.DrawLine(Vector2D(half_len + guide_size, -half_len), Vector2D(half_len + guide_size + guide_size / 2, -half_len));
	gl.DrawLine(Vector2D(half_len + guide_size, half_len), Vector2D(half_len + guide_size + guide_size / 2, half_len));
	gl.DrawLine(Vector2D(-half_len, half_len + guide_size), Vector2D(-half_len, half_len + guide_size + guide_size / 2));
	gl.DrawLine(Vector2D(half_len, half_len + guide_size), Vector2D(half_len, half_len + guide_size + guide_size / 2));

	gl.ResetTransform();
}

bool VisualToolScale::InitializeHold() {
	initial_scale = scale;
	return true;
}

void VisualToolScale::UpdateHold() {
	Vector2D delta = (mouse_pos - drag_start) * Vector2D(1, -1);
	if (shift_down)
		delta = delta.SingleAxis();

	scale = Vector2D(0, 0).Max(delta * 1.25f + initial_scale);
	if (ctrl_down)
		scale = scale.Round(25.f);

	SetSelectedOverride("\\fscx", wxString::Format("%d", (int)scale.X()));
	SetSelectedOverride("\\fscy", wxString::Format("%d", (int)scale.Y()));
}

void VisualToolScale::DoRefresh() {
	if (!active_line) return;

	GetLineScale(active_line, scale);
	GetLineRotation(active_line, rx, ry, rz);
	pos = FromScriptCoords(GetLinePosition(active_line));
}
