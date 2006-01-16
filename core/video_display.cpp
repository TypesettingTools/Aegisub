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
#include "video_display.h"
#include "avisynth_wrap.h"
#include "vfr.h"
#include "ass_file.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "subs_grid.h"
#include "vfw_wrap.h"
#include "options.h"
#include "subs_edit_box.h"
#include "audio_display.h"
#include "main.h"
#include "video_slider.h"
#include <wx/image.h>
#include <string.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/config.h>


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
	curLine = NULL;
	curFrame = NULL;
	backbuffer = NULL;
	ControlSlider = NULL;
	PositionDisplay = NULL;
	loaded = false;
	frame_n = 0;
	origSize = size;
	zoom = 0.75;
	arType = 0;
	isLocked = false;
	gettingFrame = false;
	IsPlaying = false;
	threaded = Options.AsBool(_T("Threaded Video"));
	nextFrame = -1;
	framesSkipped = 0;

	// Create PNG handler
	wxPNGHandler *png = new wxPNGHandler;
	wxImage::AddHandler(png);

	// Set cursor
	wxCursor cursor(wxCURSOR_BLANK);
	SetCursor(cursor);
}


//////////////
// Destructor
VideoDisplay::~VideoDisplay () {
	ControlSlider = NULL;
	Unload();
	if (backbuffer) delete backbuffer;
	backbuffer = NULL;
}

///////////////////////
// Sets video filename
void VideoDisplay::SetVideo(const wxString &filename) {
	try {
		if (!filename.empty()) {
			// Verify if file exists
			wxFileName filetest(filename);
			if (!filetest.FileExists()) throw _T("File not found.");

			// OK to load
			if (videoName != filename) {
				Unload();
				videoName = filename;
				OpenAVSVideo();
				if (!loaded) Reset();
				else RefreshVideo();

				// Set keyframes
				if (filename.Right(4).Lower() == _T(".avi")) KeyFrames = VFWWrapper::GetKeyFrames(filename);
				else KeyFrames.Clear();

				// Add to recent
				Options.AddToRecentList(filename,_T("Recent vid"));
			}

			else Reset();
		}
		else Reset();
	}

	catch (wchar_t *error) {
		wxMessageBox(error,_T("Error setting video"),wxICON_ERROR | wxOK);
		Reset();
	}

	catch (...) {
		wxMessageBox(_T("Unhandled exception"),_T("Error setting video"),wxICON_ERROR | wxOK);
		Reset();
	}
}


///////////////////////
// Sets subtitles filename
void VideoDisplay::SetSubtitles(const wxString &filename) {
	if (!loaded) return;
	wxString old = subsName;
	subsName = filename;

	if (!videoName.empty()) {
		OpenAVSSubs();
		RefreshVideo();
	}
}


