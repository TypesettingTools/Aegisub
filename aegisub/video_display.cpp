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


////////////
// Includes
#include "setup.h"
#include <wx/image.h>
#include <string.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/config.h>
#include "utils.h"
#include "video_display.h"
#include "video_display_visual.h"
#include "video_provider.h"
#include "vfr.h"
#include "ass_file.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "subs_grid.h"
#include "vfw_wrap.h"
#include "mkv_wrap.h"
#include "options.h"
#include "subs_edit_box.h"
#include "audio_display.h"
#include "main.h"
#include "video_slider.h"
#if USE_FEXTRACKER == 1
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#endif


///////
// IDs
enum {
	VIDEO_MENU_COPY_TO_CLIPBOARD = 1230,
	VIDEO_MENU_COPY_COORDS,
	VIDEO_MENU_SAVE_SNAPSHOT,
	VIDEO_PLAY_TIMER
};


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoDisplay, wxWindow)
    EVT_MOUSE_EVENTS(VideoDisplay::OnMouseEvent)
	EVT_KEY_DOWN(VideoDisplay::OnKey)
	EVT_LEAVE_WINDOW(VideoDisplay::OnMouseLeave)
    EVT_PAINT(VideoDisplay::OnPaint)

	EVT_TIMER(VIDEO_PLAY_TIMER,VideoDisplay::OnPlayTimer)

	EVT_MENU(VIDEO_MENU_COPY_TO_CLIPBOARD,VideoDisplay::OnCopyToClipboard)
	EVT_MENU(VIDEO_MENU_SAVE_SNAPSHOT,VideoDisplay::OnSaveSnapshot)
	EVT_MENU(VIDEO_MENU_COPY_COORDS,VideoDisplay::OnCopyCoords)
END_EVENT_TABLE()


///////////////
// Constructor
VideoDisplay::VideoDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
             : wxWindow (parent, id, pos, size, style, name)
{
	audio = NULL;
	provider = NULL;
	curLine = NULL;
	ControlSlider = NULL;
	PositionDisplay = NULL;
	loaded = false;
	keyFramesLoaded = false;
	overKeyFramesLoaded = false;
	frame_n = 0;
	origSize = size;
	arType = 0;
	IsPlaying = false;
	threaded = Options.AsBool(_T("Threaded Video"));
	nextFrame = -1;
	zoomValue = 0.5;
	visual = new VideoDisplayVisual(this);
}


//////////////
// Destructor
VideoDisplay::~VideoDisplay () {
	wxRemoveFile(tempfile);
	tempfile = _T("");
	SetVideo(_T(""));
	delete visual;
}

void  VideoDisplay::UpdateSize() {
	if (provider) {
		w = provider->GetWidth();
		h = provider->GetHeight();

		// Set the size for this control
		SetSizeHints(w,h,w,h);
		SetClientSize(w,h);
		int _w,_h;
		GetSize(&_w,&_h);
		SetSizeHints(_w,_h,_w,_h);
	}
}


//////////
// Resets
void VideoDisplay::Reset() {
	w = origSize.GetX();
	h = origSize.GetY();
	SetClientSize(w,h);
	int _w,_h;
	GetSize(&_w,&_h);
	SetSizeHints(_w,_h,_w,_h);

	KeyFrames.Clear();
	keyFramesLoaded = false;

	// Remove temporary audio provider
	if (audio && audio->temporary) {
		delete audio->provider;
		audio->provider = NULL;
		delete audio->player;
		audio->player = NULL;
		audio->temporary = false;
	}
}


