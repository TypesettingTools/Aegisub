// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////////////
// Include headers
#include "setup.h"
#if USE_FEXTRACKER == 1
#include <wx/wxprec.h>
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/tglbtn.h>
#include <wx/rawbmp.h>
#include "subs_grid.h"
#include "frame_main.h"
#include "video_provider.h"
#include "video_display.h"
#include "video_box.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "options.h"
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#include "dialog_progress.h"
#include "dialog_fextracker.h"



///////////////////
// Tracker Menu
void FrameMain::OnVideoTrackerMenu(wxCommandEvent &event) {
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
void FrameMain::OnVideoTrackerMenu2(wxCommandEvent &event) {
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
void FrameMain::OnVideoTrackPoints(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();

	// Get line
	AssDialogue *curline = SubsBox->GetDialogue(EditBox->linen);
	if (!curline) return;

	FexTrackerConfig config;
	DialogFexTracker configDlg( this, &config );
	configDlg.ShowModal();

	if( !config.FeatureNumber ) return;

	// Get Video
	VideoProvider *movie = VideoProvider::GetProvider(videoBox->videoDisplay->videoName, wxString(_T("")));

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

	videoBox->videoDisplay->RefreshVideo();
}


///////////////////
// Track current line
void FrameMain::OnVideoTrackMovement(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();

	// Get line
	AssDialogue *curline = SubsBox->GetDialogue(EditBox->linen);
	if (!curline) return;
	if( !curline->Tracker ) return;

	// Create Movement
	if( curline->Movement ) DeleteMovement( curline->Movement );
	curline->Movement = curline->Tracker->GetMovement();

	videoBox->videoDisplay->RefreshVideo();
}


///////////////////
// split current line
void FrameMain::OnVideoTrackSplitLine(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();

	// Get line
	AssDialogue *curline = SubsBox->GetDialogue(EditBox->linen);
	if (!curline) return;
	if( !curline->Movement ) return;

	// Create split lines
	int StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	int EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);

	AssFile *subs = AssFile::top;
	int ResXValue,ResYValue;
	swscanf( subs->GetScriptInfo(_T("PlayResX")), _T("%d"), &ResXValue );
	swscanf( subs->GetScriptInfo(_T("PlayResY")), _T("%d"), &ResYValue );
	int SrcXValue = videoBox->videoDisplay->provider->GetSourceWidth();
	int SrcYValue = videoBox->videoDisplay->provider->GetSourceHeight();

	float sx = float(ResXValue)/float(SrcXValue);
	float sy = float(ResYValue)/float(SrcYValue);

	for( int Frame = StartFrame; Frame < EndFrame; Frame ++ )
	{
		int localframe = Frame - StartFrame;

		while( curline->Movement->Frames.size() <= localframe ) localframe--;
		FexMovementFrame f = curline->Movement->Frames[localframe];
//		f.Pos.x /= videoBox->videoDisplay->GetW

		AssDialogue *cur = new AssDialogue( curline->GetEntryData() );
		cur->Start.SetMS(VFR_Output.GetTimeAtFrame(Frame,true));
		cur->End.SetMS(VFR_Output.GetTimeAtFrame(Frame,false));
		cur->Text = wxString::Format( _T("{\\pos(%.0f,%.0f)\\fscx%.2f\\fscy%.2f}"), f.Pos.x*sx, f.Pos.y*sy, f.Scale.x*100, f.Scale.y*100 ) + cur->Text;
		cur->UpdateData();

		SubsBox->InsertLine(cur,EditBox->linen + Frame - StartFrame,true,false);
	}

	// Remove Movement
	DeleteMovement( curline->Movement );
	curline->Movement = 0;

	// Remove Tracker
	delete curline->Tracker;
	curline->Tracker = 0;

	// Remove this line
	SubsBox->DeleteLines(SubsBox->GetRangeArray(EditBox->linen, EditBox->linen));

	videoBox->videoDisplay->RefreshVideo();
}



///////////////////
// generate empty movement
void FrameMain::OnVideoTrackMovementEmpty(wxCommandEvent &event) {
	// Get line
	AssDialogue *curline = SubsBox->GetDialogue(EditBox->linen);
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
void FrameMain::OnVideoTrackLinkFile(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();

	// Get line
	AssDialogue *curline = SubsBox->GetDialogue(EditBox->linen);
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
void FrameMain::OnVideoTrackPointAdd(wxCommandEvent &event) {
	videoBox->videoDisplay->TrackerEdit = 1;
	videoBox->videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Decrease Influence
void FrameMain::OnVideoTrackPointDel(wxCommandEvent &event) {
	videoBox->videoDisplay->TrackerEdit = -1;
	videoBox->videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move All
void FrameMain::OnVideoTrackMovementMoveAll(wxCommandEvent &event) {
	videoBox->videoDisplay->MovementEdit = 1;
	videoBox->videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move One
void FrameMain::OnVideoTrackMovementMoveOne(wxCommandEvent &event) {
	videoBox->videoDisplay->MovementEdit = 2;
	videoBox->videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move Before
void FrameMain::OnVideoTrackMovementMoveBefore(wxCommandEvent &event) {
	videoBox->videoDisplay->MovementEdit = 3;
	videoBox->videoDisplay->bTrackerEditing = 0;
}


///////////////////
// Move After
void FrameMain::OnVideoTrackMovementMoveAfter(wxCommandEvent &event) {
	videoBox->videoDisplay->MovementEdit = 4;
	videoBox->videoDisplay->bTrackerEditing = 0;
}

#endif
