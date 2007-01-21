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
#include <wx/wxprec.h>
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/tglbtn.h>
#include <wx/rawbmp.h>
#include "video_box.h"
#include "video_display.h"
#include "video_display_visual.h"
#include "video_display_fextracker.h"
#include "video_zoom.h"
#include "video_slider.h"
#include "frame_main.h"
#include "toggle_bitmap.h"
#include "options.h"
#include "setup.h"
#include "subs_grid.h"
#include "video_provider.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "utils.h"
#include "main.h"
#include "toggle_bitmap.h"


///////////////
// Constructor
VideoBox::VideoBox(wxWindow *parent) 
: wxPanel (parent,5000)
{
	// Parent
	videoPage = this;
	frame = AegisubApp::Get()->frame;

	// Buttons
	wxBitmapButton *VideoPlayButton = new wxBitmapButton(videoPage,Video_Play,wxBITMAP(button_play),wxDefaultPosition,wxSize(25,-1));
	VideoPlayButton->SetToolTip(_("Play video starting on this position"));
	wxBitmapButton *VideoPlayLineButton = new wxBitmapButton(videoPage,Video_Play_Line,wxBITMAP(button_playline),wxDefaultPosition,wxSize(25,-1));
	VideoPlayLineButton->SetToolTip(_("Play current line"));
	wxBitmapButton *VideoStopButton = new wxBitmapButton(videoPage,Video_Stop,wxBITMAP(button_stop),wxDefaultPosition,wxSize(25,-1));
	VideoStopButton->SetToolTip(_("Stop video playback"));
	AutoScroll = new ToggleBitmap(videoPage,Video_Auto_Scroll,wxBITMAP(toggle_video_autoscroll),wxSize(30,-1));
	AutoScroll->SetToolTip(_("Toggle autoscroll of video"));
	AutoScroll->SetValue(Options.AsBool(_T("Sync video with subs")));

	// Fextracker
	#if USE_FEXTRACKER == 1
	wxBitmapButton *VideoTrackerMenuButton = new wxBitmapButton(videoPage,Video_Tracker_Menu,wxBITMAP(button_track_points));
	VideoTrackerMenuButton->SetToolTip(_("FexTracker"));
	wxBitmapButton *VideoTrackerMenu2Button = new wxBitmapButton(videoPage,Video_Tracker_Menu2,wxBITMAP(button_track_trail));
	VideoTrackerMenu2Button->SetToolTip(_("FexMovement"));
	#endif

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
	videoDisplay = new VideoDisplay(videoPage,-1,wxDefaultPosition,wxDefaultSize,wxSUNKEN_BORDER,_T("VideoBox"));
	videoDisplay->ControlSlider = videoSlider;
	videoDisplay->PositionDisplay = VideoPosition;
	videoDisplay->SubsPosition = VideoSubsPos;
	videoDisplay->box = this;
	VideoContext::Get()->AddDisplay(videoDisplay);
	videoDisplay->Reset();

	// Set display
	videoSlider->Display = videoDisplay;
	
	// Typesetting buttons
	standard = new wxBitmapButton(videoPage,Video_Mode_Standard,wxBITMAP(visual_standard));
	standard->SetToolTip(_("Standard mode, double click sets position."));
	drag = new wxBitmapButton(videoPage,Video_Mode_Drag,wxBITMAP(visual_move));
	drag->SetToolTip(_("Drag subtitles."));
	rotatez = new wxBitmapButton(videoPage,Video_Mode_Rotate_Z,wxBITMAP(visual_rotatez));
	rotatez->SetToolTip(_("Rotate subtitles on their Z axis."));
	rotatexy = new wxBitmapButton(videoPage,Video_Mode_Rotate_XY,wxBITMAP(visual_rotatexy));
	rotatexy->SetToolTip(_("Rotate subtitles on their X and Y axes."));
	scale = new wxBitmapButton(videoPage,Video_Mode_Scale,wxBITMAP(visual_scale));
	scale->SetToolTip(_("Scale subtitles on X and Y axes."));
	clip = new wxBitmapButton(videoPage,Video_Mode_Clip,wxBITMAP(visual_clip));
	clip->SetToolTip(_("Clip subtitles to a rectangle."));
	realtime = new ToggleBitmap(videoPage,Video_Mode_Realtime,wxBITMAP(visual_realtime),wxSize(20,20));
	realtime->SetToolTip(_("Toggle realtime display of changes."));
	bool isRealtime = Options.AsBool(_T("Video Visual Realtime"));
	realtime->SetValue(isRealtime);
	wxSizer *typeSizer = new wxBoxSizer(wxVERTICAL);
	typeSizer->Add(standard,0,wxEXPAND,0);
	typeSizer->Add(drag,0,wxEXPAND,0);
	typeSizer->Add(rotatez,0,wxEXPAND,0);
	typeSizer->Add(rotatexy,0,wxEXPAND,0);
	typeSizer->Add(scale,0,wxEXPAND,0);
	typeSizer->Add(clip,0,wxEXPAND | wxBOTTOM,5);
	typeSizer->Add(realtime,0,wxEXPAND,0);
	#if USE_FEXTRACKER == 1
	typeSizer->Add(new wxStaticLine(videoPage),0,wxEXPAND|wxBOTTOM|wxTOP,5);
	typeSizer->Add(VideoTrackerMenuButton,0,wxEXPAND,0);
	typeSizer->Add(VideoTrackerMenu2Button,0,wxEXPAND,0);
	#endif
	typeSizer->AddStretchSpacer(1);

	// Top sizer
	wxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(typeSizer,0,wxEXPAND,0);
	topSizer->Add(videoDisplay,1,wxEXPAND,0);

	// Sizers
	videoSliderSizer = new wxBoxSizer(wxHORIZONTAL);
	videoSliderSizer->Add(videoSlider,1,wxEXPAND|wxLEFT,0);
	wxSizer *videoBottomSizer = new wxBoxSizer(wxHORIZONTAL);
	//videoBottomSizer->Add(zoomSlider,1,wxEXPAND,0);
	videoBottomSizer->Add(VideoPlayButton,0,wxTOP|wxLEFT|wxBOTTOM|wxALIGN_CENTER,2);
	videoBottomSizer->Add(VideoPlayLineButton,0,wxTOP|wxBOTTOM|wxALIGN_CENTER,2);
	videoBottomSizer->Add(VideoStopButton,0,wxTOP|wxBOTTOM|wxALIGN_CENTER,2);
	videoBottomSizer->Add(AutoScroll,0,wxTOP|wxBOTTOM|wxALIGN_CENTER|wxEXPAND,2);
	videoBottomSizer->Add(VideoPosition,1,wxLEFT|wxALIGN_CENTER,5);
	videoBottomSizer->Add(VideoSubsPos,1,wxALIGN_CENTER,0);
	VideoSizer = new wxBoxSizer(wxVERTICAL);
	VideoSizer->Add(topSizer,1,wxEXPAND,0);
	VideoSizer->Add(videoSliderSizer,0,wxEXPAND,0);
	VideoSizer->Add(videoBottomSizer,0,wxEXPAND,0);
	SetSizer(VideoSizer);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoBox, wxPanel)
	EVT_BUTTON(Video_Play, VideoBox::OnVideoPlay)
	EVT_BUTTON(Video_Play_Line, VideoBox::OnVideoPlayLine)
	EVT_BUTTON(Video_Stop, VideoBox::OnVideoStop)
	EVT_TOGGLEBUTTON(Video_Auto_Scroll, VideoBox::OnVideoToggleScroll)

	EVT_BUTTON(Video_Mode_Standard, VideoBox::OnModeStandard)
	EVT_BUTTON(Video_Mode_Drag, VideoBox::OnModeDrag)
	EVT_BUTTON(Video_Mode_Rotate_Z, VideoBox::OnModeRotateZ)
	EVT_BUTTON(Video_Mode_Rotate_XY, VideoBox::OnModeRotateXY)
	EVT_BUTTON(Video_Mode_Scale, VideoBox::OnModeScale)
	EVT_BUTTON(Video_Mode_Clip, VideoBox::OnModeClip)
	EVT_TOGGLEBUTTON(Video_Mode_Realtime, VideoBox::OnToggleRealtime)

