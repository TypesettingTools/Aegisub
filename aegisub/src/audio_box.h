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

/// @file audio_box.h
/// @see audio_box.cpp
/// @ingroup audio_ui
///


#pragma once


///////////
// Headers
#ifndef AGI_PRE
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sashwin.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#endif


//////////////
// Prototypes
class AudioDisplay;
class AudioKaraoke;
class FrameMain;
class wxToggleButton;
class ToggleBitmap;



/// DOCME
/// @class AudioBox
/// @brief DOCME
///
/// DOCME
class AudioBox : public wxPanel {
	friend class AudioDisplay;

private:

	/// DOCME
	wxScrollBar *audioScroll;

	/// DOCME
	wxSlider *HorizontalZoom;

	/// DOCME
	wxSlider *VerticalZoom;

	/// DOCME
	wxSlider *VolumeBar;

	/// DOCME
	wxSizer *MainSizer;

	/// DOCME
	wxSizer *TopSizer;

	/// DOCME
	wxSizer *sashSizer;

	/// DOCME
	wxSizer *DisplaySizer;

	/// DOCME
	wxSashWindow *Sash;

	/// DOCME
	ToggleBitmap *VerticalLink;

	/// Karaoke box sizer
	wxSizer *karaokeSizer;

	/// Karaoke mode join syllabel button.
	wxButton *JoinButton;

	/// Karaoke mode split word button.
	wxButton *SplitButton;

	/// Karaoke mode split/join cancel button.
	wxButton *CancelButton;

	/// Karaoke mode split/join accept button.
	wxButton *AcceptButton;

	/// Join/Split button sizer.
	wxSizer *JoinSplitSizer;

	/// Cancel/Accept sizer.
	wxSizer *CancelAcceptSizer;

	/// DOCME
	ToggleBitmap *AutoScroll;

	/// DOCME
	ToggleBitmap *NextCommit;

	/// DOCME
	ToggleBitmap *MedusaMode;

	/// DOCME
	ToggleBitmap *AutoCommit;

	/// DOCME
	ToggleBitmap *SpectrumMode;

	void OnScrollbar(wxScrollEvent &event);
	void OnHorizontalZoom(wxScrollEvent &event);
	void OnVerticalZoom(wxScrollEvent &event);
	void OnVolume(wxScrollEvent &event);
	void OnVerticalLink(wxCommandEvent &event);
	void OnSash(wxSashEvent &event);

	void OnPlaySelection(wxCommandEvent &event);
	void OnPlayDialogue(wxCommandEvent &event);
	void OnStop(wxCommandEvent &event);
	void OnNext(wxCommandEvent &event);
	void OnPrev(wxCommandEvent &event);
	void OnPlay500Before(wxCommandEvent &event);
	void OnPlay500After(wxCommandEvent &event);
	void OnPlay500First(wxCommandEvent &event);
	void OnPlay500Last(wxCommandEvent &event);
	void OnPlayToEnd(wxCommandEvent &event);
	void OnCommit(wxCommandEvent &event);
	void OnKaraoke(wxCommandEvent &event);
	void OnJoin(wxCommandEvent &event);
	void OnSplit(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnAccept(wxCommandEvent &event);
	void OnGoto(wxCommandEvent &event);
	void OnLeadIn(wxCommandEvent &event);
	void OnLeadOut(wxCommandEvent &event);

	void OnAutoGoto(wxCommandEvent &event);
	void OnAutoCommit(wxCommandEvent &event);
	void OnMedusaMode(wxCommandEvent &event);
	void OnSpectrumMode(wxCommandEvent &event);
	void OnNextLineCommit(wxCommandEvent &event);

public:

	/// DOCME
	AudioDisplay *audioDisplay;

	/// DOCME
	AudioKaraoke *audioKaraoke;

	/// DOCME
	wxBitmapToggleButton *KaraokeButton;

	/// DOCME
	FrameMain *frameMain;

	/// DOCME
	wxString audioName;

	/// DOCME
	bool loaded;

	/// DOCME
	bool karaokeMode;

	AudioBox(wxWindow *parent);
	~AudioBox();

	void SetFile(wxString file,bool FromVideo);
	void SetKaraokeButtons();

	DECLARE_EVENT_TABLE()
};



/// DOCME
/// @class FocusEvent
/// @brief DOCME
///
/// DOCME
class FocusEvent : public wxEvtHandler {
	
private:
	void OnSetFocus(wxFocusEvent &event);
	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	Audio_Scrollbar = 1600,

	/// DOCME
	Audio_Horizontal_Zoom,

	/// DOCME
	Audio_Vertical_Zoom,

	/// DOCME
	Audio_Volume,

	/// DOCME
	Audio_Sash,

	/// DOCME
	Audio_Vertical_Link,

	/// DOCME
	Audio_Button_Play,

	/// DOCME
	Audio_Button_Stop,

	/// DOCME
	Audio_Button_Prev,

	/// DOCME
	Audio_Button_Next,

	/// DOCME
	Audio_Button_Play_500ms_Before,

	/// DOCME
	Audio_Button_Play_500ms_After,

	/// DOCME
	Audio_Button_Play_500ms_First,

	/// DOCME
	Audio_Button_Play_500ms_Last,

	/// DOCME
	Audio_Button_Play_Row,

	/// DOCME
	Audio_Button_Play_To_End,

	/// DOCME
	Audio_Button_Commit,

	/// DOCME
	Audio_Button_Karaoke,

	/// DOCME
	Audio_Button_Goto,

	Audio_Button_Join,		/// Karaoke -> Enter join mode.
	Audio_Button_Split,		/// Karaoke -> Enter split mode.
	Audio_Button_Accept,	/// Karaoke -> Split/Join mode -> Accept.
	Audio_Button_Cancel,	/// KAraoke -> Split/Join mode -> Cancel.

	/// DOCME
	Audio_Button_Leadin,

	/// DOCME
	Audio_Button_Leadout,


	/// DOCME
	Audio_Check_AutoCommit,

	/// DOCME
	Audio_Check_NextCommit,

	/// DOCME
	Audio_Check_AutoGoto,

	/// DOCME
	Audio_Check_Medusa,

	/// DOCME
	Audio_Check_Spectrum
};
