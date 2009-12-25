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
#include "config.h"

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
#include "video_provider_manager.h"
#include "vfr.h"
#include "ass_file.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "subs_grid.h"
#include "vfw_wrap.h"
#include "mkv_wrap.h"
#include "options.h"
#include "utils.h"
#include "video_out_gl.h"
#include "vfr.h"
#include "video_box.h"
#include "gl_wrap.h"
#include "video_slider.h"
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
: wxGLCanvas (parent, id, attribList, pos, size, style, name)
, videoOut(new VideoOutGL())
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

void VideoDisplay::SetFrame(int frameNumber) {
	VideoContext *context = VideoContext::Get();
	ControlSlider->SetValue(frameNumber);

	// Get time for frame
	int time = VFR_Output.GetTimeAtFrame(frameNumber, true, true);
	int h = time / 3600000;
	int m = time % 3600000 / 60000;
	int s = time % 60000 / 1000;
	int ms = time % 1000;

	// Set the text box for frame number and time
	PositionDisplay->SetValue(wxString::Format(_T("%01i:%02i:%02i.%03i - %i"), h, m, s, ms, frameNumber));
	if (context->GetKeyFrames().Index(frameNumber) != wxNOT_FOUND) {
		// Set the background color to indicate this is a keyframe
		PositionDisplay->SetBackgroundColour(Options.AsColour(_T("Grid selection background")));
		PositionDisplay->SetForegroundColour(Options.AsColour(_T("Grid selection foreground")));
	}
	else {
		PositionDisplay->SetBackgroundColour(wxNullColour);
		PositionDisplay->SetForegroundColour(wxNullColour);
	}

	wxString startSign;
	wxString endSign;
	int startOff = 0;
	int endOff = 0;

	if (AssDialogue *curLine = context->curLine) {
		startOff = time - curLine->Start.GetMS();
		endOff = time - curLine->End.GetMS();
	}

	// Positive signs
	if (startOff > 0) startSign = _T("+");
	if (endOff > 0) endSign = _T("+");

	// Set the text box for time relative to active subtitle line
	SubsPosition->SetValue(wxString::Format(_T("%s%ims; %s%ims"), startSign.c_str(), startOff, endSign.c_str(), endOff));

	if (IsShownOnScreen() && visual) visual->Refresh();

	// Render the new frame
	if (context->IsLoaded()) {
		AegiVideoFrame frame;
		try {
			frame = context->GetFrame(frameNumber);
		}
		catch (const wxChar *err) {
			wxLogError(
				_T("Failed seeking video. The video file may be corrupt or incomplete.\n")
				_T("Error message reported: %s"),
				err);
		}
		catch (...) {
			wxLogError(
				_T("Failed seeking video. The video file may be corrupt or incomplete.\n")
				_T("No further error message given."));
		}
		try {
			videoOut->UploadFrameData(frame);
		}
		catch (const VideoOutInitException& err) {
			wxLogError(
				L"Failed to initialize video display. Closing other running programs and updating your video card drivers may fix this.\n"
				L"Error message reported: %s",
				err.GetMessage().c_str());
			context->Reset();
		}
		catch (const VideoOutRenderException& err) {
			wxLogError(
				L"Could not upload video frame to graphics card.\n"
				L"Error message reported: %s",
				err.GetMessage().c_str());
		}
	}
	Render();

	currentFrame = frameNumber;
}

//////////
// Render
void VideoDisplay::Render() try {
	if (!IsShownOnScreen()) return;
	if (!wxIsMainThread()) throw _T("Error: trying to render from non-primary thread");

	VideoContext *context = VideoContext::Get();
	wxASSERT(context);
	if (!context->IsLoaded()) return;

	// Set GL context
	SetCurrent(*context->GetGLContext(this));

	// Get sizes
	int w, h, sw, sh, pw, ph;
	GetClientSize(&w, &h);
	wxASSERT(w > 0);
	wxASSERT(h > 0);
	context->GetScriptSize(sw, sh);
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
		float vidAr = context->GetAspectRatioType() == 0 ? float(pw)/float(ph) : context->GetAspectRatioValue();

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

	glDisable(GL_BLEND);
	if (glGetError()) throw _T("Error disabling blending.");

	videoOut->Render(sw, sh);

	DrawTVEffects();

	visual->Draw();

	glFinish();
	SwapBuffers();
}
catch (const VideoOutException &err) {
	wxLogError(
		_T("An error occurred trying to render the video frame on the screen.\n")
		_T("Error message reported: %s"),
		err.GetMessage().c_str());
	VideoContext::Get()->Reset();
}
catch (const wxChar *err) {
	wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
		_T("Error message reported: %s"),
		err);
	VideoContext::Get()->Reset();
}
catch (...) {
	wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
		_T("No further error message given."));
	VideoContext::Get()->Reset();
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

	VideoContext *con = VideoContext::Get();
	wxASSERT(con);
	if (!con->IsLoaded()) return;
	if (!IsShownOnScreen()) return;

	if (freeSize) {
		GetClientSize(&w,&h);
	}
	else {
		if (con->GetAspectRatioType() == 0) w = int(con->GetWidth() * zoomValue);
		else w = int(con->GetHeight() * zoomValue * con->GetAspectRatioValue());
		h = int(con->GetHeight() * zoomValue);
		SetSizeHints(w,h,w,h);

		locked = true;
		box->VideoSizer->Fit(box);
		box->GetParent()->Layout();
		locked = false;
	}
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

	// Mouse coordinates
	mouse_x = event.GetX();
	mouse_y = event.GetY();

	// Disable when playing
	if (VideoContext::Get()->IsPlaying()) return;

	// Right click
	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
		// Create menu
		wxMenu menu;
		menu.Append(VIDEO_MENU_SAVE_SNAPSHOT,_("Save PNG snapshot"));
		menu.Append(VIDEO_MENU_COPY_TO_CLIPBOARD,_("Copy image to Clipboard"));
		menu.AppendSeparator();
		menu.Append(VIDEO_MENU_SAVE_SNAPSHOT_RAW,_("Save PNG snapshot (no subtitles)"));
		menu.Append(VIDEO_MENU_COPY_TO_CLIPBOARD_RAW,_("Copy image to Clipboard (no subtitles)"));
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
		int vx = (sw * mouse_x + w/2) / w;
		int vy = (sh * mouse_y + h/2) / h;
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
