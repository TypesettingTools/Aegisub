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

/// @file visual_tool_scale.h
/// @see visual_tool_scale.cpp
/// @ingroup visual_ts
///

#include "visual_feature.h"
#include "visual_tool.h"

class VisualToolScale : public VisualTool<VisualDraggableFeature> {
	Vector2D scale; ///< The current scale
	Vector2D initial_scale; ///< The scale at the beginning of the current hold
	Vector2D pos; ///< Position of the line

	float rx = 0.f; ///< X rotation
	float ry = 0.f; ///< Y rotation
	float rz = 0.f; ///< Z rotation

	bool InitializeHold() override;
	void UpdateHold() override;

	void DoRefresh() override;
	void Draw() override;
public:
	VisualToolScale(VideoDisplay *parent, agi::Context *context);
};
