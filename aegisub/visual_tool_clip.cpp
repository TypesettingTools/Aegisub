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


#pragma once


///////////
// Headers
#include "visual_tool_clip.h"
#include "subs_grid.h"
#include "subs_edit_box.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "utils.h"


///////////////
// Constructor
VisualToolClip::VisualToolClip(VideoDisplay *_parent)
: VisualTool(_parent)
{
	_parent->ShowCursor(false);
}


//////////
// Update
void VisualToolClip::Update() {
	// Render parent
	GetParent()->Render();
}


////////
// Draw
void VisualToolClip::Draw() {
	// Get position
	int dx1 = curX1;
	int dy1 = curY1;
	int dx2 = curX2;
	int dy2 = curY2;

	// Draw rectangle
	SetLineColour(colour[3]);
	SetFillColour(colour[3],0.0f);
	DrawRectangle(dx1,dy1,dx2,dy2);

	// Draw outside area
	SetLineColour(colour[3],0.0f);
	SetFillColour(colour[3],0.3f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	DrawRectangle(0,0,sw,dy1);
	DrawRectangle(0,dy2,sw,sh);
	DrawRectangle(0,dy1,dx1,dy2);
	DrawRectangle(dx2,dy1,sw,dy2);
	glDisable(GL_BLEND);

	// Draw circles
	SetLineColour(colour[0]);
	SetFillColour(colour[1],0.5);
	DrawCircle(dx1,dy1,4);
	DrawCircle(dx2,dy1,4);
	DrawCircle(dx2,dy2,4);
	DrawCircle(dx1,dy2,4);
}


/////////////////
// Start holding
void VisualToolClip::InitializeHold() {
	startX = mouseX;
	startY = mouseY;
	curDiag->StripTag(_T("\\clip"));
}


///////////////
// Update hold
void VisualToolClip::UpdateHold() {
	// Coordinates
	curX1 = startX * sw / w;
	curY1 = startY * sh / h;
	curX2 = mouseX * sw / w;
	curY2 = mouseY * sh / h;
	if (curX1 > curX2) IntSwap(curX1,curX2);
	if (curY1 > curY2) IntSwap(curY1,curY2);
}


///////////////
// Commit hold
void VisualToolClip::CommitHold() {
	VideoContext::Get()->grid->editBox->SetOverride(_T("\\clip"),wxString::Format(_T("(%i,%i,%i,%i)"),curX1,curY1,curX2,curY2),0,false);
}
