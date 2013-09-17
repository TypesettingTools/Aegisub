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

/// @file visual_tool_vector_clip.h
/// @see visual_tool_vector_clip.cpp
/// @ingroup visual_ts
///

#include "visual_feature.h"
#include "visual_tool.h"
#include "spline.h"

class wxToolBar;

/// @class VisualToolVectorClipDraggableFeature
/// @brief VisualDraggableFeature with information about a feature's location
///        in the spline
struct VisualToolVectorClipDraggableFeature : public VisualDraggableFeature {
	/// Which curve in the spline this feature is a point on
	Spline::iterator curve;
	/// 0-3; indicates which part of the curve this point is
	int point;
	/// @brief Constructor
	VisualToolVectorClipDraggableFeature()
	: VisualDraggableFeature()
	, point(0)
	{ }
};

class VisualToolVectorClip : public VisualTool<VisualToolVectorClipDraggableFeature> {
	Spline spline; /// The current spline
	wxToolBar *toolBar; /// The subtoolbar
	int mode; /// 0-7
	bool inverse; /// is iclip?

	/// @brief Set the mode
	/// @param mode 0-7
	void SetMode(int mode);

	void Save();

	void SelectAll();
	void MakeFeature(Spline::iterator cur);
	void MakeFeatures();

	bool InitializeHold();
	void UpdateHold();

	void UpdateDrag(Feature *feature);
	bool InitializeDrag(Feature *feature);

	void DoRefresh();
	void Draw();
public:
	VisualToolVectorClip(VideoDisplay *parent, agi::Context *context);
	void SetToolbar(wxToolBar *tb);
};
