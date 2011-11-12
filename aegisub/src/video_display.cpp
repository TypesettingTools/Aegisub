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

#include <wx/combobox.h>
#include <wx/dataobj.h>
#include <wx/dcclient.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/toolbar.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "video_display.h"

#include "ass_file.h"
#include "command/command.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "include/aegisub/menu.h"
#include "main.h"
#include "threaded_frame_source.h"
#include "utils.h"
#include "video_out_gl.h"
#include "video_box.h"
#include "video_context.h"
#include "visual_tool.h"

/// Attribute list for gl canvases; set the canvases to doublebuffered rgba with an 8 bit stencil buffer
int attribList[] = { WX_GL_RGBA , WX_GL_DOUBLEBUFFER, WX_GL_STENCIL_SIZE, 8, 0 };

/// @class VideoOutRenderException
/// @extends VideoOutException
/// @brief An OpenGL error occurred while uploading or displaying a frame
class OpenGlException : public agi::Exception {
public:
	OpenGlException(const char *func, int err)
		: agi::Exception(STD_STR(wxString::Format("%s failed with error code %d", func, err)))
	{ }
	const char * GetName() const { return "video/opengl"; }
	Exception * Copy() const { return new OpenGlException(*this); }
};

#define E(cmd) cmd; if (GLenum err = glGetError()) throw OpenGlException(#cmd, err)

VideoDisplay::VideoDisplay(
	VideoBox *box,
	bool freeSize,
	wxComboBox *zoomBox,
	wxWindow* parent,
	agi::Context *c)
: wxGLCanvas (parent, -1, attribList, wxDefaultPosition, wxDefaultSize, 0, wxPanelNameStr)
, alwaysShowTools(OPT_GET("Tool/Visual/Always Show"))
, con(c)
, w(8)
, h(8)
, mouse_pos(Vector2D::Bad())
, viewport_left(0)
, viewport_width(0)
, viewport_bottom(0)
, viewport_top(0)
, viewport_height(0)
, zoomValue(OPT_GET("Video/Default Zoom")->GetInt() * .125 + .125)
, videoOut(new VideoOutGL())
, toolBar(box->visualSubToolBar)
, zoomBox(zoomBox)
, box(box)
, freeSize(freeSize)
{
	assert(box);

	zoomBox->SetValue(wxString::Format("%g%%", zoomValue * 100.));
	zoomBox->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &VideoDisplay::SetZoomFromBox, this);

	con->videoController->Bind(EVT_FRAME_READY, &VideoDisplay::UploadFrameData, this);
	slots.push_back(con->videoController->AddVideoOpenListener(&VideoDisplay::OnVideoOpen, this));
	slots.push_back(con->videoController->AddARChangeListener(&VideoDisplay::UpdateSize, this));

	Bind(wxEVT_PAINT, std::tr1::bind(&VideoDisplay::Render, this));
	if (freeSize) {
		Bind(wxEVT_SIZE, &VideoDisplay::OnSizeEvent, this);
	}
	Bind(wxEVT_CONTEXT_MENU, &VideoDisplay::OnContextMenu, this);
	Bind(wxEVT_ENTER_WINDOW, &VideoDisplay::OnMouseEvent, this);
	Bind(wxEVT_KEY_DOWN, &VideoDisplay::OnKeyDown, this);
	Bind(wxEVT_LEAVE_WINDOW, &VideoDisplay::OnMouseLeave, this);
	Bind(wxEVT_LEFT_DCLICK, &VideoDisplay::OnMouseEvent, this);
	Bind(wxEVT_LEFT_DOWN, &VideoDisplay::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &VideoDisplay::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &VideoDisplay::OnMouseEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &VideoDisplay::OnMouseWheel, this);

	SetCursor(wxNullCursor);

	c->videoDisplay = this;

	if (con->videoController->IsLoaded())
		OnVideoOpen();
}

VideoDisplay::~VideoDisplay () {
	con->videoController->Unbind(EVT_FRAME_READY, &VideoDisplay::UploadFrameData, this);
}

bool VideoDisplay::InitContext() {
	if (!IsShownOnScreen()) return false;
	if (!glContext.get()) {
		glContext.reset(new wxGLContext(this));
	}
	SetCurrent(*glContext.get());
	return true;
}

