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
#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <wx/image.h>
#include <string.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/config.h>
#include "utils.h"
#include "video_display.h"
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
#include "gl_wrap.h"
#include "visual_tool.h"
#include "visual_tool_cross.h"
#include "visual_tool_rotatez.h"
#include "visual_tool_rotatexy.h"
#include "visual_tool_scale.h"
#include "visual_tool_clip.h"
#include "visual_tool_vector_clip.h"
#include "visual_tool_drag.h"


///////
// IDs
enum {
	VIDEO_MENU_COPY_COORDS = 1230,
	VIDEO_MENU_COPY_TO_CLIPBOARD,
	VIDEO_MENU_COPY_TO_CLIPBOARD_RAW,
	VIDEO_MENU_SAVE_SNAPSHOT,
	VIDEO_MENU_SAVE_SNAPSHOT_RAW
};


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoDisplay, wxGLCanvas)
	EVT_MOUSE_EVENTS(VideoDisplay::OnMouseEvent)
	EVT_KEY_DOWN(VideoDisplay::OnKey)
	EVT_PAINT(VideoDisplay::OnPaint)
	EVT_SIZE(VideoDisplay::OnSizeEvent)
	EVT_ERASE_BACKGROUND(VideoDisplay::OnEraseBackground)

	EVT_MENU(VIDEO_MENU_COPY_COORDS,VideoDisplay::OnCopyCoords)
	EVT_MENU(VIDEO_MENU_COPY_TO_CLIPBOARD,VideoDisplay::OnCopyToClipboard)
	EVT_MENU(VIDEO_MENU_SAVE_SNAPSHOT,VideoDisplay::OnSaveSnapshot)
	EVT_MENU(VIDEO_MENU_COPY_TO_CLIPBOARD_RAW,VideoDisplay::OnCopyToClipboardRaw)
	EVT_MENU(VIDEO_MENU_SAVE_SNAPSHOT_RAW,VideoDisplay::OnSaveSnapshotRaw)
END_EVENT_TABLE()


//////////////
// Parameters
int attribList[] = { WX_GL_RGBA , WX_GL_DOUBLEBUFFER, WX_GL_STENCIL_SIZE, 8, 0 };

///////////////
// Constructor
VideoDisplay::VideoDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
#ifdef __WXMAC__
: wxGLCanvas (parent, id, pos, size, style, name, attribList)
#else
: wxGLCanvas (parent, id, attribList, pos, size, style, name)
#endif
{
	// Set options
	box = NULL;
	locked = false;
	ControlSlider = NULL;
	PositionDisplay = NULL;
	w=h=dx2=dy2=8;
	dx1=dy1=0;
	origSize = size;
	zoomValue = 1.0;
	freeSize = false;
	visual = NULL;
	SetCursor(wxNullCursor);
	visualMode = -1;
	SetVisualMode(0);
}


//////////////
// Destructor
VideoDisplay::~VideoDisplay () {
	delete visual;
	visual = NULL;
	VideoContext::Get()->RemoveDisplay(this);
}


///////////////
// Show cursor
void VideoDisplay::ShowCursor(bool show) {
	// Show
	if (show) SetCursor(wxNullCursor);

	// Hide
	else {
		// Bleeeh! Hate this 'solution':
		#if __WXGTK__
		static char cursor_image[] = {0};
		wxCursor cursor(cursor_image, 8, 1, -1, -1, cursor_image);
		#else
		wxCursor cursor(wxCURSOR_BLANK);
		#endif // __WXGTK__
		SetCursor(cursor);
	}
}


