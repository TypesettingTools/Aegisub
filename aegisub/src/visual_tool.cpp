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

/// @file visual_tool.cpp
/// @brief Base class for visual typesetting functions
/// @ingroup visual_ts

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>
#include <wx/glcanvas.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "ass_style.h"
#include "ass_time.h"
#include "include/aegisub/context.h"
#include "main.h"
#include "selection_controller.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"
#include "visual_feature.h"
#include "visual_tool.h"
#include "visual_tool_clip.h"
#include "visual_tool_drag.h"
#include "visual_tool_vector_clip.h"

const wxColour IVisualTool::colour[4] = {wxColour(106,32,19), wxColour(255,169,40), wxColour(255,253,185), wxColour(187,0,0)};

template<class FeatureType>
VisualTool<FeatureType>::VisualTool(VideoDisplay *parent, agi::Context *context, VideoState const& video)
: dragStartX(0)
, dragStartY(0)
, commitId(-1)
, selChanged(false)
, selectedFeatures(selFeatures)
, grid(context->subsGrid)
, parent(parent)
, holding(false)
, dragging(false)
, externalChange(true)
, video(video)
, leftClick(false)
, leftDClick(false)
, shiftDown(false)
, ctrlDown(false)
, altDown(false)
{
	frameNumber = context->videoController->GetFrameN();
	curDiag = GetActiveDialogueLine();
	grid->AddSelectionListener(this);
	curFeature = features.begin();
}

template<class FeatureType>
VisualTool<FeatureType>::~VisualTool() {
	grid->RemoveSelectionListener(this);
}

