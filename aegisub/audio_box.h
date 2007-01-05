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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#ifndef AUDIO_BOX_H
#define AUDIO_BOX_H


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/sashwin.h>


//////////////
// Prototypes
class AudioDisplay;
class VideoDisplay;
class AudioKaraoke;
class FrameMain;
class wxToggleButton;
class ToggleBitmap;


///////////////////
// Audio box class
class AudioBox : public wxPanel {
	friend class AudioDisplay;

private:
	wxScrollBar *audioScroll;
	wxSlider *HorizontalZoom;
	wxSlider *VerticalZoom;
	wxSlider *VolumeBar;
	wxSizer *MainSizer;
	wxSizer *TopSizer;
	wxSizer *sashSizer;
	wxSizer *DisplaySizer;
	wxSashWindow *Sash;
	ToggleBitmap *VerticalLink;

	wxToggleButton *SplitButton;
	wxButton *JoinButton;
	ToggleBitmap *AutoScroll;
	ToggleBitmap *MedusaMode;
	ToggleBitmap *AutoCommit;
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
	void OnGoto(wxCommandEvent &event);
	void OnLeadIn(wxCommandEvent &event);
	void OnLeadOut(wxCommandEvent &event);

	void OnAutoGoto(wxCommandEvent &event);
	void OnAutoCommit(wxCommandEvent &event);
	void OnMedusaMode(wxCommandEvent &event);
	void OnSpectrumMode(wxCommandEvent &event);

public:
	AudioDisplay *audioDisplay;
	AudioKaraoke *audioKaraoke;
	wxToggleButton *KaraokeButton;
	FrameMain *frameMain;
	wxString audioName;
	bool loaded;
	bool karaokeMode;

	AudioBox(wxWindow *parent,VideoDisplay *display);
	~AudioBox();

	void SetFile(wxString file,bool FromVideo);
	void SetKaraokeButtons(bool join,bool split);

	DECLARE_EVENT_TABLE()
};


class FocusEvent : public wxEvtHandler {
	
private:
	void OnSetFocus(wxFocusEvent &event);
	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
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
	Audio_Button_Join,
	Audio_Button_Split,
	Audio_Button_Leadin,
	Audio_Button_Leadout,

	Audio_Check_AutoCommit,
	Audio_Check_AutoGoto,
	Audio_Check_Medusa,
	Audio_Check_Spectrum
};


#endif
