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

/// @file visual_feature->cpp
/// @brief Feature on video the user can interact with using mouse
/// @ingroup visual_ts
///

#include "config.h"

#include "gl_wrap.h"
#include "visual_feature.h"

VisualDraggableFeature::VisualDraggableFeature()
: type(DRAG_NONE)
, layer(0)
, line(nullptr)
{
}

bool VisualDraggableFeature::IsMouseOver(Vector2D mouse_pos) const {
	if (!pos) return false;

	Vector2D delta = mouse_pos - pos;

	switch (type) {
		case DRAG_BIG_SQUARE:
			return fabs(delta.X()) < 8 && fabs(delta.Y()) < 8;

		case DRAG_BIG_CIRCLE:
			return delta.SquareLen() < 64;

		case DRAG_BIG_TRIANGLE: {
			if (delta.Y() < -10 || delta.Y() > 6) return false;
			float dy = delta.Y() - 6;
			return 16 * delta.X() + 9 * dy < 0 && 16 * delta.X() - 9 * dy > 0;
		}

		case DRAG_SMALL_SQUARE:
			return fabs(delta.X()) < 4 && fabs(delta.Y()) < 4;

		case DRAG_SMALL_CIRCLE:
			return delta.SquareLen() < 16;

		default:
			return false;
	}
}

void VisualDraggableFeature::Draw(OpenGLWrapper const& gl) const {
	if (!pos) return;

	switch (type) {
		case DRAG_BIG_SQUARE:
			gl.DrawRectangle(pos - 8, pos + 8);
			gl.DrawLine(pos - Vector2D(0, 16), pos + Vector2D(0, 16));
			gl.DrawLine(pos - Vector2D(16, 0), pos + Vector2D(16, 0));
			break;

		case DRAG_BIG_CIRCLE:
			gl.DrawCircle(pos, 8);
			gl.DrawLine(pos - Vector2D(0, 16), pos + Vector2D(0, 16));
			gl.DrawLine(pos - Vector2D(16, 0), pos + Vector2D(16, 0));
			break;

		case DRAG_BIG_TRIANGLE:
			gl.DrawTriangle(pos - Vector2D(9, 6), pos + Vector2D(9, -6), pos + Vector2D(0, 10));
			gl.DrawLine(pos, pos + Vector2D(0, -16));
			gl.DrawLine(pos, pos + Vector2D(-14, 8));
			gl.DrawLine(pos, pos + Vector2D(14, 8));
			break;

		case DRAG_SMALL_SQUARE:
			gl.DrawRectangle(pos - 4, pos + 4);
			break;

		case DRAG_SMALL_CIRCLE:
			gl.DrawCircle(pos, 4);
			break;
		default:
			break;
	}
}

void VisualDraggableFeature::StartDrag() {
	start = pos;
}

void VisualDraggableFeature::UpdateDrag(Vector2D d, bool single_axis) {
	if (single_axis)
		d = d.SingleAxis();

	pos = start + d;
}

bool VisualDraggableFeature::HasMoved() const {
	return pos != start;
}
