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


#ifndef AUDIO_DISPLAY_H
#define AUDIO_DISPLAY_H


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/window.h>
#include <wx/bitmap.h>
#include <stdint.h>
#include "audio_provider_manager.h"
#include "audio_player_manager.h"
#include "audio_spectrum.h"


//////////////
// Prototypes
class AssDialogue;
class StreamAudioProvider;
class SubtitlesGrid;
class AudioBox;
class AudioKaraoke;
class VideoProvider;
class FrameMain;


/////////////////
// Display class
class AudioDisplay: public wxWindow {
	friend class FrameMain;
private:
	SubtitlesGrid *grid;
	int line_n;
	AssDialogue *dialogue;

	AudioSpectrum *spectrumRenderer;

	wxBitmap *origImage;
	wxBitmap *spectrumDisplay;
	wxBitmap *spectrumDisplaySelected;
	int64_t PositionSample;
	float scale;
	int samples;
	int64_t Position;
	int samplesPercent;
	int oldCurPos;
	bool hasFocus;
	bool blockUpdate;
	bool dontReadTimes;
	bool playingToEnd;

	bool needImageUpdate;
	bool needImageUpdateWeak;

	bool hasSel;
	bool hasKaraoke;
	bool diagUpdated;
	bool holding;
	bool draggingScale;
	int64_t selStart;
	int64_t selEnd;
	int64_t lineStart;
	int64_t lineEnd;
	int64_t selStartCap;
	int64_t selEndCap;
	int hold;
	int lastX;
	int lastDragX;
	int curStartMS;
	int curEndMS;
	int holdSyl;

	int *peak;
	int *min;

	int scrubTime;
	int64_t scrubLastPos;
	bool scrubbing;
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
	AudioProvider *provider;
	StreamAudioProvider *scrubProvider;
	AudioPlayer *player;

	bool NeedCommit;
	bool loaded;
	bool temporary;
	int w,h;
	AudioBox *box;
	AudioKaraoke *karaoke;
	wxScrollBar *ScrollBar;
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
	Audio_Update_Timer = 1700
};


#endif
