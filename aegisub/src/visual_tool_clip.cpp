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

#include "config.h"

#ifndef AGI_PRE
#include <utility>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_display.h"
#include "visual_tool_clip.h"

VisualToolClip::VisualToolClip(VideoDisplay *parent, agi::Context *context, VideoState const& video, wxToolBar *)
: VisualTool<ClipCorner>(parent, context, video)
, curX1(0)
, curY1(0)
, curX2(video.w)
, curY2(video.h)
, inverse(false)
{
	if (curDiag) {
		GetLineClip(curDiag,curX1,curY1,curX2,curY2,inverse);
	}

	Feature feat;
	feat.type = DRAG_SMALL_CIRCLE;
	features.resize(4, feat);

	feature_iterator cur = features.begin();
	feats[0] = &*(cur++);
	feats[1] = &*(cur++);
	feats[2] = &*(cur++);
	feats[3] = &*(cur++);


	// Top-left
	int i = 0;
	feats[i]->horiz = feats[1];
	feats[i]->vert = feats[2];
	i++;

	// Top-right
	feats[i]->horiz = feats[0];
	feats[i]->vert = feats[3];
	i++;

	// Bottom-left
	feats[i]->horiz = feats[3];
	feats[i]->vert = feats[0];
	i++;

	// Bottom-right
	feats[i]->horiz = feats[2];
	feats[i]->vert = feats[1];
	i++;
}

void VisualToolClip::Draw() {
	if (!curDiag) return;

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
		DrawRectangle(0,0,video.w,dy1);
		DrawRectangle(0,dy2,video.w,video.h);
		DrawRectangle(0,dy1,dx1,dy2);
		DrawRectangle(dx2,dy1,video.w,dy2);
	}

	// Draw circles
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.5);
	DrawAllFeatures();
}

bool VisualToolClip::InitializeHold() {
	startX = video.x;
	startY = video.y;
	curDiag->StripTag(L"\\clip");
	curDiag->StripTag(L"\\iclip");
	return true;
}

void VisualToolClip::UpdateHold() {
	curX1 = startX;
	curY1 = startY;
	curX2 = video.x;
	curY2 = video.y;

	// Make sure 1 is smaller than 2
	if (curX1 > curX2) std::swap(curX1,curX2);
	if (curY1 > curY2) std::swap(curY1,curY2);

	// Limit to video area
	curX1 = mid(0,curX1,video.w);
	curX2 = mid(0,curX2,video.w);
	curY1 = mid(0,curY1,video.h);
	curY2 = mid(0,curY2,video.h);

	SetFeaturePositions();
}

void VisualToolClip::CommitHold() {
	int x1 = curX1;
	int x2 = curX2;
	int y1 = curY1;
	int y2 = curY2;
	parent->ToScriptCoords(&x1, &y1);
	parent->ToScriptCoords(&x2, &y2);
	SetOverride(curDiag, inverse ? L"\\iclip" : L"\\clip",wxString::Format(L"(%i,%i,%i,%i)",x1,y1,x2,y2));
}

bool VisualToolClip::InitializeDrag(feature_iterator) {
	curDiag->StripTag(L"\\clip");
	curDiag->StripTag(L"\\iclip");
	return true;
}

void VisualToolClip::UpdateDrag(feature_iterator feature) {
	// Update brothers
	feature->horiz->y = feature->y;
	feature->vert->x = feature->x;

	// Get "cur" from features
	curX1 = feats[0]->x;
	curX2 = feats[3]->x;
	curY1 = feats[0]->y;
	curY2 = feats[3]->y;

	// Make sure p1 < p2
	if (curX1 > curX2) std::swap(curX1,curX2);
	if (curY1 > curY2) std::swap(curY1,curY2);
}

void VisualToolClip::CommitDrag(feature_iterator) {
	CommitHold();
}

void VisualToolClip::SetFeaturePositions() {
	// Top-left
	int i = 0;
	feats[i]->x = curX1;
	feats[i]->y = curY1;
	i++;

	// Top-right
	feats[i]->x = curX2;
	feats[i]->y = curY1;
	i++;

	// Bottom-left
	feats[i]->x = curX1;
	feats[i]->y = curY2;
	i++;

	// Bottom-right
	feats[i]->x = curX2;
	feats[i]->y = curY2;
}

void VisualToolClip::DoRefresh() {
	if (curDiag) {
		GetLineClip(curDiag,curX1,curY1,curX2,curY2,inverse);
		SetFeaturePositions();
	}
}
