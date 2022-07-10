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

/// @file dialog_detached_video.cpp
/// @brief Detached video window
/// @ingroup main_ui
///

#include "dialog_detached_video.h"

#include "format.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "options.h"
#include "persist_location.h"
#include "project.h"
#include "utils.h"
#include "video_box.h"
#include "video_controller.h"
#include "video_display.h"

#include <libaegisub/format_path.h>
#include <libaegisub/make_unique.h>

#include <boost/filesystem/path.hpp>

#include <wx/display.h> /// Must be included last.
#include <wx/sizer.h>

DialogDetachedVideo::DialogDetachedVideo(agi::Context* context)
    : wxDialog(context->parent, -1, "Detached Video", wxDefaultPosition, wxSize(400, 300),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX |
                   wxWANTS_CHARS),
      context(context), old_display(context->videoDisplay), old_slider(context->videoSlider),
      video_open(
          context->project->AddVideoProviderListener(&DialogDetachedVideo::OnVideoOpen, this)) {
	// Set obscure stuff
	SetExtraStyle((GetExtraStyle() & ~wxWS_EX_BLOCK_EVENTS) | wxWS_EX_PROCESS_UI_UPDATES);

	SetTitle(fmt_tl("Video: %s", context->project->VideoName().filename()));

	old_display->Unload();

	// Video area;
	auto videoBox = new VideoBox(this, true, context);
	context->videoDisplay->SetMinClientSize(old_display->GetClientSize());
	videoBox->Layout();

	// Set sizer
	wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(videoBox, 1, wxEXPAND);
	SetSizerAndFit(mainSizer);

	// Ensure we can grow smaller, without these the window is locked to at least the initial size
	context->videoDisplay->SetMinSize(wxSize(1, 1));
	videoBox->SetMinSize(wxSize(1, 1));
	SetMinSize(wxSize(1, 1));

	persist = agi::make_unique<PersistLocation>(this, "Video/Detached");

	int display_index = wxDisplay::GetFromWindow(this);
	// Ensure that the dialog is no larger than the screen
	if(display_index != wxNOT_FOUND) {
		wxRect bounds_rect = GetRect();
		wxRect disp_rect = wxDisplay(display_index).GetClientArea();
		SetSize(std::min(bounds_rect.width, disp_rect.width),
		        std::min(bounds_rect.height, disp_rect.height));
	}

	OPT_SET("Video/Detached/Enabled")->SetBool(true);

	Bind(wxEVT_CLOSE_WINDOW, &DialogDetachedVideo::OnClose, this);
	Bind(wxEVT_ICONIZE, &DialogDetachedVideo::OnMinimize, this);
	Bind(wxEVT_CHAR_HOOK, &DialogDetachedVideo::OnKeyDown, this);

	AddFullScreenButton(this);
}

DialogDetachedVideo::~DialogDetachedVideo() {}

void DialogDetachedVideo::OnClose(wxCloseEvent& evt) {
	context->videoDisplay->Destroy();

	context->videoDisplay = old_display;
	context->videoSlider = old_slider;

	OPT_SET("Video/Detached/Enabled")->SetBool(false);

	context->videoController->JumpToFrame(context->videoController->GetFrameN());

	evt.Skip();
}

void DialogDetachedVideo::OnMinimize(wxIconizeEvent& event) {
	if(event.IsIconized()) {
		// Force the video display to repaint as otherwise the last displayed
		// frame stays visible even though the dialog is minimized
		Hide();
		Show();
	}
}

void DialogDetachedVideo::OnKeyDown(wxKeyEvent& evt) {
	hotkey::check("Video Display", context, evt);
}

void DialogDetachedVideo::OnVideoOpen(AsyncVideoProvider* new_provider) {
	if(new_provider)
		SetTitle(fmt_tl("Video: %s", context->project->VideoName().filename()));
	else {
		Close();
		OPT_SET("Video/Detached/Enabled")->SetBool(true);
	}
}
