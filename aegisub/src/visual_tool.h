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
#include <map>
#include <set>
#include <vector>

#include <wx/log.h>
#include <wx/event.h>
#include <wx/button.h>
#endif

#include "base_grid.h"
#include "gl_wrap.h"

class VideoDisplay;
class AssDialogue;
class SubtitlesGrid;
struct VideoState;
namespace agi {
	class OptionValue;
}

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
	virtual bool Update()=0;
	virtual void Draw()=0;
	virtual void Refresh()=0;
	virtual void SetFrame(int frame)=0;
	virtual ~IVisualTool() { };
};

struct ltaddr {
	template<class T>
	bool operator()(T lft, T rgt) const {
		return &*lft < &*rgt;
	}
};

/// DOCME
/// @class VisualTool
/// @brief DOCME
/// DOCME
template<class FeatureType>
class VisualTool : public IVisualTool, protected SubtitleSelectionListener {
protected:
	typedef FeatureType Feature;
	typedef typename std::list<FeatureType>::iterator feature_iterator;
	typedef typename std::list<FeatureType>::const_iterator feature_const_iterator;
private:
	agi::OptionValue* realtime; /// Realtime updating option
	int dragStartX; /// Starting x coordinate of the current drag, if any
	int dragStartY; /// Starting y coordinate of the current drag, if any

	/// Set curFeature to the topmost feature under the mouse, or end() if there
	/// are none
	void GetHighlightedFeature();

	/// @brief Get the dialogue line currently in the edit box
	/// @return NULL if the line is not active on the current frame
	AssDialogue *GetActiveDialogueLine();

	typedef typename std::set<feature_iterator, ltaddr>::iterator selection_iterator;

	std::set<feature_iterator, ltaddr> selFeatures; /// Currently selected visual features
	std::map<AssDialogue*, int> lineSelCount; /// Number of selected features for each line

	bool selChanged; /// Has the selection already been changed in the current click?

	/// @brief Called when a hold is begun
	/// @return Should the hold actually happen?
	virtual bool InitializeHold() { return false; }
	/// @brief Called on every mouse event during a hold
	virtual void UpdateHold() { }
	/// @brief Called at the end of a hold
	virtual void CommitHold() { }

	/// @brief Called at the beginning of a drag
	/// @param feature The visual feature clicked on
	/// @return Should the drag happen?
	virtual bool InitializeDrag(feature_iterator feature) { return true; }
	/// @brief Called on every mouse event during a drag
	/// @param feature The current feature to process; not necessarily the one clicked on
	virtual void UpdateDrag(feature_iterator feature) { }
	/// @brief Called at the end of a drag
	virtual void CommitDrag(feature_iterator feature) { }

	/// Called when the file is changed by something other than a visual tool
	virtual void OnFileChanged() { DoRefresh(); }
	/// Called when the frame number changes
	virtual void OnFrameChanged() { }
	/// Called when curDiag changes
	virtual void OnLineChanged() { DoRefresh(); }
	/// Generic refresh to simplify tools which do the same thing for any
	/// external change (i.e. almost all of them). Called only by the above
	/// methods.
	virtual void DoRefresh() { }

	/// @brief Called when there's stuff
	/// @return Should the display rerender?
	virtual bool Update() { return false; };
	/// @brief Draw stuff
	virtual void Draw()=0;

protected:
	/// Read-only reference to the set of selected features for subclasses
	const std::set<feature_iterator, ltaddr> &selectedFeatures;
	typedef typename std::set<feature_iterator, ltaddr>::const_iterator sel_iterator;
	SubtitlesGrid *grid;
	VideoDisplay *parent; /// VideoDisplay which this belongs to, used to frame conversion
	bool holding; /// Is a hold currently in progress?
	AssDialogue *curDiag; /// Active dialogue line; NULL if it is not visible on the current frame
	bool dragging; /// Is a drag currently in progress?
	bool externalChange; /// Only invalid drag lists when refreshing due to external changes

	feature_iterator curFeature; /// Topmost feature under the mouse; generally only valid during a drag
	std::list<FeatureType> features; /// List of features which are drawn and can be clicked on

	int frameNumber; /// Current frame number
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

	/// Draw all of the features in the list
	void DrawAllFeatures();
	/// @brief Commit the current file state
	/// @param message Description of changes for undo
	void Commit(bool full=false, wxString message = L"");

	/// @brief Add a feature (and its line) to the selection
	/// @param i Index in the feature list
	void AddSelection(feature_iterator feat);

	/// @brief Remove a feature from the selection
	/// @param i Index in the feature list
	/// Also deselects lines if all features for that line have been deselected
	void RemoveSelection(feature_iterator feat);

	/// @brief Set the selection to a single feature, deselecting everything else
	/// @param i Index in the feature list
	void SetSelection(feature_iterator feat);

	/// @brief Clear the selection
	void ClearSelection();

	// SubtitleSelectionListener implementation
	void OnActiveLineChanged(AssDialogue *new_line);
	virtual void OnSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed) { }

public:
	/// @brief Handler for all mouse events
	/// @param event Shockingly enough, the mouse event
	void OnMouseEvent(wxMouseEvent &event);

	/// @brief Event handler for the subtoolbar
	virtual void OnSubTool(wxCommandEvent &) { }

	/// @brief Signal that the file has changed
	void Refresh();
	/// @brief Signal that the current frame number has changed
	/// @param newFrameNumber The new frame number
	void SetFrame(int newFrameNumber);


	/// @brief Constructor
	/// @param parent The VideoDisplay to use for coordinate conversion
	/// @param video Video and mouse information passing blob
	VisualTool(VideoDisplay *parent, VideoState const& video);

	/// @brief Destructor
	virtual ~VisualTool();
};
