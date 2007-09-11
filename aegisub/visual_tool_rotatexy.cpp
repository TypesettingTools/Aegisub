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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "visual_tool_rotatexy.h"
#include "subs_grid.h"
#include "subs_edit_box.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "utils.h"


///////////////
// Constructor
VisualToolRotateXY::VisualToolRotateXY(VideoDisplay *_parent)
: VisualTool(_parent)
{
	_parent->ShowCursor(false);
	DoRefresh();
}


//////////
// Update
void VisualToolRotateXY::Update() {
	// Render parent
	GetParent()->Render();
}


////////
// Draw
void VisualToolRotateXY::Draw() {
	// Get line to draw
	AssDialogue *line = GetActiveDialogueLine();
	if (!line) return;

	// Pivot coordinates
	int dx=0,dy=0;
	if (dragging) GetLinePosition(line,dx,dy);
	else GetLinePosition(line,dx,dy,orgx,orgy);
	dx = orgx;
	dy = orgy;

	// Rotation
	float rx,ry;
	GetLineRotation(line,rx,ry,rz);
	if (line == curDiag) {
		rx = curAngleX;
		ry = curAngleY;
	}

	// Set colours
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.3f);

	// Draw pivot
	DrawAllFeatures();
		
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

	// Draw grid
	glShadeModel(GL_SMOOTH);
	SetLineColour(colour[0],0.5f,2);
	SetModeLine();
	float r = colour[0].Red()/255.0f;
	float g = colour[0].Green()/255.0f;
	float b = colour[0].Blue()/255.0f;
	glBegin(GL_LINES);
	for (int i=0;i<11;i++) {
		float a = 1.0f - abs(i-5)*0.18f;
		int pos = 20*(i-5);
		glColor4f(r,g,b,0.0f);
		glVertex2i(pos,120);
		glColor4f(r,g,b,a);
		glVertex2i(pos,0);
		glVertex2i(pos,0);
		glColor4f(r,g,b,0.0f);
		glVertex2i(pos,-120);
		glVertex2i(120,pos);
		glColor4f(r,g,b,a);
		glVertex2i(0,pos);
		glVertex2i(0,pos);
		glColor4f(r,g,b,0.0f);
		glVertex2i(-120,pos);
	}
	glEnd();

	// Draw vectors
	SetLineColour(colour[3],1.0f,2);
	SetModeLine();
	glBegin(GL_LINES);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(50.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,50.0f,0.0f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,50.0f);
	glEnd();

	// Draw arrow tops
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(60.0f,0.0f,0.0f);
		glVertex3f(50.0f,-3.0f,-3.0f);
		glVertex3f(50.0f,3.0f,-3.0f);
		glVertex3f(50.0f,3.0f,3.0f);
		glVertex3f(50.0f,-3.0f,3.0f);
		glVertex3f(50.0f,-3.0f,-3.0f);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0.0f,60.0f,0.0f);
		glVertex3f(-3.0f,50.0f,-3.0f);
		glVertex3f(3.0f,50.0f,-3.0f);
		glVertex3f(3.0f,50.0f,3.0f);
		glVertex3f(-3.0f,50.0f,3.0f);
		glVertex3f(-3.0f,50.0f,-3.0f);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0.0f,0.0f,60.0f);
		glVertex3f(-3.0f,-3.0f,50.0f);
		glVertex3f(3.0f,-3.0f,50.0f);
		glVertex3f(3.0f,3.0f,50.0f);
		glVertex3f(-3.0f,3.0f,50.0f);
		glVertex3f(-3.0f,-3.0f,50.0f);
	glEnd();

	// Restore gl's state
	glPopMatrix();
	glShadeModel(GL_FLAT);
}


/////////////////
// Start holding
void VisualToolRotateXY::InitializeHold() {
	GetLinePosition(curDiag,odx,ody,orgx,orgy);
	GetLineRotation(curDiag,origAngleX,origAngleY,rz);
	startAngleX = (orgy-mouseY*sh/h)*2.0;
	startAngleY = (mouseX*sw/w-orgx)*2.0;
	curAngleX = origAngleX;
	curAngleY = origAngleY;
	curDiag->StripTag(_T("\\frx"));
	curDiag->StripTag(_T("\\fry"));
}


///////////////
// Update hold
void VisualToolRotateXY::UpdateHold() {
	// Find screen angles
	float screenAngleX = (orgy-mouseY*sh/h)*2.0;
	float screenAngleY = (mouseX*sw/w-orgx)*2.0;

	// Deltas
	float deltaX = screenAngleX - startAngleX;
	float deltaY = screenAngleY - startAngleY;
	if (ctrlDown) {
		if (fabs(deltaX) >= fabs(deltaY)) deltaY = 0;
		else deltaX = 0;
	}

	// Calculate
	curAngleX = deltaX + origAngleX;
	curAngleY = deltaY + origAngleY;
	while (curAngleX < 0.0) curAngleX += 360.0;
	while (curAngleX >= 360.0) curAngleX -= 360.0;
	while (curAngleY < 0.0) curAngleY += 360.0;
	while (curAngleY >= 360.0) curAngleY -= 360.0;

	// Oh Snap
	if (shiftDown) {
		curAngleX = (float)((int)((curAngleX+15.0f)/30.0f))*30.0f;
		curAngleY = (float)((int)((curAngleY+15.0f)/30.0f))*30.0f;
		if (curAngleX == 360.0f) curAngleX = 0.0f;
		if (curAngleY == 360.0f) curAngleX = 0.0f;
	}
}


///////////////
// Commit hold
void VisualToolRotateXY::CommitHold() {
	SetOverride(_T("\\frx"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngleX)));
	SetOverride(_T("\\fry"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngleY)));
}


//////////////////
// Get \org pivot
void VisualToolRotateXY::PopulateFeatureList() {
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


///////////////////////////
// Update dragging of \org
void VisualToolRotateXY::UpdateDrag(VisualDraggableFeature &feature) {
	orgx = feature.x;
	orgy = feature.y;
}


///////////////////////////
// Commit dragging of \org
void VisualToolRotateXY::CommitDrag(VisualDraggableFeature &feature) {
	SetOverride(_T("\\org"),wxString::Format(_T("(%i,%i)"),feature.x,feature.y));
}


///////////
// Refresh
void VisualToolRotateXY::DoRefresh() {
	AssDialogue *line = GetActiveDialogueLine();
	GetLinePosition(line,odx,ody,orgx,orgy);
	GetLineRotation(line,curAngleX,curAngleY,rz);
}
