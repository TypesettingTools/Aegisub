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


////////////
// Includes
#include "config.h"

#ifndef AGI_PRE
#include <wx/settings.h>
#endif

#include "ass_dialogue.h"
#include "main.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"
#include "video_display.h"
#include "video_slider.h"


/// @brief Constructor 
/// @param parent 
/// @param id     
///
VideoSlider::VideoSlider (wxWindow* parent, wxWindowID id)
: wxWindow (parent,id,wxDefaultPosition,wxDefaultSize,wxWANTS_CHARS | wxFULL_REPAINT_ON_RESIZE)
{
	val = 0;
	min = 0;
	max = 1;
	Display = NULL;
	SetClientSize(20,25);
	SetMinSize(wxSize(20, 25));
	locked = false;
	SetRange(0,1);
}



/// @brief Get value 
/// @return 
///
int VideoSlider::GetValue() {
	return val;
}



/// @brief Set value 
/// @param value 
/// @return 
///
void VideoSlider::SetValue(int value) {
	if (locked) return;
	val = value;
	if (val < min) val = min;
	if (val > max) val = max;
	Refresh(false);
}



/// @brief Set range 
/// @param from 
/// @param to   
///
void VideoSlider::SetRange(int from,int to) {
	if (from > to) from = to;
	locked = false;
	min = from;
	max = to;
	if (val < from) val = from;
	if (val > to) val = to;
}



/// @brief Get value at X 
/// @param x 
/// @return 
///
int VideoSlider::GetValueAtX(int x) {
	// Get dimensions
	int w,h;
	GetClientSize(&w,&h);

	// Special case
	if (w <= 10) return 0;

	// Calculate
	return (int64_t)(x-5)*(int64_t)(max-min)/(int64_t)(w-10)+min;
}



/// @brief Get X at value 
/// @param value 
/// @return 
///
int VideoSlider::GetXAtValue(int value) {
	// Get dimensions
	int w,h;
	GetClientSize(&w,&h);

	// Special case
	if (max-min <= 0) return 0;

	// Calculate
	return (int64_t)(value-min)*(int64_t)(w-10)/(int64_t)(max-min)+5;
}



/// @brief Next frame hotkey 
/// @return 
///
void VideoSlider::NextFrame() {
	if (VideoContext::Get()->IsPlaying()) return;

	//don't request out of range frames
	if (GetValue() < max) VideoContext::Get()->PlayNextFrame();
	Refresh(false);
	Update();
}