////////////
// Open AVS
void VideoDisplay::OpenAVSVideo() {
	if (videoName.empty()) return;
	bool directShow = false;

	// Create AviSynth environment
	{
		wxMutexLocker lock(AviSynthMutex);
		AVSValue script;

		// Load VSFilter
		try {
			env->Invoke("LoadPlugin", env->SaveString(GetVSFilter().mb_str(wxConvLocal)));
		}
		catch (AvisynthError &) {
			throw _T("Failed opening VSfilter");
		}

		try {
			// Prepare filename
			char *videoFilename = env->SaveString(videoName.mb_str(wxConvLocal));

			wxString extension = videoName.Right(4);
			extension.LowerCase();

			// Load depending on extension
			if (extension == _T(".avs")) { 
				script = env->Invoke("Import", videoFilename);
			} else if (extension == _T(".avi")) {
				try {
					const char *argnames[2] = { 0, "audio" };
					AVSValue args[2] = { videoFilename, false };
					script = env->Invoke("AviSource", AVSValue(args,2), argnames);
					//fix me, check for video?
				} catch (AvisynthError &) {
					const char *argnames[3] = { 0, "video", "audio" };
					AVSValue args[3] = { videoFilename, true, false };
					script = env->Invoke("DirectShowSource", AVSValue(args,3),argnames);
					directShow = true;
				}
			}
			else if (extension == _T(".d2v") && env->FunctionExists("mpeg2dec3_Mpeg2Source")) //prefer mpeg2dec3 
				script = env->Invoke("mpeg2dec3_Mpeg2Source", videoFilename);
			else if (extension == _T(".d2v")) //try other mpeg2source
				script = env->Invoke("Mpeg2Source", videoFilename);
			else {
				const char *argnames[3] = { 0, "video", "audio" };
				AVSValue args[3] = { videoFilename, true, false };
				script = env->Invoke("DirectShowSource", AVSValue(args,3),argnames);
				directShow = true;
			}
		} catch (AvisynthError &err) {
			wxMessageBox (wxString(_T("AviSynth error: ")) + wxString(err.msg,wxConvLocal), _T("Error"), wxOK | wxICON_ERROR);
		}

		// Check if video is valid
		PClip clip = script.AsClip();
		VideoInfo vi = clip->GetVideoInfo();

		if (!vi.HasVideo())
			wxMessageBox (wxString(_T("No video found: ")), _T("Error"), wxOK | wxICON_ERROR);

		// Get resolution
		orig_w = vi.width;
		orig_h = vi.height;

		// Convert to RGB32 if needed
		script = env->Invoke("ConvertToRGB32", clip);

		// Cache
		sublessVideo = (env->Invoke("InternalCache", script)).AsClip();
	}

	// Continue with subs
	OpenAVSSubs();

	// Issue warning if it's DirectShow
	if (directShow) wxMessageBox (_("This file is being loaded using DirectShow, which has UNRELIABLE seeking. Frame numbers MIGHT NOT match display, so precise timing CANNOT be trusted."), _("Warning!"), wxOK | wxICON_WARNING);
}


///////////////////
// Opens subtitles
void VideoDisplay::OpenAVSSubs () {
	// Vars
	{
		wxMutexLocker lock(AviSynthMutex);
		AVSValue script;
		video = sublessVideo;

		// Make sure there is a workfile
		wxString workfile = grid->GetTempWorkFile();
		wxFileName file(workfile);
		if (!file.FileExists()) {
			grid->CommitChanges(true);
		}
		if (!file.FileExists()) {
			throw _T("Failed creating temporary subs file. Make sure you have write permission on folder.");
		}

		// Insert subs
		try {
			PClip clip = video;
			char temp[512];
			strcpy(temp,workfile.mb_str(wxConvLocal));
			AVSValue args1[2] = { clip, temp };
			script = env->Invoke("TextSub", AVSValue(args1,2));
			video = script.AsClip();
		} catch (AvisynthError &err) {
			wxMessageBox (wxString(_T("AviSynth error: ")) + wxString(err.msg,wxConvLocal), _T("Error"), wxOK | wxICON_ERROR);
			return;
		}

		// Zoom & AR
		if (zoom != 1.0 || arType != 0) {
			try {
				// Get video data
				PClip clip = video;
				VideoInfo vi = clip->GetVideoInfo();

				// Get aspect ratio data
				int pos_h = vi.height;
				int pos_w = vi.width;
				switch (arType) {
					case 1: pos_w = pos_h * 4 / 3; break;
					case 2: pos_w = pos_h * 16 / 9; break;
				}

				// Resize
				AVSValue args[3] = { clip, int(zoom*pos_w), int(zoom*pos_h) };
				script = env->Invoke(Options.AsText(_T("Video resizer")).mb_str(wxConvLocal), AVSValue(args,3));
				video = script.AsClip();
			} catch (AvisynthError &err) {
				wxMessageBox (wxString(_T("AviSynth error: ")) + wxString(err.msg,wxConvLocal), _T("Error"), wxOK | wxICON_ERROR);
				return;
			}
		}

		// Cache
		video = (env->Invoke("InternalCache", video)).AsClip();
	}

	// Set final
	PrepareAfterAVS();
}


