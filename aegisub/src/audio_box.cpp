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

#include <wx/recguard.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/laywin.h> // Keep this last so wxSW_3D is set.
#endif

#include <libaegisub/log.h>

#include "include/aegisub/audio_player.h"
#include "selection_controller.h"
#include "audio_controller.h"
#include "audio_box.h"
#include "audio_display.h"
#include "audio_karaoke.h"
#include "audio_timing.h"
#include "frame_main.h"
#include "include/aegisub/audio_player.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "toggle_bitmap.h"
#include "tooltip_manager.h"

enum AudioBoxControlIDs {
	Audio_Scrollbar = 1600,
	Audio_Horizontal_Zoom,
	Audio_Vertical_Zoom,
	Audio_Volume,
	Audio_Sash,
	Audio_Vertical_Link,
	Audio_Button_Play,
	Audio_Button_Stop,
	Audio_Button_Prev,
	Audio_Button_Next,
	Audio_Button_Play_500ms_Before,
	Audio_Button_Play_500ms_After,
	Audio_Button_Play_500ms_First,
	Audio_Button_Play_500ms_Last,
	Audio_Button_Play_Row,
	Audio_Button_Play_To_End,
	Audio_Button_Commit,
	Audio_Button_Karaoke,
	Audio_Button_Goto,

	Audio_Button_Join,		/// Karaoke -> Enter join mode.
	Audio_Button_Split,		/// Karaoke -> Enter split mode.
	Audio_Button_Accept,	/// Karaoke -> Split/Join mode -> Accept.
	Audio_Button_Cancel,	/// KAraoke -> Split/Join mode -> Cancel.

	Audio_Button_Leadin,
	Audio_Button_Leadout,

	Audio_Check_AutoCommit,
	Audio_Check_NextCommit,
	Audio_Check_AutoGoto,
	Audio_Check_Medusa,
	Audio_Check_Spectrum
};



