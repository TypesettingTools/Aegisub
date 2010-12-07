// Copyright (c) 2005-2010, Rodrigo Braz Monteiro
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
#include <algorithm>

#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/dcclient.h>
#include <wx/glcanvas.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#endif

#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "video_display.h"

#include "ass_dialogue.h"
#include "hotkeys.h"
#include "main.h"
#include "subs_grid.h"
#include "threaded_frame_source.h"
#include "video_out_gl.h"
#include "video_box.h"
#include "video_context.h"
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

BEGIN_EVENT_TABLE(VideoDisplay, wxGLCanvas)
	EVT_MOUSE_EVENTS(VideoDisplay::OnMouseEvent)
	EVT_KEY_DOWN(VideoDisplay::OnKey)
	EVT_PAINT(VideoDisplay::OnPaint)
	EVT_SIZE(VideoDisplay::OnSizeEvent)
	EVT_ERASE_BACKGROUND(VideoDisplay::OnEraseBackground)
	EVT_SHOW(VideoDisplay::OnShow)

	EVT_MENU(VIDEO_MENU_COPY_COORDS,VideoDisplay::OnCopyCoords)
	EVT_MENU(VIDEO_MENU_COPY_TO_CLIPBOARD,VideoDisplay::OnCopyToClipboard)
	EVT_MENU(VIDEO_MENU_SAVE_SNAPSHOT,VideoDisplay::OnSaveSnapshot)
	EVT_MENU(VIDEO_MENU_COPY_TO_CLIPBOARD_RAW,VideoDisplay::OnCopyToClipboardRaw)
	EVT_MENU(VIDEO_MENU_SAVE_SNAPSHOT_RAW,VideoDisplay::OnSaveSnapshotRaw)
END_EVENT_TABLE()

/// Attribute list for gl canvases; set the canvases to doublebuffered rgba with an 8 bit stencil buffer
int attribList[] = { WX_GL_RGBA , WX_GL_DOUBLEBUFFER, WX_GL_STENCIL_SIZE, 8, 0 };

/// @class VideoOutRenderException
/// @extends VideoOutException
/// @brief An OpenGL error occurred while uploading or displaying a frame
class OpenGlException : public agi::Exception {
public:
	OpenGlException(const wxChar *func, int err)
		: agi::Exception(STD_STR(wxString::Format("%s failed with error code %d", func, err)))
	{ }
	const char * GetName() const { return "video/opengl"; }
	Exception * Copy() const { return new OpenGlException(*this); }
};

#define E(cmd) cmd; if (GLenum err = glGetError()) throw OpenGlException(L###cmd, err)

