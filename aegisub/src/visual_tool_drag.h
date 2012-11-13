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

/// @file visual_tool_drag.h
/// @see visual_tool_drag.cpp
/// @ingroup visual_ts
///

#include "visual_feature.h"
#include "visual_tool.h"

/// @class VisualToolDragDraggableFeature
/// @brief VisualDraggableFeature with a time value
class VisualToolDragDraggableFeature : public VisualDraggableFeature {
public:
	int time;
	boost::container::list<VisualToolDragDraggableFeature>::iterator parent;
	VisualToolDragDraggableFeature() : VisualDraggableFeature(), time(0) { }
};

class wxBitmapButton;
class wxToolBar;

/// @class VisualToolDrag
/// @brief Moveable features for the positions of each visible line
class VisualToolDrag : public VisualTool<VisualToolDragDraggableFeature> {
	/// The subtoolbar for the move/pos conversion button
	wxToolBar *toolbar;
	/// The feature last clicked on for the double-click handler
	/// Equal to curFeature during drags; possibly different at all other times
	/// nullptr if no features have been clicked on or the last clicked on one no
	/// longer exists
	Feature *primary;
	/// The last announced selection set
	SubtitleSelection selection;

	/// When the button is pressed, will it convert the line to a move (vs. from
	/// move to pos)? Used to avoid changing the button's icon unnecessarily
	bool button_is_move;

	/// @brief Create the features for a line
	/// @param diag Line to create the features for
	/// @param pos Insertion point in the feature list
	void MakeFeatures(AssDialogue *diag, feature_iterator pos);
	void MakeFeatures(AssDialogue *diag);

	void OnSelectedSetChanged(SubtitleSelection const& lines_added, SubtitleSelection const& lines_removed);

	void OnFrameChanged();
	void OnFileChanged();
	void OnLineChanged();
	void OnCoordinateSystemsChanged() { OnFileChanged(); }

	bool InitializeDrag(feature_iterator feature);
	void UpdateDrag(feature_iterator feature);
	void Draw();
	void OnDoubleClick();

	/// Set the pos/move button to the correct icon based on the active line
	void UpdateToggleButtons();
	void OnSubTool(wxCommandEvent &event);
public:
	VisualToolDrag(VideoDisplay *parent, agi::Context *context);
	void SetToolbar(wxToolBar *tb);
};