void VideoDisplay::UploadFrameData(FrameReadyEvent &evt) {
	if (!InitContext()) return;

	try {
		videoOut->UploadFrameData(*evt.frame);
	}
	catch (const VideoOutInitException& err) {
		wxLogError(
			"Failed to initialize video display. Closing other running "
			"programs and updating your video card drivers may fix this.\n"
			"Error message reported: %s",
			err.GetMessage());
		con->videoController->Reset();
	}
	catch (const VideoOutRenderException& err) {
		wxLogError(
			"Could not upload video frame to graphics card.\n"
			"Error message reported: %s",
			err.GetMessage());
	}
	Render();
}

void VideoDisplay::OnVideoOpen() {
	if (!con->videoController->IsLoaded()) return;
	if (!tool.get())
		cmd::call("video/tool/cross", con);
	UpdateSize();
	con->videoController->JumpToFrame(0);
}

void VideoDisplay::Render() try {
	if (!InitContext()) return;
	if (!con->videoController->IsLoaded()) return;
	assert(wxIsMainThread());
	if (!viewport_height || !viewport_width) UpdateSize();

	videoOut->Render(viewport_left, viewport_bottom, viewport_width, viewport_height);
	E(glViewport(0, std::min(viewport_bottom, 0), w, h));

	E(glMatrixMode(GL_PROJECTION));
	E(glLoadIdentity());
	E(glOrtho(0.0f, w, h, 0.0f, -1000.0f, 1000.0f));

	if (OPT_GET("Video/Overscan Mask")->GetBool()) {
		double ar = con->videoController->GetAspectRatioValue();

		// Based on BBC's guidelines: http://www.bbc.co.uk/guidelines/dq/pdf/tv/tv_standards_london.pdf
		// 16:9 or wider
		if (ar > 1.75) {
			DrawOverscanMask(.1f, .05f);
			DrawOverscanMask(0.035f, 0.035f);
		}
		// Less wide than 16:9 (use 4:3 standard)
		else {
			DrawOverscanMask(.067f, .05f);
			DrawOverscanMask(0.033f, 0.035f);
		}
	}

	if (mouse_pos || alwaysShowTools->GetBool()) {
		if (!con->videoController->IsPlaying())
			tool->Draw();
	}

	SwapBuffers();
}
catch (const agi::Exception &err) {
	wxLogError(
		"An error occurred trying to render the video frame on the screen.\n"
		"Error message reported: %s",
		err.GetChainedMessage());
	con->videoController->Reset();
}

void VideoDisplay::DrawOverscanMask(float horizontal_percent, float vertical_percent) const {
	Vector2D size(w * horizontal_percent / 2, h * vertical_percent / 2);
	int rad1 = h * 0.05;
	Vector2D gap = size + rad1;
	int rad2 = gap.Len() + 1;
	Vector2D v(w, h);
	Vector2D igap = v - gap;
	Vector2D isize = v - size;

	OpenGLWrapper gl;
	gl.SetFillColour(wxColor(30, 70, 200), .5f);
	gl.SetLineColour(*wxBLACK, 0, 1);

	// Draw sides
	gl.DrawRectangle(Vector2D(gap, 0),     Vector2D(igap, size)); // Top
	gl.DrawRectangle(Vector2D(isize, gap), Vector2D(v, igap));    // Right
	gl.DrawRectangle(Vector2D(gap, isize), Vector2D(igap, v));    // Bottom
	gl.DrawRectangle(Vector2D(0, gap),     Vector2D(size, igap)); // Left

	// Draw rounded corners
	gl.DrawRing(gap,                 rad1, rad2, 1.f,  90.f, 180.f); // Top-left
	gl.DrawRing(Vector2D(igap, gap), rad1, rad2, 1.f,   0.f, 90.f);  // Top-right
	gl.DrawRing(v - gap,             rad1, rad2, 1.f, 270.f, 360.f); // Bottom-right
	gl.DrawRing(Vector2D(gap, igap), rad1, rad2, 1.f, 180.f, 270.f); // Bottom-left

	E(glDisable(GL_BLEND));
}