/// @brief Constructor 
/// @param parent 
///
AudioBox::AudioBox(wxWindow *parent, AudioController *_controller, SelectionController<AssDialogue> *selection_controller, AssFile *ass)
: wxPanel(parent,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL|wxBORDER_RAISED)
, selection_controller(selection_controller)
, controller(_controller)
{
	// Setup
	karaokeMode = false;

	// Sash and Display
	audioDisplay = new AudioDisplay(this, controller);

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
	VerticalLink = new ToggleBitmap(this,Audio_Vertical_Link,GETIMAGE(toggle_audio_link_16));
	VerticalLink->SetToolTip(_("Link vertical zoom and volume sliders"));
	VerticalLink->SetValue(link);

	// VertVol sider
	wxSizer *VertVol = new wxBoxSizer(wxHORIZONTAL);
	VertVol->Add(VerticalZoom,1,wxEXPAND,0);
	VertVol->Add(VolumeBar,1,wxEXPAND,0);
	wxSizer *VertVolArea = new wxBoxSizer(wxVERTICAL);
	VertVolArea->Add(VertVol,1,wxEXPAND,0);
	VertVolArea->Add(VerticalLink,0,wxEXPAND,0);

	// Top sizer
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(audioDisplay,1,wxEXPAND,0);
	TopSizer->Add(HorizontalZoom,0,wxEXPAND,0);
	TopSizer->Add(VertVolArea,0,wxEXPAND,0);

	// Buttons sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *temp;
	temp = new wxBitmapButton(this,Audio_Button_Prev,GETIMAGE(button_prev_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Previous line or syllable (%KEY%/%KEY%)"),_T("Audio Prev Line"),_T("Audio Prev Line Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Next,GETIMAGE(button_next_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Next line/syllable (%KEY%/%KEY%)"),_T("Audio Next Line"),_T("Audio Next Line Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play,GETIMAGE(button_playsel_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play selection (%KEY%/%KEY%)"),_T("Audio Play"),_T("Audio Play Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_Row,GETIMAGE(button_playline_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play current line (%KEY%)"),_T("Audio Play Original Line"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Stop,GETIMAGE(button_stop_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Stop (%KEY%)"),_T("Audio Stop"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_Before,GETIMAGE(button_playfivehbefore_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play 500 ms before selection (%KEY%)"),_T("Audio Play 500ms Before"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_After,GETIMAGE(button_playfivehafter_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play 500 ms after selection (%KEY%)"),_T("Audio Play 500ms after"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_First,GETIMAGE(button_playfirstfiveh_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play first 500ms of selection (%KEY%)"),_T("Audio Play First 500ms"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_Last,GETIMAGE(button_playlastfiveh_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play last 500ms of selection (%KEY%)"),_T("Audio Play Last 500ms"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_To_End,GETIMAGE(button_playtoend_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Play from selection start to end of file (%KEY%)"),_T("Audio Play To End"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	temp = new wxBitmapButton(this,Audio_Button_Leadin,GETIMAGE(button_leadin_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Add lead in (%KEY%)"),_T("Audio Add Lead In"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Leadout,GETIMAGE(button_leadout_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Add lead out (%KEY%)"),_T("Audio Add Lead Out"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	temp = new wxBitmapButton(this,Audio_Button_Commit,GETIMAGE(button_audio_commit_16),wxDefaultPosition,wxDefaultSize);
	ToolTipManager::Bind(temp,_("Commit changes (%KEY%/%KEY%)"),_T("Audio Commit (Stay)"),_T("Audio Commit Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Goto,GETIMAGE(button_audio_goto_16),wxDefaultPosition,wxDefaultSize);
	temp->SetToolTip(_("Go to selection"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	AutoCommit = new ToggleBitmap(this,Audio_Check_AutoCommit,GETIMAGE(toggle_audio_autocommit_16), wxSize(20, -1));
	AutoCommit->SetToolTip(_("Automatically commit all changes"));
	AutoCommit->SetValue(OPT_GET("Audio/Auto/Commit")->GetBool());
	ButtonSizer->Add(AutoCommit,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	NextCommit = new ToggleBitmap(this,Audio_Check_NextCommit,GETIMAGE(toggle_audio_nextcommit_16), wxSize(20, -1));
	NextCommit->SetToolTip(_("Auto goes to next line on commit"));
	NextCommit->SetValue(OPT_GET("Audio/Next Line on Commit")->GetBool());
	ButtonSizer->Add(NextCommit,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	AutoScroll = new ToggleBitmap(this,Audio_Check_AutoGoto,GETIMAGE(toggle_audio_autoscroll_16), wxSize(20, -1));
	AutoScroll->SetToolTip(_("Auto scrolls audio display to selected line"));
	AutoScroll->SetValue(OPT_GET("Audio/Auto/Scroll")->GetBool());
	ButtonSizer->Add(AutoScroll,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,10);

	ButtonSizer->AddStretchSpacer(1);

	KaraokeButton = new wxBitmapToggleButton(this,Audio_Button_Karaoke,GETIMAGE(kara_mode_16),wxDefaultPosition,wxDefaultSize);
	KaraokeButton->SetToolTip(_("Toggle karaoke mode"));
	ButtonSizer->Add(KaraokeButton,0,wxRIGHT|wxEXPAND,0);

	// Karaoke sizer
	karaokeSizer = new wxBoxSizer(wxHORIZONTAL);

	JoinSplitSizer = new wxBoxSizer(wxHORIZONTAL);
	JoinButton = new wxBitmapButton(this,Audio_Button_Join,GETIMAGE(kara_join_16),wxDefaultPosition,wxDefaultSize);
	JoinButton->SetToolTip(_("Join selected syllables"));
	SplitButton = new wxBitmapButton(this,Audio_Button_Split,GETIMAGE(kara_split_16),wxDefaultPosition,wxDefaultSize);
	SplitButton->SetToolTip(_("Enter split-mode"));
	JoinSplitSizer->Add(JoinButton,0,wxRIGHT|wxEXPAND,0);
	JoinSplitSizer->Add(SplitButton,0,wxRIGHT|wxEXPAND,0);

	CancelAcceptSizer = new wxBoxSizer(wxHORIZONTAL);
	CancelButton = new wxBitmapButton(this,Audio_Button_Cancel,GETIMAGE(kara_split_accept_16),wxDefaultPosition,wxDefaultSize);
	CancelButton->SetToolTip(_("Commit splits and leave split-mode"));
	AcceptButton = new wxBitmapButton(this,Audio_Button_Accept,GETIMAGE(kara_split_cancel_16),wxDefaultPosition,wxDefaultSize);
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
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,1,wxEXPAND|wxALL,3);
	MainSizer->Add(ButtonSizer,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,3);
	//MainSizer->Add(new wxStaticLine(this),0,wxEXPAND|wxTOP|wxBOTTOM,2);
	MainSizer->Add(karaokeSizer,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,3);
	MainSizer->AddSpacer(3);
	//MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);

	SetKaraokeButtons(); // Decide which one to show or hide.

	timing_controller_dialogue = CreateDialogueTimingController(controller, selection_controller, ass);
	controller->SetTimingController(timing_controller_dialogue);
}



/// @brief Destructor 
///
AudioBox::~AudioBox()
{
}



///////////////
// Event table
BEGIN_EVENT_TABLE(AudioBox,wxPanel)
	EVT_COMMAND_SCROLL(Audio_Horizontal_Zoom, AudioBox::OnHorizontalZoom)
	EVT_COMMAND_SCROLL(Audio_Vertical_Zoom, AudioBox::OnVerticalZoom)
	EVT_COMMAND_SCROLL(Audio_Volume, AudioBox::OnVolume)

	EVT_BUTTON(Audio_Button_Play, AudioBox::OnPlaySelection)
	EVT_BUTTON(Audio_Button_Play_Row, AudioBox::OnPlayDialogue)
	EVT_BUTTON(Audio_Button_Stop, AudioBox::OnStop)
	EVT_BUTTON(Audio_Button_Next, AudioBox::OnNext)
	EVT_BUTTON(Audio_Button_Prev, AudioBox::OnPrev)
	EVT_BUTTON(Audio_Button_Play_500ms_Before, AudioBox::OnPlay500Before)
	EVT_BUTTON(Audio_Button_Play_500ms_After, AudioBox::OnPlay500After)
	EVT_BUTTON(Audio_Button_Play_500ms_First, AudioBox::OnPlay500First)
	EVT_BUTTON(Audio_Button_Play_500ms_Last, AudioBox::OnPlay500Last)
	EVT_BUTTON(Audio_Button_Play_To_End, AudioBox::OnPlayToEnd)
	EVT_BUTTON(Audio_Button_Commit, AudioBox::OnCommit)
	EVT_BUTTON(Audio_Button_Goto, AudioBox::OnGoto)
	EVT_BUTTON(Audio_Button_Join,AudioBox::OnJoin)
	EVT_BUTTON(Audio_Button_Split,AudioBox::OnSplit)
	EVT_BUTTON(Audio_Button_Cancel,AudioBox::OnCancel)
	EVT_BUTTON(Audio_Button_Accept,AudioBox::OnAccept)
	EVT_BUTTON(Audio_Button_Leadin,AudioBox::OnLeadIn)
	EVT_BUTTON(Audio_Button_Leadout,AudioBox::OnLeadOut)

	EVT_TOGGLEBUTTON(Audio_Vertical_Link, AudioBox::OnVerticalLink)
	EVT_TOGGLEBUTTON(Audio_Button_Karaoke, AudioBox::OnKaraoke)
	EVT_TOGGLEBUTTON(Audio_Check_AutoGoto,AudioBox::OnAutoGoto)
	EVT_TOGGLEBUTTON(Audio_Check_AutoCommit,AudioBox::OnAutoCommit)
	EVT_TOGGLEBUTTON(Audio_Check_NextCommit,AudioBox::OnNextLineCommit)
END_EVENT_TABLE()



/// @brief Horizontal zoom bar changed 
/// @param event 
///
void AudioBox::OnHorizontalZoom(wxScrollEvent &event) {
	// Negate the value, we want zoom out to be on bottom and zoom in on top,
	// but the control doesn't want negative on bottom and positive on top.
	audioDisplay->SetZoomLevel(-event.GetPosition());
}



/// @brief Vertical zoom bar changed 
/// @param event 
///
void AudioBox::OnVerticalZoom(wxScrollEvent &event) {
	int pos = event.GetPosition();
	if (pos < 1) pos = 1;
	if (pos > 100) pos = 100;
	float value = pow(float(pos)/50.0f,3);
	audioDisplay->SetAmplitudeScale(value);
	if (VerticalLink->GetValue()) {
		controller->SetVolume(value);
		VolumeBar->SetValue(pos);
	}
}



/// @brief Volume bar changed 
/// @param event 
///
void AudioBox::OnVolume(wxScrollEvent &event) {
	if (!VerticalLink->GetValue()) {
		int pos = event.GetPosition();
		if (pos < 1) pos = 1;
		if (pos > 100) pos = 100;
		controller->SetVolume(pow(float(pos)/50.0f,3));
	}
}



/// @brief Bars linked/unlinked 
/// @param event 
///
void AudioBox::OnVerticalLink(wxCommandEvent &event) {
	int pos = VerticalZoom->GetValue();
	if (pos < 1) pos = 1;
	if (pos > 100) pos = 100;
	float value = pow(float(pos)/50.0f,3);
	if (VerticalLink->GetValue()) {
		controller->SetVolume(value);
		VolumeBar->SetValue(pos);
	}
	VolumeBar->Enable(!VerticalLink->GetValue());

	OPT_SET("Audio/Link")->SetBool(VerticalLink->GetValue());
}



/// @brief Play selection 
/// @param event 
///
void AudioBox::OnPlaySelection(wxCommandEvent &event) {
	controller->PlayPrimaryRange();
}



/// @brief Play dialogue 
/// @param event 
///
void AudioBox::OnPlayDialogue(wxCommandEvent &event) {
	if (controller->GetTimingController())
		controller->GetTimingController()->Revert();
	controller->PlayPrimaryRange();
}



/// @brief Stop Playing 
/// @param event 
///
void AudioBox::OnStop(wxCommandEvent &event) {
	controller->Stop();
}



/// @brief Next 
/// @param event 
///
void AudioBox::OnNext(wxCommandEvent &event) {
	//audioDisplay->SetFocus();
	controller->Stop();
	if (controller->GetTimingController())
		controller->GetTimingController()->Next();
	controller->PlayPrimaryRange();
}



/// @brief Previous 
/// @param event 
///
void AudioBox::OnPrev(wxCommandEvent &event) {
	//audioDisplay->SetFocus();
	controller->Stop();
	if (controller->GetTimingController())
		controller->GetTimingController()->Prev();
	controller->PlayPrimaryRange();
}



/// @brief 500 ms before 
/// @param event 
///
void AudioBox::OnPlay500Before(wxCommandEvent &event) {
	SampleRange times(controller->GetPrimaryPlaybackRange());
	controller->PlayRange(SampleRange(
		times.begin() - controller->SamplesFromMilliseconds(500),
		times.begin()));
}



/// @brief 500 ms after 
/// @param event 
///
void AudioBox::OnPlay500After(wxCommandEvent &event) {
	SampleRange times(controller->GetPrimaryPlaybackRange());
	controller->PlayRange(SampleRange(
		times.end(),
		times.end() + controller->SamplesFromMilliseconds(500)));
}



/// @brief First 500 ms 
/// @param event 
///
void AudioBox::OnPlay500First(wxCommandEvent &event) {
	SampleRange times(controller->GetPrimaryPlaybackRange());
	controller->PlayRange(SampleRange(
		times.begin(),
		times.begin() + std::min(
			controller->SamplesFromMilliseconds(500),
			times.length())));
}



/// @brief Last 500 ms 
/// @param event 
///
void AudioBox::OnPlay500Last(wxCommandEvent &event) {
	SampleRange times(controller->GetPrimaryPlaybackRange());
	controller->PlayRange(SampleRange(
		times.end() - std::min(
			controller->SamplesFromMilliseconds(500),
			times.length()),
		times.end()));
}



/// @brief Start to end of file 
/// @param event 
///
void AudioBox::OnPlayToEnd(wxCommandEvent &event) {
	controller->PlayToEnd(controller->GetPrimaryPlaybackRange().begin());
}



/// @brief Commit changes 
/// @param event 
/// @return 
///
void AudioBox::OnCommit(wxCommandEvent &event) {
	LOG_D("audio/box") << "OnCommit";
	audioDisplay->SetFocus();
	LOG_D("audio/box") << "has set focus, now committing changes";
	controller->GetTimingController()->Commit();
	LOG_D("audio/box") << "returning";
}



/// @brief Toggle karaoke 
/// @param event 
/// @return 
///
void AudioBox::OnKaraoke(wxCommandEvent &event) {
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



/// @brief Sets karaoke buttons 
///
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

/// @brief Join button in karaoke mode
/// @param event wxEvent
///
void AudioBox::OnJoin(wxCommandEvent &event) {
	LOG_D("audio/box") << "join";
	audioDisplay->SetFocus();
	audioKaraoke->Join();
}

/// @brief Split button in karaoke mode
/// @param event wxEvent
///
void AudioBox::OnSplit(wxCommandEvent &event) {
	LOG_D("audio/box") << "split";
	audioDisplay->SetFocus();
		audioKaraoke->BeginSplit();
}

/// @brief Cancel join/split in karaoke mode.
/// @param event wxEvent
///
void AudioBox::OnCancel(wxCommandEvent &event) {
	LOG_D("audio/box") << "cancel";
	audioDisplay->SetFocus();
	audioKaraoke->EndSplit(true);
}

/// @brief Accept join/split button in karaoke mode.
/// @param event wxEvent
///
void AudioBox::OnAccept(wxCommandEvent &event) {
	LOG_D("audio/box") << "accept";
	audioDisplay->SetFocus();
	audioKaraoke->EndSplit(false);
}


/// @brief Goto button 
/// @param event 
///
void AudioBox::OnGoto(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	if (controller->GetTimingController())
		audioDisplay->ScrollSampleRangeInView(controller->GetTimingController()->GetIdealVisibleSampleRange());
}



/// @brief Auto Goto 
/// @param event 
///
void AudioBox::OnAutoGoto(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	OPT_SET("Audio/Auto/Scroll")->SetBool(AutoScroll->GetValue());
}



/// @brief Auto Commit 
/// @param event 
///
void AudioBox::OnAutoCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	OPT_SET("Audio/Auto/Commit")->SetBool(AutoCommit->GetValue());
}



/// @brief Next line on Commit 
/// @param event 
///
void AudioBox::OnNextLineCommit(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	OPT_SET("Audio/Next Line on Commit")->SetBool(NextCommit->GetValue());
}



/// @todo Put global audio hotkeys toggling into the menu bar
/*
void AudioBox::OnMedusaMode(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	OPT_SET("Audio/Medusa Timing Hotkeys")->SetBool(MedusaMode->GetValue());
	frameMain->SetAccelerators();
}
*/



/// @brief Lead in/out 
/// @param event 
///
void AudioBox::OnLeadIn(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	//audioDisplay->AddLead(true,false);
}


/// @brief DOCME
/// @param event 
///
void AudioBox::OnLeadOut(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	//audioDisplay->AddLead(false,true);
}