//////////
// Render
void VideoDisplay::Render() {
	// Is shown?
	if (!IsShownOnScreen()) return;
	if (!wxIsMainThread()) throw _T("Error: trying to render from non-primary thread");

	// Get video context
	VideoContext *context = VideoContext::Get();
	wxASSERT(context);
	if (!context->IsLoaded()) return;

	// Set GL context
	//wxMutexLocker glLock(OpenGLWrapper::glMutex);
#ifdef __WXMAC__
	SetCurrent();
#else
	SetCurrent(*context->GetGLContext(this));
#endif

	// Get sizes
	int w,h,sw,sh,pw,ph;
	GetClientSize(&w,&h);
	wxASSERT(w > 0);
	wxASSERT(h > 0);
	context->GetScriptSize(sw,sh);
	pw = context->GetWidth();
	ph = context->GetHeight();
	wxASSERT(pw > 0);
	wxASSERT(ph > 0);

	// Clear frame buffer
	glClearColor(0,0,0,0);
	if (glGetError()) throw _T("Error setting glClearColor().");
	glClearStencil(0);
	if (glGetError()) throw _T("Error setting glClearStencil().");
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	if (glGetError()) throw _T("Error calling glClear().");

	// Freesized transform
	dx1 = 0;
	dy1 = 0;
	dx2 = w;
	dy2 = h;
	if (freeSize) {
		// Set aspect ratio
		float thisAr = float(w)/float(h);
		float vidAr;
		if (context->GetAspectRatioType() == 0) vidAr = float(pw)/float(ph);
		else vidAr = context->GetAspectRatioValue();

		// Window is wider than video, blackbox left/right
		if (thisAr - vidAr > 0.01f) {
			int delta = int(w-vidAr*h);
			dx1 += delta/2;
			dx2 -= delta;
		}

		// Video is wider than window, blackbox top/bottom
		else if (vidAr - thisAr > 0.01f) {
			int delta = int(h-w/vidAr);
			dy1 += delta/2;
			dy2 -= delta;
		}
	}

	// Set viewport
	glEnable(GL_TEXTURE_2D);
	if (glGetError()) throw _T("Error enabling texturing.");
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(dx1,dy1,dx2,dy2);
	if (glGetError()) throw _T("Error setting GL viewport.");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f,sw,sh,0.0f,-1000.0f,1000.0f);
	glMatrixMode(GL_MODELVIEW);
	if (glGetError()) throw _T("Error setting up matrices (wtf?).");
	glShadeModel(GL_FLAT);

	// Texture mode
	if (w != pw || h != ph) {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		if (glGetError()) throw _T("Error setting texture parameter min filter.");
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		if (glGetError()) throw _T("Error setting texture parameter mag filter.");
	}
	else {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		if (glGetError()) throw _T("Error setting texture parameter min filter.");
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		if (glGetError()) throw _T("Error setting texture parameter mag filter.");
	}

	// Texture coordinates
	float top = 0.0f;
	float bot = context->GetTexH();
	wxASSERT(bot != 0.0f);
	if (context->IsInverted()) {
		top = context->GetTexH();
		bot = 0.0f;
	}
	float left = 0.0;
	float right = context->GetTexW();
	wxASSERT(right != 0.0f);

	// Draw frame
	glDisable(GL_BLEND);
	if (glGetError()) throw _T("Error disabling blending.");
	context->SetShader(true);
	glBindTexture(GL_TEXTURE_2D, VideoContext::Get()->GetFrameAsTexture(-1));
	if (glGetError()) throw _T("Error binding texture.");
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
	glDisable(GL_TEXTURE_2D);

	// TV effects
	DrawTVEffects();

	// Draw overlay
	if (visual) visual->Draw();

	// Swap buffers
	glFinish();
	//if (glGetError()) throw _T("Error finishing gl operation.");
	SwapBuffers();
}


///////////////////////////////////
// TV effects (overscan and so on)
void VideoDisplay::DrawTVEffects() {
	// Get coordinates
	int sw,sh;
	VideoContext *context = VideoContext::Get();
	context->GetScriptSize(sw,sh);
	bool drawOverscan = Options.AsBool(_T("Show Overscan Mask"));

	// Draw overscan mask
	if (drawOverscan) {
		// Get aspect ration
		double ar = context->GetAspectRatioValue();

		// Based on BBC's guidelines: http://www.bbc.co.uk/guidelines/dq/pdf/tv/tv_standards_london.pdf
		// 16:9 or wider
		if (ar > 1.75) {
			DrawOverscanMask(int(sw * 0.1),int(sh * 0.05),wxColour(30,70,200),0.5);
			DrawOverscanMask(int(sw * 0.035),int(sh * 0.035),wxColour(30,70,200),0.5);
		}

		// Less wide than 16:9 (use 4:3 standard)
		else {
			DrawOverscanMask(int(sw * 0.067),int(sh * 0.05),wxColour(30,70,200),0.5);
			DrawOverscanMask(int(sw * 0.033),int(sh * 0.035),wxColour(30,70,200),0.5);
		}
	}
}


