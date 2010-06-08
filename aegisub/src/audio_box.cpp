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


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <math.h>

#include <wx/recguard.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/laywin.h> // Keep this last so wxSW_3D is set.
#endif

#include <libaegisub/log.h>

#include "audio_box.h"
#include "audio_display.h"
#include "audio_karaoke.h"
#include "frame_main.h"
#include "hotkeys.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "options.h"
#include "toggle_bitmap.h"
#include "tooltip_manager.h"


/// @brief Constructor 
/// @param parent 
///
AudioBox::AudioBox(wxWindow *parent) :
wxPanel(parent,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL|wxBORDER_RAISED)
{
	// Setup
	loaded = false;
	karaokeMode = false;

	// Sash and Display
	audioScroll = new wxScrollBar(this,Audio_Scrollbar);
	audioScroll->PushEventHandler(new FocusEvent());
	audioScroll->SetToolTip(_("Seek bar"));
	Sash = new wxSashWindow(this,Audio_Sash,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN | wxSW_3DBORDER);
	sashSizer = new wxBoxSizer(wxVERTICAL);
	audioDisplay = new AudioDisplay(Sash);
	sashSizer->Add(audioDisplay,1,wxEXPAND,0);
	Sash->SetSizer(sashSizer);
	Sash->SetSashVisible(wxSASH_BOTTOM,true);
	//Sash->SetSashBorder(wxSASH_BOTTOM,true);
	Sash->SetMinimumSizeY(50);
	audioDisplay->ScrollBar = audioScroll;
	audioDisplay->box = this;
	int _w,_h;
	audioDisplay->GetSize(&_w,&_h);
	audioDisplay->SetSizeHints(-1,_h,-1,_h);

	// Zoom
	HorizontalZoom = new wxSlider(this,Audio_Horizontal_Zoom,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH);
	HorizontalZoom->PushEventHandler(new FocusEvent());
	HorizontalZoom->SetToolTip(_("Horizontal zoom"));
	VerticalZoom = new wxSlider(this,Audio_Vertical_Zoom,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	VerticalZoom->PushEventHandler(new FocusEvent());
	VerticalZoom->SetToolTip(_("Vertical zoom"));
	VolumeBar = new wxSlider(this,Audio_Volume,50,0,100,wxDefaultPosition,wxSize(-1,20),wxSL_VERTICAL|wxSL_BOTH|wxSL_INVERSE);
	VolumeBar->PushEventHandler(new FocusEvent());
	VolumeBar->SetToolTip(_("Audio Volume"));
	bool link = OPT_GET("Audio/Link")->GetBool();
	if (link) {
		VolumeBar->SetValue(VerticalZoom->GetValue());
		VolumeBar->Enable(false);
	}
	VerticalLink = new ToggleBitmap(this,Audio_Vertical_Link,GETIMAGE(toggle_audio_link_24));
	VerticalLink->SetToolTip(_("Link vertical zoom and volume sliders"));
	VerticalLink->SetValue(link);

	// Display sizer
	DisplaySizer = new wxBoxSizer(wxVERTICAL);
	//DisplaySizer->Add(audioDisplay,1,wxEXPAND,0);
	DisplaySizer->Add(Sash,0,wxEXPAND,0);
	DisplaySizer->Add(audioScroll,0,wxEXPAND,0);

	// VertVol sider
	wxSizer *VertVol = new wxBoxSizer(wxHORIZONTAL);
	VertVol->Add(VerticalZoom,1,wxEXPAND,0);
	VertVol->Add(VolumeBar,1,wxEXPAND,0);
	wxSizer *VertVolArea = new wxBoxSizer(wxVERTICAL);
	VertVolArea->Add(VertVol,1,wxEXPAND,0);
	VertVolArea->Add(VerticalLink,0,wxEXPAND,0);

	// Top sizer
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(DisplaySizer,1,wxEXPAND,0);
	TopSizer->Add(HorizontalZoom,0,wxEXPAND,0);
	TopSizer->Add(VertVolArea,0,wxEXPAND,0);

	// Buttons sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *temp;
	temp = new wxBitmapButton(this,Audio_Button_Prev,GETIMAGE(button_prev_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Previous line or syllable (%KEY%/%KEY%)"),_T("Audio Prev Line"),_T("Audio Prev Line Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Next,GETIMAGE(button_next_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Next line/syllable (%KEY%/%KEY%)"),_T("Audio Next Line"),_T("Audio Next Line Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play,GETIMAGE(button_playsel_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play selection (%KEY%/%KEY%)"),_T("Audio Play"),_T("Audio Play Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_Row,GETIMAGE(button_playline_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play current line (%KEY%)"),_T("Audio Play Original Line"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Stop,GETIMAGE(button_stop_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Stop (%KEY%)"),_T("Audio Stop"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_Before,GETIMAGE(button_playfivehbefore_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play 500 ms before selection (%KEY%)"),_T("Audio Play 500ms Before"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_After,GETIMAGE(button_playfivehafter_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play 500 ms after selection (%KEY%)"),_T("Audio Play 500ms after"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_First,GETIMAGE(button_playfirstfiveh_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play first 500ms of selection (%KEY%)"),_T("Audio Play First 500ms"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_500ms_Last,GETIMAGE(button_playlastfiveh_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play last 500ms of selection (%KEY%)"),_T("Audio Play Last 500ms"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Play_To_End,GETIMAGE(button_playtoend_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Play from selection start to end of file (%KEY%)"),_T("Audio Play To End"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	temp = new wxBitmapButton(this,Audio_Button_Leadin,GETIMAGE(button_leadin_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Add lead in (%KEY%)"),_T("Audio Add Lead In"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Leadout,GETIMAGE(button_leadout_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Add lead out (%KEY%)"),_T("Audio Add Lead Out"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	temp = new wxBitmapButton(this,Audio_Button_Commit,GETIMAGE(button_audio_commit_24),wxDefaultPosition,wxSize(30,-1));
	ToolTipManager::Bind(temp,_("Commit changes (%KEY%/%KEY%)"),_T("Audio Commit (Stay)"),_T("Audio Commit Alt"));
	ButtonSizer->Add(temp,0,wxRIGHT,0);
	temp = new wxBitmapButton(this,Audio_Button_Goto,GETIMAGE(button_audio_goto_24),wxDefaultPosition,wxSize(30,-1));
	temp->SetToolTip(_("Go to selection"));
	ButtonSizer->Add(temp,0,wxRIGHT,10);

	AutoCommit = new ToggleBitmap(this,Audio_Check_AutoCommit,GETIMAGE(toggle_audio_autocommit_24),wxSize(30,-1));
	AutoCommit->SetToolTip(_("Automatically commit all changes"));
	AutoCommit->SetValue(OPT_GET("Audio/Auto/Commit")->GetBool());
	ButtonSizer->Add(AutoCommit,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	NextCommit = new ToggleBitmap(this,Audio_Check_NextCommit,GETIMAGE(toggle_audio_nextcommit_24),wxSize(30,-1));
	NextCommit->SetToolTip(_("Auto goes to next line on commit"));
	NextCommit->SetValue(OPT_GET("Audio/Next Line on Commit")->GetBool());
	ButtonSizer->Add(NextCommit,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	AutoScroll = new ToggleBitmap(this,Audio_Check_AutoGoto,GETIMAGE(toggle_audio_autoscroll_24),wxSize(30,-1));
	AutoScroll->SetToolTip(_("Auto scrolls audio display to selected line"));
	AutoScroll->SetValue(OPT_GET("Audio/Auto/Scroll")->GetBool());
	ButtonSizer->Add(AutoScroll,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	SpectrumMode = new ToggleBitmap(this,Audio_Check_Spectrum,GETIMAGE(toggle_audio_spectrum_24),wxSize(30,-1));
	SpectrumMode->SetToolTip(_("Spectrum analyzer mode"));
	SpectrumMode->SetValue(OPT_GET("Audio/Spectrum")->GetBool());
	ButtonSizer->Add(SpectrumMode,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	MedusaMode = new ToggleBitmap(this,Audio_Check_Medusa,GETIMAGE(toggle_audio_medusa_24),wxSize(30,-1));
	MedusaMode->SetToolTip(_("Enable Medusa-Style Timing Shortcuts"));
	MedusaMode->SetValue(OPT_GET("Audio/Medusa Timing Hotkeys")->GetBool());
	ButtonSizer->Add(MedusaMode,0,wxRIGHT | wxALIGN_CENTER | wxEXPAND,0);
	ButtonSizer->AddStretchSpacer(1);

	// Karaoke sizer
	karaokeSizer = new wxBoxSizer(wxHORIZONTAL);
	KaraokeButton = new wxBitmapToggleButton(this,Audio_Button_Karaoke,GETIMAGE(kara_mode_24),wxDefaultPosition,wxSize(33,30));
	KaraokeButton->SetToolTip(_("Toggle karaoke mode"));
	karaokeSizer->Add(KaraokeButton,0,wxRIGHT|wxEXPAND,0);

	JoinSplitSizer = new wxBoxSizer(wxHORIZONTAL);
	JoinButton = new wxBitmapButton(this,Audio_Button_Join,GETIMAGE(kara_join_24),wxDefaultPosition,wxSize(33,30));
	JoinButton->SetToolTip(_("Join selected syllables"));
	SplitButton = new wxBitmapButton(this,Audio_Button_Split,GETIMAGE(kara_split_24),wxDefaultPosition,wxSize(33,30));
	SplitButton->SetToolTip(_("Enter split-mode"));
	JoinSplitSizer->Add(JoinButton,0,wxRIGHT|wxEXPAND,0);
	JoinSplitSizer->Add(SplitButton,0,wxRIGHT|wxEXPAND,0);

	CancelAcceptSizer = new wxBoxSizer(wxHORIZONTAL);
	CancelButton = new wxBitmapButton(this,Audio_Button_Cancel,GETIMAGE(kara_split_accept_24),wxDefaultPosition,wxSize(33,30));
	CancelButton->SetToolTip(_("Commit splits and leave split-mode"));
	AcceptButton = new wxBitmapButton(this,Audio_Button_Accept,GETIMAGE(kara_split_cancel_24),wxDefaultPosition,wxSize(33,30));
	AcceptButton->SetToolTip(_("Discard all splits and leave split-mode"));
	CancelAcceptSizer->Add(CancelButton,0,wxRIGHT|wxEXPAND,0);
	CancelAcceptSizer->Add(AcceptButton,0,wxRIGHT|wxEXPAND,0);

	karaokeSizer->Add(JoinSplitSizer,0,wxRIGHT|wxEXPAND,0);
	karaokeSizer->Add(CancelAcceptSizer,0,wxRIGHT|wxEXPAND,0);

	audioKaraoke = new AudioKaraoke(this);
	audioKaraoke->box = this;
	audioKaraoke->display = audioDisplay;
	audioDisplay->karaoke = audioKaraoke;
	karaokeSizer->Add(audioKaraoke,1,wxEXPAND,0);

	SetKaraokeButtons(); // Decide which one to show or hide.

	// Main sizer
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxEXPAND,0);
	MainSizer->Add(ButtonSizer,0,wxEXPAND,0);
	MainSizer->Add(new wxStaticLine(this),0,wxEXPAND|wxTOP|wxBOTTOM,2);
	MainSizer->Add(karaokeSizer,0,wxEXPAND,0);
	//MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);

}



/// @brief Destructor 
///
AudioBox::~AudioBox() {
	audioScroll->PopEventHandler(true);
	HorizontalZoom->PopEventHandler(true);
	VerticalZoom->PopEventHandler(true);
	VolumeBar->PopEventHandler(true);
}



/// @brief Set file 
/// @param file      
/// @param FromVideo 
/// @return 
///
void AudioBox::SetFile(wxString file,bool FromVideo) {
	LOG_D("audio/box") << "file=" << file << " FromVideo: " << FromVideo;
	loaded = false;

	if (FromVideo) {
		audioDisplay->SetFromVideo();
		loaded = audioDisplay->loaded;
		audioName = _T("?video");
	}

	else {
		audioDisplay->SetFile(file);
		if (file != _T("")) loaded = audioDisplay->loaded;
		audioName = file;
	}

	LOG_D("audio/box") << "setting up acceleraters in frameMain";
	frameMain->SetAccelerators();
	LOG_D("audio/box") << "finished setting up accelerators in frameMain";
}


///////////////
// Event table
BEGIN_EVENT_TABLE(AudioBox,wxPanel)
	EVT_COMMAND_SCROLL(Audio_Scrollbar, AudioBox::OnScrollbar)
	EVT_COMMAND_SCROLL(Audio_Horizontal_Zoom, AudioBox::OnHorizontalZoom)
	EVT_COMMAND_SCROLL(Audio_Vertical_Zoom, AudioBox::OnVerticalZoom)
	EVT_COMMAND_SCROLL(Audio_Volume, AudioBox::OnVolume)
	EVT_SASH_DRAGGED(Audio_Sash,AudioBox::OnSash)

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
	EVT_TOGGLEBUTTON(Audio_Check_Medusa,AudioBox::OnMedusaMode)
	EVT_TOGGLEBUTTON(Audio_Check_Spectrum,AudioBox::OnSpectrumMode)
	EVT_TOGGLEBUTTON(Audio_Check_AutoCommit,AudioBox::OnAutoCommit)
	EVT_TOGGLEBUTTON(Audio_Check_NextCommit,AudioBox::OnNextLineCommit)
END_EVENT_TABLE()



/// @brief Scrollbar changed 
/// @param event 
///
void AudioBox::OnScrollbar(wxScrollEvent &event) {
	audioDisplay->SetPosition(event.GetPosition()*12);
}



/// @brief Horizontal zoom bar changed 
/// @param event 
///
void AudioBox::OnHorizontalZoom(wxScrollEvent &event) {
	audioDisplay->SetSamplesPercent(event.GetPosition());
}



/// @brief Vertical zoom bar changed 
/// @param event 
///
void AudioBox::OnVerticalZoom(wxScrollEvent &event) {
	int pos = event.GetPosition();
	if (pos < 1) pos = 1;
	if (pos > 100) pos = 100;
	float value = pow(float(pos)/50.0f,3);
	audioDisplay->SetScale(value);
	if (VerticalLink->GetValue()) {
		audioDisplay->player->SetVolume(value);
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
		audioDisplay->player->SetVolume(pow(float(pos)/50.0f,3));
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
		audioDisplay->player->SetVolume(value);
		VolumeBar->SetValue(pos);
	}
	VolumeBar->Enable(!VerticalLink->GetValue());

	OPT_SET("Audio/Link")->SetBool(VerticalLink->GetValue());
}



/// @brief Sash 
/// @param event 
/// @return 
///
void AudioBox::OnSash(wxSashEvent& event) {
	// OK?
	if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE) return;

	// Recursion guard
	static wxRecursionGuardFlag inside;
	wxRecursionGuard guard(inside);
	if (guard.IsInside()) {
		return;
	}
	
	// Get size
	wxRect newSize = event.GetDragRect();
	int w = newSize.GetWidth();
	int h = newSize.GetHeight();
	if (h < 50) h = 50;
	int oldh = audioDisplay->GetSize().GetHeight();
	if (oldh == h) return;

	// Resize
	audioDisplay->SetSizeHints(w,h,-1,h);
	audioDisplay->SetSize(w,h);
	sashSizer->Layout();
	Sash->GetParent()->Layout();

	// Store new size
	OPT_SET("Audio/Display Height")->SetInt(h);

	// Fix layout
	frameMain->Freeze();
	DisplaySizer->Layout();
	//TopSizer->Layout();
	//MainSizer->Layout();
	Layout();
	frameMain->ToolSizer->Layout();
	frameMain->MainSizer->Layout();
	frameMain->Layout();
	frameMain->Refresh();
	frameMain->Thaw();

	//event.Skip();
}



/// @brief Play selection 
/// @param event 
///
void AudioBox::OnPlaySelection(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(start,end);
}



/// @brief Play dialogue 
/// @param event 
///
void AudioBox::OnPlayDialogue(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesDialogue(start,end);
	audioDisplay->SetSelection(start, end);
	audioDisplay->Play(start,end);
}



/// @brief Stop Playing 
/// @param event 
///
void AudioBox::OnStop(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->Stop();
}



/// @brief Next 
/// @param event 
///
void AudioBox::OnNext(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->Stop();
	audioDisplay->Next();
}



/// @brief Previous 
/// @param event 
///
void AudioBox::OnPrev(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->Stop();
	audioDisplay->Prev();
}



/// @brief 500 ms before 
/// @param event 
///
void AudioBox::OnPlay500Before(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(start-500,start);
}



/// @brief 500 ms after 
/// @param event 
///
void AudioBox::OnPlay500After(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(end,end+500);
}



/// @brief First 500 ms 
/// @param event 
///
void AudioBox::OnPlay500First(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	int endp = start+500;
	if (endp > end) endp = end;
	audioDisplay->Play(start,endp);
}



/// @brief Last 500 ms 
/// @param event 
///
void AudioBox::OnPlay500Last(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	int startp = end-500;
	if (startp < start) startp = start;
	audioDisplay->Play(startp,end);
}



/// @brief Start to end of file 
/// @param event 
///
void AudioBox::OnPlayToEnd(wxCommandEvent &event) {
	int start=0,end=0;
	audioDisplay->SetFocus();
	audioDisplay->GetTimesSelection(start,end);
	audioDisplay->Play(start,-1);
}



/// @brief Commit changes 
/// @param event 
/// @return 
///
void AudioBox::OnCommit(wxCommandEvent &event) {
	LOG_D("audio/box") << "OnCommit";
	audioDisplay->SetFocus();
	LOG_D("audio/box") << "has set focus, now committing changes";
	audioDisplay->CommitChanges(true);
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
		audioDisplay->SetDialogue();
		audioKaraoke->Refresh(false);
	}

	else {
		LOG_D("audio/box") << "karaoke disabled, enabling";
		karaokeMode = true;
		audioKaraoke->enabled = true;
		audioDisplay->SetDialogue();
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
	if (audioKaraoke->splitting) {
		karaokeSizer->Show(CancelAcceptSizer);
		karaokeSizer->Hide(JoinSplitSizer);
		karaokeSizer->Layout();
	} else {
		karaokeSizer->Hide(CancelAcceptSizer);
		karaokeSizer->Show(JoinSplitSizer);
		karaokeSizer->Layout();
	}
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
	audioDisplay->MakeDialogueVisible(true);
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



/// @brief Medusa Mode 
/// @param event 
///
void AudioBox::OnMedusaMode(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	OPT_SET("Audio/Medusa Timing Hotkeys")->SetBool(MedusaMode->GetValue());
	frameMain->SetAccelerators();
}



/// @brief Spectrum Analyzer Mode 
/// @param event 
///
void AudioBox::OnSpectrumMode(wxCommandEvent &event) {
	OPT_SET("Audio/Spectrum")->SetBool(SpectrumMode->GetValue());
	audioDisplay->UpdateImage(false);
	audioDisplay->SetFocus();
	audioDisplay->Refresh(false);
}



/// @brief Lead in/out 
/// @param event 
///
void AudioBox::OnLeadIn(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->AddLead(true,false);
}


/// @brief DOCME
/// @param event 
///
void AudioBox::OnLeadOut(wxCommandEvent &event) {
	audioDisplay->SetFocus();
	audioDisplay->AddLead(false,true);
}


//////////////////////////////////////////
// Focus event handling for the scrollbar
BEGIN_EVENT_TABLE(FocusEvent,wxEvtHandler)
	EVT_SET_FOCUS(FocusEvent::OnSetFocus)
END_EVENT_TABLE()


/// @brief DOCME
/// @param event 
///
void FocusEvent::OnSetFocus(wxFocusEvent &event) {
	wxWindow *previous = event.GetWindow();
	if (previous) previous->SetFocus();
}
