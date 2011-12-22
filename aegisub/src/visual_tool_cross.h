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
//
// $Id$

/// @file visual_tool_cross.h
/// @see visual_tool_cross.cpp
/// @ingroup visual_ts
///

#include <libaegisub/scoped_ptr.h>

#include "visual_feature.h"
#include "visual_tool.h"

class OpenGLText;

/// @class VisualToolCross
/// @brief A crosshair which shows the current mouse position and on double-click
///        shifts the selected lines to the clicked point
class VisualToolCross : public VisualTool<VisualDraggableFeature> {
	agi::scoped_ptr<OpenGLText> gl_text;

	void OnDoubleClick();
	void Draw();
	wxString Text(Vector2D v);
public:
	VisualToolCross(VideoDisplay *parent, agi::Context *context);
	~VisualToolCross();
};
