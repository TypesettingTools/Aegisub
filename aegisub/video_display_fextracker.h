// Copyright (c) 2005-2007, Rodrigo Braz Monteiro, Hajo Krabbenhöft
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
#include "gl_wrap.h"


//////////////
// Prototypes
class VideoDisplay;


///////////////////////////////
// Fex tracker video interface
class VideoDisplayFexTracker : public wxEvtHandler, public OpenGLWrapper {
public:
	bool bTrackerEditing;
	int MovementEdit;
	double TrackerEdit;
	int MouseDownX, MouseDownY;

	VideoDisplay *parent;

	VideoDisplayFexTracker(VideoDisplay *parent);

	void OnMouseEvent(wxMouseEvent &event);
	void Render();

	void OnVideoTrackPoints(wxCommandEvent &event);
	void OnVideoTrackPointAdd(wxCommandEvent &event);
	void OnVideoTrackPointDel(wxCommandEvent &event);
	void OnVideoTrackMovement(wxCommandEvent &event);
	void OnVideoTrackMovementMoveAll(wxCommandEvent &event);
	void OnVideoTrackMovementMoveOne(wxCommandEvent &event);
	void OnVideoTrackMovementMoveBefore(wxCommandEvent &event);
	void OnVideoTrackMovementMoveAfter(wxCommandEvent &event);
	void OnVideoTrackSplitLine(wxCommandEvent &event);
	void OnVideoTrackLinkFile(wxCommandEvent &event);
	void OnVideoTrackMovementEmpty(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};
