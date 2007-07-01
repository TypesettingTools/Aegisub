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
#include "visual_tool_drag.h"
#include "subs_grid.h"
#include "subs_edit_box.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "utils.h"
#include "vfr.h"


///////////////
// Constructor
VisualToolDrag::VisualToolDrag(VideoDisplay *_parent)
: VisualTool(_parent)
{
	_parent->ShowCursor(false);
}


//////////
// Update
void VisualToolDrag::Update() {
	// Render parent
	GetParent()->Render();
}


////////
// Draw
void VisualToolDrag::Draw() {
	DrawAllFeatures();
}


/////////////////
// Populate list
void VisualToolDrag::PopulateFeatureList() {
	// Clear features
	features.clear();

	// Get video data
	int numRows = VideoContext::Get()->grid->GetRows();
	int framen = VideoContext::Get()->GetFrameN();

	// For each line
	AssDialogue *diag;
	for (int i=numRows;--i>=0;) {
		diag = VideoContext::Get()->grid->GetDialogue(i);
		if (diag) {
			// Line visible?
			int f1 = VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true);
			int f2 = VFR_Output.GetFrameAtTime(diag->End.GetMS(),false);
			if (f1 <= framen && f2 >= framen) {
				// Get position
				int lineX,lineY;
				int torgx,torgy;
				GetLinePosition(diag,lineX,lineY,torgx,torgy);

				// Create \pos feature
				VisualDraggableFeature feat;
				feat.x = lineX;
				feat.y = lineY;
				feat.layer = 0;
				feat.type = DRAG_BIG_SQUARE;
				feat.value = 0;
				feat.line = diag;
				feat.lineN = i;
				features.push_back(feat);
			}
		}
	}
}


//////////////////
// Start dragging
void VisualToolDrag::InitializeDrag(VisualDraggableFeature &feature) {
}


///////////////
// Update drag
void VisualToolDrag::UpdateDrag(VisualDraggableFeature &feature) {
}


///////////////
// Commit drag
void VisualToolDrag::CommitDrag(VisualDraggableFeature &feature) {
	SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),feature.x,feature.y));
}
