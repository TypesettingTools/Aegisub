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
#include <wx/glcanvas.h>
#include <GL/gl.h>
#include <GL/glu.h>
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
#include "video_box.h"
#include "video_display_fextracker.h"


///////
// IDs
enum {
	VIDEO_MENU_COPY_TO_CLIPBOARD = 1230,
	VIDEO_MENU_COPY_COORDS,
	VIDEO_MENU_SAVE_SNAPSHOT,
};


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoDisplay, wxGLCanvas)
	EVT_MOUSE_EVENTS(VideoDisplay::OnMouseEvent)
	EVT_LEAVE_WINDOW(VideoDisplay::OnMouseLeave)
	EVT_KEY_DOWN(VideoDisplay::OnKey)
	EVT_PAINT(VideoDisplay::OnPaint)
	EVT_SIZE(VideoDisplay::OnSizeEvent)
	EVT_ERASE_BACKGROUND(VideoDisplay::OnEraseBackground)

	EVT_MENU(VIDEO_MENU_COPY_TO_CLIPBOARD,VideoDisplay::OnCopyToClipboard)
	EVT_MENU(VIDEO_MENU_SAVE_SNAPSHOT,VideoDisplay::OnSaveSnapshot)
	EVT_MENU(VIDEO_MENU_COPY_COORDS,VideoDisplay::OnCopyCoords)
END_EVENT_TABLE()


//////////////
// Parameters
int attribList[3] = { WX_GL_RGBA , WX_GL_DOUBLEBUFFER, 0 };

///////////////
// Constructor
VideoDisplay::VideoDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxGLCanvas (parent, id, attribList, pos, size, style, name)
{
	// Set options
	locked = false;
	ControlSlider = NULL;
	PositionDisplay = NULL;
	w=h=dx2=dy2=8;
	dx1=dy1=0;
	origSize = size;
	zoomValue = 1.0;
	freeSize = false;
	visual = new VideoDisplayVisual(this);
	tracker = NULL;
#if USE_FEXTRACKER == 1
	tracker = new VideoDisplayFexTracker(this);
#endif
	SetCursor(wxNullCursor);
}


//////////////
// Destructor
VideoDisplay::~VideoDisplay () {
	delete visual;
#if USE_FEXTRACKER == 1
	delete tracker;
#endif
	VideoContext::Get()->RemoveDisplay(this);
}


//////////
// Render
void VideoDisplay::Render() {
	// Is shown?
	if (!IsShownOnScreen()) return;

	// Set GL context
	VideoContext *context = VideoContext::Get();
	SetCurrent(*context->GetGLContext(this));

	// Get sizes
	int w,h,sw,sh,pw,ph;
	GetClientSize(&w,&h);
	context->GetScriptSize(sw,sh);
	pw = context->GetWidth();
	ph = context->GetHeight();

	// Freesized transform
	dx1 = 0;
	dy1 = 0;
	dx2 = w;
	dy2 = h;
	if (freeSize) {
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);
		float thisAr = float(w)/float(h);
		float vidAr;
		if (context->GetAspectRatioType() == 0) vidAr = float(pw)/float(ph);
		else vidAr = context->GetAspectRatioValue();

		// Window is wider than video, blackbox left/right
		if (thisAr - vidAr > 0.01f) {
			int delta = (w-vidAr*h);
			dx1 += delta/2;
			dx2 -= delta;
		}

		// Video is wider than window, blackbox top/bottom
		else if (vidAr - thisAr > 0.01f) {
			int delta = (h-w/vidAr);
			dy1 += delta/2;
			dy2 -= delta;
		}
	}

	// Set viewport
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(dx1,dy1,dx2,dy2);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f,sw,sh,0.0f,-1000.0f,1000.0f);
	glMatrixMode(GL_MODELVIEW);

	// Texture mode
	if (w != pw || h != ph) {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}

	// Texture coordinates
	float top = 0.0f;
	float bot = context->GetTexH();
	if (context->IsInverted()) {
		top = context->GetTexH();
		bot = 0.0f;
	}
	float left = 0.0;
	float right = context->GetTexW();

	// Draw frame
	glDisable(GL_BLEND);
	context->SetShader(true);
	glBindTexture(GL_TEXTURE_2D, VideoContext::Get()->GetFrameAsTexture(-1));
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
		// Top-left
		glTexCoord2f(left,top);
		glVertex2f(0,0);
		// Bottom-left
		glTexCoord2f(left,bot);
		glVertex2f(0,sh);
		// Bottom-right
		glTexCoord2f(right,bot);
		glVertex2f(sw,sh);
		// Top-right
		glTexCoord2f(right,top);
		glVertex2f(sw,0);
	glEnd();
	context->SetShader(false);

	// Draw overlay
	visual->DrawOverlay();

	// Swap buffers
	glFinish();
	SwapBuffers();
}


