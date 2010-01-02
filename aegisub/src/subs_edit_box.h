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

/// @file subs_edit_box.h
/// @see subs_edit_box.cpp
/// @ingroup main_ui
///




////////////
// Includes
#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/spinctrl.h>
#endif

#include "subs_edit_ctrl.h"


//////////////
// Prototypes
class SubtitlesGrid;
class TimeEdit;
class SubsEditBox;
class AudioDisplay;
class HiliModTextCtrl;
class wxStyledTextCtrl;



/// DOCME
/// @class SubsEditBox
/// @brief DOCME
///
/// DOCME
class SubsEditBox : public wxPanel {
	friend class SubsTextEditHandler;
	friend class SubsTextEditCtrl;
	friend class AudioDisplay;

private:

	/// DOCME
	bool splitLineMode;

	/// DOCME
	bool setupDone;

	/// DOCME
	bool enabled;

	/// DOCME
	bool textEditReady;

	/// DOCME
	bool controlState;

	/// DOCME
	wxColour origBgColour;

	/// DOCME
	wxColour disabledBgColour;


	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	wxCheckBox *CommentBox;

	/// DOCME
	wxComboBox *StyleBox;

	/// DOCME
	wxComboBox *ActorBox;

	/// DOCME
	TimeEdit *StartTime;

	/// DOCME
	TimeEdit *EndTime;

	/// DOCME
	TimeEdit *Duration;

	/// DOCME
	wxSpinCtrl *Layer;

	/// DOCME
	HiliModTextCtrl *MarginL;

	/// DOCME
	HiliModTextCtrl *MarginR;

	/// DOCME
	HiliModTextCtrl *MarginV;

	/// DOCME
	HiliModTextCtrl *Effect;

	/// DOCME
	wxRadioButton *ByTime;

	/// DOCME
	wxRadioButton *ByFrame;

	/// DOCME
	wxCheckBox *SyntaxHighlight;


	/// DOCME
	wxButton *Bold;

	/// DOCME
	wxButton *Italics;

	/// DOCME
	wxButton *Underline;

	/// DOCME
	wxButton *Strikeout;

	/// DOCME
	wxButton *FontName;

	/// DOCME
	wxButton *Color1;

	/// DOCME
	wxButton *Color2;

	/// DOCME
	wxButton *Color3;

	/// DOCME
	wxButton *Color4;

	/// DOCME
	wxButton *CommitButton;


	/// DOCME
	wxSizer *TopSizer;

	/// DOCME
	wxSizer *MiddleBotSizer;

	/// DOCME
	wxSizer *MiddleSizer;

	/// DOCME
	wxSizer *MainSizer;

	/// DOCME
	wxSizer *DummySizer;

	/// DOCME
	wxSizer *BottomSizer;

	void SetControlsState(bool state);
	void CommitTimes(bool start,bool end,bool fromStart,bool commit=true);

	int BlockAtPos(int pos);

	void OnEditText(wxStyledTextEvent &event);
	void OnNeedStyle(wxStyledTextEvent &event);
	void OnCharAdded(wxStyledTextEvent &event);
	void OnUpdateUI(wxStyledTextEvent &event);

	void OnButtonColor1(wxCommandEvent &event);
	void OnButtonColor2(wxCommandEvent &event);
	void OnButtonColor3(wxCommandEvent &event);
	void OnButtonColor4(wxCommandEvent &event);
	void OnButtonFontFace(wxCommandEvent &event);
	void OnButtonBold(wxCommandEvent &event);
	void OnButtonItalics(wxCommandEvent &event);
	void OnButtonUnderline(wxCommandEvent &event);
	void OnButtonStrikeout(wxCommandEvent &event);
	void OnButtonCommit(wxCommandEvent &event);

	void OnSyntaxBox(wxCommandEvent &event);
	void OnFrameRadio(wxCommandEvent &event);
	void OnTimeRadio(wxCommandEvent &event);
	void OnKeyDown(wxStyledTextEvent &event);
	void OnStyleChange(wxCommandEvent &event);
	void OnActorChange(wxCommandEvent &event);
	void OnLayerEnter(wxCommandEvent &event);
	void OnLayerChange(wxSpinEvent &event);
	void OnStartTimeChange(wxCommandEvent &event);
	void OnEndTimeChange(wxCommandEvent &event);
	void OnDurationChange(wxCommandEvent &event);
	void OnMarginLChange(wxCommandEvent &event);
	void OnMarginRChange(wxCommandEvent &event);
	void OnMarginVChange(wxCommandEvent &event);
	void OnCommentChange(wxCommandEvent &event);
	void OnEffectChange(wxCommandEvent &event);
	void OnSize(wxSizeEvent &event);

public:

	/// DOCME
	int linen;

	/// DOCME
	AudioDisplay *audio;

	/// DOCME
	SubsTextEditCtrl *TextEdit;

	SubsEditBox(wxWindow *parent,SubtitlesGrid *gridp);
	~SubsEditBox();

	void SetOverride (wxString tag,wxString preValue=_T(""),int pos=-1,bool getFocus=true);
	void SetStyleFlag (wxString tag,wxString preValue=_T(""),int pos=-1);

	void SetSplitLineMode(wxSize size=wxSize(-1,-1));
	void CommitText(bool weak=false);
	void Update(bool timeOnly=false,bool weak=false,bool video=true);
	void UpdateGlobals();
	void SetToLine(int n,bool weak=false);
	void UpdateFrameTiming();
	void DoKeyPress(wxKeyEvent &event);
	void Commit(bool stay);

	DECLARE_EVENT_TABLE()
};



/// DOCME
/// @class SubsEditBoxEvent
/// @brief DOCME
///
/// DOCME
class SubsEditBoxEvent : public wxEvtHandler {
private:

	/// DOCME
	SubsEditBox *control;
	void OnKeyPress(wxKeyEvent &event);

public:
	SubsEditBoxEvent(SubsEditBox *control);
	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	EDIT_BOX = 1300,

	/// DOCME
	SYNTAX_BOX,

	/// DOCME
	RADIO_TIME_BY_FRAME,

	/// DOCME
	RADIO_TIME_BY_TIME,

	/// DOCME
	STYLE_COMBOBOX,

	/// DOCME
	ACTOR_COMBOBOX,

	/// DOCME
	LAYER_BOX,

	/// DOCME
	STARTTIME_BOX,

	/// DOCME
	ENDTIME_BOX,

	/// DOCME
	DURATION_BOX,

	/// DOCME
	MARGINL_BOX,

	/// DOCME
	MARGINR_BOX,

	/// DOCME
	MARGINV_BOX,

	/// DOCME
	EFFECT_BOX,

	/// DOCME
	COMMENT_CHECKBOX,


	/// DOCME
	BUTTON_BOLD,

	/// DOCME
	BUTTON_ITALICS,

	/// DOCME
	BUTTON_UNDERLINE,

	/// DOCME
	BUTTON_STRIKEOUT,

	/// DOCME
	BUTTON_FONT_NAME,

	/// DOCME
	BUTTON_COLOR1,

	/// DOCME
	BUTTON_COLOR2,

	/// DOCME
	BUTTON_COLOR3,

	/// DOCME
	BUTTON_COLOR4,

	/// DOCME
	BUTTON_COMMIT
};


