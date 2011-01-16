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
#include <cmath>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"
#include "video_display.h"
#include "visual_tool_rotatez.h"

static const float deg2rad = 3.1415926536f / 180.f;
static const float rad2deg = 180.f / 3.1415926536f;

VisualToolRotateZ::VisualToolRotateZ(VideoDisplay *parent, agi::Context *context, VideoState const& video, wxToolBar *)
: VisualTool<VisualDraggableFeature>(parent, context, video)
{
	features.resize(1);
	org = &features.back();
	org->type = DRAG_BIG_TRIANGLE;
	DoRefresh();
}

void VisualToolRotateZ::Draw() {
	if (!curDiag) return;

	// Draw pivot
	DrawAllFeatures();

	int radius = (int)sqrt(double((posx-org->x)*(posx-org->x)+(posy-org->y)*(posy-org->y)));
	int oRadius = radius;
	if (radius < 50) radius = 50;

	int deltax = int(cos(curAngle*deg2rad)*radius);
	int deltay = int(-sin(curAngle*deg2rad)*radius);

	// Set colours
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.3f);

	// Set up the projection
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(org->x,org->y,-1.f);
	float matrix[16] = { 2500, 0, 0, 0, 0, 2500, 0, 0, 0, 0, 1, 1, 0, 0, 2500, 2500 };
	glMultMatrixf(matrix);
	glScalef(1.f,1.f,8.f);
	glRotatef(ry,0.f,-1.f,0.f);
	glRotatef(rx,-1.f,0.f,0.f);
	glScalef(scaleX/100.f,scaleY/100.f,1.f);

	// Draw the circle
	DrawRing(0,0,radius+4,radius-4);

	// Draw markers around circle
	int markers = 6;
	float markStart = -90.f / markers;
	float markEnd = markStart+(180.f/markers);
	for (int i=0;i<markers;i++) {
		float angle = i*(360.f/markers);
		DrawRing(0,0,radius+30,radius+12,radius/radius,angle+markStart,angle+markEnd);
	}

	// Draw the baseline
	SetLineColour(colour[3],1.f,2);
	DrawLine(deltax,deltay,-deltax,-deltay);

	// Draw the connection line
	if (org->x != posx || org->y != posy) {
		double angle = atan2(double(org->y-posy),double(posx-org->x)) + curAngle*deg2rad;
		int fx = int(cos(angle)*oRadius);
		int fy = -int(sin(angle)*oRadius);
		DrawLine(0,0,fx,fy);
		int mdx = int(cos(curAngle*deg2rad)*20);
		int mdy = int(-sin(curAngle*deg2rad)*20);
		DrawLine(fx-mdx,fy-mdy,fx+mdx,fy+mdy);
	}

	// Draw the rotation line
	SetLineColour(colour[0],1.f,1);
	SetFillColour(colour[1],0.3f);
	DrawCircle(deltax,deltay,4);

	glPopMatrix();

	// Draw line to mouse
	if (!dragging && curFeature == features.end() && video.x > INT_MIN && video.y > INT_MIN) {
		SetLineColour(colour[0]);
		DrawLine(org->x,org->y,video.x,video.y);
	}
}

bool VisualToolRotateZ::InitializeHold() {
	startAngle = atan2(double(org->y-video.y),double(video.x-org->x)) * rad2deg;
	origAngle = curAngle;
	curDiag->StripTag(L"\\frz");
	curDiag->StripTag(L"\\fr");

	return true;
}

void VisualToolRotateZ::UpdateHold() {
	float screenAngle = atan2(double(org->y-video.y),double(video.x-org->x)) * rad2deg;
	curAngle = fmodf(screenAngle - startAngle + origAngle + 360.f, 360.f);

	// Oh Snap
	if (ctrlDown) {
		curAngle = floorf(curAngle/30.f+.5f)*30.f;
		if (curAngle > 359.f) curAngle = 0.f;
	}
}

void VisualToolRotateZ::CommitHold() {
	Selection sel = grid->GetSelectedSet();
	for (Selection::const_iterator cur = sel.begin(); cur != sel.end(); ++cur) {
		SetOverride(*cur, L"\\frz",wxString::Format(L"(%0.3g)",curAngle));
	}
}

void VisualToolRotateZ::CommitDrag(feature_iterator feature) {
	int x = feature->x;
	int y = feature->y;
	parent->ToScriptCoords(&x, &y);
	SetOverride(curDiag, L"\\org",wxString::Format(L"(%i,%i)",x,y));
}

void VisualToolRotateZ::DoRefresh() {
	if (!curDiag) return;
	GetLinePosition(curDiag, posx, posy, org->x, org->y);
	GetLineRotation(curDiag, rx, ry, curAngle);
	GetLineScale(curDiag, scaleX, scaleY);
}
