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

/// @file visual_tool_rotatexy.h
/// @see visual_tool_rotatexy.cpp
/// @ingroup visual_ts
///

#include "visual_feature.h"
#include "visual_tool.h"

class VisualToolRotateXY final : public VisualTool<VisualDraggableFeature> {
	float angle_x = 0.f; /// Current x rotation
	float angle_y = 0.f; /// Current y rotation
	float angle_z = 0.f; /// Current z rotation

	float fax = 0.f;
	float fay = 0.f;

	float orig_x = 0.f; ///< x rotation at the beginning of the current hold
	float orig_y = 0.f; ///< y rotation at the beginning of the current hold

	Feature* org;

	void DoRefresh() override;
	void Draw() override;
	void UpdateDrag(Feature* feature) override;
	bool InitializeHold() override;
	void UpdateHold() override;

  public:
	VisualToolRotateXY(VideoDisplay* parent, agi::Context* context);
};
