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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_display.cpp
/// @brief Control displaying a video frame obtained from the video context
/// @ingroup video main_ui
///

// Includes
#include "config.h"

#ifndef AGI_PRE
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/dcclient.h>
#include <wx/glcanvas.h>
#include <wx/menu.h>
#endif

#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "ass_dialogue.h"
#include "hotkeys.h"
#include "options.h"
#include "utils.h"
#include "video_out_gl.h"
#include "vfr.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"
#include "video_slider.h"
#include "visual_tool.h"
#include "visual_tool_clip.h"
#include "visual_tool_cross.h"
#include "visual_tool_drag.h"
#include "visual_tool_rotatexy.h"
#include "visual_tool_rotatez.h"
#include "visual_tool_scale.h"
#include "visual_tool_vector_clip.h"


// Menu item IDs
enum {
	/// Copy mouse coordinates to clipboard
	VIDEO_MENU_COPY_COORDS = 1230,
	/// Copy frame to clipboard with subtitles
	VIDEO_MENU_COPY_TO_CLIPBOARD,
	/// Copy frame to clipboard without subtitles
	VIDEO_MENU_COPY_TO_CLIPBOARD_RAW,
	/// Save frame with subtitles
	VIDEO_MENU_SAVE_SNAPSHOT,
	/// Save frame without subtitles
	VIDEO_MENU_SAVE_SNAPSHOT_RAW
};

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

/// Attribute list for gl canvases; set the canvases to doublebuffered rgba with an 8 bit stencil buffer
int attribList[] = { WX_GL_RGBA , WX_GL_DOUBLEBUFFER, WX_GL_STENCIL_SIZE, 8, 0 };


/// @brief Constructor
/// @param parent Pointer to a parent window.
/// @param id     Window identifier. If -1, will automatically create an identifier.
/// @param pos    Window position. wxDefaultPosition is (-1, -1) which indicates that wxWidgets should generate a default position for the window.
/// @param size   Window size. wxDefaultSize is (-1, -1) which indicates that wxWidgets should generate a default size for the window. If no suitable size can be found, the window will be sized to 20x20 pixels so that the window is visible but obviously not correctly sized.
/// @param style  Window style.
/// @param name   Window name.
VideoDisplay::VideoDisplay(VideoBox *box, VideoSlider *ControlSlider, wxTextCtrl *PositionDisplay, wxTextCtrl *SubsPosition, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxGLCanvas (parent, id, attribList, pos, size, style, name)
, visualMode(-1)
, origSize(size)
, currentFrame(-1)
, w(8), h(8), dx1(0), dx2(8), dy1(0), dy2(8)
, mouse_x(-1), mouse_y(-1)
, locked(false)
, zoomValue(1.0)
, ControlSlider(ControlSlider)
, SubsPosition(SubsPosition)
, PositionDisplay(PositionDisplay)
, visual(NULL)
, videoOut(new VideoOutGL())
, box(box)
, freeSize(false)
{
	SetCursor(wxNullCursor);
}

/// @brief Destructor 
VideoDisplay::~VideoDisplay () {
	if (visual) delete visual;
	visual = NULL;
	VideoContext::Get()->RemoveDisplay(this);
}

/// @brief Set the cursor to either default or blank
/// @param show Whether or not the cursor should be visible
void VideoDisplay::ShowCursor(bool show) {
	if (show) SetCursor(wxNullCursor);
	else {
		wxCursor cursor(wxCURSOR_BLANK);
		SetCursor(cursor);
	}
}

void VideoDisplay::SetFrame(int frameNumber) {
	ControlSlider->SetValue(frameNumber);

	// Get time for frame
	int time = VFR_Output.GetTimeAtFrame(frameNumber, true, true);
	int h = time / 3600000;
	int m = time % 3600000 / 60000;
	int s = time % 60000 / 1000;
	int ms = time % 1000;

	// Set the text box for frame number and time
	PositionDisplay->SetValue(wxString::Format(_T("%01i:%02i:%02i.%03i - %i"), h, m, s, ms, frameNumber));
	if (VideoContext::Get()->GetKeyFrames().Index(frameNumber) != wxNOT_FOUND) {
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

	if (AssDialogue *curLine = VideoContext::Get()->curLine) {
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
	Render(frameNumber);

	currentFrame = frameNumber;
}

void VideoDisplay::SetFrameRange(int from, int to) {
	ControlSlider->SetRange(from, to);
}


/// @brief Render the currently visible frame
void VideoDisplay::Render(int frameNumber) try {
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

	videoOut->DisplayFrame(context->GetFrame(frameNumber), sw, sh);

	DrawTVEffects();

	if (visualMode == -1) SetVisualMode(0, false);
	if (visual) visual->Draw();

	glFinish();
	SwapBuffers();
}
catch (const VideoOutUnsupportedException &err) {
		wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
		_T("Your graphics card does not appear to have a functioning OpenGL driver.\n")
		_T("Error message reported: %s"),
		err.GetMessage());
	VideoContext::Get()->Reset();
}
catch (const VideoOutException &err) {
		wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
		_T("If you get this error regardless of which video file you use, and also if you use dummy video, Aegisub might not work with your graphics card's OpenGL driver.\n")
		_T("Error message reported: %s"),
		err.GetMessage());
	VideoContext::Get()->Reset();
}
catch (const wxChar *err) {
	wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
		_T("If you get this error regardless of which video file you use, and also if you use dummy video, Aegisub might not work with your graphics card's OpenGL driver.\n")
		_T("Error message reported: %s"),
		err);
	VideoContext::Get()->Reset();
}
catch (...) {
	wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
		_T("If you get this error regardless of which video file you use, and also if you use dummy video, Aegisub might not work with your graphics card's OpenGL driver.\n")
		_T("No further error message given."));
	VideoContext::Get()->Reset();
}

