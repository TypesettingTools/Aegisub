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

/// @file visual_tool_clip.h
/// @see visual_tool_clip.cpp
/// @ingroup visual_ts
///

#include "visual_feature.h"
#include "visual_tool.h"

/// @class ClipCorner
/// @brief VisualDraggableFeature with siblings
struct ClipCorner : public VisualDraggableFeature {
	ClipCorner *horiz; /// Other corner on this corner's horizontal line
	ClipCorner *vert;  /// Other corner on this corner's vertical line
	ClipCorner() : VisualDraggableFeature() , horiz(0) , vert(0) { }
};

/// DOCME
/// @class VisualToolClip
/// @brief DOCME
///
/// DOCME
class VisualToolClip : public VisualTool<ClipCorner> {
	Vector2D cur_1;
	Vector2D cur_2;

	bool inverse; ///< Is this currently in iclip mode?

	bool InitializeHold();
	void UpdateHold();
	void CommitHold();

	void DoRefresh();
	void SetFeaturePositions();

	bool InitializeDrag(feature_iterator feature);
	void UpdateDrag(feature_iterator feature);

	void Draw();
public:
	VisualToolClip(VideoDisplay *parent, agi::Context *context);
};
