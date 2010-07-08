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

/// @file visual_tool_drag.cpp
/// @brief Position all visible subtitles by dragging visual typesetting tool
/// @ingroup visual_ts

#include "config.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "libresrc/libresrc.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"
#include "video_display.h"
#include "visual_tool_drag.h"

enum {
	BUTTON_TOGGLE_MOVE = VISUAL_SUB_TOOL_START
};
static const DraggableFeatureType DRAG_ORIGIN = DRAG_BIG_TRIANGLE;
static const DraggableFeatureType DRAG_START = DRAG_BIG_SQUARE;
static const DraggableFeatureType DRAG_END = DRAG_BIG_CIRCLE;

/// @brief Constructor 
/// @param _parent 
/// @param toolBar 
VisualToolDrag::VisualToolDrag(VideoDisplay *parent, VideoState const& video, wxToolBar * toolBar)
: VisualTool<VisualToolDragDraggableFeature>(parent, video)
, toolBar(toolBar)
, primary(NULL)
, toggleMoveOnMove(true)
{
	toolBar->AddTool(BUTTON_TOGGLE_MOVE, _("Toggle between \\move and \\pos"), GETIMAGE(visual_move_conv_move_24));
	toolBar->Realize();
	toolBar->Show(true);

	grid->GetSelectedSet(selection);
	OnFileChanged();
}

void VisualToolDrag::UpdateToggleButtons() {
	// Check which bitmap to use
	bool toMove = true;
	if (curDiag) {
		int x1,y1,x2,y2,t1,t2;
		bool hasMove;
		GetLineMove(curDiag,hasMove,x1,y1,x2,y2,t1,t2);
		toMove = !hasMove;
	}

	// No change needed
	if (toMove == toggleMoveOnMove) return;

	// Change bitmap
	if (toMove) {
		toolBar->SetToolNormalBitmap(BUTTON_TOGGLE_MOVE, GETIMAGE(visual_move_conv_move_24));
	}
	else {
		toolBar->SetToolNormalBitmap(BUTTON_TOGGLE_MOVE, GETIMAGE(visual_move_conv_pos_24));
	}
	toggleMoveOnMove = toMove;
}

/// @brief Toggle button pressed 
/// @param event 
void VisualToolDrag::OnSubTool(wxCommandEvent &) {
	// Toggle \move <-> \pos
	for (Selection::const_iterator cur = selection.begin(); cur != selection.end(); ++cur) {
		AssDialogue *line = *cur;
		int x1,y1,x2,y2,t1,t2;
		bool hasMove;

		GetLinePosition(line,x1,y1);
		GetLineMove(line,hasMove,x1,y1,x2,y2,t1,t2);
		parent->ToScriptCoords(&x1, &y1);
		parent->ToScriptCoords(&x2, &y2);

		if (hasMove) SetOverride(line, L"\\pos",wxString::Format(L"(%i,%i)",x1,y1));
		else SetOverride(line, L"\\move",wxString::Format(L"(%i,%i,%i,%i,%i,%i)",x1,y1,x1,y1,0,line->End.GetMS() - line->Start.GetMS()));
	}

	Commit(true);
	Refresh();
}

void VisualToolDrag::OnLineChanged() {
	UpdateToggleButtons();
}

void VisualToolDrag::OnFileChanged() {
	/// @todo it should be possible to preserve the selection in some cases
	features.clear();
	ClearSelection();
	primary = NULL;

	for (int i = grid->GetRows() - 1; i >=0; i--) {
		AssDialogue *diag = grid->GetDialogue(i);
		if (BaseGrid::IsDisplayed(diag)) {
			MakeFeatures(diag);
		}
	}
}