template<class FeatureType>
void VisualTool<FeatureType>::OnMouseEvent(wxMouseEvent &event) {
	bool needRender = false;

	if (event.Leaving()) {
		Update();
		parent->Render();
		return;
	}
	else if (event.Entering() && !OPT_GET("Tool/Visual/Always Show")->GetBool()) {
		needRender = true;
	}
	externalChange = false;

	leftClick = event.ButtonDown(wxMOUSE_BTN_LEFT);
	leftDClick = event.LeftDClick();
	shiftDown = event.m_shiftDown;
#ifdef __APPLE__
	ctrlDown = event.m_metaDown; // Cmd key
#else
	ctrlDown = event.m_controlDown;
#endif
	altDown = event.m_altDown;

	if (!dragging) {
		feature_iterator oldHigh = curFeature;
		GetHighlightedFeature();
		if (curFeature != oldHigh) needRender = true;
	}

	if (dragging) {
		// continue drag
		if (event.LeftIsDown()) {
			for (selection_iterator cur = selFeatures.begin(); cur != selFeatures.end(); ++cur) {
				(*cur)->x = (video.x - dragStartX + (*cur)->origX);
				(*cur)->y = (video.y - dragStartY + (*cur)->origY);
				if (shiftDown) {
					if (abs(video.x - dragStartX) > abs(video.y - dragStartY)) {
						(*cur)->y = (*cur)->origY;
					}
					else {
						(*cur)->x = (*cur)->origX;
					}
				}
				UpdateDrag(*cur);
				CommitDrag(*cur);
			}
			Commit();
			needRender = true;
		}
		// end drag
		else {
			dragging = false;

			// mouse didn't move, fiddle with selection
			if (curFeature->x == curFeature->origX && curFeature->y == curFeature->origY) {
				// Don't deselect stuff that was selected in this click's mousedown event
				if (!selChanged) {
					if (ctrlDown) {
						// deselect this feature
						RemoveSelection(curFeature);
					}
					else {
						SetSelection(curFeature);
					}
				}
			}
			else {
				for (selection_iterator cur = selFeatures.begin(); cur != selFeatures.end(); ++cur) {
					CommitDrag(*cur);
				}
				Commit();
			}

			curFeature = features.end();
			parent->ReleaseMouse();
			parent->SetFocus();
		}
	}
	else if (holding) {
		// continue hold
		if (event.LeftIsDown()) {
			UpdateHold();
			needRender = true;
		}
		// end hold
		else {
			holding = false;

			parent->ReleaseMouse();
			parent->SetFocus();
		}
		CommitHold();
		Commit();

	}
	else if (leftClick) {
		// start drag
		if (curFeature != features.end()) {
			if (selFeatures.find(curFeature) == selFeatures.end()) {
				selChanged = true;
				if (ctrlDown) {
					AddSelection(curFeature);
				}
				else {
					SetSelection(curFeature);
				}
			}
			else {
				selChanged = false;
			}
			if (curFeature->line) grid->SetActiveLine(curFeature->line);

			if (InitializeDrag(curFeature)) {
				dragStartX = video.x;
				dragStartY = video.y;
				for (selection_iterator cur = selFeatures.begin(); cur != selFeatures.end(); ++cur) {
					(*cur)->origX = (*cur)->x;
					(*cur)->origY = (*cur)->y;
				}

				dragging = true;
				parent->CaptureMouse();
			}
		}
		// start hold
		else {
			if (!altDown) {
				ClearSelection();
				Selection sel;
				sel.insert(grid->GetActiveLine());
				grid->SetSelectedSet(sel);
				needRender = true;
			}
			if (curDiag && InitializeHold()) {
				holding = true;
				parent->CaptureMouse();
			}
		}
	}

	if (Update() || needRender) parent->Render();
	externalChange = true;

	if (!event.LeftIsDown()) {
		// Only coalesce the changes made in a single drag
		commitId = -1;
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::Commit(wxString message) {
	externalChange = false;
	if (message.empty()) {
		message = _("visual typesetting");
	}
	commitId = grid->ass->Commit(message, AssFile::COMMIT_TEXT, commitId);
	externalChange = true;
}

template<class FeatureType>
AssDialogue* VisualTool<FeatureType>::GetActiveDialogueLine() {
	AssDialogue *diag = grid->GetActiveLine();
	if (diag && grid->IsDisplayed(diag))
		return diag;
	return NULL;
}

template<class FeatureType>
void VisualTool<FeatureType>::GetHighlightedFeature() {
	int highestLayerFound = INT_MIN;
	curFeature = features.end();
	for (feature_iterator cur = features.begin(); cur != features.end(); ++cur) {
		if (cur->IsMouseOver(video.x, video.y) && cur->layer > highestLayerFound) {
			curFeature = cur;
			highestLayerFound = cur->layer;
		}
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::DrawAllFeatures() {
	SetLineColour(colour[0],1.0f,2);
	for (feature_iterator cur = features.begin(); cur != features.end(); ++cur) {
		int fill;
		if (cur == curFeature)
			fill = 2;
		else if (selFeatures.find(cur) != selFeatures.end())
			fill = 3;
		else
			fill = 1;
		SetFillColour(colour[fill],0.6f);
		cur->Draw(*this);
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::Refresh() {
	if (externalChange) {
		curDiag = GetActiveDialogueLine();
		curFeature = features.end();
		OnFileChanged();
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::SetFrame(int newFrameNumber) {
	if (frameNumber == newFrameNumber) return;
	frameNumber = newFrameNumber;
	curFeature = features.end();
	OnFrameChanged();
	AssDialogue *newCurDiag = GetActiveDialogueLine();
	if (newCurDiag != curDiag) {
		curDiag = newCurDiag;
		OnLineChanged();
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::OnActiveLineChanged(AssDialogue *new_line) {
	if (new_line && !grid->IsDisplayed(new_line)) {
		new_line = NULL;
	}
	if (new_line != curDiag) {
		curDiag = new_line;
		OnLineChanged();
	}
}


template<class FeatureType>
void VisualTool<FeatureType>::SetSelection(feature_iterator feat) {
	selFeatures.clear();
	lineSelCount.clear();

	selFeatures.insert(feat);

	AssDialogue *line = feat->line;
	if (line) {
		lineSelCount[line] = 1;

		Selection sel;
		sel.insert(line);
		grid->SetSelectedSet(sel);
	}
}


template<class FeatureType>
void VisualTool<FeatureType>::AddSelection(feature_iterator feat) {
	if (selFeatures.insert(feat).second && feat->line) {
		lineSelCount[feat->line] += 1;
		Selection sel = grid->GetSelectedSet();
		if (sel.insert(feat->line).second) {
			grid->SetSelectedSet(sel);
		}
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::RemoveSelection(feature_iterator feat) {
	if (selFeatures.erase(feat) > 0 && feat->line) {
		// Deselect a line only if all features for that line have been
		// deselected
		AssDialogue* line = feat->line;
		lineSelCount[line] -= 1;
		assert(lineSelCount[line] >= 0);
		if (lineSelCount[line] <= 0) {
			Selection sel = grid->GetSelectedSet();

			// Don't deselect the only selected line
			if (sel.size() <= 1) return;

			sel.erase(line);

			// Set the active line to an arbitrary selected line if we just
			// deselected the active line
			if (line == grid->GetActiveLine()) {
				grid->SetActiveLine(*sel.begin());
			}

			grid->SetSelectedSet(sel);
		}
	}
}

template<class FeatureType>
void VisualTool<FeatureType>::ClearSelection() {
	selFeatures.clear();
	lineSelCount.clear();
}

enum TagFoundType {
	TAG_NOT_FOUND = 0,
	PRIMARY_TAG_FOUND,
	ALT_TAG_FOUND
};

/// @brief Get the first value set for a tag
/// @param line Line to get the value from
/// @param tag  Tag to get the value of
/// @param n    Number of parameters passed
/// @return     Which tag (if any) was found
template<class T>
static TagFoundType get_value(const AssDialogue *line, wxString tag, size_t n, ...) {
	wxString alt;
	if (tag == L"\\pos") alt = L"\\move";
	else if (tag == L"\\an") alt = L"\\a";
	else if (tag == L"\\clip") alt = L"\\iclip";

	for (size_t i = 0; i < line->Blocks.size(); i++) {
		const AssDialogueBlockOverride *ovr = dynamic_cast<const AssDialogueBlockOverride*>(line->Blocks[i]);
		if (!ovr) continue;

		for (size_t j=0; j < ovr->Tags.size(); j++) {
			const AssOverrideTag *cur = ovr->Tags[j];
			if ((cur->Name == tag || cur->Name == alt) && cur->Params.size() >= n) {
				va_list argp;
				va_start(argp, n);
				for (size_t j = 0; j < n; j++) {
					T *val = va_arg(argp, T *);
					*val = cur->Params[j]->Get<T>(*val);
				}
				va_end(argp);
				return cur->Name == alt ? ALT_TAG_FOUND : PRIMARY_TAG_FOUND;
			}
		}
	}
	return TAG_NOT_FOUND;
}

template<class FeatureType>
void VisualTool<FeatureType>::GetLinePosition(AssDialogue *diag,int &x, int &y) {
	int orgx,orgy;
	GetLinePosition(diag,x,y,orgx,orgy);
}

template<class FeatureType>
void VisualTool<FeatureType>::GetLinePosition(AssDialogue *diag,int &x, int &y, int &orgx, int &orgy) {
	int margin[4];
	for (int i=0;i<4;i++) margin[i] = diag->Margin[i];
	int align = 2;

	AssStyle *style = grid->ass->GetStyle(diag->Style);
	if (style) {
		align = style->alignment;
		for (int i=0;i<4;i++) {
			if (margin[i] == 0) margin[i] = style->Margin[i];
		}
	}

	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);

	// Process margins
	margin[1] = sw - margin[1];
	margin[3] = sh - margin[2];

	// Overrides processing
	diag->ParseASSTags();

	if (!get_value<int>(diag, "\\pos", 2, &x, &y)) {
		if (get_value<int>(diag, "\\an", 1, &align) == ALT_TAG_FOUND) {
			switch(align) {
				case 1: case 2: case 3:
					break;
				case 5: case 6: case 7:
					align += 2;
					break;
				case 9: case 10: case 11:
					align -= 5;
					break;
				default:
					align = 2;
					break;
			}
		}
		// Alignment type
		int hor = (align - 1) % 3;
		int vert = (align - 1) / 3;

		// Calculate positions
		if (hor == 0) x = margin[0];
		else if (hor == 1) x = (margin[0] + margin[1])/2;
		else if (hor == 2) x = margin[1];
		if (vert == 0) y = margin[3];
		else if (vert == 1) y = (margin[2] + margin[3])/2;
		else if (vert == 2) y = margin[2];
	}

	parent->FromScriptCoords(&x, &y);

	if (!get_value<int>(diag, "\\org", 2, &orgx, &orgy)) {
		orgx = x;
		orgy = y;
	}
	else {
		parent->FromScriptCoords(&orgx, &orgy);
	}

	diag->ClearBlocks();
}

template<class FeatureType>
void VisualTool<FeatureType>::GetLineMove(AssDialogue *diag,bool &hasMove,int &x1,int &y1,int &x2,int &y2,int &t1,int &t2) {
	diag->ParseASSTags();

	hasMove =
		get_value<int>(diag, "\\move", 6, &x1, &y1, &x2, &y2, &t1, &t2) ||
		get_value<int>(diag, "\\move", 4, &x1, &y1, &x2, &y2);

	if (hasMove) {
		parent->FromScriptCoords(&x1, &y1);
		parent->FromScriptCoords(&x2, &y2);
	}

	diag->ClearBlocks();
}

template<class FeatureType>
void VisualTool<FeatureType>::GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz) {
	rx = ry = rz = 0.f;

	AssStyle *style = grid->ass->GetStyle(diag->Style);
	if (style) {
		rz = style->angle;
	}

	diag->ParseASSTags();

	get_value<float>(diag, L"\\frx", 1, &rx);
	get_value<float>(diag, L"\\fry", 1, &ry);
	get_value<float>(diag, L"\\frz", 1, &rz);

	diag->ClearBlocks();
}

template<class FeatureType>
void VisualTool<FeatureType>::GetLineScale(AssDialogue *diag,float &scalX,float &scalY) {
	scalX = scalY = 100.f;

	AssStyle *style = grid->ass->GetStyle(diag->Style);
	if (style) {
		scalX = style->scalex;
		scalY = style->scaley;
	}

	diag->ParseASSTags();

	get_value<float>(diag, L"\\fscx", 1, &scalX);
	get_value<float>(diag, L"\\fscy", 1, &scalY);

	diag->ClearBlocks();
}

template<class FeatureType>
void VisualTool<FeatureType>::GetLineClip(AssDialogue *diag,int &x1,int &y1,int &x2,int &y2,bool &inverse) {
	x1 = y1 = 0;
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);
	x2 = sw-1;
	y2 = sh-1;
	inverse = false;

	diag->ParseASSTags();
	inverse = get_value<int>(diag, L"\\clip", 4, &x1, &y1, &x2, &y2) == ALT_TAG_FOUND;
	diag->ClearBlocks();

	parent->FromScriptCoords(&x1, &y1);
	parent->FromScriptCoords(&x2, &y2);
}

template<class FeatureType>
wxString VisualTool<FeatureType>::GetLineVectorClip(AssDialogue *diag,int &scale,bool &inverse) {
	scale = 1;
	inverse = false;
	diag->ParseASSTags();

	int x1, y1, x2, y2;
	TagFoundType res = get_value<int>(diag, L"\\clip", 4, &x1, &y1, &x2, &y2);
	if (res) {
		inverse = res == ALT_TAG_FOUND;
		diag->ClearBlocks();
		return wxString::Format(L"m %d %d l %d %d %d %d %d %d", x1, y1, x2, y1, x2, y2, x1, y2);
	}
	wxString result;
	wxString scaleStr;
	res = get_value<wxString>(diag, L"\\clip", 2, &scaleStr, &result);
	inverse = res == ALT_TAG_FOUND;
	if (!scaleStr.empty()) {
		long s;
		scaleStr.ToLong(&s);
		scale = s;
	}
	diag->ClearBlocks();
	return result;
}

/// @brief Set override 
/// @param tag   
/// @param value 
template<class FeatureType>
void VisualTool<FeatureType>::SetOverride(AssDialogue* line, wxString tag, wxString value) {
	if (!line) return;

	wxString removeTag;
	if (tag == L"\\1c") removeTag = L"\\c";
	else if (tag == L"\\fr") removeTag = L"\\frz";
	else if (tag == L"\\pos") removeTag = L"\\move";
	else if (tag == L"\\move") removeTag = L"\\pos";
	else if (tag == L"\\clip") removeTag = L"\\iclip";
	else if (tag == L"\\iclip") removeTag = L"\\clip";

	wxString insert = tag + value;

	// Get block at start
	line->ParseASSTags();
	AssDialogueBlock *block = line->Blocks.at(0);

	// Get current block as plain or override
	AssDialogueBlockPlain *plain = dynamic_cast<AssDialogueBlockPlain*>(block);
	AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride*>(block);
	assert(dynamic_cast<AssDialogueBlockDrawing*>(block) == NULL);

	if (plain) {
		line->Text = L"{" + insert + L"}" + line->Text;
	}
	else if (ovr) {
		// Remove old of same
		for (size_t i = 0; i < ovr->Tags.size(); i++) {
			wxString name = ovr->Tags[i]->Name;
			if (tag == name || removeTag == name) {
				delete ovr->Tags[i];
				ovr->Tags.erase(ovr->Tags.begin() + i);
				i--;
			}
		}
		ovr->AddTag(insert);

		line->UpdateText();
	}

	parent->SetFocus();
}

// If only export worked
template class VisualTool<VisualDraggableFeature>;
template class VisualTool<ClipCorner>;
template class VisualTool<VisualToolDragDraggableFeature>;
template class VisualTool<VisualToolVectorClipDraggableFeature>;