///////////////////////
// Sets video filename
void VideoDisplay::SetVideo(const wxString &filename) {
	// Unload video
	delete provider;
	provider = NULL;
	//if (VFR_Output.GetFrameRateType() == VFR) VFR_Output.Unload();
	//VFR_Input.Unload();
	videoName = _T("");
	loaded = false;
	frame_n = 0;
	Reset();
	
	// Load video
	if (!filename.IsEmpty()) {
		try {
			grid->CommitChanges(true);
			bool isVfr = false;
			double overFps = 0;
			FrameRate temp;

			// Unload timecodes
			//int unload = wxYES;
			//if (VFR_Output.IsLoaded()) unload = wxMessageBox(_("Do you want to unload timecodes, too?"),_("Unload timecodes?"),wxYES_NO | wxICON_QUESTION);
			//if (unload == wxYES) VFR_Output.Unload();

			// Read extra data from file
			bool mkvOpen = MatroskaWrapper::wrapper.IsOpen();
			wxString ext = filename.Right(4).Lower();
			KeyFrames.Clear();
			if (ext == _T(".mkv") || mkvOpen) {
				// Parse mkv
				if (!mkvOpen) MatroskaWrapper::wrapper.Open(filename);

				// Get keyframes
				KeyFrames = MatroskaWrapper::wrapper.GetKeyFrames();
				keyFramesLoaded = true;

				// Ask to override timecodes
				int override = wxYES;
				if (VFR_Output.IsLoaded()) override = wxMessageBox(_("You already have timecodes loaded. Replace them with the timecodes from the Matroska file?"),_("Replace timecodes?"),wxYES_NO | wxICON_QUESTION);
				if (override == wxYES) {
					MatroskaWrapper::wrapper.SetToTimecodes(temp);
					isVfr = temp.GetFrameRateType() == VFR;
					if (isVfr) {
						overFps = temp.GetCommonFPS();
						MatroskaWrapper::wrapper.SetToTimecodes(VFR_Input);
	 					MatroskaWrapper::wrapper.SetToTimecodes(VFR_Output);
					}
				}

				// Close mkv
				MatroskaWrapper::wrapper.Close();
			}
#ifdef __WINDOWS__
			else if (ext == _T(".avi")) {
				KeyFrames = VFWWrapper::GetKeyFrames(filename);
				keyFramesLoaded = true;
			}
#endif

			// Choose a provider
			provider = VideoProvider::GetProvider(filename,GetTempWorkFile(),overFps);
			if (isVfr) provider->OverrideFrameTimeList(temp.GetFrameTimeList());
			provider->SetZoom(zoomValue);
			if (arType != 4) arValue = GetARFromType(arType); // 4 = custom
			provider->SetDAR(arValue);

			// Update size
			UpdateSize();

			//Gather video parameters
			length = provider->GetFrameCount();
			fps = provider->GetFPS();
			if (!isVfr) {
				VFR_Input.SetCFR(fps);
				if (VFR_Output.GetFrameRateType() != VFR) VFR_Output.SetCFR(fps);
			}

			// Set range of slider
			ControlSlider->SetRange(0,length-1);
			ControlSlider->SetValue(0);

			videoName = filename;

			// Add to recent
			Options.AddToRecentList(filename,_T("Recent vid"));

			RefreshVideo();
			UpdatePositionDisplay();
		}
		
		catch (wxString &e) {
			wxMessageBox(e,_T("Error setting video"),wxICON_ERROR | wxOK);
		}
	}

	loaded = provider != NULL;
}


/////////////////////
// Refresh subtitles
void VideoDisplay::RefreshSubtitles() {
	provider->RefreshSubtitles();
	RefreshVideo();
}


/////////////////
// OnPaint event
void VideoDisplay::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);

	// Draw frame
	if (provider) dc.DrawBitmap(GetFrame(frame_n),0,0);
}


///////////////
// Mouse stuff
void VideoDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Disable when playing
	if (IsPlaying) return;

	if (event.Leaving()) {
		// OnMouseLeave isn't called as long as we have an OnMouseEvent
		// Just check for it and call it manually instead
		OnMouseLeave(event);
		event.Skip(true);
		return;
	}

	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		wxMenu menu;
		menu.Append(VIDEO_MENU_SAVE_SNAPSHOT,_("Save PNG snapshot"));
		menu.Append(VIDEO_MENU_COPY_TO_CLIPBOARD,_("Copy image to Clipboard"));
		menu.Append(VIDEO_MENU_COPY_COORDS,_("Copy coordinates to Clipboard"));
		PopupMenu(&menu);
		return;
	}

	// Click?
	if (event.ButtonDown(wxMOUSE_BTN_ANY)) {
		SetFocus();
	}

	// Send to visual
	visual->OnMouseEvent(event);
}


