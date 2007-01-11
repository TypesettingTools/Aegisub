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
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#include "dialog_progress.h"
#include "dialog_fextracker.h"
#include "utils.h"
#include "main.h"


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
	wxBitmapButton *VideoTrackerMenuButton = new wxBitmapButton(videoPage,Video_Tracker_Menu,wxBITMAP(button_track_points),wxDefaultPosition,wxSize(25,-1));
	VideoTrackerMenuButton->SetToolTip(_("FexTracker"));
	wxBitmapButton *VideoTrackerMenu2Button = new wxBitmapButton(videoPage,Video_Tracker_Menu2,wxBITMAP(button_track_trail),wxDefaultPosition,wxSize(25,-1));
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
	videoDisplay->Reset();

	// Set display
	videoSlider->Display = videoDisplay;
	
	// Typesetting buttons
	standard = new wxButton(videoPage,Video_Mode_Standard,_T("n"),wxDefaultPosition,wxSize(20,20));
	standard->SetToolTip(_("Standard mode, double click sets position."));
	drag = new wxButton(videoPage,Video_Mode_Drag,_T("d"),wxDefaultPosition,wxSize(20,20));
	drag->SetToolTip(_("Drag subtitles."));
	rotatez = new wxButton(videoPage,Video_Mode_Rotate_Z,_T("z"),wxDefaultPosition,wxSize(20,20));
	rotatez->SetToolTip(_("Rotate subtitles on their Z axis."));
	rotatexy = new wxButton(videoPage,Video_Mode_Rotate_XY,_T("x"),wxDefaultPosition,wxSize(20,20));
	rotatexy->SetToolTip(_("Rotate subtitles on their X and Y axes."));
	scale = new wxButton(videoPage,Video_Mode_Scale,_T("s"),wxDefaultPosition,wxSize(20,20));
	scale->SetToolTip(_("Scale subtitles on X and Y axes."));
	clip = new wxButton(videoPage,Video_Mode_Clip,_T("c"),wxDefaultPosition,wxSize(20,20));
	clip->SetToolTip(_("Clip subtitles to a rectangle."));
	realtime = new wxToggleButton(videoPage,Video_Mode_Realtime,_T("r"),wxDefaultPosition,wxSize(20,20));
	realtime->SetToolTip(_("Toggle realtime display of changes."));
	bool isRealtime = Options.AsBool(_T("Video Visual Realtime"));
	realtime->SetValue(isRealtime);
	wxSizer *typeSizer = new wxBoxSizer(wxVERTICAL);
	typeSizer->Add(standard,0,0,0);
	typeSizer->Add(drag,0,0,0);
	typeSizer->Add(rotatez,0,0,0);
	typeSizer->Add(rotatexy,0,0,0);
	typeSizer->Add(scale,0,0,0);
	typeSizer->Add(clip,0,wxBOTTOM,5);
	typeSizer->Add(realtime,0,0,0);
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
	#if USE_FEXTRACKER == 1
	videoBottomSizer->Add(VideoTrackerMenuButton,0,wxTOP|wxBOTTOM|wxALIGN_CENTER|wxEXPAND,2);
	videoBottomSizer->Add(VideoTrackerMenu2Button,0,wxTOP|wxBOTTOM|wxALIGN_CENTER|wxEXPAND,2);
	#endif
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
	EVT_MENU(Video_Track_Points, VideoBox::OnVideoTrackPoints)
	EVT_MENU(Video_Track_Point_Add, VideoBox::OnVideoTrackPointAdd)
	EVT_MENU(Video_Track_Point_Del, VideoBox::OnVideoTrackPointDel)
	EVT_MENU(Video_Track_Movement, VideoBox::OnVideoTrackMovement)
	EVT_BUTTON(Video_Tracker_Menu2, VideoBox::OnVideoTrackerMenu2)
	EVT_MENU(Video_Track_Movement_MoveAll, VideoBox::OnVideoTrackMovementMoveAll)
	EVT_MENU(Video_Track_Movement_MoveOne, VideoBox::OnVideoTrackMovementMoveOne)
	EVT_MENU(Video_Track_Movement_MoveBefore, VideoBox::OnVideoTrackMovementMoveBefore)
	EVT_MENU(Video_Track_Movement_MoveAfter, VideoBox::OnVideoTrackMovementMoveAfter)
	EVT_MENU(Video_Track_Split_Line, VideoBox::OnVideoTrackSplitLine)
	EVT_MENU(Video_Track_Link_File, VideoBox::OnVideoTrackLinkFile)
	EVT_MENU(Video_Track_Movement_Empty, VideoBox::OnVideoTrackMovementEmpty)
