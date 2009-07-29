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

/// @file visual_tool_clip.cpp
/// @brief Rectangular clipping visual typesetting tool
/// @ingroup visual_ts
///


///////////
// Headers
#include "config.h"

#include "visual_tool_clip.h"
#include "subs_grid.h"
#include "subs_edit_box.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "utils.h"



/// @brief Constructor 
/// @param _parent 
///
VisualToolClip::VisualToolClip(VideoDisplay *_parent)
: VisualTool(_parent)
{
	_parent->ShowCursor(false);

	// Set defaults
	curX1 = curY1 = 0;
	curX2 = sw;
	curY2 = sh;
	inverse = false;
	AssDialogue *line = GetActiveDialogueLine();
	if (line) GetLineClip(line,curX1,curY1,curX2,curY2,inverse);
}



/// @brief Update 
///
void VisualToolClip::Update() {
	// Render parent
	GetParent()->Render();
}



/// @brief Draw 
/// @return 
///
void VisualToolClip::Draw() {
	// Get current line
	AssDialogue *line = GetActiveDialogueLine();
	if (!line) return;

	// Get position
	if (!dragging && !holding) GetLineClip(line,curX1,curY1,curX2,curY2,inverse);
	int dx1 = curX1;
	int dy1 = curY1;
	int dx2 = curX2;
	int dy2 = curY2;

	// Draw rectangle
	SetLineColour(colour[3],1.0f,2);
	SetFillColour(colour[3],0.0f);
	DrawRectangle(dx1,dy1,dx2,dy2);

	// Draw outside area
	SetLineColour(colour[3],0.0f);
	SetFillColour(wxColour(0,0,0),0.5f);
	if (inverse) {
		DrawRectangle(dx1,dy1,dx2,dy2);
	}
	else {
		DrawRectangle(0,0,sw,dy1);
		DrawRectangle(0,dy2,sw,sh);
		DrawRectangle(0,dy1,dx1,dy2);
		DrawRectangle(dx2,dy1,sw,dy2);
	}

	// Draw circles
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.5);
	if (CanDrag()) DrawAllFeatures();
	else {
		DrawCircle(dx1,dy1,4);
		DrawCircle(dx2,dy1,4);
		DrawCircle(dx2,dy2,4);
		DrawCircle(dx1,dy2,4);
	}
}



/// @brief Start holding 
///
void VisualToolClip::InitializeHold() {
	startX = mouseX;
	startY = mouseY;
	curDiag->StripTag(_T("\\clip"));
	curDiag->StripTag(_T("\\iclip"));
}



/// @brief Update hold 
///
void VisualToolClip::UpdateHold() {
	// Coordinates
	curX1 = startX * sw / w;
	curY1 = startY * sh / h;
	curX2 = mouseX * sw / w;
	curY2 = mouseY * sh / h;

	// Make sure 1 is smaller than 2
	if (curX1 > curX2) IntSwap(curX1,curX2);
	if (curY1 > curY2) IntSwap(curY1,curY2);

	// Limit to video area
	curX1 = MID(0,curX1,sw);
	curX2 = MID(0,curX2,sw);
	curY1 = MID(0,curY1,sh);
	curY2 = MID(0,curY2,sh);
	
	// Features
	if (CanDrag()) PopulateFeatureList();
}



/// @brief Commit hold 
///
void VisualToolClip::CommitHold() {
	if (inverse)
		SetOverride(_T("\\iclip"),wxString::Format(_T("(%i,%i,%i,%i)"),curX1,curY1,curX2,curY2));
	else
		SetOverride(_T("\\clip"),wxString::Format(_T("(%i,%i,%i,%i)"),curX1,curY1,curX2,curY2));
}



/// @brief Populate feature list 
///
void VisualToolClip::PopulateFeatureList() {
	// Clear
	if (features.size() != 4) {
		features.clear();
		features.resize(4);
	}

	// Top-left
	int i = 0;
	features[i].x = curX1;
	features[i].y = curY1;
	features[i].brother[0] = 1;
	features[i].brother[1] = 2;
	features[i].brother[2] = 3;
	features[i].type = DRAG_SMALL_CIRCLE;
	i++;

	// Top-right
	features[i].x = curX2;
	features[i].y = curY1;
	features[i].brother[0] = 0;
	features[i].brother[1] = 3;
	features[i].brother[2] = 2;
	features[i].type = DRAG_SMALL_CIRCLE;
	i++;

	// Bottom-left
	features[i].x = curX1;
	features[i].y = curY2;
	features[i].brother[0] = 3;
	features[i].brother[1] = 0;
	features[i].brother[2] = 1;
	features[i].type = DRAG_SMALL_CIRCLE;
	i++;

	// Bottom-right
	features[i].x = curX2;
	features[i].y = curY2;
	features[i].brother[0] = 2;
	features[i].brother[1] = 1;
	features[i].brother[2] = 0;
	features[i].type = DRAG_SMALL_CIRCLE;
	i++;
}



/// @brief Initialize 
/// @param feature 
///
void VisualToolClip::InitializeDrag(VisualDraggableFeature &feature) {
	curDiag = GetActiveDialogueLine();
	curDiag->StripTag(_T("\\clip"));
	curDiag->StripTag(_T("\\iclip"));
}



/// @brief Update drag 
/// @param feature 
///
void VisualToolClip::UpdateDrag(VisualDraggableFeature &feature) {
	// Update brothers
	features[feature.brother[0]].y = feature.y;
	features[feature.brother[1]].x = feature.x;

	// Get "cur" from features
	curX1 = features[0].x;
	curX2 = features[3].x;
	curY1 = features[0].y;
	curY2 = features[3].y;

	// Make sure p1 < p2
	if (curX1 > curX2) IntSwap(curX1,curX2);
	if (curY1 > curY2) IntSwap(curY1,curY2);
}



/// @brief Done dragging 
/// @param feature 
///
void VisualToolClip::CommitDrag(VisualDraggableFeature &feature) {
	CommitHold();
}


