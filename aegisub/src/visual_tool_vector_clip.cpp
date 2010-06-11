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

/// @file visual_tool_vector_clip.cpp
/// @brief Vector clipping visual typesetting tool
/// @ingroup visual_ts

#ifndef AGI_PRE
#include <wx/toolbar.h>
#include "gl/glext.h"
#endif

#include "config.h"

#include "ass_dialogue.h"
#include "libresrc/libresrc.h"
#include "video_display.h"
#include "visual_tool_vector_clip.h"

enum {
	BUTTON_DRAG = VISUAL_SUB_TOOL_START,
	BUTTON_LINE,
	BUTTON_BICUBIC,
	BUTTON_CONVERT,
	BUTTON_INSERT,
	BUTTON_REMOVE,
	BUTTON_FREEHAND,
	BUTTON_FREEHAND_SMOOTH,
	BUTTON_LAST // Leave this at the end and don't use it
};

/// @brief Constructor 
/// @param parent   
/// @param _toolBar 
VisualToolVectorClip::VisualToolVectorClip(VideoDisplay *parent, VideoState const& video, wxToolBar * toolBar)
: VisualTool<VisualToolVectorClipDraggableFeature>(parent, video)
, spline(*parent)
, toolBar(toolBar)
{
	DoRefresh();
	mode = 0;

	// Create toolbar
	toolBar->AddTool(BUTTON_DRAG,_("Drag"),GETIMAGE(visual_vector_clip_drag_24),_("Drag control points."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_LINE,_("Line"),GETIMAGE(visual_vector_clip_line_24),_("Appends a line."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_BICUBIC,_("Bicubic"),GETIMAGE(visual_vector_clip_bicubic_24),_("Appends a bezier bicubic curve."),wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_CONVERT,_("Convert"),GETIMAGE(visual_vector_clip_convert_24),_("Converts a segment between line and bicubic."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_INSERT,_("Insert"),GETIMAGE(visual_vector_clip_insert_24),_("Inserts a control point."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_REMOVE,_("Remove"),GETIMAGE(visual_vector_clip_remove_24),_("Removes a control point."),wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_FREEHAND,_("Freehand"),GETIMAGE(visual_vector_clip_freehand_24),_("Draws a freehand shape."),wxITEM_CHECK);
	toolBar->AddTool(BUTTON_FREEHAND_SMOOTH,_("Freehand smooth"),GETIMAGE(visual_vector_clip_freehand_smooth_24),_("Draws a smoothed freehand shape."),wxITEM_CHECK);
	toolBar->ToggleTool(BUTTON_DRAG,true);
	toolBar->Realize();
	toolBar->Show(true);

	// Set default mode
	PopulateFeatureList();
	if (features.size() == 0) SetMode(1);
}

void VisualToolVectorClip::OnSubTool(wxCommandEvent &event) {
	SetMode(event.GetId() - BUTTON_DRAG);
}

void VisualToolVectorClip::SetMode(int newMode) {
	// Manually enforce radio behavior as we want one selection in the bar
	// rather than one per group
	for (int i=BUTTON_DRAG;i<BUTTON_LAST;i++) {
		toolBar->ToggleTool(i,i == newMode + BUTTON_DRAG);
	}
	mode = newMode;
}

// Substitute for glMultiDrawArrays for sub-1.4 OpenGL
static void APIENTRY glMultiDrawArraysFallback(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount) {
	for (int i = 0; i < primcount; ++i) {
		glDrawArrays(mode, *first++, *count++);
	}
}

void VisualToolVectorClip::Draw() {
	if (spline.empty()) return;

	GL_EXT(PFNGLMULTIDRAWARRAYSPROC, glMultiDrawArrays);

	// Get line
	AssDialogue *line = GetActiveDialogueLine();
	if (!line) return;

	// Parse vector
	std::vector<float> points;
	std::vector<int> start;
	std::vector<int> count;

	spline.GetPointList(points, start, count);
	assert(!start.empty());
	assert(!count.empty());
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &points[0]);

	// The following is nonzero winding-number PIP based on stencils

	// Draw to stencil only
	glEnable(GL_STENCIL_TEST);
	glColorMask(0, 0, 0, 0);

	// GL_INCR_WRAP was added in 1.4, so instead set the entire stencil to 128
	// and wobble from there
	glStencilFunc(GL_NEVER, 128, 0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	DrawRectangle(0,0,video.w,video.h);

	// Increment the winding number for each forward facing triangle
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	glEnable(GL_CULL_FACE);

	glCullFace(GL_BACK);
	glMultiDrawArrays(GL_TRIANGLE_FAN, &start[0], &count[0], start.size());

	// Decrement the winding number for each backfacing triangle
	glStencilOp(GL_DECR, GL_DECR, GL_DECR);
	glCullFace(GL_FRONT);
	glMultiDrawArrays(GL_TRIANGLE_FAN, &start[0], &count[0], start.size());
	glDisable(GL_CULL_FACE);

	// Draw the actual rectangle
	glColorMask(1,1,1,1);
	SetLineColour(colour[3],0.0f);
	SetFillColour(wxColour(0,0,0),0.5f);

	// VSFilter draws when the winding number is nonzero, so we want to draw the
	// mask when the winding number is zero (where 128 is zero due to the lack of
	// wrapping combined with unsigned numbers)
	glStencilFunc(inverse ? GL_NOTEQUAL : GL_EQUAL, 128, 0xFF);
	DrawRectangle(0,0,video.w,video.h);
	glDisable(GL_STENCIL_TEST);

	// Draw lines
	SetFillColour(colour[3],0.0f);
	SetLineColour(colour[3],1.0f,2);
	SetModeLine();
	glMultiDrawArrays(GL_LINE_LOOP, &start[0], &count[0], start.size());

	Vector2D pt;
	float t;
	Spline::iterator highCurve;
	spline.GetClosestParametricPoint(Vector2D(video.x, video.y), highCurve, t, pt);

	// Draw highlighted line
	if ((mode == 3 || mode == 4) && !curFeature && points.size() > 2) {
		std::vector<float> highPoints;
		spline.GetPointList(highPoints, highCurve);
		if (!highPoints.empty()) {
			glVertexPointer(2, GL_FLOAT, 0, &highPoints[0]);
			SetLineColour(colour[2], 1.f, 2);
			SetModeLine();
			glDrawArrays(GL_LINE_STRIP, 0, highPoints.size() / 2);
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	// Draw lines connecting the bicubic features
	SetLineColour(colour[3],0.9f,1);
	for (Spline::iterator cur=spline.begin();cur!=spline.end();cur++) {
		if (cur->type == CURVE_BICUBIC) {
			DrawDashedLine(cur->p1.x,cur->p1.y,cur->p2.x,cur->p2.y,6);
			DrawDashedLine(cur->p3.x,cur->p3.y,cur->p4.x,cur->p4.y,6);
		}
	}

	DrawAllFeatures();

	// Draw preview of inserted line
	if (mode == 1 || mode == 2) {
		if (spline.size() && video.x > INT_MIN && video.y > INT_MIN) {
			SplineCurve *c0 = &spline.front();
			SplineCurve *c1 = &spline.back();
			DrawDashedLine(video.x,video.y,c0->p1.x,c0->p1.y,6);
			DrawDashedLine(video.x,video.y,c1->EndPoint().x,c1->EndPoint().y,6);
		}
	}
	
	// Draw preview of insert point
	if (mode == 4) DrawCircle(pt.x,pt.y,4);
}

/// @brief Populate feature list 
void VisualToolVectorClip::PopulateFeatureList() {
	ClearSelection(false);
	features.clear();
	// This is perhaps a bit conservative as there can be up to 3N+1 features
	features.reserve(spline.size());
	VisualToolVectorClipDraggableFeature feat;
	
	// Go through each curve
	int j = 0;
	for (Spline::iterator cur=spline.begin();cur!=spline.end();cur++) {
		if (cur->type == CURVE_POINT) {
			feat.x = (int)cur->p1.x;
			feat.y = (int)cur->p1.y;
			feat.type = DRAG_SMALL_CIRCLE;
			feat.curve = cur;
			feat.point = 0;
			features.push_back(feat);
			AddSelection(j++);
		}

		else if (cur->type == CURVE_LINE) {
			feat.x = (int)cur->p2.x;
			feat.y = (int)cur->p2.y;
			feat.type = DRAG_SMALL_CIRCLE;
			feat.curve = cur;
			feat.point = 1;
			features.push_back(feat);
			AddSelection(j++);
		}

		else if (cur->type == CURVE_BICUBIC) {
			// Control points
			feat.x = (int)cur->p2.x;
			feat.y = (int)cur->p2.y;
			feat.curve = cur;
			feat.point = 1;
			feat.type = DRAG_SMALL_SQUARE;
			features.push_back(feat);
			feat.x = (int)cur->p3.x;
			feat.y = (int)cur->p3.y;
			feat.point = 2;
			features.push_back(feat);

			// End point
			feat.x = (int)cur->p4.x;
			feat.y = (int)cur->p4.y;
			feat.type = DRAG_SMALL_CIRCLE;
			feat.point = 3;
			features.push_back(feat);

			AddSelection(j++);
			AddSelection(j++);
			AddSelection(j++);
		}
	}
}

/// @brief Update 
/// @param feature 
void VisualToolVectorClip::UpdateDrag(VisualToolVectorClipDraggableFeature* feature) {
	spline.MovePoint(feature->curve,feature->point,Vector2D(feature->x,feature->y));
}

/// @brief Commit 
/// @param feature 
void VisualToolVectorClip::CommitDrag(VisualToolVectorClipDraggableFeature* feature) {
	SetOverride(GetActiveDialogueLine(), inverse ? L"\\iclip" : L"\\clip", L"(" + spline.EncodeToASS() + L")");
}

/// @brief Clicked a feature 
/// @param feature 
/// @return 
bool VisualToolVectorClip::InitializeDrag(VisualToolVectorClipDraggableFeature* feature) {
	// Delete a control point
	if (mode == 5) {
		// Update next
		Spline::iterator next = feature->curve;
		next++;
		if (next != spline.end()) {
			if (feature->curve->type == CURVE_POINT) {
				next->p1 = next->EndPoint();
				next->type = CURVE_POINT;
			}
			else {
				next->p1 = feature->curve->p1;
			}
		}

		// Erase and save changes
		spline.erase(feature->curve);
		CommitDrag(feature);
		PopulateFeatureList();
		curFeature = NULL;
		Commit(true);
		return false;
	}
	return true;
}

/// @brief Initialize hold 
/// @return 
bool VisualToolVectorClip::InitializeHold() {
	// Insert line/bicubic
	if (mode == 1 || mode == 2) {
		SplineCurve curve;

		// Set start position
		if (!spline.empty()) {
			curve.p1 = spline.back().EndPoint();
			if (mode == 1) curve.type = CURVE_LINE;
			else curve.type = CURVE_BICUBIC;
		}
		
		// First point
		else {
			curve.p1 = Vector2D(video.x,video.y);
			curve.type = CURVE_POINT;
		}

		// Insert
		spline.push_back(curve);
		UpdateHold();
		return true;
	}

	// Convert and insert
	if (mode == 3 || mode == 4) {
		// Get closest point
		Vector2D pt;
		Spline::iterator curve;
		float t;
		spline.GetClosestParametricPoint(Vector2D(video.x,video.y),curve,t,pt);

		// Convert
		if (mode == 3) {
			if (curve != spline.end()) {
				if (curve->type == CURVE_LINE) {
					curve->type = CURVE_BICUBIC;
					curve->p4 = curve->p2;
					curve->p2 = curve->p1 * 0.75 + curve->p4 * 0.25;
					curve->p3 = curve->p1 * 0.25 + curve->p4 * 0.75;
				}

				else if (curve->type == CURVE_BICUBIC) {
					curve->type = CURVE_LINE;
					curve->p2 = curve->p4;
				}
			}
		}

		// Insert
		else {
			// Check if there is at least one curve to split
			if (spline.empty()) return false;

			// Split the curve
			if (curve == spline.end()) {
				SplineCurve ct;
				ct.type = CURVE_LINE;
				ct.p1 = spline.back().EndPoint();
				ct.p2 = spline.front().p1;
				ct.p2 = ct.p1*(1-t) + ct.p2*t;
				spline.push_back(ct);
			}
			else {
				SplineCurve c2;
				curve->Split(*curve,c2,t);
				spline.insert(++curve, c2);
			}
		}

		// Commit
		SetOverride(GetActiveDialogueLine(), inverse ? L"\\iclip" : L"\\clip", L"(" + spline.EncodeToASS() + L")");
		Commit(true);
		DoRefresh();
		return false;
	}

	// Freehand
	if (mode == 6 || mode == 7) {
		ClearSelection(false);
		features.clear();
		spline.clear();
		SplineCurve curve;
		curve.type = CURVE_POINT;
		curve.p1.x = video.x;
		curve.p1.y = video.y;
		spline.push_back(curve);
		return true;
	}
	return false;
}

/// @brief Update hold 
void VisualToolVectorClip::UpdateHold() {
	// Insert line
	if (mode == 1) {
		spline.back().p2 = Vector2D(video.x,video.y);
	}

	// Insert bicubic
	if (mode == 2) {
		SplineCurve &curve = spline.back();
		curve.p4 = Vector2D(video.x,video.y);

		// Control points
		if (spline.size() > 1) {
			std::list<SplineCurve>::reverse_iterator iter = spline.rbegin();
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
		// See if distance is enough
		Vector2D const& last = spline.back().EndPoint();
		int len = (int)Vector2D(last.x-video.x, last.y-video.y).Len();
		if (mode == 6 && len < 30) return;
		if (mode == 7 && len < 60) return;

		// Generate curve and add it
		SplineCurve curve;
		curve.type = CURVE_LINE;
		curve.p1 = Vector2D(last.x,last.y);
		curve.p2 = Vector2D(video.x,video.y);
		spline.push_back(curve);
	}
}

/// @brief Commit hold 
void VisualToolVectorClip::CommitHold() {
	// Smooth spline
	if (!holding && mode == 7) spline.Smooth();

	// Save it
	if (mode != 3 && mode != 4) {
		SetOverride(curDiag, inverse ? L"\\iclip" : L"\\clip", L"(" + spline.EncodeToASS() + L")");
	}

	// End freedraw
	if (!holding && (mode == 6 || mode == 7)) SetMode(0);

	PopulateFeatureList();
}

/// @brief Refresh 
void VisualToolVectorClip::DoRefresh() {
	if (!dragging && !holding) {
		// Get line
		AssDialogue *line = GetActiveDialogueLine();
		if (!line) return;

		// Get clip vector
		wxString vect;
		int scale;
		vect = GetLineVectorClip(line,scale,inverse);
		spline.DecodeFromASS(vect);
		PopulateFeatureList();
	}
}
