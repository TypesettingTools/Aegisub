// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file dialog_detached_video.cpp
/// @brief Detached video window
/// @ingroup main_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/filename.h>
#endif

#include "dialog_detached_video.h"
#include "frame_main.h"
#include "options.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_slider.h"

#undef min
#undef max


/// @brief Constructor
/// @param par FrameMain this was spawned from
/// @param initialDisplaySize Initial size of the window
DialogDetachedVideo::DialogDetachedVideo(FrameMain *par, const wxSize &initialDisplaySize)
: wxDialog(par,-1,_T("Detached Video"),wxDefaultPosition,wxSize(400,300),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxWANTS_CHARS)
{
	parent = par;

	// Set up window
	int x = Options.AsInt(_T("Detached video last x"));
	int y = Options.AsInt(_T("Detached video last y"));
	if (x != -1 && y != -1) SetPosition(wxPoint(x,y));
	if (Options.AsBool(_T("Detached video maximized"))) Maximize();

	// Set obscure stuff
	SetExtraStyle((GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS) | wxWS_EX_PROCESS_UI_UPDATES);

	// Set title
	wxFileName fn(VideoContext::Get()->videoName);
	SetTitle(wxString::Format(_("Video: %s"),fn.GetFullName().c_str()));

	// Set a background panel
	wxPanel *panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);
	
	// Video area;
	videoBox = new VideoBox(panel, true);
	videoBox->videoDisplay->freeSize = true;
	videoBox->videoDisplay->SetClientSize(initialDisplaySize);
	videoBox->videoSlider->grid = par->SubsBox;

	// Set sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(videoBox,1,wxEXPAND | wxALL,5);
	panel->SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);

	// Ensure we can grow smaller, without these the window is locked to at least the initial size
	videoBox->videoDisplay->SetMinSize(wxSize(1,1));
	videoBox->SetMinSize(wxSize(1,1));
	SetMinSize(wxSize(1,1));

	int display_index = wxDisplay::GetFromWindow(this);
	if (display_index == wxNOT_FOUND)
	{
		int caption_size = wxSystemSettings::GetMetric(wxSYS_CAPTION_Y, this);
		Move(par->GetPosition() + wxPoint(caption_size, caption_size));
	}
	else
	{
		wxRect bounds_rect = GetRect();
		wxRect disp_rect = wxDisplay(display_index).GetClientArea();

		// Ensure our x/y position is past the top left of the display
		int new_x = std::max(bounds_rect.x, disp_rect.x);
		int new_y = std::max(bounds_rect.y, disp_rect.y);
		// Pick the smallest size of display and window.
		// By doing this, we're guaranteed to get a width/height that fits on the display
		// and won't have to adjust width/height any further.
		int new_w = std::min(bounds_rect.width, disp_rect.width);
		int new_h = std::min(bounds_rect.height, disp_rect.height);

		// Check if bottom right corner is outside display and move inside then
		if (new_x + new_w > disp_rect.x + disp_rect.width)
			new_x = disp_rect.x + disp_rect.width - new_w;
		if (new_y + new_h > disp_rect.y + disp_rect.height)
			new_y = disp_rect.y + disp_rect.height - new_h;

		SetSize(new_x, new_y, new_w, new_h, wxSIZE_ALLOW_MINUS_ONE);
	}

	// Update
	parent->SetDisplayMode(0, -1);
	GetPosition(&x, &y);
	Options.SetInt(_T("Detached video last x"), x);
	Options.SetInt(_T("Detached video last y"), y);
	Options.SetBool(_T("Detached video"),true);
	Options.Save();

	// Copy the main accelerator table to this dialog
	wxAcceleratorTable *table = par->GetAcceleratorTable();
	SetAcceleratorTable(*table);
}

/// @brief Destructor
DialogDetachedVideo::~DialogDetachedVideo() {
	Options.SetBool(_T("Detached video maximized"),IsMaximized());
	Options.Save();
}

// Event table
BEGIN_EVENT_TABLE(DialogDetachedVideo,wxDialog)
	EVT_CLOSE(DialogDetachedVideo::OnClose)
	EVT_MOVE(DialogDetachedVideo::OnMove)
	EVT_ICONIZE(DialogDetachedVideo::OnMinimize)
END_EVENT_TABLE()

/// @brief Close window
/// @param event UNUSED
void DialogDetachedVideo::OnClose(wxCloseEvent &WXUNUSED(event)) {
	FrameMain *par = parent;
	Options.SetBool(_T("Detached video"),false);
	Destroy();
	par->detachedVideo = NULL;
	par->SetDisplayMode(1,-1);
}

/// @brief Move window 
/// @param event 
void DialogDetachedVideo::OnMove(wxMoveEvent &event) {
	wxPoint pos = event.GetPosition();
	Options.SetInt(_T("Detached video last x"),pos.x);
	Options.SetInt(_T("Detached video last y"),pos.y);
}

/// @brief Minimize event handler
/// @param event
void DialogDetachedVideo::OnMinimize(wxIconizeEvent &event) {
	if (event.IsIconized()) {
		// Force the video display to repaint as otherwise the last displayed
		// frame stays visible even though the dialog is minimized
		Hide();
		Show();
	}
}
