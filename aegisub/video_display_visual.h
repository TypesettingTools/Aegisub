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


//////////////
// Prototypes
class VideoDisplay;
class AssDialogue;


////////////////////////
// Visual handler class
class VideoDisplayVisual {
	friend class VideoDisplay;

private:
	wxColour colour[4];

	int mouse_x,mouse_y;
	int start_x,start_y;
	int cur_x,cur_y;
	int orig_x,orig_y;

	int mode;
	bool holding;
	int hold;

	wxBitmap *backbuffer;
	wxString mouseText;
	AssDialogue *curSelection;

	VideoDisplay *parent;

	void GetLinePosition(AssDialogue *diag,int &x,int &y);
	void GetLinePosition(AssDialogue *diag,int &x,int &y,int &orgx,int &orgy);
	void GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz);
	void DrawTrackingOverlay(wxDC &dc);
	void DrawOverlay();
	void SetMode(int mode);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyEvent(wxKeyEvent &event);

public:
	VideoDisplayVisual(VideoDisplay *parent);
	~VideoDisplayVisual();
};