/// @brief Previous frame hotkey 
/// @return 
///
void VideoSlider::PrevFrame() {
	if (VideoContext::Get()->IsPlaying()) return;

	//don't request out of range frames
	if (GetValue() > min) VideoContext::Get()->PlayPrevFrame();
	Refresh(false);
	Update();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(VideoSlider, wxWindow)
	EVT_MOUSE_EVENTS(VideoSlider::OnMouse)
	EVT_KEY_DOWN(VideoSlider::OnKeyDown)
	EVT_PAINT(VideoSlider::OnPaint)
	EVT_SET_FOCUS(VideoSlider::OnFocus)
	EVT_KILL_FOCUS(VideoSlider::OnFocus)
	EVT_ERASE_BACKGROUND(VideoSlider::OnEraseBackground)
END_EVENT_TABLE()



/// @brief Change position 
/// @return 
///
void VideoSlider::UpdateVideo() {
	if (Display) {
		if (VideoContext::Get()->IsPlaying()) return;
		locked = true;
		VideoContext::Get()->JumpToFrame(GetValue());
		locked = false;
	}
}



/// @brief Mouse events 
/// @param event 
/// @return 
///
void VideoSlider::OnMouse(wxMouseEvent &event) {
	// Coordinates
	int x = event.GetX();
	//int y = event.GetY();
	bool shift = event.m_shiftDown;

	// Left click
	if (event.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
		// Check if it's OK to drag
		bool canDrag = wxWindow::FindFocus() == this;
		if (!canDrag) {
			int tolerance = 4;
			int curX = GetXAtValue(GetValue());
			if (x-curX < -tolerance || x-curX > tolerance) canDrag = true;
		}

		// Drag
		if (canDrag) {
			// Shift click to snap to keyframe
			if (shift && Display) {
				std::vector<int> KeyFrames = VideoContext::Get()->GetKeyFrames();
				int keys = KeyFrames.size();
				int clickedFrame = GetValueAtX(x);
				int closest = 0;
				int cur;

				// Find closest
				for (int i=0;i<keys;i++) {
					cur = KeyFrames[i];
					if (abs(cur-clickedFrame) < abs(closest-clickedFrame)) {
						closest = cur;
					}
				}

				// Jump to frame
				if (closest == GetValue()) return;
				SetValue(closest);
			}

			// Normal click
			else {
				int go = GetValueAtX(x);
				if (go == GetValue()) return;
				SetValue(go);
			}
			Refresh(false);

			// Playing?
			if (VideoContext::Get()->IsPlaying()) {
				VideoContext::Get()->Stop();
				UpdateVideo();
				VideoContext::Get()->Play();
			}
			else UpdateVideo();
		}

		// Get focus
		SetFocus();
	}

	// Right/middle click
	if (event.ButtonDown(wxMOUSE_BTN_RIGHT) || event.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		SetFocus();
	}

	// Something else
	else if (!VideoContext::Get()->IsPlaying()) event.Skip();
}



/// @brief Key down event 
/// @param event 
/// @return 
///
void VideoSlider::OnKeyDown(wxKeyEvent &event) {
	if (VideoContext::Get()->IsPlaying()) return;

	// Get flags
	int key = event.GetKeyCode();
#ifdef __APPLE__
	bool ctrl = event.m_metaDown;
#else
	bool ctrl = event.m_controlDown;
#endif
	bool alt = event.m_altDown;
	bool shift = event.m_shiftDown;
	int direction = 0;

	// Pick direction
	if (key == WXK_LEFT) direction = -1;
	else if (key == WXK_RIGHT) direction = 1;

	// If a direction was actually pressed
	if (direction) {
		// Standard move
		if (!ctrl && !shift && !alt) {
			if (direction == 1) NextFrame();
			else PrevFrame();
			return;
		}

		// Fast move
		if (!ctrl && !shift && alt) {
			if (VideoContext::Get()->IsPlaying()) return;
			int target = MID(min,GetValue() + direction * OPT_GET("Video/Slider/Fast Jump Step")->GetInt(),max);
			if (target != GetValue()) VideoContext::Get()->JumpToFrame(target);
			return;
		}

		// Boundaries
		if (ctrl && !shift && !alt) {
			// Prepare
			wxArrayInt sel = grid->GetSelection();
			int cur;
			if (sel.Count() > 0) cur = sel[0];
			else {
				grid->SetActiveLine(grid->GetDialogue(0));
				grid->SelectRow(0);
				cur = 0;
			}
			AssDialogue *curDiag = grid->GetDialogue(cur);
			if (!curDiag) return;

			// Jump to next sub boundary
			if (direction != 0) {
				int target1 = VideoContext::Get()->FrameAtTime(curDiag->Start.GetMS(),agi::vfr::START);
				int target2 = VideoContext::Get()->FrameAtTime(curDiag->End.GetMS(),agi::vfr::END);
				bool drawn = false;

				// Forward
				if (direction == 1) {
					if (VideoContext::Get()->GetFrameN() < target1) VideoContext::Get()->JumpToFrame(target1);
					else if (VideoContext::Get()->GetFrameN() < target2) VideoContext::Get()->JumpToFrame(target2);
					else {
						if (cur+1 >= grid->GetRows()) return;
						grid->SetActiveLine(grid->GetDialogue(cur+1));
						grid->SelectRow(cur+1);
						grid->MakeCellVisible(cur+1,0);
						grid->SetVideoToSubs(true);
						grid->Refresh(false);
						drawn = true;
					}
					return;
				}

				// Backward
				else {
					if (VideoContext::Get()->GetFrameN() > target2) VideoContext::Get()->JumpToFrame(target2);
					else if (VideoContext::Get()->GetFrameN() > target1) VideoContext::Get()->JumpToFrame(target1);
					else {
						if (cur-1 < 0) return;
						grid->SetActiveLine(grid->GetDialogue(cur-1));
						grid->SelectRow(cur-1);
						grid->MakeCellVisible(cur-1,0);
						grid->SetVideoToSubs(false);
						grid->Refresh(false);
						drawn = true;
					}
					return;
				}
			}
		}

		// Snap to keyframe
		if (shift && !ctrl && !alt) {
			if (direction != 0) {
				// Prepare
				int prevKey = 0;
				int nextKey = VideoContext::Get()->GetLength()-1;
				std::vector<int> KeyFrames = VideoContext::Get()->GetKeyFrames();
				int keys = KeyFrames.size();
				int cur = VideoContext::Get()->GetFrameN();
				int i;
				int temp;

				// Find previous keyframe
				// This algorithm does unnecessary loops, but it ensures it works even if keyframes are out of order.
				for (i=0;i<keys;i++) {
					temp = KeyFrames[i];
					if (temp < cur && temp > prevKey) prevKey = temp;
				}

				// Find next keyframe
				for (i=0;i<keys;i++) {
					temp = KeyFrames[i];
					if (temp > cur && temp < nextKey) nextKey = KeyFrames[i];
				}

				if (direction == -1) VideoContext::Get()->JumpToFrame(prevKey);
				if (direction == 1) VideoContext::Get()->JumpToFrame(nextKey);
				return;
			}
		}
	}

	// Forward up/down to grid
	if (key == WXK_UP || key == WXK_DOWN) {
		grid->GetEventHandler()->ProcessEvent(event);
		grid->SetFocus();
		return;
	}

	// Forward other keys to video display
	if (Display) {
		Display->GetEventHandler()->ProcessEvent(event);
		return;
	}

	event.Skip();
}



/// @brief Paint event 
/// @param event 
///
void VideoSlider::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	DrawImage(dc);
}



/// @brief Draw image 
/// @param destdc 
///
void VideoSlider::DrawImage(wxDC &destdc) {
	// Get dimensions
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
	if (Display && OPT_GET("Video/Slider/Show Keyframes")->GetBool()) {
		dc.SetPen(wxPen(shad));
		std::vector<int> KeyFrames = VideoContext::Get()->GetKeyFrames();
		int keys = KeyFrames.size();
		for (int i=0;i<keys;i++) {
			curX = GetXAtValue(KeyFrames[i]);
			dc.DrawLine(curX,2,curX,8);
		}
	}

	// Draw cursor
	curX = GetXAtValue(GetValue());

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



/// @brief Update image 
///
void VideoSlider::UpdateImage () {
	//wxClientDC dc(this);
	//DrawImage(dc);
	Refresh(false);
}



/// @brief Focus change 
/// @param event 
///
void VideoSlider::OnFocus(wxFocusEvent &event) {
	Refresh(false);
}


