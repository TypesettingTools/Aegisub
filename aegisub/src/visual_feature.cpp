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

/// @file visual_feature->cpp
/// @brief Feature on video the user can interact with using mouse
/// @ingroup visual_ts
///

#include "config.h"

#include "gl_wrap.h"
#include "visual_feature.h"

VisualDraggableFeature::VisualDraggableFeature()
: type(DRAG_NONE)
, x(INT_MIN)
, y(INT_MIN)
, origX(INT_MIN)
, origY(INT_MIN)
, layer(0)
, line(NULL)
, lineN(-1)
{
}

bool VisualDraggableFeature::IsMouseOver(int mx,int my) const {
	switch (type) {
		case DRAG_BIG_SQUARE:
			return !(mx < x-8 || mx > x+8 || my < y-8 || my > y+8);
		case DRAG_BIG_CIRCLE: {
			int dx = mx-x;
			int dy = my-y;
			return dx*dx + dy*dy <= 64;
		}
		case DRAG_BIG_TRIANGLE: {
			int _my = my+2;
			if (_my < y-8 || _my > y+8) return false;
			int dx = mx-x;
			int dy = _my-y-8;
			return (16*dx+9*dy < 0 && 16*dx-9*dy > 0);
		}
		case DRAG_SMALL_SQUARE:
			return !(mx < x-4 || mx > x+4 || my < y-4 || my > y+4);
		case DRAG_SMALL_CIRCLE: {
			int dx = mx-x;
			int dy = my-y;
			return dx*dx + dy*dy <= 16;
		}
		default:
			return false;
	}
}

void VisualDraggableFeature::Draw(OpenGLWrapper const& gl) const {
	switch (type) {
		case DRAG_BIG_SQUARE:
			gl.DrawRectangle(x-8,y-8,x+8,y+8);
			gl.DrawLine(x,y-16,x,y+16);
			gl.DrawLine(x-16,y,x+16,y);
			break;
		case DRAG_BIG_CIRCLE:
			gl.DrawCircle(x,y,8);
			gl.DrawLine(x,y-16,x,y+16);
			gl.DrawLine(x-16,y,x+16,y);
			break;
		case DRAG_BIG_TRIANGLE:
			gl.DrawTriangle(x-9,y-6,x+9,y-6,x,y+10);
			gl.DrawLine(x,y,x,y-16);
			gl.DrawLine(x,y,x-14,y+8);
			gl.DrawLine(x,y,x+14,y+8);
			break;
		case DRAG_SMALL_SQUARE:
			gl.DrawRectangle(x-4,y-4,x+4,y+4);
			break;
		case DRAG_SMALL_CIRCLE:
			gl.DrawCircle(x,y,4);
			break;
		default:
			break;
	}
}
