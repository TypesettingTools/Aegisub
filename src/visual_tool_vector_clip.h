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

#pragma once

#include "visual_feature.h"
#include "visual_tool.h"
#include "spline.h"
#include "command/command.h"

class wxToolBar;

/// Button IDs
enum VisualToolVectorClipMode {
	VCLIP_DRAG = 0, // Assumed to be at the start
	VCLIP_LINE,
	VCLIP_BICUBIC,
	VCLIP_CONVERT,
	VCLIP_INSERT,
	VCLIP_REMOVE,
	VCLIP_FREEHAND,
	VCLIP_FREEHAND_SMOOTH,
	VCLIP_LAST // Leave this at the end and don't use it
};

/// @class VisualToolVectorClipDraggableFeature
/// @brief VisualDraggableFeature with information about a feature's location
///        in the spline
struct VisualToolVectorClipDraggableFeature final : public VisualDraggableFeature {
	/// Which curve in the spline this feature is a point on
	size_t idx = 0;
	/// 0-3; indicates which part of the curve this point is
	int point = 0;
};

class VisualToolVectorClip final : public VisualTool<VisualToolVectorClipDraggableFeature> {
	Spline spline; /// The current spline
	wxToolBar *toolBar = nullptr; /// The subtoolbar
	int mode = VCLIP_DRAG; /// 0-7
	bool inverse = false; /// is iclip?

	std::set<Feature *> box_added;

	void Save();
	void Commit(wxString message="") override;

	void AddTool(std::string command_name, VisualToolVectorClipMode mode);

	void MakeFeature(size_t idx);
	void MakeFeatures();

	bool InitializeHold() override;
	void UpdateHold() override;

	void UpdateDrag(Feature *feature) override;
	bool InitializeDrag(Feature *feature) override;

	void DoRefresh() override;
	void Draw() override;

public:
	VisualToolVectorClip(VideoDisplay *parent, agi::Context *context);
	void SetToolbar(wxToolBar *tb) override;

	void SetSubTool(int subtool) override;
	int GetSubTool() override;
};