/////////////
// Key event
void VideoDisplay::OnKey(wxKeyEvent &event) {
	visual->OnKeyEvent(event);
}



//////////////////////
// Mouse left display
void VideoDisplay::OnMouseLeave(wxMouseEvent& event) {
	if (IsPlaying) return;

	bTrackerEditing = 0;

	RefreshVideo();
}


///////////////////////////////////////
// Jumps to a frame and update display
void VideoDisplay::JumpToFrame(int n) {
	// Loaded?
	if (!loaded) return;

	// Prevent intervention during playback
	if (IsPlaying && n != PlayNextFrame) return;

	// Set frame
	GetFrame(n);

	// Display
	RefreshVideo();
	UpdatePositionDisplay();

	// Update slider
	ControlSlider->SetValue(n);

	// Update grid
	if (!IsPlaying && Options.AsBool(_T("Highlight subs in frame"))) grid->Refresh(false);
}


////////////////////////////
// Jumps to a specific time
void VideoDisplay::JumpToTime(int ms) {
	JumpToFrame(VFR_Output.GetFrameAtTime(ms));
}


///////////////////
// Sets zoom level
void VideoDisplay::SetZoom(double value) {
	zoomValue = value;
	if (provider) {
		provider->SetZoom(value);
		UpdateSize();
		RefreshVideo();
		GetParent()->Layout();
	}
}


//////////////////////
// Sets zoom position
void VideoDisplay::SetZoomPos(int value) {
	if (value < 0) value = 0;
	if (value > 15) value = 15;
	SetZoom(double(value+1)/8.0);
	if (zoomBox->GetSelection() != value) zoomBox->SetSelection(value);
}


//////////////////////////
// Calculate aspect ratio
double VideoDisplay::GetARFromType(int type) {
	if (type == 0) return (double)provider->GetSourceWidth()/(double)provider->GetSourceHeight();
	if (type == 1) return 4.0/3.0;
	if (type == 2) return 16.0/9.0;
	if (type == 3) return 2.35;
	return 1.0;  //error
}


/////////////////////
// Sets aspect ratio
void VideoDisplay::SetAspectRatio(int _type, double value) {
	if (provider) {
		// Get value
		if (_type != 4) value = GetARFromType(_type);
		if (value < 0.5) value = 0.5;
		if (value > 5.0) value = 5.0;

		// Set
		provider->SetDAR(value);
		arType = _type;
		arValue = value;
		UpdateSize();
		RefreshVideo();
		GetParent()->Layout();
	}
}


////////////////////////////
// Updates position display
void VideoDisplay::UpdatePositionDisplay() {
	// Update position display control
	if (!PositionDisplay) {
		throw _T("Position Display not set!");
	}

	// Get time
	int time = VFR_Output.GetTimeAtFrame(frame_n,true,true);
	int temp = time;
	int h=0, m=0, s=0, ms=0;
	while (temp >= 3600000) {
		temp -= 3600000;
		h++;
	}
	while (temp >= 60000) {
		temp -= 60000;
		m++;
	}
	while (temp >= 1000) {
		temp -= 1000;
		s++;
	}
	ms = temp;

	// Position display update
	PositionDisplay->SetValue(wxString::Format(_T("%01i:%02i:%02i.%03i - %i"),h,m,s,ms,frame_n));
	if (GetKeyFrames().Index(frame_n) != wxNOT_FOUND) {
		PositionDisplay->SetBackgroundColour(Options.AsColour(_T("Grid selection background")));
		PositionDisplay->SetForegroundColour(Options.AsColour(_T("Grid selection foreground")));
	}
	else {
		PositionDisplay->SetBackgroundColour(wxNullColour);
		PositionDisplay->SetForegroundColour(wxNullColour);
	}

	// Subs position display update
	UpdateSubsRelativeTime();
}


