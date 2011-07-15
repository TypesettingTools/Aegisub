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

/// @file video_box.cpp
/// @brief The video area in the main window, including surrounding tool bars
/// @ingroup main_ui video
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/filename.h>
#include <wx/mimetype.h>
#include <wx/rawbmp.h>
#include <wx/statline.h>
#endif

#include "include/aegisub/context.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "command/command.h"
#include "compat.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "toggle_bitmap.h"
#include "tooltip_manager.h"
#include "utils.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_slider.h"

static void add_button(wxWindow *parent, wxSizer *sizer, const char *command) {
	cmd::Command *c = cmd::get(command);
	wxBitmapButton *btn = new wxBitmapButton(parent, cmd::id(command), c->Icon(24));
	ToolTipManager::Bind(btn, c->StrHelp(), "Video", command);
	sizer->Add(btn, 0, wxTOP | wxLEFT | wxBOTTOM | wxALIGN_CENTER, 2);;
}

static void add_option(wxWindow *parent, wxSizer *sizer, const char *command, const char *option) {
	cmd::Command *c = cmd::get(command);
	ToggleBitmap *btn = new ToggleBitmap(parent, cmd::id(command), c->Icon(24));
	ToolTipManager::Bind(btn, c->StrHelp(), "Video", command);
	btn->SetValue(OPT_GET(option)->GetBool());
	sizer->Add(btn, 0, wxTOP | wxLEFT | wxBOTTOM | wxALIGN_CENTER, 2);
}

VideoBox::VideoBox(wxWindow *parent, bool isDetached, agi::Context *context)
: wxPanel (parent,-1)
, context(context)
{
	// Visual controls sub-toolbar
	visualSubToolBar = new wxToolBar(this,-1,wxDefaultPosition,wxDefaultSize,wxTB_HORIZONTAL | wxTB_BOTTOM | wxTB_FLAT);

	// Buttons
	wxSizer *videoBottomSizer = new wxBoxSizer(wxHORIZONTAL);
	add_button(this, videoBottomSizer, "video/play");
	add_button(this, videoBottomSizer, "video/play/line");
	add_button(this, videoBottomSizer, "video/stop");
	add_option(this, videoBottomSizer, "video/opt/autoscroll", "Video/Subtitle Sync");

	// Seek
	videoSlider = new VideoSlider(this, context);
	videoSlider->SetToolTip(_("Seek video."));

	// Position
	VideoPosition = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(110,20),wxTE_READONLY);
	VideoPosition->SetToolTip(_("Current frame time and number."));

	// Times of sub relative to video
	VideoSubsPos = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(110,20),wxTE_READONLY);
	VideoSubsPos->SetToolTip(_("Time of this frame relative to start and end of current subs."));

	// Zoom box
	wxArrayString choices;
	for (int i = 1 ; i <= 24; ++i) {
		choices.Add(wxString::Format("%g%%", i * 12.5));
	}
	zoomBox = new wxComboBox(this, -1, "75%", wxDefaultPosition, wxDefaultSize, choices, wxCB_DROPDOWN);

	// Typesetting buttons
	visualToolBar = new wxToolBar(this,-1,wxDefaultPosition,wxDefaultSize,wxTB_VERTICAL|wxTB_FLAT|wxTB_NODIVIDER);
	visualToolBar->AddTool(Video_Mode_Standard,_("Standard"),GETIMAGE(visual_standard_24),_("Standard mode, double click sets position."),wxITEM_RADIO);
	visualToolBar->AddTool(Video_Mode_Drag,_("Drag"),GETIMAGE(visual_move_24),_("Drag subtitles."),wxITEM_RADIO);
	visualToolBar->AddTool(Video_Mode_Rotate_Z,_("Rotate Z"),GETIMAGE(visual_rotatez_24),_("Rotate subtitles on their Z axis."),wxITEM_RADIO);
	visualToolBar->AddTool(Video_Mode_Rotate_XY,_("Rotate XY"),GETIMAGE(visual_rotatexy_24),_("Rotate subtitles on their X and Y axes."),wxITEM_RADIO);
	visualToolBar->AddTool(Video_Mode_Scale,_("Scale"),GETIMAGE(visual_scale_24),_("Scale subtitles on X and Y axes."),wxITEM_RADIO);
	visualToolBar->AddTool(Video_Mode_Clip,_("Clip"),GETIMAGE(visual_clip_24),_("Clip subtitles to a rectangle."),wxITEM_RADIO);
	visualToolBar->AddTool(Video_Mode_Vector_Clip,_("Vector Clip"),GETIMAGE(visual_vector_clip_24),_("Clip subtitles to a vectorial area."),wxITEM_RADIO);
	visualToolBar->AddSeparator();
	visualToolBar->AddTool(cmd::id("help/video"),_("Help"),cmd::get("help/video")->Icon(24),_("Open the manual page for Visual Typesetting."));
	visualToolBar->Realize();
	// Avoid ugly themed background on Vista and possibly also Win7
	visualToolBar->SetBackgroundStyle(wxBG_STYLE_COLOUR);
	visualToolBar->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

	// Display
	videoDisplay = new VideoDisplay(this,isDetached,zoomBox,this,context);

	// Top sizer
	// Detached and attached video needs different flags, see bugs #742 and #853
	int highSizerFlags = isDetached ? wxEXPAND : 0;
	wxSizer *topTopSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	visualSubToolBar->Show(false);
	topTopSizer->Add(visualToolBar,0,highSizerFlags,0);
	topTopSizer->Add(videoDisplay,1,highSizerFlags,0);
	topSizer->Add(topTopSizer,1,wxEXPAND,0);
	topSizer->Add(visualSubToolBar,0,wxEXPAND | wxBOTTOM,4);
	topSizer->Add(new wxStaticLine(this),0,wxEXPAND,0);

	// Sizers
	wxSizer *videoSliderSizer = new wxBoxSizer(wxHORIZONTAL);
	videoSliderSizer->Add(videoSlider,1,wxEXPAND|wxLEFT,0);
	videoBottomSizer->Add(VideoPosition,1,wxLEFT|wxALIGN_CENTER,5);
	videoBottomSizer->Add(VideoSubsPos,1,wxALIGN_CENTER,0);
	videoBottomSizer->Add(zoomBox, 0, wxALIGN_CENTER, 5);

	// If we're detached we do want to fill out as much space we can.
	// But if we're in the main window, the subs grid needs space more than us.
	wxSizer *VideoSizer = new wxBoxSizer(wxVERTICAL);
	VideoSizer->Add(topSizer,isDetached?1:0,wxEXPAND,0);
	VideoSizer->Add(videoSliderSizer,0,wxEXPAND,0);
	VideoSizer->Add(videoBottomSizer,0,wxEXPAND,0);
	if (!isDetached)
		VideoSizer->AddStretchSpacer(1);
	SetSizer(VideoSizer);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VideoBox::OnButton, this);
	Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &VideoBox::OnButton, this);

	slots.push_back(context->videoController->AddSeekListener(&VideoBox::UpdateTimeBoxes, this));
	slots.push_back(context->videoController->AddKeyframesListener(&VideoBox::UpdateTimeBoxes, this));
	slots.push_back(context->videoController->AddTimecodesListener(&VideoBox::UpdateTimeBoxes, this));
	slots.push_back(context->videoController->AddVideoOpenListener(&VideoBox::UpdateTimeBoxes, this));
	slots.push_back(context->ass->AddCommitListener(&VideoBox::UpdateTimeBoxes, this));
}