#endif
END_EVENT_TABLE()


//////////////
// Play video
void VideoBox::OnVideoPlay(wxCommandEvent &event) {
	videoDisplay->Play();
}


///////////////////
// Play video line
void VideoBox::OnVideoPlayLine(wxCommandEvent &event) {
	videoDisplay->PlayLine();
}


//////////////
// Stop video
void VideoBox::OnVideoStop(wxCommandEvent &event) {
	videoDisplay->Stop();
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



/////////////////////// HERE BE DRAGONS //////////////////////////////

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

	
///////////////////
// Track current line
void VideoBox::OnVideoTrackPoints(wxCommandEvent &event) {
	videoDisplay->Stop();

	// Get line
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;

	FexTrackerConfig config;
	DialogFexTracker configDlg( this, &config );
	configDlg.ShowModal();

	if( !config.FeatureNumber ) return;

	// Get Video
	VideoProvider *movie = VideoProvider::GetProvider(videoDisplay->videoName, wxString(_T("")));

	// Create Tracker
	if( curline->Tracker ) delete curline->Tracker;
	curline->Tracker = new FexTracker( movie->GetWidth(), movie->GetHeight(), config.FeatureNumber );
	curline->Tracker->minFeatures = config.FeatureNumber;
	curline->Tracker->Cfg = config;

	// Start progress
	volatile bool canceled = false;
	DialogProgress *progress = new DialogProgress(this,_("FexTracker"),&canceled,_("Tracking points"),0,1);
	progress->Show();

	// Allocate temp image
	float* FloatImg = new float[ movie->GetWidth()*movie->GetHeight() ];

	int StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	int EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);

	for( int Frame = StartFrame; Frame <= EndFrame; Frame ++ )
	{
		progress->SetProgress( Frame-StartFrame, EndFrame-StartFrame );
		if( canceled ) break;

		movie->GetFloatFrame( FloatImg, Frame );
		curline->Tracker->ProcessImage( FloatImg );
	}

	delete FloatImg;
	delete movie;

	// Clean up progress
	if (!canceled) 
		progress->Destroy();
	else
	{
		delete curline->Tracker;
		curline->Tracker = 0;
	}

	videoDisplay->RefreshVideo();
}


///////////////////
// Track current line
void VideoBox::OnVideoTrackMovement(wxCommandEvent &event) {
	videoDisplay->Stop();

	// Get line
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;
	if( !curline->Tracker ) return;

	// Create Movement
	if( curline->Movement ) DeleteMovement( curline->Movement );
	curline->Movement = curline->Tracker->GetMovement();

	videoDisplay->RefreshVideo();
}