////////////////////////////////////////////////////
// Updates box with subs position relative to frame
void VideoDisplay::UpdateSubsRelativeTime() {
	// Set variables
	wxString startSign;
	wxString endSign;
	int startOff,endOff;

	// Set start/end
	if (curLine) {
		int time = VFR_Output.GetTimeAtFrame(frame_n,true,true);
		startOff = time - curLine->Start.GetMS();
		endOff = time - curLine->End.GetMS();
	}

	// Fallback to zero
	else {
		startOff = 0;
		endOff = 0;
	}

	// Positive signs
	if (startOff > 0) startSign = _T("+");
	if (endOff > 0) endSign = _T("+");

	// Update line
	SubsPosition->SetValue(wxString::Format(_T("%s%ims; %s%ims"),startSign.c_str(),startOff,endSign.c_str(),endOff));
}


/////////////////////
// Copy to clipboard
void VideoDisplay::OnCopyToClipboard(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxBitmapDataObject(GetFrame(frame_n)));
		wxTheClipboard->Close();
	}
}


/////////////////
// Save snapshot
void VideoDisplay::OnSaveSnapshot(wxCommandEvent &event) {
	SaveSnapshot();
}

void VideoDisplay::SaveSnapshot() {
	// Get folder
	wxString option = Options.AsText(_("Video Screenshot Path"));
	wxFileName videoFile(videoName);
	wxString basepath;
	if (option == _T("?video")) {
		basepath = videoFile.GetPath();
	}
	else if (option == _T("?script")) {
		if (grid->ass->filename.IsEmpty()) basepath = videoFile.GetPath();
		else {
			wxFileName file2(grid->ass->filename);
			basepath = file2.GetPath();
		}
	}
	else basepath = DecodeRelativePath(option,((AegisubApp*)wxTheApp)->folderName);
	basepath += _T("/") + videoFile.GetName();

	// Get full path
	int session_shot_count = 1;
	wxString path;
	while (1) {
		path = basepath + wxString::Format(_T("_%03i_%i.png"),session_shot_count,frame_n);
		++session_shot_count;
		wxFileName tryPath(path);
		if (!tryPath.FileExists()) break;
	}

	// Save
	GetFrame(frame_n).ConvertToImage().SaveFile(path,wxBITMAP_TYPE_PNG);
}


/////////////////////
// Copy coordinates
void VideoDisplay::OnCopyCoords(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		int sw,sh;
		GetScriptSize(sw,sh);
		int vx = (sw * visual->mouseX + w/2) / w;
		int vy = (sh * visual->mouseY + h/2) / h;
		wxTheClipboard->SetData(new wxTextDataObject(wxString::Format(_T("%i,%i"),vx,vy)));
		wxTheClipboard->Close();
	}
}


//////////////////
// Refresh screen
void VideoDisplay::RefreshVideo() {
	// Draw frame
	wxClientDC dc(this);
	dc.DrawBitmap(GetFrame(),0,0);

	// Draw the control points for FexTracker
	visual->DrawTrackingOverlay(dc);
}


//////////////////
// DrawVideoWithOverlay
void VideoDisplay::DrawText( wxPoint Pos, wxString text ) {
	// Draw frame
	wxClientDC dc(this);
	dc.SetBrush(wxBrush(wxColour(128,128,128),wxSOLID));
	dc.DrawRectangle( 0,0, provider->GetWidth(), provider->GetHeight() );
	dc.SetTextForeground(wxColour(64,64,64));
	dc.DrawText(text,Pos.x+1,Pos.y-1);
	dc.DrawText(text,Pos.x+1,Pos.y+1);
	dc.DrawText(text,Pos.x-1,Pos.y-1);
	dc.DrawText(text,Pos.x-1,Pos.y+1);
	dc.SetTextForeground(wxColour(255,255,255));
	dc.DrawText(text,Pos.x,Pos.y);
}


////////////////////////
// Requests a new frame
wxBitmap VideoDisplay::GetFrame(int n) {
	frame_n = n;
	return provider->GetFrame(n);
	RefreshVideo();
}


////////////////////////////
// Get dimensions of script
void VideoDisplay::GetScriptSize(int &sw,int &sh) {
	grid->ass->GetResolution(sw,sh);
}


////////
// Play
void VideoDisplay::Play() {
	// Stop if already playing
	if (IsPlaying) {
		Stop();
		return;
	}

	// Set variables
	IsPlaying = true;
	StartFrame = frame_n;
	EndFrame = -1;

	// Start playing audio
	audio->Play(VFR_Output.GetTimeAtFrame(StartFrame),-1);

	// Start timer
	StartTime = clock();
	PlayTime = StartTime;
	Playback.SetOwner(this,VIDEO_PLAY_TIMER);
	Playback.Start(1);
}


