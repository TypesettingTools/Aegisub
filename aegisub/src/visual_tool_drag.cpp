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

/// @file visual_tool_drag.cpp
/// @brief Position all visible subtitles by dragging visual typesetting tool
/// @ingroup visual_ts

#include "config.h"

#include "visual_tool_drag.h"

#ifndef AGI_PRE
#include <algorithm>
#include <functional>

#include <wx/bmpbuttn.h>
#include <wx/toolbar.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "utils.h"
#include "video_context.h"
#include "video_display.h"

static const DraggableFeatureType DRAG_ORIGIN = DRAG_BIG_TRIANGLE;
static const DraggableFeatureType DRAG_START = DRAG_BIG_SQUARE;
static const DraggableFeatureType DRAG_END = DRAG_BIG_CIRCLE;

#define ICON(name) (OPT_GET("App/Toolbar Icon Size")->GetInt() == 16 ? GETIMAGE(name ## _16) : GETIMAGE(name ## _24))

VisualToolDrag::VisualToolDrag(VideoDisplay *parent, agi::Context *context)
: VisualTool<VisualToolDragDraggableFeature>(parent, context)
, primary(0)
, button_is_move(true)
{
	c->selectionController->GetSelectedSet(selection);
	connections.push_back(c->selectionController->AddSelectionListener(&VisualToolDrag::OnSelectedSetChanged, this));
}

void VisualToolDrag::SetToolbar(wxToolBar *tb) {
	toolbar = tb;
	toolbar->AddTool(-1, _("Toggle between \\move and \\pos"), ICON(visual_move_conv_move));
	toolbar->Realize();
	toolbar->Show(true);

	toolbar->Bind(wxEVT_COMMAND_TOOL_CLICKED, &VisualToolDrag::OnSubTool, this);
}

void VisualToolDrag::UpdateToggleButtons() {
	bool to_move = true;
	if (active_line) {
		Vector2D p1, p2;
		int t1, t2;
		to_move = !GetLineMove(active_line, p1, p2, t1, t2);
	}

	if (to_move == button_is_move) return;

	toolbar->SetToolNormalBitmap(toolbar->GetToolByPos(0)->GetId(),
		to_move ? ICON(visual_move_conv_move) : ICON(visual_move_conv_pos));
	button_is_move = to_move;
}

void VisualToolDrag::OnSubTool(wxCommandEvent &) {
	// Toggle \move <-> \pos
	VideoContext *vc = c->videoController;
	for (SubtitleSelection::const_iterator cur = selection.begin(); cur != selection.end(); ++cur) {
		AssDialogue *line = *cur;
		Vector2D p1, p2;
		int t1, t2;

		bool has_move = GetLineMove(line, p1, p2, t1, t2);

		if (has_move)
			SetOverride(line, "\\pos", p1.PStr());
		else {
			p1 = GetLinePosition(line);
			// Round the start and end times to exact frames
			int start = vc->TimeAtFrame(vc->FrameAtTime(line->Start, agi::vfr::START)) - line->Start;
			int end = vc->TimeAtFrame(vc->FrameAtTime(line->Start, agi::vfr::END)) - line->Start;
			SetOverride(line, "\\move", wxString::Format("(%s,%s,%d,%d)", p1.Str(), p1.Str(), start, end));
		}
	}

	Commit();
	OnFileChanged();
	UpdateToggleButtons();
}

void VisualToolDrag::OnLineChanged() {
	UpdateToggleButtons();
}

void VisualToolDrag::OnFileChanged() {
	/// @todo it should be possible to preserve the selection in some cases
	features.clear();
	sel_features.clear();
	primary = 0;
	active_feature = features.end();

	for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ++it) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(&*it);
		if (diag && IsDisplayed(diag))
			MakeFeatures(diag);
	}

	UpdateToggleButtons();
}

void VisualToolDrag::OnFrameChanged() {
	if (primary && !IsDisplayed(primary->line))
		primary = 0;

	feature_iterator feat = features.begin();
	feature_iterator end = features.end();

	for (entryIter it = c->ass->Line.begin(); it != c->ass->Line.end(); ++it) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(&*it);
		if (!diag) continue;

		if (IsDisplayed(diag)) {
			// Features don't exist and should
			if (feat == end || feat->line != diag)
				MakeFeatures(diag, feat);
			// Move past already existing features for the line
			else
				while (feat != end && feat->line == diag) ++feat;
		}
		else {
			// Remove all features for this line (if any)
			while (feat != end && feat->line == diag) {
				if (feat == active_feature) active_feature = features.end();
				feat->line = 0;
				RemoveSelection(feat);
				feat = features.erase(feat);
			}
		}
	}
}

template<class T> static bool cmp_line(T const& lft, T const& rgt) {
	return lft->line == rgt->line;
}

template<class C, class T> static bool line_not_present(C const& set, T const& it) {
	return find_if(set.begin(), set.end(), bind(cmp_line<T>, it, std::placeholders::_1)) == set.end();
}

void VisualToolDrag::OnSelectedSetChanged(const SubtitleSelection &added, const SubtitleSelection &removed) {
	c->selectionController->GetSelectedSet(selection);

	bool any_changed = false;
	for (feature_iterator it = features.begin(); it != features.end(); ) {
		if (removed.count(it->line)) {
			sel_features.erase(it++);
			any_changed = true;
		}
		else {
			if (added.count(it->line) && it->type == DRAG_START && line_not_present(sel_features, it)) {
				sel_features.insert(it);
				any_changed = true;
			}
			++it;
		}
	}

	if (any_changed)
		parent->Render();
}