void VisualToolDrag::OnFrameChanged() {
	if (primary && !BaseGrid::IsDisplayed(primary->line)) primary = NULL;

	feature_iterator feat = features.begin();
	feature_iterator end = features.end();

	for (int i = grid->GetRows() - 1; i >=0; i--) {
		AssDialogue *diag = grid->GetDialogue(i);
		if (BaseGrid::IsDisplayed(diag)) {
			// Features don't exist and should
			if (feat == end || feat->line != diag) {
				MakeFeatures(diag, feat);
			}
			// Move past already existing features for the line
			else {
				while (feat != end && feat->line == diag) ++feat;
			}
		}
		else {
			// Remove all features for this line (if any)
			while (feat != end && feat->line == diag) {
				feat->line = NULL;
				RemoveSelection(feat);
				feat = features.erase(feat);
			}
		}
	}
}

void VisualToolDrag::OnSelectedSetChanged(const Selection &added, const Selection &removed) {
	grid->GetSelectedSet(selection);
	if (!externalChange) return;
	externalChange = false;
	grid->BeginBatch();

	for (feature_iterator cur = features.begin(); cur != features.end(); ++cur) {
		// Remove all deselected lines
		if (removed.find(cur->line) != removed.end()) {
			RemoveSelection(cur);
		}
		// And add all newly selected lines
		else if (added.find(cur->line) != added.end() && cur->type == DRAG_START) {
			AddSelection(cur);
		}
	}

	grid->EndBatch();
	externalChange = true;
}

void VisualToolDrag::Draw() {
	DrawAllFeatures();

	// Draw arrows
	for (feature_iterator cur = features.begin(); cur != features.end(); ++cur) {
		if (cur->type == DRAG_START) continue;
		feature_iterator p2 = cur;
		feature_iterator p1 = cur->parent;

		// Has arrow?
		bool hasArrow = p2->type == DRAG_END;
		int arrowLen = hasArrow ? 10 : 0;

		// See if the distance between them is enough
		int dx = p2->x - p1->x;
		int dy = p2->y - p1->y;
		int dist = (int)sqrt(double(dx*dx + dy*dy));
		if (dist < 20+arrowLen) continue;

		// Get end points
		int x1 = p1->x + dx*10/dist;
		int x2 = p2->x - dx*(10+arrowLen)/dist;
		int y1 = p1->y + dy*10/dist;
		int y2 = p2->y - dy*(10+arrowLen)/dist;

		// Draw arrow
		if (hasArrow) {
			// Calculate angle
			double angle = atan2(double(y2-y1),double(x2-x1))+1.570796;
			int sx = int(cos(angle)*4);
			int sy = int(-sin(angle)*4);

			// Arrow line
			SetLineColour(colour[3],0.8f,2);
			DrawLine(x1,y1,x2,y2);

			// Arrow head
			DrawLine(x2+sx,y2-sy,x2-sx,y2+sy);
			DrawLine(x2+sx,y2-sy,x2+dx*10/dist,y2+dy*10/dist);
			DrawLine(x2-sx,y2+sy,x2+dx*10/dist,y2+dy*10/dist);
		}

		// Draw dashed line
		else {
			SetLineColour(colour[3],0.5f,2);
			DrawDashedLine(x1,y1,x2,y2,6);
		}
	}
}
void VisualToolDrag::MakeFeatures(AssDialogue *diag) {
	MakeFeatures(diag, features.end());
}
void VisualToolDrag::MakeFeatures(AssDialogue *diag, feature_iterator pos) {
	// Get position
	int x1,x2,y1,y2;
	int t1=0;
	int t2=diag->End.GetMS()-diag->Start.GetMS();
	int torgx,torgy;
	bool hasMove;
	GetLinePosition(diag,x1,y1,torgx,torgy);
	GetLineMove(diag,hasMove,x1,y1,x2,y2,t1,t2);

	// Create \pos feature
	Feature feat;
	feat.x = x1;
	feat.y = y1;
	feat.layer = 0;
	feat.type = DRAG_START;
	feat.time = t1;
	feat.line = diag;
	feat.parent = features.end();
	features.insert(pos, feat);
	feature_iterator cur = pos; --cur;
	feat.parent = cur;
	if (selection.find(diag) != selection.end()) {
		AddSelection(cur);
	}

	// Create move destination feature
	if (hasMove) {
		feat.x = x2;
		feat.y = y2;
		feat.layer = 1;
		feat.type = DRAG_END;
		feat.time = t2;
		feat.line = diag;
		features.insert(pos, feat);
		feat.parent->parent = --pos; ++pos;
	}
	// Create org feature
	if (torgx != x1 || torgy != y1) {
		feat.x = torgx;
		feat.y = torgy;
		feat.layer = -1;
		feat.type = DRAG_ORIGIN;
		feat.time = 0;
		feat.line = diag;
		features.insert(pos, feat);
	}
}