VideoDisplay::VideoDisplay(
	VideoBox *box,
	VideoSlider *ControlSlider,
	wxTextCtrl *PositionDisplay,
	wxTextCtrl *SubsPosition,
	wxComboBox *zoomBox,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
: wxGLCanvas (parent, id, attribList, pos, size, style, name)
, alwaysShowTools(OPT_GET("Tool/Visual/Always Show"))
, origSize(size)
, currentFrame(-1)
, w(8), h(8), viewport_x(0), viewport_width(0), viewport_bottom(0), viewport_top(0), viewport_height(0)
, locked(false)
, zoomValue(OPT_GET("Video/Default Zoom")->GetInt() * .125 + .125)
, ControlSlider(ControlSlider)
, SubsPosition(SubsPosition)
, PositionDisplay(PositionDisplay)
, videoOut(new VideoOutGL())
, activeMode(Video_Mode_Standard)
, toolBar(box->visualSubToolBar)
, scriptW(INT_MIN)
, scriptH(INT_MIN)
, zoomBox(zoomBox)
, box(box)
, freeSize(false)
{
	if (zoomBox) zoomBox->SetValue(wxString::Format("%g%%", zoomValue * 100.));
	box->Bind(wxEVT_COMMAND_TOOL_CLICKED, &VideoDisplay::OnMode, this, Video_Mode_Standard, Video_Mode_Vector_Clip);

	VideoContext *vc = VideoContext::Get();
	vc->Bind(EVT_FRAME_READY, &VideoDisplay::UploadFrameData, this);
	slots.push_back(vc->AddSeekListener(&VideoDisplay::SetFrame, this));
	slots.push_back(vc->AddVideoOpenListener(&VideoDisplay::OnVideoOpen, this));
	slots.push_back(vc->AddSubtitlesChangeListener(&VideoDisplay::Refresh, this));

	SetCursor(wxNullCursor);
}

VideoDisplay::~VideoDisplay () {
	VideoContext::Get()->Unbind(EVT_FRAME_READY, &VideoDisplay::UploadFrameData, this);
}

bool VideoDisplay::InitContext() {
	if (!IsShownOnScreen()) return false;
	if (!glContext.get()) {
		glContext.reset(new wxGLContext(this));
	}
	SetCurrent(*glContext.get());
	return true;
}

void VideoDisplay::ShowCursor(bool show) {
	if (show) {
		SetCursor(wxNullCursor);
	}
	else {
		wxCursor cursor(wxCURSOR_BLANK);
		SetCursor(cursor);
	}
}

void VideoDisplay::UpdateRelativeTimes(int time) {
	wxString startSign;
	wxString endSign;
	int startOff = 0;
	int endOff = 0;

	if (AssDialogue *curLine = VideoContext::Get()->grid->GetActiveLine()) {
		startOff = time - curLine->Start.GetMS();
		endOff = time - curLine->End.GetMS();
	}

	// Positive signs
	if (startOff > 0) startSign = L"+";
	if (endOff > 0) endSign = L"+";

	// Set the text box for time relative to active subtitle line
	SubsPosition->SetValue(wxString::Format(L"%s%ims; %s%ims", startSign.c_str(), startOff, endSign.c_str(), endOff));
}


void VideoDisplay::SetFrame(int frameNumber) {
	VideoContext *context = VideoContext::Get();

	currentFrame = frameNumber;

	// Get time for frame
	{
		int time = context->TimeAtFrame(frameNumber, agi::vfr::EXACT);
		int h = time / 3600000;
		int m = time % 3600000 / 60000;
		int s = time % 60000 / 1000;
		int ms = time % 1000;

		// Set the text box for frame number and time
		PositionDisplay->SetValue(wxString::Format(L"%01i:%02i:%02i.%03i - %i", h, m, s, ms, frameNumber));
		if (std::binary_search(context->GetKeyFrames().begin(), context->GetKeyFrames().end(), frameNumber)) {
			// Set the background color to indicate this is a keyframe
			PositionDisplay->SetBackgroundColour(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Selection")->GetColour()));
			PositionDisplay->SetForegroundColour(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));
		}
		else {
			PositionDisplay->SetBackgroundColour(wxNullColour);
			PositionDisplay->SetForegroundColour(wxNullColour);
		}

		UpdateRelativeTimes(time);
	}

	// Render the new frame
	if (context->IsLoaded()) {
		tool->SetFrame(frameNumber);
		context->GetFrameAsync(currentFrame);
	}
}

void VideoDisplay::UploadFrameData(FrameReadyEvent &evt) {
	if (!InitContext()) return;

	try {
		videoOut->UploadFrameData(*evt.frame);
	}
	catch (const VideoOutInitException& err) {
		wxLogError(
			L"Failed to initialize video display. Closing other running "
			L"programs and updating your video card drivers may fix this.\n"
			L"Error message reported: %s",
			err.GetMessage().c_str());
		VideoContext::Get()->Reset();
	}
	catch (const VideoOutRenderException& err) {
		wxLogError(
			L"Could not upload video frame to graphics card.\n"
			L"Error message reported: %s",
			err.GetMessage().c_str());
	}
	Render();
}

void VideoDisplay::Refresh() {
	if (!tool.get()) tool.reset(new VisualToolCross(this, video, toolBar));
	if (!InitContext()) return;
	VideoContext::Get()->GetFrameAsync(currentFrame);
	tool->Refresh();
	UpdateRelativeTimes(VideoContext::Get()->TimeAtFrame(currentFrame, agi::vfr::EXACT));
}

void VideoDisplay::OnVideoOpen() {
	UpdateSize();
	Refresh();
}

void VideoDisplay::Render() try {
	if (!InitContext()) return;
	VideoContext *context = VideoContext::Get();
	if (!context->IsLoaded()) return;
	assert(wxIsMainThread());
	if (!viewport_height || !viewport_width) UpdateSize();

	videoOut->Render(viewport_x, viewport_bottom, viewport_width, viewport_height);
	E(glViewport(0, std::min(viewport_bottom, 0), w, h));

	E(glMatrixMode(GL_PROJECTION));
	E(glLoadIdentity());
	E(glOrtho(0.0f, w, h, 0.0f, -1000.0f, 1000.0f));

	if (OPT_GET("Video/Overscan Mask")->GetBool()) {
		double ar = context->GetAspectRatioValue();

		// Based on BBC's guidelines: http://www.bbc.co.uk/guidelines/dq/pdf/tv/tv_standards_london.pdf
		// 16:9 or wider
		if (ar > 1.75) {
			DrawOverscanMask(w * 0.1, h * 0.05, wxColor(30,70,200),0.5);
			DrawOverscanMask(w * 0.035, h * 0.035, wxColor(30,70,200),0.5);
		}

		// Less wide than 16:9 (use 4:3 standard)
		else {
			DrawOverscanMask(w * 0.067, h * 0.05, wxColor(30,70,200),0.5);
			DrawOverscanMask(w * 0.033, h * 0.035, wxColor(30,70,200),0.5);
		}
	}

	if (video.x > INT_MIN || video.y > INT_MIN || alwaysShowTools->GetBool()) {
		context->GetScriptSize(scriptW, scriptH);
		tool->Draw();
	}

	glFinish();
	SwapBuffers();
}
catch (const VideoOutException &err) {
	wxLogError(
		L"An error occurred trying to render the video frame on the screen.\n"
		L"Error message reported: %s",
		err.GetMessage().c_str());
	VideoContext::Get()->Reset();
}
catch (const OpenGlException &err) {
	wxLogError(
		L"An error occurred trying to render visual overlays on the screen.\n"
		L"Error message reported: %s",
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
		L"An error occurred trying to render the video frame to screen.\n"
		L"No further error message given.");
	VideoContext::Get()->Reset();
}

void VideoDisplay::DrawOverscanMask(int sizeH, int sizeV, wxColor color, double alpha) const {
	int rad1 = h * 0.05;
	int gapH = sizeH+rad1;
	int gapV = sizeV+rad1;
	int rad2 = sqrt(double(gapH*gapH + gapV*gapV)) + 1;

	OpenGLWrapper gl;
	E(gl.SetFillColour(color, alpha));
	gl.SetLineColour(wxColor(0, 0, 0), 0.0, 1);

	// Draw sides
	E(gl.DrawRectangle(gapH, 0, w-gapH, sizeV));   // Top
	E(gl.DrawRectangle(w-sizeH, gapV, w, h-gapV)); // Right
	E(gl.DrawRectangle(gapH, h-sizeV, w-gapH, h)); // Bottom
	E(gl.DrawRectangle(0, gapV, sizeH, h-gapV));   // Left

	// Draw rounded corners
	E(gl.DrawRing(gapH, gapV, rad1, rad2, 1.0, 180.0, 270.0));  // Top-left
	E(gl.DrawRing(w-gapH, gapV, rad1, rad2, 1.0, 90.0, 180.0)); // Top-right
	E(gl.DrawRing(w-gapH, h-gapV, rad1, rad2, 1.0, 0.0, 90.0)); // Bottom-right
	E(gl.DrawRing(gapH, h-gapV, rad1, rad2, 1.0,270.0,360.0));  // Bottom-left

	E(glDisable(GL_BLEND));
}


void VideoDisplay::UpdateSize() {
	VideoContext *con = VideoContext::Get();
	wxASSERT(con);
	if (!con->IsLoaded()) return;
	if (!IsShownOnScreen()) return;

	int vidW = con->GetWidth();
	int vidH = con->GetHeight();

	if (freeSize) {
		GetClientSize(&w,&h);
		viewport_x = 0;
		viewport_bottom = 0;
		viewport_top = 0;
		viewport_width = w;
		viewport_height = h;

		// Set aspect ratio
		float displayAr = float(w) / float(h);
		float videoAr = con->GetAspectRatioType() == 0 ? float(vidW)/float(vidH) : con->GetAspectRatioValue();

		// Window is wider than video, blackbox left/right
		if (displayAr - videoAr > 0.01f) {
			int delta = w - videoAr * h;
			viewport_x = delta / 2;
			viewport_width = w - delta;
		}

		// Video is wider than window, blackbox top/bottom
		else if (videoAr - displayAr > 0.01f) {
			int delta = h - w / videoAr;
			viewport_top = viewport_bottom = delta / 2;
			viewport_height = h - delta;
		}
	}
	else {
		wxWindow* parent = GetParent();
		while (!parent->IsTopLevel()) parent = parent->GetParent();
		int maxH, maxW;
		parent->GetClientSize(&maxW, &maxH);

		h = vidH * zoomValue;
		w = con->GetAspectRatioType() == 0 ? vidW * zoomValue : vidH * zoomValue * con->GetAspectRatioValue();

		// Cap the canvas size to the window size
		int cw = std::min(w, maxW), ch = std::min(h, maxH);

		viewport_x = 0;
		viewport_bottom = ch - h;
		viewport_top = 0;
		viewport_width = w;
		viewport_height = h;

		wxSize size(cw, ch);
		SetMinClientSize(size);
		SetMaxClientSize(size);

		locked = true;
		box->GetParent()->Layout();

		// The sizer makes us use the full width, which at very low zoom levels
		// results in stretched video, so after using the sizer to update the 
		// parent window sizes, reset our size to the correct value
		SetSize(cw, ch);

		locked = false;
	}

	con->GetScriptSize(scriptW, scriptH);
	video.w = w;
	video.h = h;

	if (tool.get()) tool->Refresh();

	wxGLCanvas::Refresh(false);
}

void VideoDisplay::Reset() {
	// Only calculate sizes if it's visible
	if (!IsShownOnScreen()) return;
	int w = origSize.GetX();
	int h = origSize.GetY();
	wxASSERT(w > 0);
	wxASSERT(h > 0);
	SetClientSize(w,h);
	GetSize(&w,&h);
	wxASSERT(w > 0);
	wxASSERT(h > 0);
	SetSizeHints(w,h,w,h);
}

void VideoDisplay::OnPaint(wxPaintEvent&) {
	wxPaintDC dc(this);
	Render();
}

void VideoDisplay::OnSizeEvent(wxSizeEvent &event) {
	if (freeSize) UpdateSize();
	event.Skip();
}

void VideoDisplay::OnMouseEvent(wxMouseEvent& event) {
	if (locked) return;
	assert(w > 0);

	// Disable when playing
	if (VideoContext::Get()->IsPlaying()) return;

	if (event.ButtonUp(wxMOUSE_BTN_RIGHT)) {
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

	if (event.ButtonDown(wxMOUSE_BTN_ANY)) {
		SetFocus();
	}
	int wheel = event.GetWheelRotation();
	if (wheel) {
		SetZoom (zoomValue + .125 * (wheel / event.GetWheelDelta()));
	}

	if (event.Leaving()) {
		video.x  = INT_MIN;
		video.y  = INT_MIN;
	}
	else {
		video.x  = event.GetX();
		video.y  = event.GetY();
	}

	tool->OnMouseEvent(event);
	ShowCursor(activeMode != Video_Mode_Standard);
}
void VideoDisplay::OnKey(wxKeyEvent &event) {
	int key = event.GetKeyCode();
#ifdef __APPLE__
	Hotkeys.SetPressed(key, event.m_metaDown, event.m_altDown, event.m_shiftDown);
#else
	Hotkeys.SetPressed(key, event.m_controlDown, event.m_altDown, event.m_shiftDown);
#endif

	if      (Hotkeys.IsPressed(L"Visual Tool Default"))          SetMode(Video_Mode_Standard);
	else if (Hotkeys.IsPressed(L"Visual Tool Drag"))             SetMode(Video_Mode_Drag);
	else if (Hotkeys.IsPressed(L"Visual Tool Rotate Z"))         SetMode(Video_Mode_Rotate_Z);
	else if (Hotkeys.IsPressed(L"Visual Tool Rotate X/Y"))       SetMode(Video_Mode_Rotate_XY);
	else if (Hotkeys.IsPressed(L"Visual Tool Scale"))            SetMode(Video_Mode_Scale);
	else if (Hotkeys.IsPressed(L"Visual Tool Rectangular Clip")) SetMode(Video_Mode_Clip);
	else if (Hotkeys.IsPressed(L"Visual Tool Vector Clip"))      SetMode(Video_Mode_Vector_Clip);
	event.Skip();
}

void VideoDisplay::SetZoom(double value) {
	zoomValue = std::max(value, .125);
	if (zoomBox) zoomBox->SetValue(wxString::Format("%g%%", zoomValue * 100.));
	UpdateSize();
}
void VideoDisplay::SetZoomFromBox() {
	if (!zoomBox) return;
	wxString strValue = zoomBox->GetValue();
	strValue.EndsWith(L"%", &strValue);
	double value;
	if (strValue.ToDouble(&value)) {
		zoomValue = value / 100.;
		UpdateSize();
	}
}
double VideoDisplay::GetZoom() const {
	return zoomValue;
}

void VideoDisplay::OnShow(wxShowEvent&) {
	OnVideoOpen();
}

template<class T>
void VideoDisplay::SetTool() {
	tool.reset();
	tool.reset(new T(this, video, toolBar));
	box->Bind(wxEVT_COMMAND_TOOL_CLICKED, &T::OnSubTool, static_cast<T*>(tool.get()), VISUAL_SUB_TOOL_START, VISUAL_SUB_TOOL_END);
}
void VideoDisplay::OnMode(const wxCommandEvent &event) {
	SetMode(event.GetId());
}
void VideoDisplay::SetMode(int mode) {
	if (activeMode == mode) return;

	toolBar->ClearTools();
	toolBar->Realize();
	toolBar->Show(false);

	if (!box->visualToolBar->GetToolState(mode)) {
		box->visualToolBar->ToggleTool(mode, true);
	}

	activeMode = mode;
	switch (mode) {
		case Video_Mode_Standard:    SetTool<VisualToolCross>();      break;
		case Video_Mode_Drag:        SetTool<VisualToolDrag>();       break;
		case Video_Mode_Rotate_Z:    SetTool<VisualToolRotateZ>();    break;
		case Video_Mode_Rotate_XY:   SetTool<VisualToolRotateXY>();   break;
		case Video_Mode_Scale:       SetTool<VisualToolScale>();      break;
		case Video_Mode_Clip:        SetTool<VisualToolClip>();       break;
		case Video_Mode_Vector_Clip: SetTool<VisualToolVectorClip>(); break;
		default: assert(false); break;
	}

	// Update size as the new typesetting tool may have changed the subtoolbar size
	UpdateSize();
}

void VideoDisplay::ToScriptCoords(int *x, int *y) const {
	int sx = *x - viewport_x > 0 ? viewport_width : -viewport_width;
	int sy = *y - viewport_top > 0 ? viewport_height : -viewport_height;
	*x = ((*x - viewport_x) * scriptW + sx / 2) / viewport_width;
	*y = ((*y - viewport_top) * scriptH + sy / 2) / viewport_height;
}
void VideoDisplay::FromScriptCoords(int *x, int *y) const {
	int sx = *x > 0 ? scriptW : -scriptW;
	int sy = *y > 0 ? scriptH : -scriptH;
	*x = (*x * viewport_width + sx / 2) / scriptW + viewport_x;
	*y = (*y * viewport_height + sy / 2) / scriptH + viewport_top;
}

void VideoDisplay::OnCopyToClipboard(wxCommandEvent &) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxBitmapDataObject(wxBitmap(VideoContext::Get()->GetFrame(currentFrame)->GetImage(),24)));
		wxTheClipboard->Close();
	}
}

void VideoDisplay::OnCopyToClipboardRaw(wxCommandEvent &) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxBitmapDataObject(wxBitmap(VideoContext::Get()->GetFrame(currentFrame,true)->GetImage(),24)));
		wxTheClipboard->Close();
	}
}

void VideoDisplay::OnSaveSnapshot(wxCommandEvent &) {
	VideoContext::Get()->SaveSnapshot(false);
}

void VideoDisplay::OnSaveSnapshotRaw(wxCommandEvent &) {
	VideoContext::Get()->SaveSnapshot(true);
}

void VideoDisplay::OnCopyCoords(wxCommandEvent &) {
	if (wxTheClipboard->Open()) {
		int x = video.x;
		int y = video.y;
		ToScriptCoords(&x, &y);
		wxTheClipboard->SetData(new wxTextDataObject(wxString::Format(L"%i,%i",x,y)));
		wxTheClipboard->Close();
	}
}
