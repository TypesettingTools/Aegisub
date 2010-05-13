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
, mouse_x(INT_MIN), mouse_y(INT_MIN)
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

void VideoDisplay::SetFrameRange(int from, int to) {
	ControlSlider->SetRange(from, to);
}


/// @brief Render the currently visible frame
void VideoDisplay::Render() try {
	if (!IsShownOnScreen()) return;
	wxASSERT(wxIsMainThread());

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

	videoOut->SetViewport(dx1, dy1, dx2, dy2);
	videoOut->Render(sw, sh);

	DrawTVEffects();

	if (visualMode == -1) SetVisualMode(0, false);
	if (visual && (visual->mouseX > INT_MIN || visual->mouseY > INT_MIN || Options.AsBool(L"Always show visual tools"))) {
		visual->Draw();
	}

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
catch (const wchar_t *err) {
	wxLogError(
		L"An error occurred trying to render the video frame on the screen.\n"
		L"Error message reported: %s",
		err);
	VideoContext::Get()->Reset();
}
catch (...) {
	wxLogError(
		_T("An error occurred trying to render the video frame to screen.\n")
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

		// Sizers ignore SetClientSize/SetSize, so only use them to calculate
		// what size is required after including the borders
		SetClientSize(w,h);
		GetSize(&w,&h);
		wxSize size(w,h);
		SetMinSize(size);
		SetMaxSize(size);

		locked = true;
		box->VideoSizer->Fit(box);
		box->GetParent()->Layout();

		// The sizer makes us use the full width, which at very low zoom levels
		// results in stretched video, so after using the sizer to update the 
		// parent window sizes, reset our size to the correct value
		SetSize(w,h);

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
	int wheel = event.GetWheelRotation();
	if (wheel) {
		SetZoom (zoomValue + .125 * (wheel / event.GetWheelDelta()));
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

void VideoDisplay::SetZoom(double value) {
	using std::max;
	zoomValue = max(value, .125);
	zoomBox->SetValue(wxString::Format("%g%%", zoomValue * 100.));
	UpdateSize();
}
void VideoDisplay::SetZoomFromBox() {
	wxString strValue = zoomBox->GetValue();
	strValue.EndsWith(L"%", &strValue);
	double value;
	if (strValue.ToDouble(&value)) {
		zoomValue = value / 100.;
		UpdateSize();
	}
}
double VideoDisplay::GetZoom() {
	return zoomValue;
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