///////////////
// Update size
void VideoDisplay::UpdateSize() {
	// Free size?
	if (freeSize) return;

	// Loaded?
	VideoContext *con = VideoContext::Get();
	if (!con->IsLoaded()) return;
	if (!IsShownOnScreen()) return;

	// Get size
	if (con->GetAspectRatioType() == 0) w = con->GetWidth() * zoomValue;
	else w = con->GetHeight() * zoomValue * con->GetAspectRatioValue();
	h = con->GetHeight() * zoomValue;
	int _w,_h;
	if (w <= 1 || h <= 1) return;
	locked = true;

	// Set the size for this control
	SetSizeHints(w,h,w,h);
	SetClientSize(w,h);
	GetSize(&_w,&_h);
	SetSizeHints(_w,_h,_w,_h);
	box->VideoSizer->Fit(box);

	// Layout
	box->GetParent()->Layout();
	SetClientSize(w,h);

	// Refresh
	locked = false;
	Refresh(false);
}


//////////
// Resets
void VideoDisplay::Reset() {
	int w = origSize.GetX();
	int h = origSize.GetY();
	SetClientSize(w,h);
	int _w,_h;
	GetSize(&_w,&_h);
	SetSizeHints(_w,_h,_w,_h);
}


///////////////
// Paint event
void VideoDisplay::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);
	Render();
}


//////////////
// Size Event
void VideoDisplay::OnSizeEvent(wxSizeEvent &event) {
	//Refresh(false);
	event.Skip();
}


///////////////
// Mouse stuff
void VideoDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Locked?
	if (locked) return;

	// Disable when playing
	if (VideoContext::Get()->IsPlaying()) return;

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
	if (VideoContext::Get()->IsPlaying()) return;
	visual->OnMouseEvent(event);
	if (tracker) tracker->bTrackerEditing = 0;
}


///////////////////
// Sets zoom level
void VideoDisplay::SetZoom(double value) {
	zoomValue = value;
	UpdateSize();
}


//////////////////////
// Sets zoom position
void VideoDisplay::SetZoomPos(int value) {
	if (value < 0) value = 0;
	if (value > 15) value = 15;
	SetZoom(double(value+1)/8.0);
	if (zoomBox->GetSelection() != value) zoomBox->SetSelection(value);
}


////////////////////////////
// Updates position display
void VideoDisplay::UpdatePositionDisplay() {
	// Update position display control
	if (!PositionDisplay) {
		throw _T("Position Display not set!");
	}

	// Get time
	int frame_n = VideoContext::Get()->GetFrameN();
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
	if (VideoContext::Get()->GetKeyFrames().Index(frame_n) != wxNOT_FOUND) {
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
	int frame_n = VideoContext::Get()->GetFrameN();
	AssDialogue *curLine = VideoContext::Get()->curLine;

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
		wxTheClipboard->SetData(new wxBitmapDataObject(wxBitmap(VideoContext::Get()->GetFrame(-1).GetImage(),24)));
		wxTheClipboard->Close();
	}
}


/////////////////
// Save snapshot
void VideoDisplay::OnSaveSnapshot(wxCommandEvent &event) {
	VideoContext::Get()->SaveSnapshot();
}


/////////////////////
// Copy coordinates
void VideoDisplay::OnCopyCoords(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		int sw,sh;
		VideoContext::Get()->GetScriptSize(sw,sh);
		int vx = (sw * visual->mouseX + w/2) / w;
		int vy = (sh * visual->mouseY + h/2) / h;
		wxTheClipboard->SetData(new wxTextDataObject(wxString::Format(_T("%i,%i"),vx,vy)));
		wxTheClipboard->Close();
	}
}


//////////////////
// DrawVideoWithOverlay
void VideoDisplay::DrawText( wxPoint Pos, wxString text ) {
	//// Draw frame
	//wxClientDC dc(this);
	//dc.SetBrush(wxBrush(wxColour(128,128,128),wxSOLID));
	//dc.DrawRectangle( 0,0, provider->GetWidth(), provider->GetHeight() );
	//dc.SetTextForeground(wxColour(64,64,64));
	//dc.DrawText(text,Pos.x+1,Pos.y-1);
	//dc.DrawText(text,Pos.x+1,Pos.y+1);
	//dc.DrawText(text,Pos.x-1,Pos.y-1);
	//dc.DrawText(text,Pos.x-1,Pos.y+1);
	//dc.SetTextForeground(wxColour(255,255,255));
	//dc.DrawText(text,Pos.x,Pos.y);
}


/////////////////////////////
// Convert mouse coordinates
void VideoDisplay::ConvertMouseCoords(int &x,int &y) {
	int w,h;
	GetClientSize(&w,&h);
	x = (x-dx1)*w/dx2;
	y = (y-dy1)*h/dy2;
}