////////////////////////////////////////
// Sets control up after loading an AVS
void VideoDisplay::PrepareAfterAVS() {
	{
		// Gather video parameters
		wxMutexLocker lock(AviSynthMutex);
		VideoInfo vi = video->GetVideoInfo();
		w = vi.width;
		h = vi.height;
		length = vi.num_frames;
		if (vi.fps_denominator != 0) fps = double(vi.fps_numerator) / double(vi.fps_denominator);
		else fps = 0;
		VFR_Input.SetCFR(fps);
		if (!VFR_Output.loaded) VFR_Output.SetCFR(fps,true);

		// Set the size for this control
		SetClientSize(w,h);
		int _w,_h;
		GetSize(&_w,&_h);
		SetSizeHints(_w,_h,_w,_h);

		// Set range of slider
		if (ControlSlider) ControlSlider->SetRange(0,vi.num_frames-1);

		// Clear frame
		if (curFrame) delete curFrame;
		curFrame = NULL;

		// Flag as loaded
		loaded = true;
	}
}


///////////
// Unloads
void VideoDisplay::Unload() {
	{
		wxMutexLocker lock(AviSynthMutex);

		// AviSynth cleanup
		video = NULL;
		sublessVideo = NULL;

		// Internal cleanup
		loaded = false;
		if (curFrame) delete curFrame;
		curFrame = NULL;
		videoName = _T("");
		if (VFR_Output.vfr == NULL) VFR_Output.Unload();
		VFR_Input.Unload();

		frame_n = 0;
		if (ControlSlider) {
			ControlSlider->SetRange(0,0);
			ControlSlider->SetValue(0);
		}
	}
}


//////////
// Resets
void VideoDisplay::Reset() {
	Unload();
	w = origSize.GetX();
	h = origSize.GetY();
	SetClientSize(w,h);
	int _w,_h;
	GetSize(&_w,&_h);
	SetSizeHints(_w,_h,_w,_h);
	RefreshVideo();
}


