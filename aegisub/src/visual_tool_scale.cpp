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

/// @file visual_tool_scale.cpp
/// @brief X/Y scaling visual typesetting tool
/// @ingroup visual_ts

#include "config.h"

#ifndef AGI_PRE
#include <math.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_display.h"
#include "visual_tool_scale.h"

/// @brief Constructor 
/// @param _parent 
VisualToolScale::VisualToolScale(VideoDisplay *parent, VideoState const& video, wxToolBar *)
: VisualTool<VisualDraggableFeature>(parent, video)
, curScaleX(0.f)
, startScaleX(0.f)
, origScaleX(0.f)
, curScaleY(0.f)
, startScaleY(0.f)
, origScaleY(0.f)
, startX(0)
, startY(0)
{
}

/// @brief Draw 
void VisualToolScale::Draw() {
	// Get line to draw
	AssDialogue *line = GetActiveDialogueLine();
	if (!line) return;

	// Get scale
	if (line != curDiag) GetLineScale(line,curScaleX,curScaleY);

	// Get line position and rotation
	int dx,dy;
	float rx,ry,rz;
	GetLinePosition(line,dx,dy);
	GetLineRotation(line,rx,ry,rz);

	// Set dx/dy
	int len = 160;
	dx = MID(len/2+10,dx,video.w-len/2-30);
	dy = MID(len/2+10,dy,video.h-len/2-30);

	// Set colours
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.3f);

	// Transform grid
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(dx,dy,0.0f);
	float matrix[16] = { 2500, 0, 0, 0, 0, 2500, 0, 0, 0, 0, 1, 1, 0, 0, 2500, 2500 };
	glMultMatrixf(matrix);
	glScalef(1.0f,1.0f,8.0f);
	if (ry != 0.0f) glRotatef(ry,0.0f,-1.0f,0.0f);
	if (rx != 0.0f) glRotatef(rx,-1.0f,0.0f,0.0f);
	if (rz != 0.0f) glRotatef(rz,0.0f,0.0f,-1.0f);
	
	// Scale parameters
	int lenx = int(1.6 * curScaleX);
	int leny = int(1.6 * curScaleY);
	int drawX = len/2 + 10;
	int drawY = len/2 + 10;

	// Draw length markers
	SetLineColour(colour[3],1.0f,2);
	DrawLine(-lenx/2,drawY+10,lenx/2,drawY+10);
	DrawLine(drawX+10,-leny/2,drawX+10,leny/2);
	SetLineColour(colour[0],1.0f,1);
	SetFillColour(colour[1],0.3f);
	DrawCircle(lenx/2,drawY+10,4);
	DrawCircle(drawX+10,-leny/2,4);

	// Draw horizontal scale
	SetLineColour(colour[0],1.0f,1);
	DrawRectangle(-len/2,drawY,len/2+1,drawY+5);
	SetLineColour(colour[0],1.0f,2);
	DrawLine(-len/2+1,drawY+5,-len/2+1,drawY+15);
	DrawLine(len/2,drawY+5,len/2,drawY+15);

	// Draw vertical scale
	SetLineColour(colour[0],1.0f,1);
	DrawRectangle(drawX,-len/2,drawX+5,len/2+1);
	SetLineColour(colour[0],1.0f,2);
	DrawLine(drawX+5,-len/2+1,drawX+15,-len/2+1);
	DrawLine(drawX+5,len/2,drawX+15,len/2);

	// Restore gl's state
	glPopMatrix();
}

/// @brief Start holding 
bool VisualToolScale::InitializeHold() {
	startX = video.x;
	startY = video.y;
	GetLineScale(curDiag,origScaleX,origScaleY);
	curScaleX = origScaleX;
	curScaleY = origScaleY;
	curDiag->StripTag(L"\\fscx");
	curDiag->StripTag(L"\\fscy");

	return true;
}

/// @brief Update hold 
void VisualToolScale::UpdateHold() {
	using std::max;
	// Deltas
	int deltaX = video.x - startX;
	int deltaY = startY - video.y;
	if (shiftDown) {
		if (abs(deltaX) >= abs(deltaY)) deltaY = 0;
		else deltaX = 0;
	}

	// Calculate
	curScaleX = max(deltaX*1.25f + origScaleX, 0.f);
	curScaleY = max(deltaY*1.25f + origScaleY, 0.f);

	// Oh Snap
	if (ctrlDown) {
		curScaleX = floorf(curScaleX/25.f+.5f)*25.0f;
		curScaleY = floorf(curScaleY/25.f+.5f)*25.0f;
	}
}

/// @brief Commit hold 
void VisualToolScale::CommitHold() {
	SetOverride(GetActiveDialogueLine(), L"\\fscx",wxString::Format(L"(%0.3g)",curScaleX));
	SetOverride(GetActiveDialogueLine(), L"\\fscy",wxString::Format(L"(%0.3g)",curScaleY));
}

