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
class VisualToolVectorClipDraggableFeature : public VisualDraggableFeature {
public:
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

/// DOCME
/// @class VisualToolVectorClip
/// @brief DOCME
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
	void CommitHold();

	void UpdateDrag(feature_iterator feature);
	void CommitDrag(feature_iterator feature);
	bool InitializeDrag(feature_iterator feature);

	void DoRefresh();
	void Draw();
	bool Update() { return mode >= 1 && mode <= 4; }
public:
	VisualToolVectorClip(VideoDisplay *parent, agi::Context *context, VideoState const& video, wxToolBar *toolbar);

	/// Subtoolbar button click handler
	void OnSubTool(wxCommandEvent &event);
};
