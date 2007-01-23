// Copyright (c) 2005-2007, Rodrigo Braz Monteiro
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
#include <wx/wxprec.h>
#ifdef __WINDOWS__
#include <windows.h>
#endif
#include <time.h>
#include "video_context.h"


//////////////
// Prototypes
class SubtitlesGrid;
class VideoSlider;
class AudioProvider;
class AudioDisplay;
class AssDialogue;
class VideoProvider;
class VideoDisplayVisual;
class VideoDisplayFexTracker;
class VideoBox;


//////////////
// Main class
class VideoDisplay: public wxGLCanvas {
	friend class AudioProvider;
	friend class VideoDisplayVisual;

private:
	wxSize origSize;
	int w,h;
	int dx1,dx2,dy1,dy2;
	bool locked;

	void OnPaint(wxPaintEvent& event);
	void OnKey(wxKeyEvent &event);
    void OnMouseEvent(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnCopyToClipboard(wxCommandEvent &event);
	void OnSaveSnapshot(wxCommandEvent &event);
	void OnCopyCoords(wxCommandEvent &event);
	void OnEraseBackground(wxEraseEvent &event) {}
	void OnSizeEvent(wxSizeEvent &event);

public:
	VideoDisplayVisual *visual;
	VideoDisplayFexTracker *tracker;
	VideoBox *box;

	double zoomValue;
	bool freeSize;

	VideoSlider *ControlSlider;
	wxComboBox *zoomBox;
	wxTextCtrl *PositionDisplay;
	wxTextCtrl *SubsPosition;

	VideoDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~VideoDisplay();
	void Reset();

	void Render();

	void ConvertMouseCoords(int &x,int &y);
	void DrawText(wxPoint Pos, wxString Text);
	void UpdatePositionDisplay();
	void UpdateSize();
	void SetZoom(double value);
	void SetZoomPos(int pos);
	void UpdateSubsRelativeTime();

	DECLARE_EVENT_TABLE()
};

