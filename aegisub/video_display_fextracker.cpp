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


////////////////////////////// HERE BE DRAGONS //////////////////////////////


///////////
// Headers
#include "setup.h"
#if USE_FEXTRACKER == 1
#include "video_display_fextracker.h"
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#include "dialog_progress.h"
#include "dialog_fextracker.h"
#include "ass_dialogue.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "subs_grid.h"
#include "subs_edit_box.h"
#include "vfr.h"
#include "main.h"
#include "frame_main.h"
#include "video_provider.h"
#include "ass_file.h"


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoDisplayFexTracker,wxEvtHandler)
	EVT_MENU(Video_Track_Points, VideoDisplayFexTracker::OnVideoTrackPoints)
	EVT_MENU(Video_Track_Point_Add, VideoDisplayFexTracker::OnVideoTrackPointAdd)
	EVT_MENU(Video_Track_Point_Del, VideoDisplayFexTracker::OnVideoTrackPointDel)
	EVT_MENU(Video_Track_Movement, VideoDisplayFexTracker::OnVideoTrackMovement)
	EVT_MENU(Video_Track_Movement_MoveAll, VideoDisplayFexTracker::OnVideoTrackMovementMoveAll)
	EVT_MENU(Video_Track_Movement_MoveOne, VideoDisplayFexTracker::OnVideoTrackMovementMoveOne)
	EVT_MENU(Video_Track_Movement_MoveBefore, VideoDisplayFexTracker::OnVideoTrackMovementMoveBefore)
	EVT_MENU(Video_Track_Movement_MoveAfter, VideoDisplayFexTracker::OnVideoTrackMovementMoveAfter)
	EVT_MENU(Video_Track_Split_Line, VideoDisplayFexTracker::OnVideoTrackSplitLine)
	EVT_MENU(Video_Track_Link_File, VideoDisplayFexTracker::OnVideoTrackLinkFile)
	EVT_MENU(Video_Track_Movement_Empty, VideoDisplayFexTracker::OnVideoTrackMovementEmpty)
END_EVENT_TABLE()


///////////////
// Constructor
VideoDisplayFexTracker::VideoDisplayFexTracker(VideoDisplay *par) {
	parent = par;
}


///////////////
// Mouse event
void VideoDisplayFexTracker::OnMouseEvent(wxMouseEvent &event) {
	// Variables
	int frame_n = VideoContext::Get()->GetFrameN();
	int dw,dh;
	parent->GetClientSize(&dw,&dh);
	int x = event.GetX();
	int y = event.GetY();
	int mx = x * VideoContext::Get()->GetWidth() / dw;
	int my = y * VideoContext::Get()->GetHeight() / dh;

	// Click
	if (event.ButtonDown(wxMOUSE_BTN_LEFT)) {
		MouseDownX = mx;
		MouseDownY = my;
		bTrackerEditing = 1;
	}
	if (event.ButtonUp(wxMOUSE_BTN_LEFT)) bTrackerEditing = 0;

	// Do tracker influence if needed
	if( bTrackerEditing ) {
		AssDialogue *curline = VideoContext::Get()->grid->GetDialogue(VideoContext::Get()->grid->editBox->linen);
		int StartFrame, EndFrame, localframe;

		// Visible?
		if (curline && (StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true)) <= frame_n	&& (EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false)) >= frame_n ) {
			localframe = frame_n - StartFrame;
			if (TrackerEdit!=0 && curline->Tracker && localframe < curline->Tracker->GetFrame())
				curline->Tracker->InfluenceFeatures (localframe, float(mx), float(my), TrackerEdit);
			if (MovementEdit!=0 && curline->Movement && localframe < curline->Movement->Frames.size()) {
				// Set start/end
				int movMode = MovementEdit;
				int start = 0;
				int end = localframe+1;
				if (movMode == 2 || movMode == 4) start = localframe;
				if (movMode == 1 || movMode == 4) end = curline->Movement->Frames.size();

				// Apply
				for (int i=0;i<curline->Movement->Frames.size();i++) {
					curline->Movement->Frames[i].Pos.x += float(mx-MouseDownX);
					curline->Movement->Frames[i].Pos.y += float(my-MouseDownY);
				}
			}
			MouseDownX = mx;
			MouseDownY = my;
		}
	}
}


