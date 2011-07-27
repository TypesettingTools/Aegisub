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
#include <wx/settings.h>
#include <wx/display.h> /// Must be included last.
#endif

#include "include/aegisub/context.h"
#include "dialog_detached_video.h"
#include "frame_main.h"
#include "main.h"
#include "persist_location.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_slider.h"

/// @brief Constructor
/// @param par FrameMain this was spawned from
/// @param initialDisplaySize Initial size of the window
DialogDetachedVideo::DialogDetachedVideo(FrameMain *parent, agi::Context *context, const wxSize &initialDisplaySize)
: wxDialog(parent,-1,_T("Detached Video"),wxDefaultPosition,wxSize(400,300),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxWANTS_CHARS)
, parent(parent)
{
	// Set obscure stuff
	SetExtraStyle((GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS) | wxWS_EX_PROCESS_UI_UPDATES);

	// Set title
	wxFileName fn(context->videoController->videoName);
	SetTitle(wxString::Format(_("Video: %s"),fn.GetFullName().c_str()));

	// Set a background panel
	wxPanel *panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);
	
	// Video area;
	videoBox = new VideoBox(panel, true, context);
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
	parent->SetDisplayMode(0, -1);
	OPT_SET("Video/Detached/Enabled")->SetBool(true);

	// Copy the main accelerator table to this dialog
	wxAcceleratorTable *table = parent->GetAcceleratorTable();
	SetAcceleratorTable(*table);
}

/// @brief Destructor
DialogDetachedVideo::~DialogDetachedVideo() {
}

// Event table
BEGIN_EVENT_TABLE(DialogDetachedVideo,wxDialog)
	EVT_CLOSE(DialogDetachedVideo::OnClose)
	EVT_ICONIZE(DialogDetachedVideo::OnMinimize)
END_EVENT_TABLE()

/// @brief Close window
/// @param event UNUSED
void DialogDetachedVideo::OnClose(wxCloseEvent &WXUNUSED(event)) {
	OPT_SET("Video/Detached/Enabled")->SetBool(false);
	Destroy();
	parent->context->detachedVideo = 0;
	parent->SetDisplayMode(1,-1);
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
