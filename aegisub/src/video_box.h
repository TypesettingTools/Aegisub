// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file video_box.h
/// @see video_box.cpp
/// @ingroup main_ui video
///


#pragma once


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/tglbtn.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>


//////////////
// Prototypes
class VideoDisplay;
class VideoSlider;
class ToggleBitmap;
class FrameMain;


///////////////////
// Video box class
class VideoBox : public wxPanel {
private:
	void OnVideoPlay(wxCommandEvent &event);
	void OnVideoPlayLine(wxCommandEvent &event);
	void OnVideoStop(wxCommandEvent &event);
	void OnVideoToggleScroll(wxCommandEvent &event);

	void OnModeChange(wxCommandEvent &event);
	void OnSubTool(wxCommandEvent &event);
	void OnToggleRealtime(wxCommandEvent &event);
	void OnHelp(wxCommandEvent &event);

public:
	wxToolBar *visualToolBar;
	wxToolBar *visualSubToolBar;
	//wxSizer *visualSubToolBar;

	ToggleBitmap *AutoScroll;
	wxBoxSizer *VideoSizer;
	wxBoxSizer *videoSliderSizer;
	wxWindow *videoPage;
	wxTextCtrl *VideoPosition;
	wxTextCtrl *VideoSubsPos;
	VideoDisplay *videoDisplay;
	VideoSlider *videoSlider;

	VideoBox (wxWindow *parent, bool isDetached);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	Video_Play = 500,
	Video_Play_Line,
	Video_Stop,
	Video_Auto_Scroll,

	Video_Mode_Standard,
	Video_Mode_Drag,
	Video_Mode_Rotate_Z,
	Video_Mode_Rotate_XY,
	Video_Mode_Scale,
	Video_Mode_Clip,
	Video_Mode_Vector_Clip,
	Video_Mode_Realtime,
	Video_Mode_Help
};

