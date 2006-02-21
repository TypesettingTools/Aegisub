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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


////////////
// Includes
#include "video_box.h"
#include "video_display.h"
#include "video_zoom.h"
#include "video_slider.h"
#include "frame_main.h"
#include "toggle_bitmap.h"
#include "options.h"


///////////////
// Constructor
VideoBox::VideoBox(wxPanel *parent) {
	// Buttons
	videoPage = parent;
	wxBitmapButton *VideoPlayButton = new wxBitmapButton(videoPage,Video_Play,wxBITMAP(button_play),wxDefaultPosition,wxSize(25,-1));
	VideoPlayButton->SetToolTip(_("Play video starting on this position"));
	wxBitmapButton *VideoPlayLineButton = new wxBitmapButton(videoPage,Video_Play_Line,wxBITMAP(button_playline),wxDefaultPosition,wxSize(25,-1));
	VideoPlayLineButton->SetToolTip(_("Play current line"));
	wxBitmapButton *VideoStopButton = new wxBitmapButton(videoPage,Video_Stop,wxBITMAP(button_stop),wxDefaultPosition,wxSize(25,-1));
	VideoStopButton->SetToolTip(_("Stop video playback"));
	AutoScroll = new ToggleBitmap(videoPage,Video_Auto_Scroll,wxBITMAP(toggle_video_autoscroll),wxSize(30,-1));
	AutoScroll->SetToolTip(_("Toggle autoscroll of video"));
	AutoScroll->SetValue(Options.AsBool(_T("Sync video with subs")));

	wxBitmapButton *VideoTrackerMenuButton = new wxBitmapButton(videoPage,Video_Tracker_Menu,wxBITMAP(button_track_points),wxDefaultPosition,wxSize(25,-1));
	VideoTrackerMenuButton->SetToolTip(_("FexTracker"));
	wxBitmapButton *VideoTrackerMenu2Button = new wxBitmapButton(videoPage,Video_Tracker_Menu2,wxBITMAP(button_track_trail),wxDefaultPosition,wxSize(25,-1));
	VideoTrackerMenu2Button->SetToolTip(_("FexMovement"));

	// Seek
	videoSlider = new VideoSlider(videoPage,-1);
	videoSlider->SetToolTip(_("Seek video."));

	// Position
	VideoPosition = new wxTextCtrl(videoPage,-1,_T(""),wxDefaultPosition,wxSize(110,20),wxTE_READONLY);
	VideoPosition->SetToolTip(_("Current frame time and number."));

	// Times of sub relative to video
	VideoSubsPos = new wxTextCtrl(videoPage,-1,_T(""),wxDefaultPosition,wxSize(110,20),wxTE_READONLY);
	VideoSubsPos->SetToolTip(_("Time of this frame relative to start and end of current subs."));

	// Display
	videoDisplay = new VideoDisplay(videoPage,-1,wxDefaultPosition,wxSize(20,20),wxSUNKEN_BORDER,_T("VideoBox"));
	videoDisplay->ControlSlider = videoSlider;
	videoDisplay->PositionDisplay = VideoPosition;
	videoDisplay->SubsPosition = VideoSubsPos;
	videoDisplay->Reset();

	// Set display
	videoSlider->Display = videoDisplay;

	// Sizers
	videoSliderSizer = new wxBoxSizer(wxHORIZONTAL);
	videoSliderSizer->Add(videoSlider,1,wxEXPAND|wxLEFT,0);
	wxSizer *videoBottomSizer = new wxBoxSizer(wxHORIZONTAL);
	//videoBottomSizer->Add(zoomSlider,1,wxEXPAND,0);
	videoBottomSizer->Add(VideoPlayButton,0,wxTOP|wxLEFT|wxBOTTOM|wxALIGN_CENTER,2);
	videoBottomSizer->Add(VideoPlayLineButton,0,wxTOP|wxBOTTOM|wxALIGN_CENTER,2);
	videoBottomSizer->Add(VideoStopButton,0,wxTOP|wxBOTTOM|wxALIGN_CENTER,2);
	videoBottomSizer->Add(AutoScroll,0,wxTOP|wxBOTTOM|wxALIGN_CENTER|wxEXPAND,2);
	videoBottomSizer->Add(VideoTrackerMenuButton,0,wxTOP|wxBOTTOM|wxALIGN_CENTER|wxEXPAND,2);
	videoBottomSizer->Add(VideoTrackerMenu2Button,0,wxTOP|wxBOTTOM|wxALIGN_CENTER|wxEXPAND,2);
	videoBottomSizer->Add(VideoPosition,1,wxLEFT|wxALIGN_CENTER,5);
	videoBottomSizer->Add(VideoSubsPos,1,wxALIGN_CENTER,0);
	VideoSizer = new wxBoxSizer(wxVERTICAL);
	VideoSizer->Add(videoDisplay,0,wxEXPAND,0);
	VideoSizer->Add(videoSliderSizer,0,wxEXPAND,0);
	VideoSizer->Add(videoBottomSizer,0,wxEXPAND,0);
}