///////////////////
// split current line
void VideoBox::OnVideoTrackSplitLine(wxCommandEvent &event) {
	videoDisplay->Stop();

	// Get line
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;
	if( !curline->Movement ) return;

	// Create split lines
	int StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	int EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);

	AssFile *subs = AssFile::top;
	int ResXValue,ResYValue;
	swscanf( subs->GetScriptInfo(_T("PlayResX")), _T("%d"), &ResXValue );
	swscanf( subs->GetScriptInfo(_T("PlayResY")), _T("%d"), &ResYValue );
	int SrcXValue = videoDisplay->provider->GetSourceWidth();
	int SrcYValue = videoDisplay->provider->GetSourceHeight();

	float sx = float(ResXValue)/float(SrcXValue);
	float sy = float(ResYValue)/float(SrcYValue);

	for( int Frame = StartFrame; Frame < EndFrame; Frame ++ )
	{
		int localframe = Frame - StartFrame;

		while( curline->Movement->Frames.size() <= localframe ) localframe--;
		FexMovementFrame f = curline->Movement->Frames[localframe];
//		f.Pos.x /= videoDisplay->GetW

		AssDialogue *cur = new AssDialogue( curline->GetEntryData() );
		cur->Start.SetMS(VFR_Output.GetTimeAtFrame(Frame,true));
		cur->End.SetMS(VFR_Output.GetTimeAtFrame(Frame,false));
		cur->Text = wxString::Format( _T("{\\pos(%.0f,%.0f)\\fscx%.2f\\fscy%.2f}"), f.Pos.x*sx, f.Pos.y*sy, f.Scale.x*100, f.Scale.y*100 ) + cur->Text;
		cur->UpdateData();

		frame->SubsBox->InsertLine(cur,frame->EditBox->linen + Frame - StartFrame,true,false);
	}

	// Remove Movement
	DeleteMovement( curline->Movement );
	curline->Movement = 0;

	// Remove Tracker
	delete curline->Tracker;
	curline->Tracker = 0;

	// Remove this line
	frame->SubsBox->DeleteLines(frame->SubsBox->GetRangeArray(frame->EditBox->linen, frame->EditBox->linen));

	videoDisplay->RefreshVideo();
}


///////////////////
// generate empty movement
void VideoBox::OnVideoTrackMovementEmpty(wxCommandEvent &event) {
	// Get line
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;
	if( curline->Movement ) DeleteMovement( curline->Movement );
	curline->Movement = CreateMovement();

	// Create split lines
	int StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	int EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);

	FexMovementFrame f;
	memset( &f, 0x00, sizeof(f) );
	f.Scale.x = f.Scale.y = 1;

	for( int i=StartFrame;i<EndFrame;i++ )
		curline->Movement->Frames.Add( f );
}


///////////////////
// link line to move file
void VideoBox::OnVideoTrackLinkFile(wxCommandEvent &event) {
	videoDisplay->Stop();

	// Get line
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;

	wxString link = wxGetTextFromUser(_("Link name:"), _("Link line to movement file"), curline->Movement?curline->Movement->FileName:_T(""), this);
	if( link.empty() ) curline->Effect = _T("");
	else curline->Effect = _T("FexMovement:")+link;
	
	curline->UpdateData();

	if( !curline->Effect.empty() && curline->Movement )
		SaveMovement( curline->Movement, curline->Effect.AfterFirst(':').c_str() );
}


///////////////////
// Increase Influence
void VideoBox::OnVideoTrackPointAdd(wxCommandEvent &event) {
	videoDisplay->TrackerEdit = 1;
	videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Decrease Influence
void VideoBox::OnVideoTrackPointDel(wxCommandEvent &event) {
	videoDisplay->TrackerEdit = -1;
	videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move All
void VideoBox::OnVideoTrackMovementMoveAll(wxCommandEvent &event) {
	videoDisplay->MovementEdit = 1;
	videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move One
void VideoBox::OnVideoTrackMovementMoveOne(wxCommandEvent &event) {
	videoDisplay->MovementEdit = 2;
	videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move Before
void VideoBox::OnVideoTrackMovementMoveBefore(wxCommandEvent &event) {
	videoDisplay->MovementEdit = 3;
	videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move After
void VideoBox::OnVideoTrackMovementMoveAfter(wxCommandEvent &event) {
	videoDisplay->MovementEdit = 4;
	videoDisplay->bTrackerEditing = 0;
}

#endif