void VideoBox::OnButton(wxCommandEvent &evt) {
	if (evt.GetId() >= Video_Mode_Standard) {
		evt.Skip();
		return;
	}
#ifdef __APPLE__
	context->videoController->EnableAudioSync(!wxGetMouseState().CmdDown());
#else
	context->videoController->EnableAudioSync(!wxGetMouseState().ControlDown());
#endif
	cmd::call(context, evt.GetId());
}

void VideoBox::UpdateTimeBoxes() {
	if (!context->videoController->IsLoaded()) return;

	int frame = context->videoController->GetFrameN();
	int time = context->videoController->TimeAtFrame(frame, agi::vfr::EXACT);

	int h = time / 3600000;
	int m = time % 3600000 / 60000;
	int s = time % 60000 / 1000;
	int ms = time % 1000;

	// Set the text box for frame number and time
	VideoPosition->SetValue(wxString::Format("%01i:%02i:%02i.%03i - %i", h, m, s, ms, frame));
	if (binary_search(context->videoController->GetKeyFrames().begin(), context->videoController->GetKeyFrames().end(), frame)) {
		// Set the background color to indicate this is a keyframe
		VideoPosition->SetBackgroundColour(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Selection")->GetColour()));
		VideoPosition->SetForegroundColour(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));
	}
	else {
		VideoPosition->SetBackgroundColour(wxNullColour);
		VideoPosition->SetForegroundColour(wxNullColour);
	}

	AssDialogue *active_line = context->selectionController->GetActiveLine();
	if (!active_line) {
		VideoSubsPos->SetValue("");
	}
	else {
		VideoSubsPos->SetValue(wxString::Format(
			"%+dms; %+dms",
			time - active_line->Start.GetMS(),
			time - active_line->End.GetMS()));
	}

}