//////////////////////
// Gets a frame image
void VideoDisplay::GetFrameImage(int n) {
	// Prepare copy
	unsigned char *data;
	VideoInfo vi = video->GetVideoInfo();
	PVideoFrame avsFrame;
	try {
		avsFrame = video->GetFrame(n,env);
	}
	catch (AvisynthError err) {
		wxMessageBox (wxString(_T("AviSynth error: ")) + wxString(err.msg,wxConvLocal), _T("Error getting frame"), wxOK | wxICON_ERROR);
		return;
	}
	catch (...) {
		wxMessageBox(_T("AviSynth threw an exception while trying to retrieve frame."),_T("Error getting frame"),wxICON_ERROR | wxOK);
		return;
	}
	unsigned int pitch = avsFrame->GetPitch();
	unsigned int read_w = avsFrame->GetRowSize();
	unsigned int read_h = avsFrame->GetHeight();
	int depth = wxDisplayDepth();
	int bpp = depth/8;
	unsigned int x,y,dx;

	// Output
	data = (unsigned char*) malloc(w*h*bpp);

	// RGB24
	if (vi.IsRGB24()) {
		if (depth == 16) {
			// Get pointers
			const unsigned char *read_ptr = avsFrame->GetReadPtr();
			unsigned short *write_ptr = (unsigned short*) (data+(w*h*2));
			unsigned char r,g,b;

			for (y=0;y<read_h;y++) {
				write_ptr = write_ptr - read_w/3;
				for (x=0,dx=0;x<read_w;x+=3,dx++) {
					r = *(read_ptr+x+2);
					g = *(read_ptr+x+1);
					b = *(read_ptr+x);
					*(write_ptr+dx) = ((r>>3)<<11) | ((g>>2)<<5) | b>>3;
				}
				read_ptr = read_ptr + pitch;
			}
		}

		if (depth == 24) {
			// Get pointers
			const unsigned char *read_ptr = avsFrame->GetReadPtr();
			unsigned char *write_ptr = data+(w*h*3);

			for (y=0;y<read_h;y++) {
				write_ptr = write_ptr - read_w;
				for (x=0;x<read_w;x+=3) {
					*(write_ptr+x) = *(read_ptr+x);
					*(write_ptr+x+1) = *(read_ptr+x+1);
					*(write_ptr+x+2) = *(read_ptr+x+2);
				}
				read_ptr = read_ptr + pitch;
			}
		}

		if (depth == 32) {
			// Get pointers
			const unsigned char *read_ptr = avsFrame->GetReadPtr();
			unsigned char *write_ptr = data+(w*h*4);
			unsigned int delta = pitch-read_w;
			unsigned int linelen = read_w*4/3;
			int wid = read_w/3;
			int i,j;

			for (j=read_h;--j>=0;) {
				write_ptr -= linelen;
				for (i=wid;--i>=0;) {
					*(write_ptr++) = *(read_ptr++);
					*(write_ptr++) = *(read_ptr++);
					*(write_ptr++) = *(read_ptr++);
					write_ptr++;
				}
				read_ptr = read_ptr += delta;
				write_ptr -= linelen;
			}
		}
	}

	// RGB32
	else if (vi.IsRGB32()) {
		if (depth == 8) {
			throw _T("8-bit display not supported");
		}

		if (depth == 15) {
			throw _T("15-bit display not supported");
		}

		if (depth == 16) {
			// Get pointers
			const unsigned char *read_ptr = avsFrame->GetReadPtr();
			unsigned short *write_ptr = (unsigned short*) (data+(w*h*2));
			unsigned char r,g,b;

			for (y=0;y<read_h;y++) {
				write_ptr = write_ptr - read_w/4;
				for (x=0,dx=0;x<read_w;x+=4,dx++) {
					r = *(read_ptr+x+2);
					g = *(read_ptr+x+1);
					b = *(read_ptr+x);
					*(write_ptr+dx) = ((r>>3)<<11) | ((g>>2)<<5) | b>>3;
				}
				read_ptr = read_ptr + pitch;
			}
		}

		if (depth == 24) {
			throw _T("24-bit display not supported");
		}

		if (depth == 32) {
			// Get pointers
			const unsigned int *read_ptr = (const unsigned int *) avsFrame->GetReadPtr();
			unsigned int *write_ptr = ((unsigned int *) (data))+(w*h);
			unsigned int delta = (pitch-read_w)/4;
			unsigned int linelen = read_w/4;
			int i;

			for (i=read_h;--i>=0;) {
				write_ptr -= linelen;
				memcpy(write_ptr,read_ptr,read_w);
				read_ptr += linelen + delta;
			}
		}
	}

	else {
		throw _T("Wrong colour format.");
	}

	// Copy to image
	BitmapMutex.Lock();
	try {
		if (curFrame) {
			delete curFrame;
			curFrame = NULL;
		}
		curFrame = new wxBitmap((const char*)data,w,h,depth);
	}
	catch (...) {}
	BitmapMutex.Unlock();
	free(data);

	// Done
	frame_n = n;
}


/////////////////
// OnPaint event
void VideoDisplay::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);

	// Try to grab frame if none
	{
		wxMutexLocker locker(BitmapMutex);
		if (!curFrame) GetFrame(frame_n);
	}

	// Draw frame
	if (curFrame) {
		wxMutexLocker locker(BitmapMutex);
		dc.BeginDrawing();
		dc.DrawBitmap(*curFrame,0,0);
		dc.EndDrawing();
	}

	// Failed
	else {
		dc.BeginDrawing();
		dc.SetBrush(*wxBLUE_BRUSH);
		dc.DrawRectangle(0,0,w,h);
		dc.EndDrawing();
	}
}