/// @brief Draw the appropriate overscan masks for the current aspect ratio
void VideoDisplay::DrawTVEffects() {
	// Get coordinates
	int sw,sh;
	VideoContext *context = VideoContext::Get();
	context->GetScriptSize(sw,sh);
	bool drawOverscan = Options.AsBool(_T("Show Overscan Mask"));

	if (drawOverscan) {
		// Get aspect ratio
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

/// @brief Draw an overscan mask 
/// @param sizeH  The amount of horizontal overscan on one side
/// @param sizeV  The amount of vertical overscan on one side
/// @param colour The color of the mask
/// @param alpha  The alpha of the mask
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

/// @brief Update the size of the display
void VideoDisplay::UpdateSize() {
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

/// @brief Reset the size of the display to the video size
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

/// @brief Paint event 
/// @param event 
void VideoDisplay::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);
	Render();
}

/// @brief Handle resize events
/// @param event
void VideoDisplay::OnSizeEvent(wxSizeEvent &event) {
	if (freeSize) UpdateSize();
	event.Skip();
}

/// @brief Handle mouse events
/// @param event 
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

/// @brief Handle keypress events for switching visual typesetting modes
/// @param event
void VideoDisplay::OnKey(wxKeyEvent &event) {
	int key = event.GetKeyCode();
#ifdef __APPLE__
	Hotkeys.SetPressed(key,event.m_metaDown,event.m_altDown,event.m_shiftDown);
#else
	Hotkeys.SetPressed(key,event.m_controlDown,event.m_altDown,event.m_shiftDown);
#endif

	if (Hotkeys.IsPressed(_T("Visual Tool Default")))               SetVisualMode(0);
	else if (Hotkeys.IsPressed(_T("Visual Tool Drag")))             SetVisualMode(1);
	else if (Hotkeys.IsPressed(_T("Visual Tool Rotate Z")))         SetVisualMode(2);
	else if (Hotkeys.IsPressed(_T("Visual Tool Rotate X/Y")))       SetVisualMode(3);
	else if (Hotkeys.IsPressed(_T("Visual Tool Scale")))            SetVisualMode(4);
	else if (Hotkeys.IsPressed(_T("Visual Tool Rectangular Clip"))) SetVisualMode(5);
	else if (Hotkeys.IsPressed(_T("Visual Tool Vector Clip")))      SetVisualMode(6);
	event.Skip();
}

/// @brief Set the zoom level
/// @param value The new zoom level
void VideoDisplay::SetZoom(double value) {
	zoomValue = value;
	UpdateSize();
}

/// @brief Set the position of the zoom dropdown and switch to that zoom
/// @param value The new zoom position
void VideoDisplay::SetZoomPos(int value) {
	if (value < 0) value = 0;
	if (value > 23) value = 23;
	SetZoom(double(value+1)/8.0);
	if (zoomBox->GetSelection() != value) zoomBox->SetSelection(value);
}

/// @brief Copy the currently display frame to the clipboard, with subtitles
/// @param event Unused
void VideoDisplay::OnCopyToClipboard(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxBitmapDataObject(wxBitmap(VideoContext::Get()->GetFrame(-1).GetImage(),24)));
		wxTheClipboard->Close();
	}
}

/// @brief Copy the currently display frame to the clipboard, without subtitles
/// @param event Unused
void VideoDisplay::OnCopyToClipboardRaw(wxCommandEvent &event) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxBitmapDataObject(wxBitmap(VideoContext::Get()->GetFrame(-1,true).GetImage(),24)));
		wxTheClipboard->Close();
	}
}

/// @brief Save the currently display frame to a file, with subtitles
/// @param event Unused
void VideoDisplay::OnSaveSnapshot(wxCommandEvent &event) {
	VideoContext::Get()->SaveSnapshot(false);
}

/// @brief Save the currently display frame to a file, without subtitles
/// @param event Unused
void VideoDisplay::OnSaveSnapshotRaw(wxCommandEvent &event) {
	VideoContext::Get()->SaveSnapshot(true);
}

/// @brief Copy coordinates of the mouse to the clipboard
/// @param event Unused
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

/// @brief Convert mouse coordinates relative to the display to coordinates relative to the video
/// @param x X coordinate
/// @param y Y coordinate
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

/// @brief Set the current visual typesetting mode
/// @param mode The new mode
/// @param render Whether the display should be rerendered
void VideoDisplay::SetVisualMode(int mode, bool render) {
	// Set visual
	if (visualMode != mode) {
		wxToolBar *toolBar = NULL;
		if (box) {
			toolBar = box->visualSubToolBar;
			toolBar->ClearTools();
			toolBar->Realize();
			toolBar->Show(false);
			if (!box->visualToolBar->GetToolState(mode + Video_Mode_Standard)) {
				box->visualToolBar->ToggleTool(mode + Video_Mode_Standard,true);
			}
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

		// Update size as the new typesetting tool may have changed the subtoolbar size
		UpdateSize();
		ControlSlider->Refresh(false);
	}
	if (render) Render();
}

void VideoDisplay::OnSubTool(wxCommandEvent &event) {
	if (visual) visual->OnSubTool(event);
}