bool VisualToolDrag::InitializeDrag(feature_iterator feature) {
	primary = &*feature;

	// Set time of clicked feature to the current frame and shift all other
	// selected features by the same amount
	if (feature->type != DRAG_ORIGIN) {
		int time = VideoContext::Get()->TimeAtFrame(frameNumber) - feature->line->Start.GetMS();
		int change = time - feature->time;

		for (sel_iterator cur = selectedFeatures.begin(); cur != selectedFeatures.end(); ++cur) {
			if ((*cur)->type != DRAG_ORIGIN) {
				(*cur)->time += change;
			}
		}
	}
	return true;
}

void VisualToolDrag::CommitDrag(feature_iterator feature) {
	if (feature->type == DRAG_ORIGIN) {
		int x = feature->x;
		int y = feature->y;
		parent->ToScriptCoords(&x, &y);
		SetOverride(feature->line, L"\\org",wxString::Format(L"(%i,%i)",x,y));
		return;
	}

	feature_iterator p = feature->parent;
	if (feature->type == DRAG_END) {
		std::swap(feature, p);
	}

	int x1 = feature->x;
	int y1 = feature->y;
	parent->ToScriptCoords(&x1, &y1);

	// Position
	if (feature->parent == features.end()) {
		SetOverride(feature->line, L"\\pos", wxString::Format(L"(%i,%i)", x1, y1));
	}
	// Move
	else {
		int x2 = p->x;
		int y2 = p->y;
		parent->ToScriptCoords(&x2, &y2);

		SetOverride(feature->line, L"\\move", wxString::Format(L"(%i,%i,%i,%i,%i,%i)", x1, y1, x2, y2, feature->time, p->time));
	}
}
bool VisualToolDrag::Update() {
	if (!leftDClick) return false;

	int dx, dy;
	int vx = video.x;
	int vy = video.y;
	parent->ToScriptCoords(&vx, &vy);
	if (primary) {
		dx = primary->x;
		dy = primary->y;
	}
	else {
		if (!curDiag) return false;
		GetLinePosition(curDiag, dx, dy);
	}
	parent->ToScriptCoords(&dx, &dy);
	dx -= vx;
	dy -= vy;

	for (Selection::const_iterator cur = selection.begin(); cur != selection.end(); ++cur) {
		int x1 = 0, y1 = 0, x2 = 0, y2 = 0, t1 = INT_MIN, t2 = INT_MIN, orgx, orgy;
		bool isMove;

		GetLinePosition(*cur, x1, y1, orgx, orgy);
		GetLineMove(*cur, isMove, x1, y1, x2, y2, t1, t2);
		parent->ToScriptCoords(&x1, &y1);
		parent->ToScriptCoords(&x2, &y2);
		parent->ToScriptCoords(&orgx, &orgy);

		if (isMove) {
			if (t1 > INT_MIN && t2 > INT_MIN)
				SetOverride(*cur, L"\\move", wxString::Format(L"(%i,%i,%i,%i,%i,%i)", x1 - dx, y1 - dy, x2 - dx, y2 - dy, t1, t2));
			else
				SetOverride(*cur, L"\\move", wxString::Format(L"(%i,%i,%i,%i)", x1, y1, x2, y2));
		}
		else {
			SetOverride(*cur, L"\\pos", wxString::Format(L"(%i,%i)", x1 - dx, y1 - dy));
		}
		if (orgx != x1 || orgy != y1) {
			SetOverride(*cur, L"\\org", wxString::Format(L"(%i,%i)", orgx - dx, orgy - dy));
		}
	}

	Commit(true, _("positioning"));

	OnFileChanged();
	return false;
}
