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
#include <vector>
#include "boost/shared_ptr.hpp"

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
	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME
	int dragStartX,dragStartY,dragOrigX,dragOrigY;
	int GetHighlightedFeature();
protected:
	/// DOCME
	VideoDisplay *parent;

	/// DOCME
	bool holding;

	/// DOCME
	AssDialogue *curDiag;

	/// DOCME
	bool dragging;

	/// DOCME
	int curFeature;

	/// DOCME
	std::vector<FeatureType> features;

	/// DOCME
	bool dragListOK;

	/// DOCME
	int frame_n;

	VideoState const& video;

	/// DOCME
	bool leftClick;

	/// DOCME
	bool leftDClick;

	/// DOCME
	bool shiftDown;

	/// DOCME
	bool ctrlDown;

	/// DOCME
	bool altDown;

	void GetLinePosition(AssDialogue *diag,int &x,int &y);
	void GetLinePosition(AssDialogue *diag,int &x,int &y,int &orgx,int &orgy);
	void GetLineMove(AssDialogue *diag,bool &hasMove,int &x1,int &y1,int &x2,int &y2,int &t1,int &t2);
	void GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz);
	void GetLineScale(AssDialogue *diag,float &scalX,float &scalY);
	void GetLineClip(AssDialogue *diag,int &x1,int &y1,int &x2,int &y2,bool &inverse);
	wxString GetLineVectorClip(AssDialogue *diag,int &scale,bool &inverse);
	void SetOverride(AssDialogue* line, wxString tag, wxString value);

	AssDialogue *GetActiveDialogueLine();
	void DrawAllFeatures();
	void Commit(bool full=false);

	/// @brief DOCME
	///
	virtual bool InitializeHold() { return false; }

	/// @brief DOCME
	///
	virtual void UpdateHold() {}

	/// @brief DOCME
	/// @return 
	///
	virtual void CommitHold() {}

	/// @brief DOCME
	///
	virtual void PopulateFeatureList() { }

	/// @brief DOCME
	/// @param feature 
	///
	virtual bool InitializeDrag(FeatureType &feature) { return true; }

	/// @brief DOCME
	/// @param feature 
	///
	virtual void UpdateDrag(FeatureType &feature) {}

	/// @brief DOCME
	/// @param feature 
	///
	virtual void CommitDrag(FeatureType &feature) {}

	/// @brief DOCME
	///
	virtual void DoRefresh() {}

public:
	void OnMouseEvent(wxMouseEvent &event);

	/// @brief DOCME
	/// @param event 
	///
	virtual void OnSubTool(wxCommandEvent &) {}
	virtual void Update() { };
	virtual void Draw()=0;
	void Refresh();

	VisualTool(VideoDisplay *parent, VideoState const& video);
	virtual ~VisualTool();
};

