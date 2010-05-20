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

/// @file visual_tool_rotatez.cpp
/// @brief 2D rotation in Z axis visual typesetting tool
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
#include "visual_tool_rotatez.h"

/// @brief Constructor 
/// @param _parent 
VisualToolRotateZ::VisualToolRotateZ(VideoDisplay *parent, VideoState const& video, wxToolBar *)
: VisualTool(parent, video)
{
	DoRefresh();
}

/// @brief Draw 
void VisualToolRotateZ::Draw() {
	// Get line to draw
	AssDialogue *line = GetActiveDialogueLine();
	if (!line) return;

	// Draw pivot
	DrawAllFeatures();

	// Radius
	int dx=0,dy=0;
	if (dragging) GetLinePosition(line,dx,dy);
	else GetLinePosition(line,dx,dy,orgx,orgy);
	int radius = (int) sqrt(double((dx-orgx)*(dx-orgx)+(dy-orgy)*(dy-orgy)));
	int oRadius = radius;
	if (radius < 50) radius = 50;

	// Pivot coordinates
	int odx = dx;
	int ody = dy;
	dx = orgx;
	dy = orgy;

	// Rotation
	float rz;
	GetLineRotation(line,rx,ry,rz);
	if (line == curDiag) rz = curAngle;

	// Get scale
	float scalX = 100.0f;
	float scalY = 100.0f;
	GetLineScale(line,scalX,scalY);

	// Get deltas
	int deltax = int(cos(rz*3.1415926536/180.0)*radius);
	int deltay = int(-sin(rz*3.1415926536/180.0)*radius);

	// Set colours
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.3f);

	// Set up the projection
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(dx,dy,-1.0f);
	float matrix[16] = { 2500, 0, 0, 0, 0, 2500, 0, 0, 0, 0, 1, 1, 0, 0, 2500, 2500 };
	glMultMatrixf(matrix);
	glScalef(1.0f,1.0f,8.0f);
	glRotatef(ry,0.0f,-1.0f,0.0f);
	glRotatef(rx,-1.0f,0.0f,0.0f);
	glScalef(scalX/100.0f,scalY/100.0f,1.0f);

	// Draw the circle
	DrawRing(0,0,radius+4,radius-4);

	// Draw markers around circle
	int markers = 6;
	float markStart = -90.0f / markers;
	float markEnd = markStart+(180.0f/markers);
	for (int i=0;i<markers;i++) {
		float angle = i*(360.0f/markers);
		DrawRing(0,0,radius+30,radius+12,radius/radius,angle+markStart,angle+markEnd);
	}

	// Draw the baseline
	SetLineColour(colour[3],1.0f,2);
	DrawLine(deltax,deltay,-deltax,-deltay);

	// Draw the connection line
	if (orgx != odx && orgy != ody) {
		double angle = atan2(double(dy-ody),double(odx-dx)) + rz*3.1415926536/180.0;
		int fx = int(cos(angle)*oRadius);
		int fy = -int(sin(angle)*oRadius);
		DrawLine(0,0,fx,fy);
		int mdx = int(cos(rz*3.1415926536/180.0)*20);
		int mdy = int(-sin(rz*3.1415926536/180.0)*20);
		DrawLine(fx-mdx,fy-mdy,fx+mdx,fy+mdy);
	}

	// Draw the rotation line
	SetLineColour(colour[0],1.0f,1);
	SetFillColour(colour[1],0.3f);
	DrawCircle(deltax,deltay,4);

	// Restore
	glPopMatrix();

	// Draw line to mouse
	if (!dragging && curFeature == -1) {
		SetLineColour(colour[0]);
		DrawLine(dx,dy,video.x,video.y);
	}
}

/// @brief Start holding 
bool VisualToolRotateZ::InitializeHold() {
	GetLinePosition(curDiag,odx,ody,orgx,orgy);
	startAngle = atan2(double(orgy-video.y),double(video.x-orgx)) * 180.0 / 3.1415926535897932;
	GetLineRotation(curDiag,rx,ry,origAngle);
	curAngle = origAngle;
	curDiag->StripTag(L"\\frz");
	curDiag->StripTag(L"\\fr");

	return true;
}

/// @brief Update hold 
void VisualToolRotateZ::UpdateHold() {
	// Find angle
	float screenAngle = atan2(double(orgy-video.y),double(video.x-orgx)) * 180.0 / 3.1415926535897932;
	curAngle = fmodf(screenAngle - startAngle + origAngle + 360.f, 360.f);

	// Oh Snap
	if (ctrlDown) {
		curAngle = floorf(curAngle/30.f+.5f)*30.0f;
		if (curAngle > 359.0f) curAngle = 0.0f;
	}
}

/// @brief Commit hold 
void VisualToolRotateZ::CommitHold() {
	SetOverride(GetActiveDialogueLine(), L"\\frz",wxString::Format(L"(%0.3g)",curAngle));
}

/// @brief Get \\org pivot 
void VisualToolRotateZ::PopulateFeatureList() {
	// Get line
	curDiag = GetActiveDialogueLine();
	GetLinePosition(curDiag,odx,ody,orgx,orgy);

	// Set features
	features.resize(1);
	VisualDraggableFeature &feat = features.back();
	feat.x = orgx;
	feat.y = orgy;
	feat.line = curDiag;
	feat.type = DRAG_BIG_TRIANGLE;
}

/// @brief Update dragging of \\org 
/// @param feature 
void VisualToolRotateZ::UpdateDrag(VisualDraggableFeature &feature) {
	orgx = feature.x;
	orgy = feature.y;
}

/// @brief Commit dragging of \\org 
/// @param feature 
void VisualToolRotateZ::CommitDrag(VisualDraggableFeature &feature) {
	int x = feature.x;
	int y = feature.y;
	parent->ToScriptCoords(&x, &y);
	SetOverride(feature.line, L"\\org",wxString::Format(L"(%i,%i)",x,y));
}

/// @brief Refresh 
void VisualToolRotateZ::DoRefresh() {
	AssDialogue *line = GetActiveDialogueLine();
	GetLinePosition(line,odx,ody,orgx,orgy);
	GetLineRotation(line,rx,ry,curAngle);
}