/////////////////////////
// Draw tracking overlay
void VideoDisplayFexTracker::Render() {
	int frame_n = VideoContext::Get()->GetFrameN();

	// Get line
	AssDialogue *curline = VideoContext::Get()->grid->GetDialogue(VideoContext::Get()->grid->editBox->linen);
	if (!curline) return;

	int StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	int EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);
	if (frame_n<StartFrame || frame_n>EndFrame) return;

	int localframe = frame_n - StartFrame;

	if (curline->Tracker) {
		if (curline->Tracker->GetFrame() <= localframe) return;

		// Draw ticks
		for (int i=0;i<curline->Tracker->GetCount();i++) {
			FexTrackingFeature* f = (*curline->Tracker)[i];
			if (f->StartTime > localframe) continue;
			int llf = localframe - f->StartTime;
			if (f->Pos.size() <= llf) continue;
			vec2 pt = f->Pos[llf];

			SetLineColour(wxColour(255*(1-f->Influence),255*f->Influence,0),1);

			DrawLine (pt.x-2, pt.y, pt.x, pt.y);
			DrawLine (pt.x, pt.y-2, pt.x, pt.y);
			DrawLine (pt.x+1, pt.y, pt.x+3, pt.y);
			DrawLine (pt.x, pt.y+1, pt.x, pt.y+3);
		}
	}

	if (curline->Movement) {
		if (curline->Movement->Frames.size() <= localframe) return;

		FexMovementFrame f = curline->Movement->Frames.lVal[localframe];
		f.Scale.x *= 30;
		f.Scale.y *= 30;

		FexMovementFrame f3 = f;
		SetLineColour(wxColour(0,0,255),1);
		int nBack = 8;
		while (--localframe>0 && nBack-- >0) {
			FexMovementFrame f2 = curline->Movement->Frames.lVal[localframe];
			DrawLine (f2.Pos.x, f2.Pos.y, f3.Pos.x, f3.Pos.y);
			f3 = f2;
		}

		SetLineColour(wxColour(200,0,0),2);
		DrawLine (f.Pos.x-f.Scale.x, f.Pos.y, f.Pos.x+f.Scale.x+1, f.Pos.y);
		DrawLine (f.Pos.x, f.Pos.y-f.Scale.y, f.Pos.x, f.Pos.y+f.Scale.y+1);

		f3 = f;
		SetLineColour(wxColour(0,255,0),1);
		int nFront = 8;
		localframe = frame_n - StartFrame;
		while( ++localframe<curline->Movement->Frames.size() && nFront-- >0 ) {
			FexMovementFrame f2 = curline->Movement->Frames.lVal[localframe];
			DrawLine (f2.Pos.x, f2.Pos.y, f3.Pos.x, f3.Pos.y);
			f3 = f2;
		}
	}
}

	
///////////////////
// Track current line
void VideoDisplayFexTracker::OnVideoTrackPoints(wxCommandEvent &event) {
	VideoContext::Get()->Stop();

	// Get line
	FrameMain *frame = AegisubApp::Get()->frame;
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;

	FexTrackerConfig config;
	DialogFexTracker configDlg (frame, &config);
	configDlg.ShowModal();

	if (!config.FeatureNumber) return;

	// Get Video
	VideoProvider *movie = VideoProviderFactory::GetProvider(VideoContext::Get()->videoName);

	// Create Tracker
	if( curline->Tracker ) delete curline->Tracker;
	curline->Tracker = new FexTracker( movie->GetWidth(), movie->GetHeight(), config.FeatureNumber );
	curline->Tracker->minFeatures = config.FeatureNumber;
	curline->Tracker->Cfg = config;

	// Start progress
	volatile bool canceled = false;
	DialogProgress *progress = new DialogProgress(frame,_("FexTracker"),&canceled,_("Tracking points"),0,1);
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

	VideoContext::Get()->Refresh(true,false);
}


///////////////////
// Track current line
void VideoDisplayFexTracker::OnVideoTrackMovement(wxCommandEvent &event) {
	VideoContext::Get()->Stop();

	// Get line
	FrameMain *frame = AegisubApp::Get()->frame;
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;
	if( !curline->Tracker ) return;

	// Create Movement
	if( curline->Movement ) DeleteMovement( curline->Movement );
	curline->Movement = curline->Tracker->GetMovement();

	VideoContext::Get()->Refresh(true,false);
}


///////////////////
// split current line
void VideoDisplayFexTracker::OnVideoTrackSplitLine(wxCommandEvent &event) {
	VideoContext::Get()->Stop();

	// Get line
	FrameMain *frame = AegisubApp::Get()->frame;
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
	int SrcXValue = VideoContext::Get()->GetWidth();
	int SrcYValue = VideoContext::Get()->GetHeight();

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

	VideoContext::Get()->Refresh(true,false);
}


///////////////////
// generate empty movement
void VideoDisplayFexTracker::OnVideoTrackMovementEmpty(wxCommandEvent &event) {
	// Get line
	FrameMain *frame = AegisubApp::Get()->frame;
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
void VideoDisplayFexTracker::OnVideoTrackLinkFile(wxCommandEvent &event) {
	VideoContext::Get()->Stop();

	// Get line
	FrameMain *frame = AegisubApp::Get()->frame;
	AssDialogue *curline = frame->SubsBox->GetDialogue(frame->EditBox->linen);
	if (!curline) return;

	wxString link = wxGetTextFromUser(_("Link name:"), _("Link line to movement file"), curline->Movement?curline->Movement->FileName:_T(""), frame);
	if( link.empty() ) curline->Effect = _T("");
	else curline->Effect = _T("FexMovement:")+link;
	
	curline->UpdateData();

	if( !curline->Effect.empty() && curline->Movement )
		SaveMovement( curline->Movement, curline->Effect.AfterFirst(':').c_str() );
}


//////////////////////
// Increase Influence
void VideoDisplayFexTracker::OnVideoTrackPointAdd(wxCommandEvent &event) {
	TrackerEdit = 1;
	bTrackerEditing = 0;
}


//////////////////////
// Decrease Influence
void VideoDisplayFexTracker::OnVideoTrackPointDel(wxCommandEvent &event) {
	TrackerEdit = -1;
	bTrackerEditing = 0;
}


////////////
// Move All
void VideoDisplayFexTracker::OnVideoTrackMovementMoveAll(wxCommandEvent &event) {
	MovementEdit = 1;
	bTrackerEditing = 0;
}


////////////
// Move One
void VideoDisplayFexTracker::OnVideoTrackMovementMoveOne(wxCommandEvent &event) {
	MovementEdit = 2;
	bTrackerEditing = 0;
}


///////////////
// Move Before
void VideoDisplayFexTracker::OnVideoTrackMovementMoveBefore(wxCommandEvent &event) {
	MovementEdit = 3;
	bTrackerEditing = 0;
}


//////////////
// Move After
void VideoDisplayFexTracker::OnVideoTrackMovementMoveAfter(wxCommandEvent &event) {
	MovementEdit = 4;
	bTrackerEditing = 0;
}

#endif
