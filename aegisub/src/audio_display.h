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

/// @file audio_display.h
/// @see audio_display.cpp
/// @ingroup audio_ui
///


#ifndef AUDIO_DISPLAY_H

/// DOCME
#define AUDIO_DISPLAY_H


///////////
// Headers
#include <wx/window.h>
#include <wx/bitmap.h>
#include <wx/scrolbar.h>
#include <stdint.h>
#include "audio_provider_manager.h"
#include "audio_player_manager.h"
#include "audio_renderer_spectrum.h"


//////////////
// Prototypes
class AssDialogue;
class StreamAudioProvider;
class SubtitlesGrid;
class AudioBox;
class AudioKaraoke;
class VideoProvider;
class FrameMain;



/// DOCME
/// @class AudioDisplay
/// @brief DOCME
///
/// DOCME
class AudioDisplay: public wxWindow {
	friend class FrameMain;
private:

	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	int line_n;

	/// DOCME
	AssDialogue *dialogue;


	/// DOCME
	AudioSpectrum *spectrumRenderer;


	/// DOCME
	wxBitmap *origImage;

	/// DOCME
	wxBitmap *spectrumDisplay;

	/// DOCME
	wxBitmap *spectrumDisplaySelected;

	/// DOCME
	int64_t PositionSample;

	/// DOCME
	float scale;

	/// DOCME
	int samples;

	/// DOCME
	int64_t Position;

	/// DOCME
	int samplesPercent;

	/// DOCME
	int oldCurPos;

	/// DOCME
	bool hasFocus;

	/// DOCME
	bool blockUpdate;

	/// DOCME
	bool dontReadTimes;

	/// DOCME
	bool playingToEnd;


	/// DOCME
	bool needImageUpdate;

	/// DOCME
	bool needImageUpdateWeak;


	/// DOCME
	bool hasSel;

	/// DOCME
	bool hasKaraoke;

	/// DOCME
	bool diagUpdated;

	/// DOCME
	bool holding;

	/// DOCME
	bool draggingScale;

	/// DOCME
	int64_t selStart;

	/// DOCME
	int64_t selEnd;

	/// DOCME
	int64_t lineStart;

	/// DOCME
	int64_t lineEnd;

	/// DOCME
	int64_t selStartCap;

	/// DOCME
	int64_t selEndCap;

	/// DOCME
	int hold;

	/// DOCME
	int lastX;

	/// DOCME
	int lastDragX;

	/// DOCME
	int curStartMS;

	/// DOCME
	int curEndMS;

	/// DOCME
	int holdSyl;


	/// DOCME
	int *peak;

	/// DOCME
	int *min;


	/// DOCME
	int scrubTime;

	/// DOCME
	int64_t scrubLastPos;

	/// DOCME
	bool scrubbing;

	/// DOCME
	int scrubLastRate;

	void OnPaint(wxPaintEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnUpdateTimer(wxTimerEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnGetFocus(wxFocusEvent &event);
	void OnLoseFocus(wxFocusEvent &event);

	void UpdateSamples();
	void Reset();
	void DrawTimescale(wxDC &dc);
	void DrawKeyframes(wxDC &dc);
	void DrawInactiveLines(wxDC &dc);
	void DrawWaveform(wxDC &dc,bool weak);
	void DrawSpectrum(wxDC &dc,bool weak);
	void GetDialoguePos(int64_t &start,int64_t &end,bool cap);
	void GetKaraokePos(int64_t &start,int64_t &end,bool cap);
	void UpdatePosition(int pos,bool IsSample=false);

	int GetBoundarySnap(int x,int range,bool shiftHeld,bool start=true);
	void DoUpdateImage();

public:

	/// DOCME
	AudioProvider *provider;

	/// DOCME
	StreamAudioProvider *scrubProvider;

	/// DOCME
	AudioPlayer *player;


	/// DOCME
	bool NeedCommit;

	/// DOCME
	bool loaded;

	/// DOCME
	bool temporary;

	/// DOCME

	/// DOCME
	int w,h;

	/// DOCME
	AudioBox *box;

	/// DOCME
	AudioKaraoke *karaoke;

	/// DOCME
	wxScrollBar *ScrollBar;

	/// DOCME
	wxTimer UpdateTimer;

	AudioDisplay(wxWindow *parent);
	~AudioDisplay();

	void UpdateImage(bool weak=false);
	void Update();
	void RecreateImage();
	void SetPosition(int pos);
	void SetSamplesPercent(int percent,bool update=true,float pivot=0.5);
	void SetScale(float scale);
	void UpdateScrollbar();
	void SetDialogue(SubtitlesGrid *_grid=NULL,AssDialogue *diag=NULL,int n=-1);
	void MakeDialogueVisible(bool force=false);
	void ChangeLine(int delta, bool block=false);
	void Next(bool play=true);
	void Prev(bool play=true);

	void UpdateTimeEditCtrls();
	void CommitChanges(bool nextLine=false);
	void AddLead(bool in,bool out);

	void SetFile(wxString file);
	void SetFromVideo();
	void Reload();

	void Play(int start,int end);
	void Stop();

	int64_t GetSampleAtX(int x);
	int GetXAtSample(int64_t n);
	int GetMSAtX(int64_t x);
	int GetXAtMS(int64_t ms);
	int GetMSAtSample(int64_t x);
	int64_t GetSampleAtMS(int64_t ms);
	int GetSyllableAtX(int x);

	void GetTimesDialogue(int &start,int &end);
	void GetTimesSelection(int &start,int &end);
	void SetSelection(int start, int end);

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	Audio_Update_Timer = 1700
};


#endif