#if USE_FEXTRACKER == 1
	EVT_BUTTON(Video_Tracker_Menu, VideoBox::OnVideoTrackerMenu)
	EVT_BUTTON(Video_Tracker_Menu2, VideoBox::OnVideoTrackerMenu2)
	EVT_MENU_RANGE(Video_Tracker_START,Video_Tracker_END, VideoBox::OnTrackerOption)
#endif
END_EVENT_TABLE()


//////////////
// Play video
void VideoBox::OnVideoPlay(wxCommandEvent &event) {
	VideoContext::Get()->Play();
}


///////////////////
// Play video line
void VideoBox::OnVideoPlayLine(wxCommandEvent &event) {
	VideoContext::Get()->PlayLine();
}


//////////////
// Stop video
void VideoBox::OnVideoStop(wxCommandEvent &event) {
	VideoContext::Get()->Stop();
}


/////////////////////
// Toggle autoscroll
void VideoBox::OnVideoToggleScroll(wxCommandEvent &event) {
	Options.SetBool(_T("Sync video with subs"),AutoScroll->GetValue());
	Options.Save();
}


/////////////////
// Standard mode
void VideoBox::OnModeStandard(wxCommandEvent &event) {
	videoDisplay->visual->SetMode(0);
}


/////////////
// Drag mode
void VideoBox::OnModeDrag(wxCommandEvent &event) {
	videoDisplay->visual->SetMode(1);
}