///////////////
// Mouse stuff
void VideoDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Disable when playing
	if (IsPlaying) return;

	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		wxMenu menu;
		menu.Append(VIDEO_MENU_SAVE_SNAPSHOT,_("Save PNG snapshot"));
		menu.Append(VIDEO_MENU_COPY_TO_CLIPBOARD,_("Copy image to Clipboard"));
		menu.Append(VIDEO_MENU_COPY_COORDS,_("Copy coordinates to Clipboard"));
		PopupMenu(&menu);
		return;
	}

	// Coords
	int x = event.GetX();
	int y = event.GetY();

	// Text of current coords
	int sw,sh;
	GetScriptSize(sw,sh);
	int vx = (sw * x + w/2) / w;
	int vy = (sh * y + h/2) / h;
	wxString text;
	if (!event.ShiftDown()) text = wxString::Format(_T("%i,%i"),vx,vy);
	else text = wxString::Format(_T("%i,%i"),vx - sw,vy - sh);

	// Double click
	if (event.LeftDClick()) {
		grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),vx,vy),0);
		grid->editBox->CommitText();
		grid->ass->FlagAsModified();
		grid->CommitChanges();
	}

	// Hover
	if (x != mouse_x || y != mouse_y) {
		// Set coords
		mouse_x = x;
		mouse_y = y;

		// Create backbuffer
		bool needCreate = false;
		if (!backbuffer) needCreate = true;
		else if (backbuffer->GetWidth() != w || backbuffer->GetHeight() != h) {
			needCreate = true;
			delete backbuffer;
		}
		if (needCreate) backbuffer = new wxBitmap(w,h);

		// Prepare drawing
		wxMutexLocker locker(BitmapMutex);
		wxMemoryDC dc;
		dc.SelectObject(*backbuffer);
		dc.BeginDrawing();
		dc.SetPen(wxPen(wxColour(255,255,255),1));
		dc.SetLogicalFunction(wxINVERT);

		// Draw frame
		dc.DrawBitmap(*curFrame,0,0);

		// Current position info
		if (x >= 0 && x < w && y >= 0 && y < h) {
			// Draw cross
			dc.DrawLine(0,y,w-1,y);
			dc.DrawLine(x,0,x,h-1);

			// Setup text
			wxFont font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
			dc.SetFont(font);
			int tw,th;
			GetTextExtent(text,&tw,&th,NULL,NULL,&font);

			// Inversion
			bool left = false;
			bool bottom = false;
			if (x > w/2) left = true;
			if (y < h/2) bottom = true;

			// Text draw coords
			int dx = x,dy = y;
			if (left) dx -= tw + 4;
			else dx += 4;
			if (bottom) dy += 3;
			else dy -= th + 3;

			// Draw text
			dc.SetTextForeground(wxColour(64,64,64));
			dc.DrawText(text,dx+1,dy-1);
			dc.DrawText(text,dx+1,dy+1);
			dc.DrawText(text,dx-1,dy-1);
			dc.DrawText(text,dx-1,dy+1);
			dc.SetTextForeground(wxColour(255,255,255));
			dc.DrawText(text,dx,dy);
		}

		// End
		dc.EndDrawing();

		// Blit to screen
		wxClientDC dcScreen(this);
		dcScreen.BeginDrawing();
		//dcScreen.DrawBitmap(backbuffer,0,0);
		dcScreen.Blit(0,0,w,h,&dc,0,0);
		dcScreen.EndDrawing();
	}
}


//////////////////////
// Mouse left display
void VideoDisplay::OnMouseLeave(wxMouseEvent& event) {
	if (IsPlaying) return;

	// Refresh display
	{
		wxMutexLocker locker(BitmapMutex);
		wxClientDC dc(this);
		dc.BeginDrawing();
		dc.DrawBitmap(*curFrame,0,0);
		dc.EndDrawing();
	}
}


///////////////////////////////////////
// Jumps to a frame and update display
void VideoDisplay::JumpToFrame(int n) {
	// Loaded?
	if (!loaded) return;

	// Prevent intervention during playback
	if (IsPlaying && n != PlayNextFrame) return;

	// Set frame
	if (frame_n != n) GetFrame(n);

	// Update slider
	if (ControlSlider) ControlSlider->SetValue(n);

	// Update grid
	if (!IsPlaying && Options.AsBool(_T("Highlight subs in frame"))) grid->UpdateRowColours();
}


////////////////////////////
// Jumps to a specific time
void VideoDisplay::JumpToTime(int ms) {
	JumpToFrame(VFR_Output.CorrectFrameAtTime(ms,true));
}


