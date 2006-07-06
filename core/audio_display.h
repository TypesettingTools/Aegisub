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
#include "audio_provider.h"
#include "audio_player.h"


//////////////
// Prototypes
class AssDialogue;
class StreamAudioProvider;
class SubtitlesGrid;
class AudioBox;
class AudioKaraoke;
class VideoDisplay;
class VideoProvider;


/////////////////
// Display class
class AudioDisplay: public wxWindow {
private:
	SubtitlesGrid *grid;
	int line_n;
	AssDialogue *dialogue;
	VideoDisplay *video;

	wxBitmap *origImage;
	wxBitmap *spectrumDisplay;
	__int64 PositionSample;
	float scale;
	int samples;
	__int64 Position;
	int samplesPercent;
	int oldCurPos;
	bool hasFocus;
	bool blockUpdate;
	bool dontReadTimes;

	bool hasSel;
	bool hasKaraoke;
	bool diagUpdated;
	bool holding;
	bool draggingScale;
	__int64 selStart;
	__int64 selEnd;
	__int64 lineStart;
	__int64 lineEnd;
	__int64 selStartCap;
	__int64 selEndCap;
	int hold;
	int lastX;
	int lastDragX;
	int curStartMS;
	int curEndMS;
	int holdSyl;

	int *peak;
	int *min;

	int scrubTime;
	__int64 scrubLastPos;
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
	void DrawWaveform(wxDC &dc,bool weak);
	void DrawSpectrum(wxDC &dc,bool weak);
	void GetDialoguePos(__int64 &start,__int64 &end,bool cap);
	void GetKaraokePos(__int64 &start,__int64 &end,bool cap);
	void UpdatePosition(int pos,bool IsSample=false);

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

	AudioDisplay(wxWindow *parent,VideoDisplay *display);
	~AudioDisplay();

	void AddLead(bool in,bool out);
	void UpdateImage(bool weak=false);
	void Update();
	void SetPosition(int pos);
	void SetSamplesPercent(int percent,bool update=true,float pivot=0.5);
	void SetScale(float scale);
	void SetFile(wxString file,VideoProvider *vprovider=NULL);
	void SetFromVideo();
	void UpdateScrollbar();
	void SetDialogue(SubtitlesGrid *_grid=NULL,AssDialogue *diag=NULL,int n=-1);

	__int64 GetSampleAtX(int x);
	int GetXAtSample(__int64 n);
	int GetMSAtX(__int64 x);
	int GetXAtMS(__int64 ms);
	int GetMSAtSample(__int64 x);
	__int64 GetSampleAtMS(__int64 ms);
	int GetSyllableAtX(int x);

	void MakeDialogueVisible(bool force=false);
	void CommitChanges();
	void ChangeLine(int delta);
	void Next();
	void Prev();

	void Play(int start,int end);
	void Stop();
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
