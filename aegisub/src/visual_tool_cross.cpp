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

///////////
// Headers
#include "config.h"

#include "ass_file.h"
#include "gl_text.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "video_context.h"
#include "video_display.h"
#include "visual_tool_cross.h"

/// @brief Constructor 
/// @param _parent 
VisualToolCross::VisualToolCross(VideoDisplay *parent, VideoState const& video, wxToolBar *)
: VisualTool(parent, video)
{
}
VisualToolCross::~VisualToolCross() { }

/// @brief Update 
void VisualToolCross::Update() {
	// Position
	if (leftDClick) {
		int vx = video.x;
		int vy = video.y;
		parent->ToScriptCoords(&vx, &vy);
		SubtitlesGrid *grid = VideoContext::Get()->grid;
		SetOverride(GetActiveDialogueLine(), L"\\pos",wxString::Format(L"(%i,%i)",vx,vy));
		grid->ass->FlagAsModified(_("positioning"));
		grid->CommitChanges(false,true);
		grid->editBox->Update(false, true, false);
	}
}

/// @brief Draw 
void VisualToolCross::Draw() {
	// Draw cross
	glDisable(GL_LINE_SMOOTH);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_INVERT);
	glLineWidth(1);
	glBegin(GL_LINES);
		glColor3f(1.0f,1.0f,1.0f);
		glVertex2f(0, video.y);
		glVertex2f(video.w, video.y);
		glVertex2f(video.x, 0);
		glVertex2f(video.x, video.h);
	glEnd();
	glDisable(GL_COLOR_LOGIC_OP);

	int tx,ty;
	if (!wxGetMouseState().ShiftDown()) {
		tx = video.x;
		ty = video.y;
	}
	else {
		tx = video.w - video.x;
		ty = video.h - video.y;
	}
	parent->ToScriptCoords(&tx, &ty);
	wxString mouseText = wxString::Format(L"%i,%i", tx, ty);

	int tw,th;
	OpenGLText::SetFont(L"Verdana", 12, true);
	OpenGLText::SetColour(wxColour(255, 255, 255));
	OpenGLText::GetExtent(mouseText, tw, th);

	// Calculate draw position
	int dx = video.x;
	int dy = video.y;
	bool left = dx > video.w / 2;
	bool bottom = dy < video.h / 2;

	// Text draw coords
	if (left) dx -= tw + 4;
	else dx += 4;
	if (bottom) dy += 3;
	else dy -= th + 3;

	// Draw text
	OpenGLText::Print(mouseText, dx, dy);
}