///////////////////
// Sets zoom level
void VideoDisplay::SetZoom(double value) {
	if (value != zoom) {
		zoom = value;
		if (loaded) {
			OpenAVSSubs();
			RefreshVideo();
			GetParent()->Layout();
		}
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


///////////////////
// Sets zoom level
void VideoDisplay::SetAspectRatio(int value) {
	if (value != arType) {
		arType = value;
		if (loaded) {
			//GetParent()->Freeze();
			OpenAVSSubs();
			RefreshVideo();
			GetParent()->Layout();
			//GetParent()->Thaw();
		}
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
	int time = VFR_Output.GetTimeAtFrame(frame_n);
	//int time = VFR_Output.CorrectTimeAtFrame(frame_n,false);
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
	if (KeyFrames.Index(frame_n) != wxNOT_FOUND) {
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
		int time = VFR_Output.GetTimeAtFrame(frame_n);
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


//////////////////////////
// Locks/unlocks updating
void VideoDisplay::Locked(bool state) {
	bool oldState = isLocked;
	isLocked = state;
	if (loaded && oldState == true && isLocked == false) RefreshVideo();
}


/////////////////////
// Copy to clipboard
void VideoDisplay::OnCopyToClipboard(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		wxMutexLocker locker(BitmapMutex);
		wxTheClipboard->SetData(new wxBitmapDataObject(*curFrame));
		wxTheClipboard->Close();
	}
}


/////////////////
// Save snapshot
void VideoDisplay::OnSaveSnapshot(wxCommandEvent &event) {
	SaveSnapshot();
}

void VideoDisplay::SaveSnapshot() {
	// Get path
	wxFileName file = videoName;
	wxString basepath = file.GetPath() + _T("/") + file.GetName();
	wxString path;
	for (int i=0;;i++) {
		path = basepath + wxString::Format(_T("_%03i.png"),i);
		wxFileName tryPath(path);
		if (!tryPath.FileExists()) break;
	}

	// Save
	curFrame->ConvertToImage().SaveFile(path,wxBITMAP_TYPE_PNG);
}


/////////////////////
// Copy coordinates
void VideoDisplay::OnCopyCoords(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		int sw,sh;
		GetScriptSize(sw,sh);
		int vx = (sw * mouse_x + w/2) / w;
		int vy = (sh * mouse_y + h/2) / h;
		wxTheClipboard->SetData(new wxTextDataObject(wxString::Format(_T("%i,%i"),vx,vy)));
		wxTheClipboard->Close();
	}
}


/////////////////////////
// Get VSFilter filename
wxString VideoDisplay::GetVSFilter() {
	if (VSFilterPath.IsEmpty()) {
		wxFileName vsfilterPath(AegisubApp::folderName + _T("vsfilter.dll"));
		if (vsfilterPath.FileExists()) {
			VSFilterPath = AegisubApp::folderName + _T("vsfilter.dll");
		}
		else {
			wxRegKey reg(_T("HKEY_CLASSES_ROOT\\CLSID\\{9852A670-F845-491B-9BE6-EBD841B8A613}\\InprocServer32"));
			if (reg.Exists()) {
				reg.QueryValue(_T(""),VSFilterPath);
				wxFileName file(VSFilterPath);
				if (!file.FileExists()) VSFilterPath = _T("vsfilter.dll");
			}
			else {
				VSFilterPath = _T("vsfilter.dll");
			}
		}
	}
	return VSFilterPath;
}


//////////////////
// Refresh screen
void VideoDisplay::RefreshVideo(bool force) {
	// Prepare
	if (isLocked) return;

	// Not loaded
	if (!loaded) {
		wxClientDC dc(this);
		dc.BeginDrawing();
		dc.SetBrush(*wxBLUE_BRUSH);
		dc.DrawRectangle(0,0,w,h);
		dc.EndDrawing();
	}

	else {
		// Forced
		if (force) {
			int n = frame_n;
			frame_n = -1;
			OpenAVSSubs();
			GetFrame(n);
		}

		// No frame, try to get one
		{
			wxMutexLocker locker(BitmapMutex);
			if (!curFrame) GetFrame(frame_n);
		}

		// Draw frame
		if (curFrame) {
			wxMutexLocker locker(BitmapMutex);
			wxClientDC dc(this);
			dc.BeginDrawing();
			dc.DrawBitmap(*curFrame,0,0);
			dc.EndDrawing();
			//Refresh(false);
		}

		// Draw black
		else {
			wxClientDC dc(this);
			dc.BeginDrawing();
			dc.Clear();
			dc.EndDrawing();
		}
	}

	// Display
	UpdatePositionDisplay();
}


////////////////////////
// Requests a new frame
void VideoDisplay::GetFrame(int n) {
	// Make sure it's loaded
	if (!loaded) {
		//throw _T("Video not loaded");
		return;
	}

	// Threaded mode
	if (threaded) {
		nextFrame = n;
		if (!gettingFrame || framesSkipped > 999999) {
			framesSkipped = 0;
			gettingFrame = true;
			wxThread *thread = new GetFrameThread(this,n);
			thread->Create();
			thread->Run();
		}
		else {
			framesSkipped++;
		}
	}

	// Simple mode
	else {
		GetFrameImage(n);
		RefreshVideo();
	}
}


////////////////////////////////
// Get frame thread constructor
GetFrameThread::GetFrameThread(VideoDisplay *parent,int n)
: wxThread(wxTHREAD_DETACHED)
{
	display = parent;
	image_n = n;
}


//////////////////////////
// Get Frame thread entry
wxThread::ExitCode GetFrameThread::Entry() {
	// Get frame
	display->gettingFrame = true;
	AviSynthWrapper::AviSynthMutex.Lock();
	try { display->GetFrameImage(image_n); }
	catch (...) { }
	AviSynthWrapper::AviSynthMutex.Unlock();

	// Refresh video
	//wxMutexGuiEnter();
	try {
		display->RefreshVideo();
	}
	catch (...) { }
	//wxMutexGuiLeave();
	display->gettingFrame = false;

	if (display->nextFrame != image_n) {
		display->GetFrame(display->nextFrame);
	}
	
	// Return
	Exit();
	return 0;
}


////////////////////////////
// Get dimensions of script
void VideoDisplay::GetScriptSize(int &sw,int &sh) {
	// Height
	wxString temp = grid->ass->GetScriptInfo(_T("PlayResY"));
	if (temp == _T("") || !temp.IsNumber()) {
		//sh = orig_h;
		sh = 384;
	}
	else {
		long templ;
		temp.ToLong(&templ);
		sh = templ;
	}

	// Width
	temp = grid->ass->GetScriptInfo(_T("PlayResX"));
	if (temp == _T("") || !temp.IsNumber()) {
		//sw = orig_w * sh / orig_h;
		sw = 288;
	}
	else {
		long templ;
		temp.ToLong(&templ);
		sw = templ;
	}
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
	StartTime = clock();
	PlayTime = StartTime;
	StartFrame = frame_n;
	EndFrame = -1;

	// Start playing audio
	audio->Play(VFR_Output.GetTimeAtFrame(StartFrame),-1);

	// Start timer
	Playback.SetOwner(this,VIDEO_PLAY_TIMER);
	Playback.Start(1);
}


/////////////
// Play line
void VideoDisplay::PlayLine() {
	// Get line
	AssDialogue *curline = grid->GetDialogue(grid->editBox->linen);
	if (!curline) return;

	// Set variables
	IsPlaying = true;
	StartFrame = VFR_Output.CorrectFrameAtTime(curline->Start.GetMS(),true);
	EndFrame = VFR_Output.CorrectFrameAtTime(curline->End.GetMS(),false);

	// Jump to start
	PlayNextFrame = StartFrame;
	JumpToFrame(StartFrame);

	// Set other variables
	StartTime = clock();
	PlayTime = StartTime;

	// Start playing audio
	audio->Play(curline->Start.GetMS(),curline->End.GetMS());

	// Start timer
	Playback.SetOwner(this,VIDEO_PLAY_TIMER);
	Playback.Start(1);
}


////////
// Stop
void VideoDisplay::Stop() {
	Playback.Stop();
	audio->Stop();
	IsPlaying = false;
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

	// Jump to next frame
	PlayNextFrame = nextFrame;
	JumpToFrame(nextFrame);
}