void VideoDisplay::UpdateSize(int arType, double arValue) {
	if (!con->videoController->IsLoaded()) return;
	if (!IsShownOnScreen()) return;

	int vidW = con->videoController->GetWidth();
	int vidH = con->videoController->GetHeight();

	if (arType == -1) {
		arType = con->videoController->GetAspectRatioType();
		arValue = con->videoController->GetAspectRatioValue();
	}

	if (freeSize) {
		GetClientSize(&w,&h);
		viewport_left = 0;
		viewport_bottom = 0;
		viewport_top = 0;
		viewport_width = w;
		viewport_height = h;

		// Set aspect ratio
		double displayAr = double(w) / h;
		double videoAr = arType == 0 ? double(vidW)/vidH : arValue;

		// Window is wider than video, blackbox left/right
		if (displayAr - videoAr > 0.01f) {
			int delta = w - videoAr * h;
			viewport_left = delta / 2;
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
		w = arType == 0 ? vidW * zoomValue : vidH * zoomValue * arValue;

		// Cap the canvas size to the window size
		int cw = std::min(w, maxW), ch = std::min(h, maxH);

		viewport_left = 0;
		viewport_bottom = ch - h;
		viewport_top = 0;
		viewport_width = w;
		viewport_height = h;

		wxSize size(cw, ch);
		SetMinClientSize(size);
		SetMaxClientSize(size);

		SetEvtHandlerEnabled(false);
		box->GetParent()->Layout();

		// The sizer makes us use the full width, which at very low zoom levels
		// results in stretched video, so after using the sizer to update the 
		// parent window sizes, reset our size to the correct value
		SetSize(cw, ch);

		SetEvtHandlerEnabled(true);
	}

	if (tool.get())
		tool->SetDisplayArea(viewport_left, viewport_top, viewport_width, viewport_height);

	Refresh(false);
}

void VideoDisplay::OnSizeEvent(wxSizeEvent &event) {
	UpdateSize();
	event.Skip();
}

void VideoDisplay::OnMouseEvent(wxMouseEvent& event) {
	assert(w > 0);

	// Disable when playing
	if (con->videoController->IsPlaying()) return;

	if (event.ButtonDown())
		SetFocus();

	mouse_pos = event.GetPosition();

	tool->OnMouseEvent(event);
}

void VideoDisplay::OnMouseLeave(wxMouseEvent& event) {
	mouse_pos = Vector2D::Bad();
	tool->OnMouseEvent(event);
}

void VideoDisplay::OnMouseWheel(wxMouseEvent& event) {
	if (int wheel = event.GetWheelRotation()) {
		if (ForwardMouseWheelEvent(this, event))
			SetZoom (zoomValue + .125 * (wheel / event.GetWheelDelta()));
	}
}

void VideoDisplay::OnContextMenu(wxContextMenuEvent&) {
	if (!context_menu.get()) context_menu.reset(menu::GetMenu("video_context", con));
	SetCursor(wxNullCursor);
	menu::OpenPopupMenu(context_menu.get(), this);
}

void VideoDisplay::OnKeyDown(wxKeyEvent &event) {
	event.StopPropagation();
	if (hotkey::check("Video", con, event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		return;
}

void VideoDisplay::SetZoom(double value) {
	zoomValue = std::max(value, .125);
	zoomBox->SetValue(wxString::Format("%g%%", zoomValue * 100.));
	UpdateSize();
}

void VideoDisplay::SetZoomFromBox(wxCommandEvent &) {
	wxString strValue = zoomBox->GetValue();
	strValue.EndsWith("%", &strValue);
	double value;
	if (strValue.ToDouble(&value)) {
		zoomValue = value / 100.;
		UpdateSize();
	}
}

void VideoDisplay::SetTool(VisualToolBase *new_tool) {
	toolBar->ClearTools();
	toolBar->Realize();
	toolBar->Show(false);

	tool.reset(new_tool);
	tool->SetToolbar(toolBar);
	tool->SetDisplayArea(viewport_left, viewport_top, viewport_width, viewport_height);

	// Update size as the new typesetting tool may have changed the subtoolbar size
	UpdateSize();
}

Vector2D VideoDisplay::GetMousePosition() const {
	return mouse_pos ? tool->ToScriptCoords(mouse_pos) : mouse_pos;
}
