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
#include "visual_tool_vector_clip.h"
#include "ass_dialogue.h"


///////
// IDs
enum {
	BUTTON_DRAG = VISUAL_SUB_TOOL_START,
	BUTTON_LINE,
	BUTTON_BICUBIC,
	BUTTON_CONVERT,
	BUTTON_INSERT,
	BUTTON_REMOVE,
	BUTTON_FREEHAND,
	BUTTON_FREEHAND_SMOOTH,
	BUTTON_LAST		// Leave this at the end and don't use it
};


///////////////
// Constructor
VisualToolVectorClip::VisualToolVectorClip(VideoDisplay *parent,wxToolBar *_toolBar)
: VisualTool(parent)
{
	DoRefresh();
	mode = 0;

	// Create toolbar
	toolBar = _toolBar;
	toolBar->AddTool(BUTTON_DRAG,_("Drag"),wxBITMAP(visual_vector_clip_drag),_("Drag control points."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_LINE,_("Line"),wxBITMAP(visual_vector_clip_line),_("Appends a line."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_BICUBIC,_("Bicubic"),wxBITMAP(visual_vector_clip_bicubic),_("Appends a bezier bicubic curve."),wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_CONVERT,_("Convert"),wxBITMAP(visual_vector_clip_convert),_("Converts a segment between line and bicubic."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_INSERT,_("Insert"),wxBITMAP(visual_vector_clip_insert),_("Inserts a control point."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_REMOVE,_("Remove"),wxBITMAP(visual_vector_clip_remove),_("Removes a control point."),wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_FREEHAND,_("Freehand"),wxBITMAP(visual_vector_clip_freehand),_("Draws a freehand shape."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_FREEHAND_SMOOTH,_("Freehand smooth"),wxBITMAP(visual_vector_clip_freehand_smooth),_("Draws a smoothed freehand shape."),wxITEM_CHECK);
	toolBar->ToggleTool(BUTTON_DRAG,true);
	toolBar->Realize();
	toolBar->Show(true);

	// Set default mode
	PopulateFeatureList();
	if (features.size() == 0) SetMode(1);
}


////////////////////
// Sub-tool pressed
void VisualToolVectorClip::OnSubTool(wxCommandEvent &event) {
	SetMode(event.GetId() - BUTTON_DRAG);
}


////////////
// Set mode
void VisualToolVectorClip::SetMode(int _mode) {
	// Make sure clicked is checked and everything else isn't. (Yes, this is radio behavior, but the separators won't let me use it)
	for (int i=BUTTON_DRAG;i<BUTTON_LAST;i++) {
		toolBar->ToggleTool(i,i == _mode + BUTTON_DRAG);
	}
	mode = _mode;
}


//////////
// Update
void VisualToolVectorClip::Update() {
	GetParent()->Render();
}


////////
// Draw
void VisualToolVectorClip::Draw() {
	// Get line
	AssDialogue *line = GetActiveDialogueLine();
	if (!line) return;

	// Parse vector
	std::vector<Vector2D> points;
	std::vector<int> pointCurve;
	spline.GetPointList(points,pointCurve);

	// Draw stencil mask
	glEnable(GL_STENCIL_TEST);
	glColorMask(0,0,0,0);
	glStencilFunc(GL_NEVER,1,1);
	glStencilOp(GL_INVERT,GL_INVERT,GL_INVERT);
	for (size_t i=2;i<points.size();i++) {
		glBegin(GL_TRIANGLES);
			glVertex2f(points[0].x,points[0].y);
			glVertex2f(points[i-1].x,points[i-1].y);
			glVertex2f(points[i].x,points[i].y);
		glEnd();
	}

	// Invert stencil
	if (inverse) {
		DrawRectangle(0,0,sw,sh);
	}

	// Draw "outside clip" mask
	glColorMask(1,1,1,1);
	glStencilFunc(GL_EQUAL, 0, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	SetLineColour(colour[3],0.0f);
	SetFillColour(wxColour(0,0,0),0.5f);
	DrawRectangle(0,0,sw,sh);
	glDisable(GL_STENCIL_TEST);

	// Get current position information for modes 3 and 4
	Vector2D pt;
	int highCurve = -1;
	if (mode == 3 || mode == 4) {
		float t;
		spline.GetClosestParametricPoint(Vector2D(mx,my),highCurve,t,pt);
	}

	// Draw lines
	SetFillColour(colour[3],0.0f);
	SetLineColour(colour[3],1.0f,2);
	int col = 3;
	for (size_t i=1;i<points.size();i++) {
		int useCol = pointCurve[i] == highCurve && curFeature == -1 ? 2 : 3;
		if (col != useCol) {
			col = useCol;
			SetLineColour(colour[col],1.0f,2);
		}
		DrawLine(points[i-1].x,points[i-1].y,points[i].x,points[i].y);
	}

	// Draw lines connecting the bicubic features
	SetLineColour(colour[3],0.9f,1);
	for (std::list<SplineCurve>::iterator cur=spline.curves.begin();cur!=spline.curves.end();cur++) {
		if (cur->type == CURVE_BICUBIC) {
			DrawDashedLine(cur->p1.x,cur->p1.y,cur->p2.x,cur->p2.y,6);
			DrawDashedLine(cur->p3.x,cur->p3.y,cur->p4.x,cur->p4.y,6);
		}
	}

	// Draw features
	DrawAllFeatures();

	// Draw preview of inserted line
	if (mode == 1 || mode == 2) {
		if (spline.curves.size()) {
			if (mx != -1 || my != -1) {
				SplineCurve *c0 = &spline.curves.front();
				SplineCurve *c1 = &spline.curves.back();
				DrawDashedLine(mx,my,c0->p1.x,c0->p1.y,6);
				DrawDashedLine(mx,my,c1->GetEndPoint().x,c1->GetEndPoint().y,6);
			}
		}
	}
	
	// Draw preview of insert point
	if (mode == 4) DrawCircle(pt.x,pt.y,4);
}


/////////////////////////
// Populate feature list
void VisualToolVectorClip::PopulateFeatureList() {
	// Clear
	features.clear();
	VisualDraggableFeature feat;
	
	// Go through each curve
	bool isFirst = true;
	int i = 0;
	for (std::list<SplineCurve>::iterator cur=spline.curves.begin();cur!=spline.curves.end();cur++,i++) {
		// First point
		if (isFirst) {
			isFirst = false;
			feat.x = (int)cur->p1.x;
			feat.y = (int)cur->p1.y;
			feat.type = DRAG_SMALL_CIRCLE;
			feat.value = i;
			feat.value2 = 0;
			features.push_back(feat);
		}

		// Line
		if (cur->type == CURVE_LINE) {
			feat.x = (int)cur->p2.x;
			feat.y = (int)cur->p2.y;
			feat.type = DRAG_SMALL_CIRCLE;
			feat.value = i;
			feat.value2 = 1;
			features.push_back(feat);
		}

		// Bicubic
		if (cur->type == CURVE_BICUBIC) {
			// Current size
			int size = features.size();

			// Control points
			feat.x = (int)cur->p2.x;
			feat.y = (int)cur->p2.y;
			feat.value = i;
			feat.value2 = 1;
			feat.brother[0] = size-1;
			feat.type = DRAG_SMALL_SQUARE;
			features.push_back(feat);
			feat.x = (int)cur->p3.x;
			feat.y = (int)cur->p3.y;
			feat.value2 = 2;
			feat.brother[0] = size+2;
			features.push_back(feat);

			// End point
			feat.x = (int)cur->p4.x;
			feat.y = (int)cur->p4.y;
			feat.type = DRAG_SMALL_CIRCLE;
			feat.value2 = 3;
			features.push_back(feat);
		}
	}
}


/////////////
// Can drag?
bool VisualToolVectorClip::DragEnabled() {
	return mode <= 4;
}


//////////
// Update
void VisualToolVectorClip::UpdateDrag(VisualDraggableFeature &feature) {
	spline.MovePoint(feature.value,feature.value2,wxPoint(feature.x,feature.y));
}


//////////
// Commit
void VisualToolVectorClip::CommitDrag(VisualDraggableFeature &feature) {
	if (inverse)
		SetOverride(_T("\\iclip"),_T("(") + spline.EncodeToASS() + _T(")"));
	else
		SetOverride(_T("\\clip"),_T("(") + spline.EncodeToASS() + _T(")"));
}


/////////////////////
// Clicked a feature
void VisualToolVectorClip::ClickedFeature(VisualDraggableFeature &feature) {
	// Delete a control point
	if (mode == 5) {
		int i = 0;
		for (std::list<SplineCurve>::iterator cur=spline.curves.begin();cur!=spline.curves.end();i++,cur++) {
			if (i == feature.value) {
				// Update next
				if (i != 0 || feature.value2 != 0) {
					std::list<SplineCurve>::iterator next = cur;
					next++;
					if (next != spline.curves.end()) next->p1 = cur->p1;
				}

				// Erase and save changes
				spline.curves.erase(cur);
				if (inverse)
					SetOverride(_T("\\iclip"),_T("(") + spline.EncodeToASS() + _T(")"));
				else
					SetOverride(_T("\\clip"),_T("(") + spline.EncodeToASS() + _T(")"));
				curFeature = -1;
				Commit(true);
				return;
			}
		}
	}
}


/////////////
// Can hold?
bool VisualToolVectorClip::HoldEnabled() {
	return mode <= 4 || mode == 6 || mode == 7;
}


///////////////////
// Initialize hold
void VisualToolVectorClip::InitializeHold() {
	// Insert line/bicubic
	if (mode == 1 || mode == 2) {
		SplineCurve curve;

		// Set start position
		if (spline.curves.size()) {
			curve.p1 = spline.curves.back().GetEndPoint();
			if (mode == 1) curve.type = CURVE_LINE;
			else curve.type = CURVE_BICUBIC;

			// Remove point if that's all there is
			if (spline.curves.size()==1 && spline.curves.front().type == CURVE_POINT) spline.curves.clear();
		}
		
		// First point
		else {
			curve.p1 = Vector2D(mx,my);
			curve.type = CURVE_POINT;
		}

		// Insert
		spline.AppendCurve(curve);
	}

	// Convert and insert
	else if (mode == 3 || mode == 4) {
		// Get closest point
		Vector2D pt;
		int curve;
		float t;
		spline.GetClosestParametricPoint(Vector2D(mx,my),curve,t,pt);

		// Convert
		if (mode == 3) {
			SplineCurve *c1 = spline.GetCurve(curve);
			if (!c1) {
			}
			else {
				if (c1->type == CURVE_LINE) {
					c1->type = CURVE_BICUBIC;
					c1->p4 = c1->p2;
					c1->p2 = c1->p1 * 0.75 + c1->p4 * 0.25;
					c1->p3 = c1->p1 * 0.25 + c1->p4 * 0.75;
				}

				else if (c1->type == CURVE_BICUBIC) {
					c1->type = CURVE_LINE;
					c1->p2 = c1->p4;
				}
			}
		}

		// Insert
		else {
			// Check if there is at least one curve to split
			if (spline.curves.size() == 0) return;

			// Split the curve
			SplineCurve *c1 = spline.GetCurve(curve);
			SplineCurve c2;
			if (!c1) {
				SplineCurve ct;
				ct.type = CURVE_LINE;
				ct.p1 = spline.curves.back().GetEndPoint();
				ct.p2 = spline.curves.front().p1;
				ct.p2 = ct.p1*(1-t) + ct.p2*t;
				spline.AppendCurve(ct);
			}
			else {
				c1->Split(*c1,c2,t);
				spline.InsertCurve(c2,curve+1);
			}
		}

		// Commit
		if (inverse)
			SetOverride(_T("\\iclip"),_T("(") + spline.EncodeToASS() + _T(")"));
		else
			SetOverride(_T("\\clip"),_T("(") + spline.EncodeToASS() + _T(")"));
		Commit(true);
	}

	// Freehand
	else if (mode == 6 || mode == 7) {
		features.clear();
		spline.curves.clear();
		lastX = -100000;
		lastY = -100000;
	}
}


///////////////
// Update hold
void VisualToolVectorClip::UpdateHold() {
	// Insert line
	if (mode == 1) {
		spline.curves.back().p2 = Vector2D(mx,my);
	}

	// Insert bicubic
	if (mode == 2) {
		SplineCurve &curve = spline.curves.back();
		curve.p4 = Vector2D(mx,my);

		// Control points
		if (spline.curves.size() > 1) {
			std::list<SplineCurve>::reverse_iterator iter = spline.curves.rbegin();
			iter++;
			SplineCurve &c0 = *iter;
			Vector2D prevVector;
			float len = (curve.p4-curve.p1).Len();
			if (c0.type == CURVE_LINE) prevVector = c0.p2-c0.p1;
			else prevVector = c0.p4-c0.p3;
			curve.p2 = prevVector.Unit() * (0.25f*len) + curve.p1;
		}
		else curve.p2 = curve.p1 * 0.75 + curve.p4 * 0.25;
		curve.p3 = curve.p1 * 0.25 + curve.p4 * 0.75;
	}

	// Freehand
	if (mode == 6 || mode == 7) {
		if (lastX != -100000 && lastY != -100000) {
			// See if distance is enough
			Vector2D delta(lastX-mx,lastY-my);
			int len = (int)delta.Len();
			if (mode == 6 && len < 30) return;
			if (mode == 7 && len < 60) return;
		
			// Generate curve and add it
			SplineCurve curve;
			curve.type = CURVE_LINE;
			curve.p1 = Vector2D(lastX,lastY);
			curve.p2 = Vector2D(mx,my);
			spline.AppendCurve(curve);
		}
		lastX = mx;
		lastY = my;
	}
}


///////////////
// Commit hold
void VisualToolVectorClip::CommitHold() {
	// Smooth spline
	if (!holding && mode == 7) spline.Smooth();

	// Save it
	if (mode != 3 && mode != 4) {
		if (inverse)
			SetOverride(_T("\\iclip"),_T("(") + spline.EncodeToASS() + _T(")"));
		else
			SetOverride(_T("\\clip"),_T("(") + spline.EncodeToASS() + _T(")"));
	}

	// End freedraw
	if (!holding && (mode == 6 || mode == 7)) SetMode(0);
}


///////////
// Refresh
void VisualToolVectorClip::DoRefresh() {
	if (!dragging) {
		// Get line
		AssDialogue *line = GetActiveDialogueLine();
		if (!line) return;

		// Get clip vector
		wxString vect;
		int scale;
		vect = GetLineVectorClip(line,scale,inverse);
		//if (!vect.IsEmpty()) return;
		spline.DecodeFromASS(vect);
		PopulateFeatureList();
	}
}
