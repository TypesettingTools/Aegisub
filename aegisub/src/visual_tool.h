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

/// @file visual_tool.h
/// @see visual_tool.cpp
/// @ingroup visual_ts

#pragma once

#ifndef AGI_PRE
#include <list>
#include <vector>

#include <wx/log.h>
#include <wx/event.h>
#include <wx/button.h>
#endif

#include "gl_wrap.h"

class VideoDisplay;
class AssDialogue;
struct VideoState;

/// First window id for visualsubtoolbar items
#define VISUAL_SUB_TOOL_START 1300

/// Last window id for visualsubtoolbar items
#define VISUAL_SUB_TOOL_END (VISUAL_SUB_TOOL_START+100)

class IVisualTool : public OpenGLWrapper {
protected:
	/// DOCME
	static const wxColour colour[4];
public:
	virtual void OnMouseEvent(wxMouseEvent &event)=0;
	virtual void OnSubTool(wxCommandEvent &)=0;
	virtual void Update()=0;
	virtual void Draw()=0;
	virtual void Refresh()=0;
	virtual ~IVisualTool() { };
};

/// DOCME
/// @class VisualTool
/// @brief DOCME
/// DOCME
template<class FeatureType>
class VisualTool : public IVisualTool {
private:
	int dragStartX; /// Starting x coordinate of the current drag, if any
	int dragStartY; /// Starting y coordinate of the current drag, if any

	/// @brief Get the topmost visual feature under the mouse, or NULL if none are under the mouse
	FeatureType* GetHighlightedFeature();

	typedef typename std::list<FeatureType*>::iterator SelFeatureIter;
	typedef typename std::list<FeatureType>::iterator FeatureIter;
	typedef typename std::list<FeatureType>::const_iterator FeatureCIter;

	std::list<FeatureType*> selFeatures; /// Currently selected visual features

	bool externalChange; /// Only invalid drag lists when refreshing due to external changes
	bool selChanged; /// Has the selection already been changed in the current click?
protected:
	VideoDisplay *parent; /// VideoDisplay which this belongs to, used to frame conversion
	bool holding; /// Is a hold currently in progress?
	AssDialogue *curDiag; /// Active dialogue line for a hold; only valid when holding = true
	bool dragging; /// Is a drag currently in progress?

	FeatureType* curFeature; /// Topmost feature under the mouse; only valid during a drag?
	std::list<FeatureType> features; /// List of features which are drawn and can be clicked on
	bool dragListOK; /// Do the features not need to be regenerated?

	int frame_n; /// Current frame number
	VideoState const& video; /// Mouse and video information

	bool leftClick; /// Is a left click event currently being processed?
	bool leftDClick; /// Is a left double click event currently being processed?
	bool shiftDown; /// Is shift down?
	bool ctrlDown; /// Is ctrl down?
	bool altDown; /// Is alt down?

	void GetLinePosition(AssDialogue *diag,int &x,int &y);
	void GetLinePosition(AssDialogue *diag,int &x,int &y,int &orgx,int &orgy);
	void GetLineMove(AssDialogue *diag,bool &hasMove,int &x1,int &y1,int &x2,int &y2,int &t1,int &t2);
	void GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz);
	void GetLineScale(AssDialogue *diag,float &scalX,float &scalY);
	void GetLineClip(AssDialogue *diag,int &x1,int &y1,int &x2,int &y2,bool &inverse);
	wxString GetLineVectorClip(AssDialogue *diag,int &scale,bool &inverse);
	void SetOverride(AssDialogue* line, wxString tag, wxString value);

	/// @brief Get the dialogue line currently in the edit box
	/// @return NULL if the line is not active on the current frame
	AssDialogue *GetActiveDialogueLine();
	void DrawAllFeatures();
	void Commit(bool full=false);

	/// @brief Called when a hold is begun
	/// @return Should the hold actually happen?
	virtual bool InitializeHold() { return false; }

	/// @brief Called on every mouse event during a hold
	virtual void UpdateHold() { }

	/// @brief Called at the end of a hold
	virtual void CommitHold() { }

	/// @brief Called when the feature list needs to be (re)generated
	virtual void PopulateFeatureList() { }

	/// @brief Called at the beginning of a drag
	/// @param feature The visual feature clicked on
	/// @return Should the drag happen?
	virtual bool InitializeDrag(FeatureType* feature) { return true; }

	/// @brief Called on every mouse event during a drag
	/// @param feature The current feature to process; not necessarily the one clicked on
	virtual void UpdateDrag(FeatureType* feature) { }

	// @brief Called at the end of a drag
	virtual void CommitDrag(FeatureType* feature) { }

	/// @brief Called when there's stuff
	virtual void DoRefresh() { }

	/// @brief Must be called before removing entries from features
	void ClearSelection();

public:
	/// @brief Handler for all mouse events
	/// @param event Shockingly enough, the mouse event
	void OnMouseEvent(wxMouseEvent &event);

	/// @brief Event handler for the subtoolbar
	virtual void OnSubTool(wxCommandEvent &) { }
	/// @brief Called when there's stuff
	virtual void Update() { };
	/// @brief Draw stuff
	virtual void Draw()=0;
	/// @brief Called when there's stuff
	void Refresh();

	/// @brief Constructor
	/// @param parent The VideoDisplay to use for coordinate conversion
	/// @param video Video and mouse information passing blob
	VisualTool(VideoDisplay *parent, VideoState const& video);

	/// @brief Destructor
	virtual ~VisualTool();
};

