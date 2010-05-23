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

/// @file visual_tool_drag.h
/// @see visual_tool_drag.cpp
/// @ingroup visual_ts
///

#ifndef AGI_PRE
#include <wx/bmpbuttn.h>
#include <wx/toolbar.h>
#endif

#include "visual_feature.h"
#include "visual_tool.h"

/// @class VisualToolDragDraggableFeature
/// @brief VisualDraggableFeature with a time value
class VisualToolDragDraggableFeature : public VisualDraggableFeature {
public:
	int time;
	VisualToolDragDraggableFeature* parent;
	VisualToolDragDraggableFeature()
		: VisualDraggableFeature()
		, time(0)
		, parent(NULL)
	{ }
};


/// DOCME
/// @class VisualToolDrag
/// @brief DOCME
///
/// DOCME
class VisualToolDrag : public VisualTool<VisualToolDragDraggableFeature> {
private:
	wxToolBar *toolBar; /// The subtoolbar
	VisualToolDragDraggableFeature* primary; /// The feature last clicked on

	/// When the button is pressed, will it convert the line to a move (vs. from
	/// move to pos)? Used to avoid changing the button's icon unnecessarily
	bool toggleMoveOnMove;

	void PopulateFeatureList();
	bool InitializeDrag(VisualToolDragDraggableFeature* feature);
	void UpdateDrag(VisualToolDragDraggableFeature* feature);
	void CommitDrag(VisualToolDragDraggableFeature* feature);

	/// Set the pos/move button to the correct icon based on the active line
	void UpdateToggleButtons();
	void DoRefresh();

public:
	VisualToolDrag(VideoDisplay *parent, VideoState const& video, wxToolBar *toolbar);

	void Draw();
	void Update();
	void OnSubTool(wxCommandEvent &event);
};