//////////////////////
// Draw overscan mask
void VideoDisplay::DrawOverscanMask(int sizeH,int sizeV,wxColour colour,double alpha) {
	// Parameters
	int sw,sh;
	VideoContext *context = VideoContext::Get();
	context->GetScriptSize(sw,sh);
	int rad1 = int(sh * 0.05);
	int gapH = sizeH+rad1;
	int gapV = sizeV+rad1;
	int rad2 = (int)sqrt(double(gapH*gapH + gapV*gapV))+1;

	// Set up GL wrapper
	OpenGLWrapper gl;
	gl.SetFillColour(colour,alpha);
	gl.SetLineColour(wxColour(0,0,0),0.0,1);

	// Draw rectangles
	gl.DrawRectangle(gapH,0,sw-gapH,sizeV);		// Top
	gl.DrawRectangle(sw-sizeH,gapV,sw,sh-gapV);	// Right
	gl.DrawRectangle(gapH,sh-sizeV,sw-gapH,sh);	// Bottom
	gl.DrawRectangle(0,gapV,sizeH,sh-gapV);		// Left

	// Draw corners
	gl.DrawRing(gapH,gapV,rad1,rad2,1.0,180.0,270.0);		// Top-left
	gl.DrawRing(sw-gapH,gapV,rad1,rad2,1.0,90.0,180.0);		// Top-right
	gl.DrawRing(sw-gapH,sh-gapV,rad1,rad2,1.0,0.0,90.0);	// Bottom-right
	gl.DrawRing(gapH,sh-gapV,rad1,rad2,1.0,270.0,360.0);	// Bottom-left

	// Done
	glDisable(GL_BLEND);
}


///////////////
// Update size
void VideoDisplay::UpdateSize() {
	// Don't do anything if it's a free sizing display
	//if (freeSize) return;

	// Loaded?
	VideoContext *con = VideoContext::Get();
	wxASSERT(con);
	if (!con->IsLoaded()) return;
	if (!IsShownOnScreen()) return;

	// Get size
	if (freeSize) {
		GetClientSize(&w,&h);
	}
	else {
		if (con->GetAspectRatioType() == 0) w = int(con->GetWidth() * zoomValue);
		else w = int(con->GetHeight() * zoomValue * con->GetAspectRatioValue());
		h = int(con->GetHeight() * zoomValue);
	}
	int _w,_h;
	if (w <= 1 || h <= 1) return;
	locked = true;

	// Set the size for this control
	SetSizeHints(w,h,w,h);
	SetClientSize(w,h);
	GetSize(&_w,&_h);
	wxASSERT(_w > 0);
	wxASSERT(_h > 0);
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
	// Only calculate sizes if it's visible
	if (!IsShownOnScreen()) return;
	int w = origSize.GetX();
	int h = origSize.GetY();
	wxASSERT(w > 0);
	wxASSERT(h > 0);
	SetClientSize(w,h);
	int _w,_h;
	GetSize(&_w,&_h);
	wxASSERT(_w > 0);
	wxASSERT(_h > 0);
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
	if (freeSize) {
		UpdateSize();
	}
	event.Skip();
}