void VisualToolDrag::Draw() {
	DrawAllFeatures();

	// Draw connecting lines
	for (feature_iterator cur = features.begin(); cur != features.end(); ++cur) {
		if (cur->type == DRAG_START) continue;

		feature_iterator p2 = cur;
		feature_iterator p1 = cur->parent;

		// Move end marker has an arrow; origin doesn't
		bool has_arrow = p2->type == DRAG_END;
		int arrow_len = has_arrow ? 10 : 0;

		// Don't show the connecting line if the features are very close
		Vector2D direction = p2->pos - p1->pos;
		if (direction.SquareLen() < (20 + arrow_len) * (20 + arrow_len)) continue;

		direction = direction.Unit();
		// Get the start and end points of the line
		Vector2D start = p1->pos + direction * 10;
		Vector2D end = p2->pos - direction * (10 + arrow_len);

		if (has_arrow) {
			gl.SetLineColour(colour[3], 0.8f, 2);

			// Arrow line
			gl.DrawLine(start, end);

			// Arrow head
			Vector2D t_half_base_w = Vector2D(-direction.Y(), direction.X()) * 4;
			gl.DrawTriangle(end + direction * arrow_len, end + t_half_base_w, end - t_half_base_w);
		}
		// Draw dashed line
		else {
			gl.SetLineColour(colour[3], 0.5f, 2);
			gl.DrawDashedLine(start, end, 6);
		}
	}
}

void VisualToolDrag::MakeFeatures(AssDialogue *diag) {
	MakeFeatures(diag, features.end());
}

void VisualToolDrag::MakeFeatures(AssDialogue *diag, feature_iterator pos) {
	Vector2D p1 = FromScriptCoords(GetLinePosition(diag));

	// Create \pos feature
	Feature feat;
	feat.pos = p1;
	feat.layer = 0;
	feat.type = DRAG_START;
	feat.time = 0;
	feat.line = diag;
	feat.parent = features.end();
	features.insert(pos, feat);
	feature_iterator cur = pos; --cur;
	feat.parent = cur;
	if (selection.count(diag))
		sel_features.insert(cur);

	Vector2D p2;
	int t1, t2;

	// Create move destination feature
	if (GetLineMove(diag, p1, p2, t1, t2)) {
		feat.pos = FromScriptCoords(p2);
		feat.layer = 1;
		feat.type = DRAG_END;
		feat.parent->time = t1;
		feat.time = t2;
		feat.line = diag;
		features.insert(pos, feat);
		feat.parent->parent = --pos; ++pos;
	}

	// Create org feature
	if (Vector2D org = GetLineOrigin(diag)) {
		feat.pos = FromScriptCoords(org);
		feat.layer = -1;
		feat.type = DRAG_ORIGIN;
		feat.time = 0;
		feat.line = diag;
		features.insert(pos, feat);
	}
}

bool VisualToolDrag::InitializeDrag(feature_iterator feature) {
	primary = &*feature;

	// Set time of clicked feature to the current frame and shift all other
	// selected features by the same amount
	if (feature->type != DRAG_ORIGIN) {
		int time = c->videoController->TimeAtFrame(frame_number) - feature->line->Start;
		int change = time - feature->time;

		for (sel_iterator cur = sel_features.begin(); cur != sel_features.end(); ++cur) {
			(*cur)->time += change;
		}
	}
	return true;
}

void VisualToolDrag::UpdateDrag(feature_iterator feature) {
	if (feature->type == DRAG_ORIGIN) {
		SetOverride(feature->line, "\\org", ToScriptCoords(feature->pos).PStr());
		return;
	}

	feature_iterator end_feature = feature->parent;
	if (feature->type == DRAG_END)
		std::swap(feature, end_feature);

	if (feature->parent == features.end())
		SetOverride(feature->line, "\\pos", ToScriptCoords(feature->pos).PStr());
	else
		SetOverride(feature->line, "\\move",
			wxString::Format("(%s,%s,%d,%d)",
				ToScriptCoords(feature->pos).Str(),
				ToScriptCoords(end_feature->pos).Str(),
				feature->time, end_feature->time));
}

void VisualToolDrag::OnDoubleClick() {
	Vector2D d = ToScriptCoords(mouse_pos) - (primary ? ToScriptCoords(primary->pos) : GetLinePosition(active_line));

	SubtitleSelection sel = c->selectionController->GetSelectedSet();
	for (SubtitleSelection::const_iterator it = sel.begin(); it != sel.end(); ++it) {
		Vector2D p1, p2;
		int t1, t2;
		if (GetLineMove(*it, p1, p2, t1, t2)) {
			if (t1 > 0 || t2 > 0)
				SetOverride(*it, "\\move", wxString::Format("(%s,%s,%d,%d)", (p1 + d).Str(), (p2 + d).Str(), t1, t2));
			else
				SetOverride(*it, "\\move", wxString::Format("(%s,%s)", (p1 + d).Str(), (p2 + d).Str()));
		}
		else
			SetOverride(*it, "\\pos", (GetLinePosition(*it) + d).PStr());

		if (Vector2D org = GetLineOrigin(*it))
			SetOverride(*it, "\\org", (org + d).PStr());
	}

	Commit(_("positioning"));

	OnFileChanged();
}
