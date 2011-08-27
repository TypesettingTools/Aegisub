// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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

/// @file audio_box.cpp
/// @brief The entire audio area in the main UI, containing display and related toolbars
/// @ingroup audio_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <math.h>

#include <wx/bmpbuttn.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/string.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/laywin.h> // Keep this last so wxSW_3D is set.
#endif

#include <libaegisub/log.h>

#include "audio_box.h"

#include "include/aegisub/context.h"
#include "include/aegisub/toolbar.h"

#include "audio_controller.h"
#include "audio_display.h"
#include "audio_karaoke.h"
#include "audio_timing.h"
#include "command/command.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "toggle_bitmap.h"
#include "selection_controller.h"
#include "tooltip_manager.h"
#include "utils.h"

enum AudioBoxControlIDs {
	Audio_Scrollbar = 1600,
	Audio_Horizontal_Zoom,
	Audio_Vertical_Zoom,
	Audio_Volume,
	Audio_Button_Karaoke,

	Audio_Button_Join,		/// Karaoke -> Enter join mode.
	Audio_Button_Split,		/// Karaoke -> Enter split mode.
	Audio_Button_Accept,	/// Karaoke -> Split/Join mode -> Accept.
	Audio_Button_Cancel		/// Karaoke -> Split/Join mode -> Cancel.
};

AudioBox::AudioBox(wxWindow *parent, agi::Context *context)
: wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxBORDER_RAISED)
, audioDisplay(new AudioDisplay(this, context->audioController, context))
, controller(context->audioController)
, timing_controller_dialogue(CreateDialogueTimingController(controller, context->selectionController, context->ass))
, context(context)
, karaokeMode(false)
{
	// Zoom
	HorizontalZoom = new wxSlider(this,Audio_Horizontal_Zoom,0,-50,30,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH);
	HorizontalZoom->SetToolTip(_("Horizontal zoom"));
	VerticalZoom = new wxSlider(this,Audio_Vertical_Zoom,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	VerticalZoom->SetToolTip(_("Vertical zoom"));
	VolumeBar = new wxSlider(this,Audio_Volume,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	VolumeBar->SetToolTip(_("Audio Volume"));
	bool link = OPT_GET("Audio/Link")->GetBool();
	if (link) {
		VolumeBar->SetValue(VerticalZoom->GetValue());
		VolumeBar->Enable(false);
	}

	// VertVol sider
	wxSizer *VertVol = new wxBoxSizer(wxHORIZONTAL);
	VertVol->Add(VerticalZoom,1,wxEXPAND,0);
	VertVol->Add(VolumeBar,1,wxEXPAND,0);
	wxSizer *VertVolArea = new wxBoxSizer(wxVERTICAL);
	VertVolArea->Add(VertVol,1,wxEXPAND,0);

	cmd::Command *link_command = cmd::get("audio/opt/vertical_link");
	ToggleBitmap *link_btn = new ToggleBitmap(this, cmd::id("audio/opt/vertical_link"), link_command->Icon(16), wxSize(20, -1));
	ToolTipManager::Bind(link_btn, link_command->StrHelp(), "Audio", "audio/opt/vertical_link");
	link_btn->SetValue(OPT_GET("Audio/Link")->GetBool());
	link_btn->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &AudioBox::OnVerticalLinkButton, this);
	VertVolArea->Add(link_btn, 0, wxRIGHT | wxALIGN_CENTER | wxEXPAND, 0);
	OPT_SUB("Audio/Link", bind(&AudioBox::OnVerticalLink, this, std::tr1::placeholders::_1));

	// Top sizer
	wxSizer *TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(audioDisplay,1,wxEXPAND,0);
	TopSizer->Add(HorizontalZoom,0,wxEXPAND,0);
	TopSizer->Add(VertVolArea,0,wxEXPAND,0);

	// Buttons sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	KaraokeButton = new wxBitmapToggleButton(this,Audio_Button_Karaoke,GETIMAGE(kara_mode_16));
	KaraokeButton->SetToolTip(_("Toggle karaoke mode"));
	ButtonSizer->Add(KaraokeButton,0,wxRIGHT|wxEXPAND,0);

	// Karaoke sizer
	karaokeSizer = new wxBoxSizer(wxHORIZONTAL);

	JoinSplitSizer = new wxBoxSizer(wxHORIZONTAL);
	JoinButton = new wxBitmapButton(this,Audio_Button_Join,GETIMAGE(kara_join_16));
	JoinButton->SetToolTip(_("Join selected syllables"));
	SplitButton = new wxBitmapButton(this,Audio_Button_Split,GETIMAGE(kara_split_16));
	SplitButton->SetToolTip(_("Enter split-mode"));
	JoinSplitSizer->Add(JoinButton,0,wxRIGHT|wxEXPAND,0);
	JoinSplitSizer->Add(SplitButton,0,wxRIGHT|wxEXPAND,0);

	CancelAcceptSizer = new wxBoxSizer(wxHORIZONTAL);
	CancelButton = new wxBitmapButton(this,Audio_Button_Cancel,GETIMAGE(kara_split_accept_16));
	CancelButton->SetToolTip(_("Commit splits and leave split-mode"));
	AcceptButton = new wxBitmapButton(this,Audio_Button_Accept,GETIMAGE(kara_split_cancel_16));
	AcceptButton->SetToolTip(_("Discard all splits and leave split-mode"));
	CancelAcceptSizer->Add(CancelButton,0,wxRIGHT|wxEXPAND,0);
	CancelAcceptSizer->Add(AcceptButton,0,wxRIGHT|wxEXPAND,0);

	karaokeSizer->Add(JoinSplitSizer,0,wxRIGHT|wxEXPAND,0);
	karaokeSizer->Add(CancelAcceptSizer,0,wxRIGHT|wxEXPAND,0);

	audioKaraoke = new AudioKaraoke(this);
	audioKaraoke->box = this;
	audioKaraoke->display = audioDisplay;
	karaokeSizer->Add(audioKaraoke,1,wxEXPAND,0);

	// Main sizer
	wxBoxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,1,wxEXPAND|wxALL,3);
	MainSizer->Add(toolbar::GetToolbar(this, "audio", context, "Audio"),0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,3);
	MainSizer->Add(ButtonSizer);
	MainSizer->Add(karaokeSizer,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,3);
	MainSizer->AddSpacer(3);
	SetSizer(MainSizer);

	SetKaraokeButtons(); // Decide which one to show or hide.

	controller->SetTimingController(timing_controller_dialogue);
}