///////////////
// Mouse stuff
void VideoDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Locked?
	if (locked) return;

	// Disable when playing
	if (VideoContext::Get()->IsPlaying()) return;

	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		// Create menu
		wxMenu menu;
		menu.Append(VIDEO_MENU_SAVE_SNAPSHOT,_("Save PNG snapshot"));
		menu.Append(VIDEO_MENU_COPY_TO_CLIPBOARD,_("Copy image to Clipboard"));
		menu.AppendSeparator();
		bool canDoRaw = VideoContext::Get()->HasIndependentSubs();
		menu.Append(VIDEO_MENU_SAVE_SNAPSHOT_RAW,_("Save PNG snapshot (no subtitles)"))->Enable(canDoRaw);
		menu.Append(VIDEO_MENU_COPY_TO_CLIPBOARD_RAW,_("Copy image to Clipboard (no subtitles)"))->Enable(canDoRaw);
		menu.AppendSeparator();
		menu.Append(VIDEO_MENU_COPY_COORDS,_("Copy coordinates to Clipboard"));

		// Show cursor and popup
		ShowCursor(true);
		PopupMenu(&menu);
		return;
	}

	// Enforce correct cursor display
	ShowCursor(visualMode != 0);

	// Click?
	if (event.ButtonDown(wxMOUSE_BTN_ANY)) {
		SetFocus();
	}

	// Send to visual
	if (visual) visual->OnMouseEvent(event);
}


/////////////
// Key event
void VideoDisplay::OnKey(wxKeyEvent &event) {
	// FIXME: should these be configurable?
	// Think of the frenchmen and other people not using qwerty layout
	if (event.GetKeyCode() == 'A') SetVisualMode(0);
	if (event.GetKeyCode() == 'S') SetVisualMode(1);
	if (event.GetKeyCode() == 'D') SetVisualMode(2);
	if (event.GetKeyCode() == 'F') SetVisualMode(3);
	if (event.GetKeyCode() == 'G') SetVisualMode(4);
	if (event.GetKeyCode() == 'H') SetVisualMode(5);
	if (event.GetKeyCode() == 'J') SetVisualMode(6);
	event.Skip();
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


//////////////////////////
// Copy to clipboard (raw)
void VideoDisplay::OnCopyToClipboardRaw(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxBitmapDataObject(wxBitmap(VideoContext::Get()->GetFrame(-1,true).GetImage(),24)));
		wxTheClipboard->Close();
	}
}


/////////////////
// Save snapshot
void VideoDisplay::OnSaveSnapshot(wxCommandEvent &event) {
	VideoContext::Get()->SaveSnapshot(false);
}


//////////////////////
// Save snapshot (raw)
void VideoDisplay::OnSaveSnapshotRaw(wxCommandEvent &event) {
	VideoContext::Get()->SaveSnapshot(true);
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


/////////////////////////////
// Convert mouse coordinates
void VideoDisplay::ConvertMouseCoords(int &x,int &y) {
	int w,h;
	GetClientSize(&w,&h);
	wxASSERT(dx2 > 0);
	wxASSERT(dy2 > 0);
	wxASSERT(w > 0);
	wxASSERT(h > 0);
	x = (x-dx1)*w/dx2;
	y = (y-dy1)*h/dy2;
}


////////////
// Set mode
void VideoDisplay::SetVisualMode(int mode) {
	// Set visual
	if (visualMode != mode) {
		// Get toolbar
		wxToolBar *toolBar = NULL;
		if (box) {
			toolBar = box->visualSubToolBar;
			toolBar->ClearTools();
			toolBar->Realize();
			toolBar->Show(false);
			if (!box->visualToolBar->GetToolState(mode + Video_Mode_Standard)) box->visualToolBar->ToggleTool(mode + Video_Mode_Standard,true);
		}

		// Replace mode
		visualMode = mode;
		delete visual;
		switch (mode) {
			case 0: visual = new VisualToolCross(this); break;
			case 1: visual = new VisualToolDrag(this,toolBar); break;
			case 2: visual = new VisualToolRotateZ(this); break;
			case 3: visual = new VisualToolRotateXY(this); break;
			case 4: visual = new VisualToolScale(this); break;
			case 5: visual = new VisualToolClip(this); break;
			case 6: visual = new VisualToolVectorClip(this,toolBar); break;
			default: visual = NULL;
		}

		// Update size to reflect toolbar changes
		UpdateSize();
	}

	// Render
	Render();
}
