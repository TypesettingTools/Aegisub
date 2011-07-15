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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_slider.cpp
/// @brief Seek-bar control for video
/// @ingroup custom_control
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/settings.h>
#endif

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "main.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"
#include "video_slider.h"

VideoSlider::VideoSlider (wxWindow* parent, agi::Context *c)
: wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS | wxFULL_REPAINT_ON_RESIZE)
, vc(c->videoController)
, grid(c->subsGrid)
, val(0)
, max(1)
{
	SetClientSize(20,25);
	SetMinSize(wxSize(20, 25));
	slots.push_back(OPT_SUB("Video/Slider/Show Keyframes", &wxWindow::Refresh, this, false, (wxRect*)NULL));
	slots.push_back(vc->AddSeekListener(&VideoSlider::SetValue, this));
	slots.push_back(vc->AddVideoOpenListener(&VideoSlider::VideoOpened, this));
	slots.push_back(vc->AddKeyframesListener(&VideoSlider::KeyframesChanged, this));

	if (vc->IsLoaded()) {
		VideoOpened();
	}
}

void VideoSlider::SetValue(int value) {
	if (val == value) return;
	val = mid(0, value, max);
	Refresh(false);
}

void VideoSlider::VideoOpened() {
	max = vc->GetLength() - 1;
	keyframes = vc->GetKeyFrames();
	Refresh(false);
}

void VideoSlider::KeyframesChanged(std::vector<int> const& newKeyframes) {
	keyframes = newKeyframes;
	Refresh(false);
}

int VideoSlider::GetValueAtX(int x) {
	int w,h;
	GetClientSize(&w,&h);

	// Special case
	if (w <= 10) return 0;

	return (int64_t)(x-5)*(int64_t)max/(int64_t)(w-10);
}

int VideoSlider::GetXAtValue(int value) {
	if (max <= 0) return 0;

	int w,h;
	GetClientSize(&w,&h);
	return (int64_t)value*(int64_t)(w-10)/(int64_t)max+5;
}

BEGIN_EVENT_TABLE(VideoSlider, wxWindow)
	EVT_MOUSE_EVENTS(VideoSlider::OnMouse)
	EVT_KEY_DOWN(VideoSlider::OnKeyDown)
	EVT_PAINT(VideoSlider::OnPaint)
	EVT_SET_FOCUS(VideoSlider::OnFocus)
	EVT_KILL_FOCUS(VideoSlider::OnFocus)
	EVT_ERASE_BACKGROUND(VideoSlider::OnEraseBackground)
END_EVENT_TABLE()

void VideoSlider::OnMouse(wxMouseEvent &event) {
	int x = event.GetX();

	if (event.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
		// If the slider didn't already have focus, don't seek if the user
		// clicked very close to the current location as they were probably
		// just trying to focus the slider
		if (wxWindow::FindFocus() != this && abs(x - GetXAtValue(val)) < 4) {
			SetFocus();
			return;
		}

		// Shift click to snap to keyframe
		if (event.m_shiftDown) {
			int clickedFrame = GetValueAtX(x);
			std::vector<int>::const_iterator pos = lower_bound(keyframes.begin(), keyframes.end(), clickedFrame);
			if (pos == keyframes.end())
				--pos;
			else if (pos + 1 != keyframes.end() && clickedFrame - *pos > (*pos + 1) - clickedFrame)
				++pos;

			if (*pos == val) return;
			SetValue(*pos);
		}

		// Normal click
		else {
			int go = GetValueAtX(x);
			if (go == val) return;
			SetValue(go);
		}

		if (vc->IsPlaying()) {
			vc->Stop();
			vc->JumpToFrame(val);
			vc->Play();
		}
		else
			vc->JumpToFrame(val);
		SetFocus();
		return;
	}

	if (event.ButtonDown(wxMOUSE_BTN_RIGHT) || event.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		SetFocus();
	}

	else if (!vc->IsPlaying())
		event.Skip();
}

void VideoSlider::OnKeyDown(wxKeyEvent &event) {
	if (vc->IsPlaying()) return;

	if (hotkey::check("Video", event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		return;

	// Forward up/down to grid
	if (event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_DOWN) {
		grid->GetEventHandler()->ProcessEvent(event);
		grid->SetFocus();
		return;
	}

	event.Skip();
}

void VideoSlider::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	DrawImage(dc);
}

void VideoSlider::DrawImage(wxDC &destdc) {
	int w,h;
	GetClientSize(&w,&h);

	// Back buffer
	wxMemoryDC dc;
	wxBitmap bmp(w,h);
	dc.SelectObject(bmp);

	// Colors
	wxColour shad = wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW);
	wxColour high = wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT);
	wxColour face = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour sel(123,251,232);
	wxColour notSel(sel.Red()*2/5,sel.Green()*2/5,sel.Blue()*2/5);
	wxColour bord(0,0,0);
	int x1,x2,y1,y2;

	// Background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(face);
	dc.DrawRectangle(0,0,w,h);

	// Selection border
	bool selected = wxWindow::FindFocus() == this;
	if (selected) {
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxPen(shad,1,wxDOT));
		dc.DrawRectangle(0,0,w,h);
	}

	// Draw slider
	x1 = 5;
	x2 = w-5;
	y1 = 8;
	y2 = h-8;
	dc.SetPen(wxPen(shad));
	dc.DrawLine(x1,y1,x2,y1);
	dc.DrawLine(x1,y1,x1,y2);
	dc.SetPen(wxPen(high));
	dc.DrawLine(x1,y2,x2,y2);
	dc.DrawLine(x2,y1,x2,y2);

	// Draw keyframes
	int curX;
	if (OPT_GET("Video/Slider/Show Keyframes")->GetBool()) {
		dc.SetPen(wxPen(shad));
		for (size_t i=0;i<keyframes.size();i++) {
			curX = GetXAtValue(keyframes[i]);
			dc.DrawLine(curX,2,curX,8);
		}
	}

	// Draw cursor
	curX = GetXAtValue(val);

	// Fill bg
	dc.SetBrush(wxBrush(face));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(curX-2,y1-1,4,y2-y1+5);
	dc.SetBrush(wxNullBrush);

	// Draw cursor highlights
	dc.SetPen(wxPen(high));
	dc.DrawLine(curX,y1-2,curX-4,y1+2);
	dc.DrawLine(curX-3,y1+2,curX-3,y2+5);

	// Draw cursor shades
	dc.SetPen(wxPen(shad));
	dc.DrawLine(curX+1,y1-1,curX+4,y1+2);
	dc.DrawLine(curX+3,y1+2,curX+3,y2+5);
	dc.DrawLine(curX-3,y2+4,curX+3,y2+4);

	// Draw cursor outline
	dc.SetPen(wxPen(bord));
	dc.DrawLine(curX,y1-3,curX-4,y1+1);
	dc.DrawLine(curX,y1-3,curX+4,y1+1);
	dc.DrawLine(curX-4,y1+1,curX-4,y2+5);
	dc.DrawLine(curX+4,y1+1,curX+4,y2+5);
	dc.DrawLine(curX-3,y2+5,curX+4,y2+5);
	dc.DrawLine(curX-3,y2,curX+4,y2);

	// Draw selection
	dc.SetPen(*wxTRANSPARENT_PEN);
	if (selected) dc.SetBrush(wxBrush(sel));
	else dc.SetBrush(wxBrush(notSel));
	dc.DrawRectangle(curX-3,y2+1,7,4);

	// Draw final
	destdc.Blit(0,0,w,h,&dc,0,0);
}

void VideoSlider::OnFocus(wxFocusEvent &event) {
	Refresh(false);
}
