// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

/// @file visual_tool_vector_clip.cpp
/// @brief Vector clipping visual typesetting tool
/// @ingroup visual_ts

#include "visual_tool_vector_clip.h"

#include "ass_dialogue.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "selection_controller.h"
#include "utils.h"

#include <libaegisub/util.h>

#include <algorithm>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <wx/toolbar.h>

/// Button IDs
enum {
	BUTTON_DRAG = 1300,
	BUTTON_LINE,
	BUTTON_BICUBIC,
	BUTTON_CONVERT,
	BUTTON_INSERT,
	BUTTON_REMOVE,
	BUTTON_FREEHAND,
	BUTTON_FREEHAND_SMOOTH,
	BUTTON_LAST // Leave this at the end and don't use it
};

VisualToolVectorClip::VisualToolVectorClip(VideoDisplay *parent, agi::Context *context)
: VisualTool<VisualToolVectorClipDraggableFeature>(parent, context)
, spline(*this)
{
}

void VisualToolVectorClip::SetToolbar(wxToolBar *toolBar) {
	this->toolBar = toolBar;

	int icon_size = OPT_GET("App/Toolbar Icon Size")->GetInt();

#define ICON(name) icon_size == 16 ? GETIMAGE(name ## _16) : GETIMAGE(name ## _24)
	toolBar->AddTool(BUTTON_DRAG, _("Drag"), ICON(visual_vector_clip_drag), _("Drag control points"), wxITEM_CHECK);
	toolBar->AddTool(BUTTON_LINE, _("Line"), ICON(visual_vector_clip_line), _("Appends a line"), wxITEM_CHECK);
	toolBar->AddTool(BUTTON_BICUBIC, _("Bicubic"), ICON(visual_vector_clip_bicubic), _("Appends a bezier bicubic curve"), wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_CONVERT, _("Convert"), ICON(visual_vector_clip_convert), _("Converts a segment between line and bicubic"), wxITEM_CHECK);
	toolBar->AddTool(BUTTON_INSERT, _("Insert"), ICON(visual_vector_clip_insert), _("Inserts a control point"), wxITEM_CHECK);
	toolBar->AddTool(BUTTON_REMOVE, _("Remove"), ICON(visual_vector_clip_remove), _("Removes a control point"), wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(BUTTON_FREEHAND, _("Freehand"), ICON(visual_vector_clip_freehand), _("Draws a freehand shape"), wxITEM_CHECK);
	toolBar->AddTool(BUTTON_FREEHAND_SMOOTH, _("Freehand smooth"), ICON(visual_vector_clip_freehand_smooth), _("Draws a smoothed freehand shape"), wxITEM_CHECK);
	toolBar->ToggleTool(BUTTON_DRAG, true);
	toolBar->Realize();
	toolBar->Show(true);
	toolBar->Bind(wxEVT_TOOL, [=](wxCommandEvent& e) { SetMode(e.GetId() - BUTTON_DRAG); });
	SetMode(features.empty());
#undef ICON
}

void VisualToolVectorClip::SetMode(int new_mode) {
	// Manually enforce radio behavior as we want one selection in the bar
	// rather than one per group
	for (int i = BUTTON_DRAG; i < BUTTON_LAST; i++)
		toolBar->ToggleTool(i, i == new_mode + BUTTON_DRAG);

	mode = new_mode;
}

void VisualToolVectorClip::Draw() {
	if (!active_line) return;
	if (spline.empty()) return;

	// Parse vector
	std::vector<float> points;
	std::vector<int> start;
	std::vector<int> count;

	spline.GetPointList(points, start, count);
	assert(!start.empty());
	assert(!count.empty());

	gl.SetLineColour(colour[3], .5f, 2);
	gl.SetFillColour(wxColour(0, 0, 0), 0.5f);

	// draw the shade over clipped out areas and line showing the clip
	gl.DrawMultiPolygon(points, start, count, video_pos, video_res, !inverse);

	if (mode == 0 && holding && drag_start && mouse_pos) {
		// Draw drag-select box
		Vector2D top_left = drag_start.Min(mouse_pos);
		Vector2D bottom_right = drag_start.Max(mouse_pos);
		gl.DrawDashedLine(top_left, Vector2D(top_left.X(), bottom_right.Y()), 6);
		gl.DrawDashedLine(Vector2D(top_left.X(), bottom_right.Y()), bottom_right, 6);
		gl.DrawDashedLine(bottom_right, Vector2D(bottom_right.X(), top_left.Y()), 6);
		gl.DrawDashedLine(Vector2D(bottom_right.X(), top_left.Y()), top_left, 6);
	}

	Vector2D pt;
	float t;
	Spline::iterator highlighted_curve;
	spline.GetClosestParametricPoint(mouse_pos, highlighted_curve, t, pt);

	// Draw highlighted line
	if ((mode == 3 || mode == 4) && !active_feature && points.size() > 2) {
		std::vector<float> highlighted_points;
		spline.GetPointList(highlighted_points, highlighted_curve);
		if (!highlighted_points.empty()) {
			gl.SetLineColour(colour[2], 1.f, 2);
			gl.DrawLineStrip(2, highlighted_points);
		}
	}

	// Draw lines connecting the bicubic features
	gl.SetLineColour(colour[3], 0.9f, 1);
	for (auto const& curve : spline) {
		if (curve.type == SplineCurve::BICUBIC) {
			gl.DrawDashedLine(curve.p1, curve.p2, 6);
			gl.DrawDashedLine(curve.p3, curve.p4, 6);
		}
	}

	// Draw features
	for (auto& feature : features) {
		int color = 3;
		if (&feature == active_feature)
			color = 1;
		else if (sel_features.count(&feature))
			color = 2;
		gl.SetFillColour(colour[color], .6f);

		if (feature.type == DRAG_SMALL_SQUARE) {
			gl.SetLineColour(colour[3], .5f, 1);
			gl.DrawRectangle(feature.pos - 3, feature.pos + 3);
		}
		else {
			gl.SetLineColour(colour[color], .5f, 1);
			gl.DrawCircle(feature.pos, 2.f);
		}
	}

	// Draw preview of inserted line
	if (mode == 1 || mode == 2) {
		if (spline.size() && mouse_pos) {
			auto c0 = std::find_if(spline.rbegin(), spline.rend(),
				[](SplineCurve const& s) { return s.type == SplineCurve::POINT; });
			SplineCurve *c1 = &spline.back();
			gl.DrawDashedLine(mouse_pos, c0->p1, 6);
			gl.DrawDashedLine(mouse_pos, c1->EndPoint(), 6);
		}
	}

	// Draw preview of insert point
	if (mode == 4)
		gl.DrawCircle(pt, 4);
}

void VisualToolVectorClip::MakeFeature(Spline::iterator cur) {
	auto feat = agi::util::make_unique<Feature>();
	feat->curve = cur;

	if (cur->type == SplineCurve::POINT) {
		feat->pos = cur->p1;
		feat->type = DRAG_SMALL_CIRCLE;
		feat->point = 0;
	}
	else if (cur->type == SplineCurve::LINE) {
		feat->pos = cur->p2;
		feat->type = DRAG_SMALL_CIRCLE;
		feat->point = 1;
	}
	else if (cur->type == SplineCurve::BICUBIC) {
		// Control points
		feat->pos = cur->p2;
		feat->point = 1;
		feat->type = DRAG_SMALL_SQUARE;
		features.push_back(*feat.release());

		feat = agi::util::make_unique<Feature>();
		feat->curve = cur;
		feat->pos = cur->p3;
		feat->point = 2;
		feat->type = DRAG_SMALL_SQUARE;
		features.push_back(*feat.release());

		// End point
		feat = agi::util::make_unique<Feature>();
		feat->curve = cur;
		feat->pos = cur->p4;
		feat->point = 3;
		feat->type = DRAG_SMALL_CIRCLE;
	}
	features.push_back(*feat.release());
}

void VisualToolVectorClip::MakeFeatures() {
	sel_features.clear();
	features.clear();
	active_feature = nullptr;
	for (auto it = spline.begin(); it != spline.end(); ++it)
		MakeFeature(it);
}

void VisualToolVectorClip::Save() {
	std::string value = "(";
	if (spline.GetScale() != 1)
		value += std::to_string(spline.GetScale()) + ",";
	value += spline.EncodeToAss() + ")";

	for (auto line : c->selectionController->GetSelectedSet()) {
		// This check is technically not correct as it could be outside of an
		// override block... but that's rather unlikely
		bool has_iclip = line->Text.get().find("\\iclip") != std::string::npos;
		SetOverride(line, has_iclip ? "\\iclip" : "\\clip", value);
	}
}

void VisualToolVectorClip::Commit(wxString message) {
	Save();
	VisualToolBase::Commit(message);
}

void VisualToolVectorClip::UpdateDrag(Feature *feature) {
	spline.MovePoint(feature->curve, feature->point, feature->pos);
}

bool VisualToolVectorClip::InitializeDrag(Feature *feature) {
	if (mode != 5) return true;

	if (feature->curve->type == SplineCurve::BICUBIC && (feature->point == 1 || feature->point == 2)) {
		// Deleting bicubic curve handles, so convert to line
		feature->curve->type = SplineCurve::LINE;
		feature->curve->p2 = feature->curve->p4;
	}
	else {
		auto next = std::next(feature->curve);
		if (next != spline.end()) {
			if (feature->curve->type == SplineCurve::POINT) {
				next->p1 = next->EndPoint();
				next->type = SplineCurve::POINT;
			}
			else {
				next->p1 = feature->curve->p1;
			}
		}

		spline.erase(feature->curve);
	}
	active_feature = nullptr;

	MakeFeatures();
	Commit(_("delete control point"));

	return false;
}

bool VisualToolVectorClip::InitializeHold() {
	// Box selection
	if (mode == 0) {
		box_added.clear();
		return true;
	}

	// Insert line/bicubic
	if (mode == 1 || mode == 2) {
		SplineCurve curve;

		// New spline beginning at the clicked point
		if (spline.empty()) {
			curve.p1 = mouse_pos;
			curve.type = SplineCurve::POINT;
		}
		else {
			// Continue from the spline in progress
			// Don't bother setting p2 as UpdateHold will handle that
			curve.p1 = spline.back().EndPoint();
			curve.type = mode == 1 ? SplineCurve::LINE : SplineCurve::BICUBIC;
		}

		spline.push_back(curve);
		sel_features.clear();
		MakeFeature(--spline.end());
		UpdateHold();
		return true;
	}

	// Convert and insert
	if (mode == 3 || mode == 4) {
		// Get closest point
		Vector2D pt;
		Spline::iterator curve;
		float t;
		spline.GetClosestParametricPoint(mouse_pos, curve, t, pt);

		// Convert line <-> bicubic
		if (mode == 3) {
			if (curve != spline.end()) {
				if (curve->type == SplineCurve::LINE) {
					curve->type = SplineCurve::BICUBIC;
					curve->p4 = curve->p2;
					curve->p2 = curve->p1 * 0.75 + curve->p4 * 0.25;
					curve->p3 = curve->p1 * 0.25 + curve->p4 * 0.75;
				}

				else if (curve->type == SplineCurve::BICUBIC) {
					curve->type = SplineCurve::LINE;
					curve->p2 = curve->p4;
				}
			}
		}
		// Insert
		else {
			if (spline.empty()) return false;

			// Split the curve
			if (curve == spline.end()) {
				SplineCurve ct(spline.back().EndPoint(), spline.front().p1);
				ct.p2 = ct.p1 * (1 - t) + ct.p2 * t;
				spline.push_back(ct);
			}
			else {
				std::pair<SplineCurve, SplineCurve> split = curve->Split(t);
				*curve = split.first;
				spline.insert(++curve, split.second);
			}
		}

		MakeFeatures();
		Commit();
		return false;
	}

	// Freehand spline draw
	if (mode == 6 || mode == 7) {
		sel_features.clear();
		features.clear();
		active_feature = nullptr;
		spline.clear();
		spline.emplace_back(mouse_pos);
		return true;
	}

	// Nothing to do for mode 5 (remove)
	return false;
}

static bool in_box(Vector2D top_left, Vector2D bottom_right, Vector2D p) {
	return p.X() >= top_left.X()
		&& p.X() <= bottom_right.X()
		&& p.Y() >= top_left.Y()
		&& p.Y() <= bottom_right.Y();
}

void VisualToolVectorClip::UpdateHold() {
	// Box selection
	if (mode == 0) {
		std::set<Feature *> boxed_features;
		Vector2D p1 = drag_start.Min(mouse_pos);
		Vector2D p2 = drag_start.Max(mouse_pos);
		for (auto& feature : features) {
			if (in_box(p1, p2, feature.pos))
				boxed_features.insert(&feature);
		}

		// Keep track of which features were selected by the box selection so
		// that only those are deselected if the user is holding ctrl
		boost::set_difference(boxed_features, sel_features,
			std::inserter(box_added, end(box_added)));

		boost::copy(boxed_features, std::inserter(sel_features, end(sel_features)));

		std::vector<Feature *> to_deselect;
		boost::set_difference(box_added, boxed_features, std::back_inserter(to_deselect));
		for (auto feature : to_deselect)
			sel_features.erase(feature);

		return;
	}

	if (mode == 1) {
		spline.back().EndPoint() = mouse_pos;
		features.back().pos = mouse_pos;
	}

	// Insert bicubic
	else if (mode == 2) {
		SplineCurve &curve = spline.back();
		curve.EndPoint() = mouse_pos;

		// Control points
		if (spline.size() > 1) {
			SplineCurve &c0 = spline.back();
			float len = (curve.p4 - curve.p1).Len();
			curve.p2 = (c0.type == SplineCurve::LINE ? c0.p2 - c0.p1 : c0.p4 - c0.p3).Unit() * (0.25f * len) + curve.p1;
		}
		else
			curve.p2 = curve.p1 * 0.75 + curve.p4 * 0.25;
		curve.p3 = curve.p1 * 0.25 + curve.p4 * 0.75;
		MakeFeatures();
	}

	// Freehand
	else if (mode == 6 || mode == 7) {
		// See if distance is enough
		Vector2D const& last = spline.back().EndPoint();
		float len = (last - mouse_pos).SquareLen();
		if ((mode == 6 && len >= 900) || (mode == 7 && len >= 3600)) {
			spline.emplace_back(last, mouse_pos);
			MakeFeature(--spline.end());
		}
	}

	if (mode == 3 || mode == 4) return;

	// Smooth spline
	if (!holding && mode == 7)
		spline.Smooth();

	// End freedraw
	if (!holding && (mode == 6 || mode == 7)) {
		SetMode(0);
		MakeFeatures();
	}
}

void VisualToolVectorClip::DoRefresh() {
	if (!active_line) return;

	int scale;
	std::string vect = GetLineVectorClip(active_line, scale, inverse);
	spline.SetScale(scale);
	spline.DecodeFromAss(vect);

	MakeFeatures();
	SelectAll();
}

void VisualToolVectorClip::SelectAll() {
	sel_features.clear();
	for (auto& feature : features)
		sel_features.insert(&feature);
}