AudioBox::~AudioBox() { }

BEGIN_EVENT_TABLE(AudioBox,wxPanel)
	EVT_COMMAND_SCROLL(Audio_Horizontal_Zoom, AudioBox::OnHorizontalZoom)
	EVT_COMMAND_SCROLL(Audio_Vertical_Zoom, AudioBox::OnVerticalZoom)
	EVT_COMMAND_SCROLL(Audio_Volume, AudioBox::OnVolume)

	EVT_BUTTON(Audio_Button_Join,AudioBox::OnJoin)
	EVT_BUTTON(Audio_Button_Split,AudioBox::OnSplit)
	EVT_BUTTON(Audio_Button_Cancel,AudioBox::OnCancel)
	EVT_BUTTON(Audio_Button_Accept,AudioBox::OnAccept)

	EVT_TOGGLEBUTTON(Audio_Button_Karaoke, AudioBox::OnKaraoke)
END_EVENT_TABLE()

void AudioBox::OnVerticalLinkButton(wxCommandEvent &) {
	cmd::call("audio/opt/vertical_link", context);
}

void AudioBox::OnHorizontalZoom(wxScrollEvent &event) {
	// Negate the value, we want zoom out to be on bottom and zoom in on top,
	// but the control doesn't want negative on bottom and positive on top.
	audioDisplay->SetZoomLevel(-event.GetPosition());
}

void AudioBox::OnVerticalZoom(wxScrollEvent &event) {
	int pos = mid(1, event.GetPosition(), 100);
	double value = pow(pos / 50.0, 3);
	audioDisplay->SetAmplitudeScale(value);
	if (!VolumeBar->IsEnabled()) {
		VolumeBar->SetValue(pos);
		controller->SetVolume(value);
	}
}

void AudioBox::OnVolume(wxScrollEvent &event) {
	int pos = mid(1, event.GetPosition(), 100);
	controller->SetVolume(pow(pos / 50.0, 3));
}

void AudioBox::OnVerticalLink(agi::OptionValue const& opt) {
	if (opt.GetBool()) {
		int pos = mid(1, VerticalZoom->GetValue(), 100);
		double value = pow(pos / 50.0, 3);
		controller->SetVolume(value);
		VolumeBar->SetValue(pos);
	}
	VolumeBar->Enable(!opt.GetBool());
}

void AudioBox::OnKaraoke(wxCommandEvent &) {
	LOG_D("audio/box") << "OnKaraoke";
	audioDisplay->SetFocus();
	if (karaokeMode) {
		LOG_D("audio/box") << "karaoke enabled, disabling";
		if (audioKaraoke->splitting) {
			audioKaraoke->EndSplit(false);
		}
		karaokeMode = false;
		audioKaraoke->enabled = false;
		/// @todo Replace this with changing timing controller
		//audioDisplay->SetDialogue();
		audioKaraoke->Refresh(false);
	}

	else {
		LOG_D("audio/box") << "karaoke disabled, enabling";
		karaokeMode = true;
		audioKaraoke->enabled = true;
		/// @todo Replace this with changing timing controller
		//audioDisplay->SetDialogue();
	}

	SetKaraokeButtons();

	LOG_D("audio/box") << "returning";
}

void AudioBox::SetKaraokeButtons() {
	// What to enable
	bool join,split;
	join = audioKaraoke->enabled && (audioKaraoke->splitting || audioKaraoke->selectionCount>=2);
	split = audioKaraoke->enabled;

	// If we set focus here, the audio display will continually steal the focus
	// when navigating via the grid and karaoke is enabled. So don't.
	//audioDisplay->SetFocus();

	JoinButton->Enable(join);
	SplitButton->Enable(split);

	karaokeSizer->Show(CancelAcceptSizer, audioKaraoke->splitting);
	karaokeSizer->Show(JoinSplitSizer, !audioKaraoke->splitting);
}

void AudioBox::OnJoin(wxCommandEvent &) {
	LOG_D("audio/box") << "join";
	audioDisplay->SetFocus();
	audioKaraoke->Join();
}

void AudioBox::OnSplit(wxCommandEvent &) {
	LOG_D("audio/box") << "split";
	audioDisplay->SetFocus();
	audioKaraoke->BeginSplit();
}

void AudioBox::OnCancel(wxCommandEvent &) {
	LOG_D("audio/box") << "cancel";
	audioDisplay->SetFocus();
	audioKaraoke->EndSplit(true);
}

void AudioBox::OnAccept(wxCommandEvent &) {
	LOG_D("audio/box") << "accept";
	audioDisplay->SetFocus();
	audioKaraoke->EndSplit(false);
}
