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

/// @file visual_tool_rotatexy.cpp
/// @brief 3D rotation in X/Y axes visual typesetting tool
/// @ingroup visual_ts

#include "config.h"

#ifndef AGI_PRE
#include <math.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"
#include "video_display.h"
#include "visual_tool_rotatexy.h"

VisualToolRotateXY::VisualToolRotateXY(VideoDisplay *parent, agi::Context *context, VideoState const& video, wxToolBar *)
: VisualTool<VisualDraggableFeature>(parent, context, video)
{
	features.resize(1);
	org = &features.back();
	org->type = DRAG_BIG_TRIANGLE;
	DoRefresh();
}

void VisualToolRotateXY::Draw() {
	if (!curDiag) return;

	// Pivot coordinates
	int dx=0,dy=0;
	if (dragging) GetLinePosition(curDiag,dx,dy);
	else GetLinePosition(curDiag,dx,dy,org->x,org->y);
	dx = org->x;
	dy = org->y;

	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.3f);

	// Draw pivot
	DrawAllFeatures();

	// Transform grid
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(dx,dy,0.f);
	float matrix[16] = { 2500, 0, 0, 0, 0, 2500, 0, 0, 0, 0, 1, 1, 0, 0, 2500, 2500 };
	glMultMatrixf(matrix);
	glScalef(1.f,1.f,8.f);
	if (curAngleY != 0.f) glRotatef(curAngleY,0.f,-1.f,0.f);
	if (curAngleX != 0.f) glRotatef(curAngleX,-1.f,0.f,0.f);
	if (curAngleZ != 0.f) glRotatef(curAngleZ,0.f,0.f,-1.f);

	// Draw grid
	glShadeModel(GL_SMOOTH);
	SetLineColour(colour[0],0.5f,2);
	SetModeLine();
	float r = colour[0].Red()/255.f;
	float g = colour[0].Green()/255.f;
	float b = colour[0].Blue()/255.f;
	glBegin(GL_LINES);
	for (int i=0;i<11;i++) {
		float a = 1.f - abs(i-5)*0.18f;
		int pos = 20*(i-5);
		glColor4f(r,g,b,0.f);
		glVertex2i(pos,120);
		glColor4f(r,g,b,a);
		glVertex2i(pos,0);
		glVertex2i(pos,0);
		glColor4f(r,g,b,0.f);
		glVertex2i(pos,-120);
		glVertex2i(120,pos);
		glColor4f(r,g,b,a);
		glVertex2i(0,pos);
		glVertex2i(0,pos);
		glColor4f(r,g,b,0.f);
		glVertex2i(-120,pos);
	}
	glEnd();

	// Draw vectors
	SetLineColour(colour[3],1.f,2);
	SetModeLine();
	glBegin(GL_LINES);
		glVertex3f(0.f,0.f,0.f);
		glVertex3f(50.f,0.f,0.f);
		glVertex3f(0.f,0.f,0.f);
		glVertex3f(0.f,50.f,0.f);
		glVertex3f(0.f,0.f,0.f);
		glVertex3f(0.f,0.f,50.f);
	glEnd();

	// Draw arrow tops
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(60.f,0.f,0.f);
		glVertex3f(50.f,-3.f,-3.f);
		glVertex3f(50.f,3.f,-3.f);
		glVertex3f(50.f,3.f,3.f);
		glVertex3f(50.f,-3.f,3.f);
		glVertex3f(50.f,-3.f,-3.f);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0.f,60.f,0.f);
		glVertex3f(-3.f,50.f,-3.f);
		glVertex3f(3.f,50.f,-3.f);
		glVertex3f(3.f,50.f,3.f);
		glVertex3f(-3.f,50.f,3.f);
		glVertex3f(-3.f,50.f,-3.f);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0.f,0.f,60.f);
		glVertex3f(-3.f,-3.f,50.f);
		glVertex3f(3.f,-3.f,50.f);
		glVertex3f(3.f,3.f,50.f);
		glVertex3f(-3.f,3.f,50.f);
		glVertex3f(-3.f,-3.f,50.f);
	glEnd();

	glPopMatrix();
	glShadeModel(GL_FLAT);
}

bool VisualToolRotateXY::InitializeHold() {
	startAngleX = (org->y-video.y)*2.f;
	startAngleY = (video.x-org->x)*2.f;
	origAngleX = curAngleX;
	origAngleY = curAngleY;

	return true;
}

void VisualToolRotateXY::UpdateHold() {
	float screenAngleX = (org->y-video.y)*2.f;
	float screenAngleY = (video.x-org->x)*2.f;

	// Deltas
	float deltaX = screenAngleX - startAngleX;
	float deltaY = screenAngleY - startAngleY;
	if (shiftDown) {
		if (fabs(deltaX) >= fabs(deltaY)) deltaY = 0.f;
		else deltaX = 0.f;
	}

	// Calculate
	curAngleX = fmodf(deltaX + origAngleX + 360.f, 360.f);
	curAngleY = fmodf(deltaY + origAngleY + 360.f, 360.f);

	// Oh Snap
	if (ctrlDown) {
		curAngleX = floorf(curAngleX/30.f+.5f)*30.f;
		curAngleY = floorf(curAngleY/30.f+.5f)*30.f;
		if (curAngleX > 359.f) curAngleX = 0.f;
		if (curAngleY > 359.f) curAngleY = 0.f;
	}
}

void VisualToolRotateXY::CommitHold() {
	Selection sel = grid->GetSelectedSet();
	for (Selection::const_iterator cur = sel.begin(); cur != sel.end(); ++cur) {
		SetOverride(*cur, L"\\frx",wxString::Format(L"(%0.3g)",curAngleX));
		SetOverride(*cur, L"\\fry",wxString::Format(L"(%0.3g)",curAngleY));
	}
}

void VisualToolRotateXY::CommitDrag(feature_iterator feature) {
	int x = feature->x;
	int y = feature->y;
	parent->ToScriptCoords(&x, &y);
	SetOverride(curDiag, L"\\org",wxString::Format(L"(%i,%i)",x,y));
}

void VisualToolRotateXY::DoRefresh() {
	if (!curDiag) return;
	int posx, posy;
	GetLinePosition(curDiag,posx,posy,org->x,org->y);
	GetLineRotation(curDiag,curAngleX,curAngleY,curAngleZ);
}
