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

/// @file visual_tool_cross.cpp
/// @brief Crosshair double-click-to-position visual typesetting tool
/// @ingroup visual_ts
///


///////////
// Headers
#include "config.h"

#include "visual_tool_cross.h"
#include "gl_text.h"
#include "subs_grid.h"
#include "subs_edit_box.h"
#include "ass_file.h"



/// @brief Constructor 
/// @param _parent 
///
VisualToolCross::VisualToolCross(VideoDisplay *_parent)
: VisualTool(_parent)
{
	_parent->ShowCursor(false);
}



/// @brief Destructor 
///
VisualToolCross::~VisualToolCross() {
	GetParent()->ShowCursor(true);
}



/// @brief Update 
///
void VisualToolCross::Update() {
	// Position
	if (leftDClick) {
		int vx = (sw * mouseX + w/2) / w;
		int vy = (sh * mouseY + h/2) / h;
		SubtitlesGrid *grid = VideoContext::Get()->grid;
		SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),vx,vy));
		grid->editBox->CommitText();
		grid->ass->FlagAsModified(_("positioning"));
		grid->CommitChanges(false,true);
	}

	// Render parent
	GetParent()->Render();
}



/// @brief Draw 
///
void VisualToolCross::Draw() {
	// Is it outside?
	if (mouseX == -1 || mouseY == -1) return;

	// Draw cross
	glDisable(GL_LINE_SMOOTH);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_INVERT);
	glLineWidth(1);
	glBegin(GL_LINES);
		glColor3f(1.0f,1.0f,1.0f);
		glVertex2f(0,my);
		glVertex2f(sw,my);
		glVertex2f(mx,0);
		glVertex2f(mx,sh);
	glEnd();
	glDisable(GL_COLOR_LOGIC_OP);

	// Switch display
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f,w,h,0.0f,-1000.0f,1000.0f);
	glMatrixMode(GL_MODELVIEW);

	// Text of current coords
	int dx = mouseX;
	int dy = mouseY;
	int vx = (sw * dx + w/2) / w;
	int vy = (sh * dy + h/2) / h;
	wxString mouseText;
	if (!wxGetMouseState().ShiftDown()) mouseText = wxString::Format(_T("%i,%i"),vx,vy);
	else mouseText = wxString::Format(_T("%i,%i"),vx - sw,vy - sh);

	// Setup gl text
	int tw,th;
	OpenGLText::SetFont(_T("Verdana"),12,true);
	OpenGLText::SetColour(wxColour(255,255,255));
	OpenGLText::GetExtent(mouseText,tw,th);

	// Calculate draw position
	bool left = dx > w/2;
	bool bottom = dy < h/2;

	// Text draw coords
	if (left) dx -= tw + 4;
	else dx += 4;
	if (bottom) dy += 3;
	else dy -= th + 3;

	// Draw text
	OpenGLText::Print(mouseText,dx,dy);

	// Restore matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