/////////////
// Play line
void VideoDisplay::PlayLine() {
	// Get line
	AssDialogue *curline = grid->GetDialogue(grid->editBox->linen);
	if (!curline) return;

	// Start playing audio
	audio->Play(curline->Start.GetMS(),curline->End.GetMS());

	// Set variables
	IsPlaying = true;
	StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);

	// Jump to start
	PlayNextFrame = StartFrame;
	JumpToFrame(StartFrame);

	// Set other variables
	StartTime = clock();
	PlayTime = StartTime;

	// Start timer
	Playback.SetOwner(this,VIDEO_PLAY_TIMER);
	Playback.Start(1);
}


////////
// Stop
void VideoDisplay::Stop() {
	if (IsPlaying) {
		Playback.Stop();
		IsPlaying = false;
		audio->Stop();
	}
}


//////////////
// Play timer
void VideoDisplay::OnPlayTimer(wxTimerEvent &event) {
	// Get time difference
	clock_t cur = clock();
	int dif = (clock() - StartTime)*1000/CLOCKS_PER_SEC;
	if (!dif) return;
	PlayTime = cur;

	// Find next frame
	int startMs = VFR_Output.GetTimeAtFrame(StartFrame);
	int nextFrame = frame_n;
	for (int i=0;i<10;i++) {
		if (nextFrame >= length) break;
		if (dif < VFR_Output.GetTimeAtFrame(nextFrame) - startMs) {
			break;
		}
		nextFrame++;
	}

	// Same frame
	if (nextFrame == frame_n) return;

	// End
	if (nextFrame >= length || (EndFrame != -1 && nextFrame > EndFrame)) {
		Stop();
		return;
	}

	// Next frame is before or over 2 frames ahead, so force audio resync
	if (nextFrame < frame_n || nextFrame > frame_n + 2) audio->player->SetCurrentPosition(audio->GetSampleAtMS(VFR_Output.GetTimeAtFrame(nextFrame)));

	// Jump to next frame
	PlayNextFrame = nextFrame;
	JumpToFrame(nextFrame);

	// Sync audio
	if (nextFrame % 10 == 0) {
		__int64 audPos = audio->GetSampleAtMS(VFR_Output.GetTimeAtFrame(nextFrame));
		__int64 curPos = audio->player->GetCurrentPosition();
		int delta = int(audPos-curPos);
		if (delta < 0) delta = -delta;
		int maxDelta = audio->provider->GetSampleRate();
		if (delta > maxDelta) audio->player->SetCurrentPosition(audPos);
	}
}


//////////////////////////////
// Get name of temp work file
wxString VideoDisplay::GetTempWorkFile () {
	if (tempfile.IsEmpty()) {
		tempfile = wxFileName::CreateTempFileName(_T("aegisub"));
		wxRemoveFile(tempfile);
		tempfile += _T(".ass");
	}
	return tempfile;
}


/////////////////
// Get keyframes
wxArrayInt VideoDisplay::GetKeyFrames() {
	if (OverKeyFramesLoaded()) return overKeyFrames;
	return KeyFrames;
}


/////////////////
// Set keyframes
void VideoDisplay::SetKeyFrames(wxArrayInt frames) {
	KeyFrames = frames;
}


/////////////////////////
// Set keyframe override
void VideoDisplay::SetOverKeyFrames(wxArrayInt frames) {
	overKeyFrames = frames;
	overKeyFramesLoaded = true;
}


///////////////////
// Close keyframes
void VideoDisplay::CloseOverKeyFrames() {
	overKeyFrames.Clear();
	overKeyFramesLoaded = false;
}


//////////////////////////////////////////
// Check if override keyframes are loaded
bool VideoDisplay::OverKeyFramesLoaded() {
	return overKeyFramesLoaded;
}


/////////////////////////////////
// Check if keyframes are loaded
bool VideoDisplay::KeyFramesLoaded() {
	return overKeyFramesLoaded || keyFramesLoaded;
}
