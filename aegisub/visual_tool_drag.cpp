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

	// Draw arrows
	for (size_t i=0;i<features.size();i++) {
		if (features[i].brother[0] != NULL && features[i].type == DRAG_BIG_SQUARE) {
			// Get features
			VisualDraggableFeature *p1,*p2;
			p1 = &features[i];
			p2 = p1->brother[0];

			// See if the distance between them is at least 30 pixels
			int dx = p2->x - p1->x;
			int dy = p2->y - p1->y;
			int dist = (int)sqrt(double(dx*dx + dy*dy));
			if (dist < 30) continue;

			// Get end points
			int x1 = p1->x + dx*10/dist;
			int x2 = p2->x - dx*20/dist;
			int y1 = p1->y + dy*10/dist;
			int y2 = p2->y - dy*20/dist;

			// Draw line
			SetLineColour(colour[3],0.8f,2);
			DrawLine(x1,y1,x2,y2);

			// Draw arrow
			double angle = atan2(double(y2-y1),double(x2-x1))+1.570796;
			int sx = int(cos(angle)*4);
			int sy = int(-sin(angle)*4);
			DrawLine(x2+sx,y2-sy,x2-sx,y2+sy);
			DrawLine(x2+sx,y2-sy,x2+dx*10/dist,y2+dy*10/dist);
			DrawLine(x2-sx,y2+sy,x2+dx*10/dist,y2+dy*10/dist);
		}
	}
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
				int x1,x2,y1,y2;
				int t1=0;
				int t2=diag->End.GetMS()-diag->Start.GetMS();
				int torgx,torgy;
				bool hasMove;
				GetLinePosition(diag,x1,y1,torgx,torgy);
				GetLineMove(diag,hasMove,x1,y1,x2,y2,t1,t2);

				// Create \pos feature
				VisualDraggableFeature feat;
				feat.x = x1;
				feat.y = y1;
				feat.layer = 0;
				feat.type = DRAG_BIG_SQUARE;
				feat.value = t1;
				feat.line = diag;
				feat.lineN = i;
				features.push_back(feat);

				// Create move destination feature
				if (hasMove) {
					feat.x = x2;
					feat.y = y2;
					feat.layer = 1;
					feat.type = DRAG_BIG_CIRCLE;
					feat.value = t2;
					feat.line = diag;
					feat.lineN = i;

					// Add each other as brothers. Yes, this looks weird.
					feat.brother[0] = &features.back();
					features.push_back(feat);
					feat.brother[0]->brother[0] = &features.back();
				}
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
	// Update "value" to reflect the time of the frame in which the feature is being dragged
	int frame_n = VideoContext::Get()->GetFrameN();
	int time = VFR_Output.GetTimeAtFrame(frame_n,true,true);
	feature.value = MID(0,time - feature.line->Start.GetMS(),feature.line->End.GetMS()-feature.line->Start.GetMS());
}


///////////////
// Commit drag
void VisualToolDrag::CommitDrag(VisualDraggableFeature &feature) {
	// Position
	if (feature.brother[0] == NULL) {
		SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),feature.x,feature.y));
	}

	// Move
	else {
		// Get source on p1 and dest on p2
		VisualDraggableFeature *p1,*p2;
		p1 = &feature;
		if (p1->type == DRAG_BIG_CIRCLE) p1 = p1->brother[0];
		p2 = p1->brother[0];

		// Set override
		SetOverride(_T("\\move"),wxString::Format(_T("(%i,%i,%i,%i,%i,%i)"),p1->x,p1->y,p2->x,p2->y,p1->value,p2->value));
	}
}
