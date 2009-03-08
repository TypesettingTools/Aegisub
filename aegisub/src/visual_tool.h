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
#include <vector>
#include <wx/log.h>
#include "video_display.h"
#include "gl_wrap.h"
#include "visual_feature.h"


//////////////
// Prototypes
class VideoDisplay;
class AssDialogue;
class VisualTool;


/////////////////////////
// Visual sub tool range
#define VISUAL_SUB_TOOL_START 1300
#define VISUAL_SUB_TOOL_END (VISUAL_SUB_TOOL_START+100)


////////////////////
// Event sink class
class VisualToolEvent : public wxEvtHandler {
private:
	VisualTool *tool;

public:
	void OnButton(wxCommandEvent &event);

	VisualToolEvent(VisualTool *tool);
};


////////////////////////
// Visual handler class
class VisualTool : public OpenGLWrapper {
	friend class VisualToolEvent;

private:
	VideoDisplay *parent;
	VisualToolEvent eventSink;

protected:
	wxColour colour[4];

	bool holding;
	AssDialogue *curDiag;

	bool dragging;
	int curFeature;
	std::vector<VisualDraggableFeature> features;
	int dragStartX,dragStartY,dragOrigX,dragOrigY;
	bool dragListOK;

	int w,h,sw,sh,mx,my;
	int frame_n;

	bool leftClick;
	bool leftDClick;
	bool shiftDown;
	bool ctrlDown;
	bool altDown;

	void GetLinePosition(AssDialogue *diag,int &x,int &y);
	void GetLinePosition(AssDialogue *diag,int &x,int &y,int &orgx,int &orgy);
	void GetLineMove(AssDialogue *diag,bool &hasMove,int &x1,int &y1,int &x2,int &y2,int &t1,int &t2);
	void GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz);
	void GetLineScale(AssDialogue *diag,float &scalX,float &scalY);
	void GetLineClip(AssDialogue *diag,int &x1,int &y1,int &x2,int &y2,bool &inverse);
	wxString GetLineVectorClip(AssDialogue *diag,int &scale,bool &inverse);
	void FillPositionData();
	void SetOverride(wxString tag,wxString value);

	VideoDisplay *GetParent() { return parent; }
	AssDialogue *GetActiveDialogueLine();
	int GetHighlightedFeature();
	void DrawAllFeatures();
	void Commit(bool full=false);

	void ConnectButton(wxButton *button);
	virtual void OnButton(wxCommandEvent &event) {}

	virtual bool CanHold() { return false; }
	virtual bool HoldEnabled() { return true; }
	virtual void InitializeHold() {}
	virtual void UpdateHold() {}
	virtual void CommitHold() {}

	virtual bool CanDrag() { return false; }
	virtual bool DragEnabled() { return true; }
	virtual void PopulateFeatureList() { wxLogMessage(_T("wtf?")); }
	virtual void InitializeDrag(VisualDraggableFeature &feature) {}
	virtual void UpdateDrag(VisualDraggableFeature &feature) {}
	virtual void CommitDrag(VisualDraggableFeature &feature) {}
	virtual void ClickedFeature(VisualDraggableFeature &feature) {}

	virtual void DoRefresh() {}

public:
	int mouseX,mouseY;

	void OnMouseEvent(wxMouseEvent &event);
	virtual void OnSubTool(wxCommandEvent &event) {}
	virtual void Update()=0;
	virtual void Draw()=0;
	void Refresh();

	VisualTool(VideoDisplay *parent);
	virtual ~VisualTool();
};


