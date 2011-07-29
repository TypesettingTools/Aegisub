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
#include <wx/display.h> /// Must be included last.
#endif

#include "dialog_detached_video.h"

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"

#include "main.h"
#include "persist_location.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"

DialogDetachedVideo::DialogDetachedVideo(agi::Context *context, const wxSize &initialDisplaySize)
: wxDialog(context->parent, -1, "Detached Video", wxDefaultPosition, wxSize(400,300), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxWANTS_CHARS)
, context(context)
, video_open(context->videoController->AddVideoOpenListener(&DialogDetachedVideo::OnVideoOpen, this))
{
	// Set obscure stuff
	SetExtraStyle((GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS) | wxWS_EX_PROCESS_UI_UPDATES);

	SetTitle(wxString::Format(_("Video: %s"), wxFileName(context->videoController->videoName).GetFullName()));

	// Set a background panel
	wxPanel *panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);
	
	// Video area;
	VideoBox *videoBox = new VideoBox(panel, true, context);
	videoBox->videoDisplay->SetClientSize(initialDisplaySize);

	// Set sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(videoBox,1,wxEXPAND | wxALL,5);
	panel->SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);

	// Ensure we can grow smaller, without these the window is locked to at least the initial size
	videoBox->videoDisplay->SetMinSize(wxSize(1,1));
	videoBox->SetMinSize(wxSize(1,1));
	SetMinSize(wxSize(1,1));

	persist.reset(new PersistLocation(this, "Video/Detached"));

	int display_index = wxDisplay::GetFromWindow(this);
	// Ensure that the dialog is no larger than the screen
	if (display_index != wxNOT_FOUND) {
		wxRect bounds_rect = GetRect();
		wxRect disp_rect = wxDisplay(display_index).GetClientArea();
		SetSize(std::min(bounds_rect.width, disp_rect.width), std::min(bounds_rect.height, disp_rect.height));
	}

	// Update
	OPT_SET("Video/Detached/Enabled")->SetBool(true);

	Bind(wxEVT_CLOSE_WINDOW, &DialogDetachedVideo::OnClose, this);
	Bind(wxEVT_ICONIZE, &DialogDetachedVideo::OnMinimize, this);
	Bind(wxEVT_KEY_DOWN, &DialogDetachedVideo::OnKeyDown, this);

	Show();
}

DialogDetachedVideo::~DialogDetachedVideo() { }

void DialogDetachedVideo::OnClose(wxCloseEvent&) {
	context->detachedVideo = 0;
	OPT_SET("Video/Detached/Enabled")->SetBool(false);
	Destroy();
}

void DialogDetachedVideo::OnMinimize(wxIconizeEvent &event) {
	if (event.IsIconized()) {
		// Force the video display to repaint as otherwise the last displayed
		// frame stays visible even though the dialog is minimized
		Hide();
		Show();
	}
}

void DialogDetachedVideo::OnKeyDown(wxKeyEvent &evt) {
	evt.StopPropagation();
	hotkey::check("Video Display", evt.GetKeyCode(), evt.GetUnicodeKey(), evt.GetModifiers());
}

void DialogDetachedVideo::OnVideoOpen() {
	if (!context->videoController->IsLoaded()) {
		context->detachedVideo = 0;
		Destroy();
	}
}