/////////////////
// Rotate Z mode
void VideoBox::OnModeRotateZ(wxCommandEvent &event) {
	videoDisplay->visual->SetMode(2);
}


//////////////////
// Rotate XY mode
void VideoBox::OnModeRotateXY(wxCommandEvent &event) {
	videoDisplay->visual->SetMode(3);
}


//////////////
// Scale mode
void VideoBox::OnModeScale(wxCommandEvent &event) {
	videoDisplay->visual->SetMode(4);
}


/////////////
// Clip mode
void VideoBox::OnModeClip(wxCommandEvent &event) {
	videoDisplay->visual->SetMode(5);
}


///////////////////
// Realtime toggle
void VideoBox::OnToggleRealtime(wxCommandEvent &event) {
	Options.SetBool(_T("Video Visual Realtime"),realtime->GetValue());
	Options.Save();
}



#if USE_FEXTRACKER == 1
///////////////////
// Tracker Menu
void VideoBox::OnVideoTrackerMenu(wxCommandEvent &event) {
	wxMenu menu( _("FexTracker") );
	AppendBitmapMenuItem(&menu, Video_Track_Points, _("Track points"), _T(""), wxBITMAP(button_track_points));
	menu.AppendSeparator();
	AppendBitmapMenuItem(&menu, Video_Track_Point_Add, _("Add points to movement"), _T(""), wxBITMAP(button_track_point_add));
	AppendBitmapMenuItem(&menu, Video_Track_Point_Del, _("Remove points from movement"), _T(""), wxBITMAP(button_track_point_del));
	menu.AppendSeparator();
	AppendBitmapMenuItem(&menu, Video_Track_Movement, _("Generate movement from points"), _T(""), wxBITMAP(button_track_movement));
	PopupMenu(&menu);
}

	
///////////////////
// Movement Menu
void VideoBox::OnVideoTrackerMenu2(wxCommandEvent &event) {
	wxMenu menu( _("FexMovement") );
	AppendBitmapMenuItem(&menu, Video_Track_Movement_Empty, _("Generate empty movement"), _T(""), wxBITMAP(button_track_move));
	menu.AppendSeparator();
	AppendBitmapMenuItem(&menu, Video_Track_Movement_MoveAll, _("Move subtitle"), _T(""), wxBITMAP(button_track_move));
	menu.AppendSeparator();
	AppendBitmapMenuItem(&menu, Video_Track_Movement_MoveBefore, _("Move subtitle (this frame and preceeding frames)"), _T(""), wxBITMAP(button_track_move));
	AppendBitmapMenuItem(&menu, Video_Track_Movement_MoveOne, _("Move subtitle (this frame)"), _T(""), wxBITMAP(button_track_move));
	AppendBitmapMenuItem(&menu, Video_Track_Movement_MoveAfter, _("Move subtitle (this frame and following frames)"), _T(""), wxBITMAP(button_track_move));
	menu.AppendSeparator();
	AppendBitmapMenuItem(&menu, Video_Track_Split_Line, _("Split line for movement"), _T(""), wxBITMAP(button_track_split_line));
	menu.AppendSeparator();
	AppendBitmapMenuItem(&menu, Video_Track_Link_File, _("Link movement file"), _T(""), wxBITMAP(button_track_move));
	PopupMenu(&menu);
}


////////////////////
// Forward options
void VideoBox::OnTrackerOption(wxCommandEvent &event) {
	videoDisplay->tracker->AddPendingEvent(event);
}

#endif
