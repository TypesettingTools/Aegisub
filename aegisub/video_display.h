// Copyright (c) 2005-2007, Rodrigo Braz Monteiro
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


#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H


///////////
// Headers
#include <wx/wxprec.h>
#ifdef __WINDOWS__
#include <windows.h>
#endif
#include <time.h>


//////////////
// Prototypes
class SubtitlesGrid;
class VideoSlider;
class AudioProvider;
class AudioDisplay;
class AssDialogue;
class VideoProvider;
class VideoDisplayVisual;


//////////////
// Main class
class VideoDisplay: public wxWindow {
	friend class AudioProvider;
	friend class VideoDisplayVisual;

private:
	wxString tempfile;

	wxSize origSize;
	bool threaded;
	int nextFrame;

	bool keyFramesLoaded;
	bool overKeyFramesLoaded;
	wxArrayInt KeyFrames;
	wxArrayInt overKeyFrames;
	wxString keyFramesFilename;

	clock_t PlayTime;
	clock_t StartTime;
	wxTimer Playback;
	int StartFrame;
	int EndFrame;
	int PlayNextFrame;
	double arValue;
	int arType;

	VideoDisplayVisual *visual;

	wxBitmap GetFrame(int n);
	wxBitmap GetFrame() { return GetFrame(frame_n); };

	void UpdateSize();
	void SaveSnapshot();

	void OnPaint(wxPaintEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnCopyToClipboard(wxCommandEvent &event);
	void OnSaveSnapshot(wxCommandEvent &event);
	void OnCopyCoords(wxCommandEvent &event);
	void OnPlayTimer(wxTimerEvent &event);

public:
	wxArrayInt GetKeyFrames();
	void SetKeyFrames(wxArrayInt frames);
	void SetOverKeyFrames(wxArrayInt frames);
	void CloseOverKeyFrames();
	bool OverKeyFramesLoaded();
	bool KeyFramesLoaded();
	wxString GetKeyFramesName() { return keyFramesFilename; }
	void SetKeyFramesName(wxString name) { keyFramesFilename = name; }

	VideoProvider *provider;

	SubtitlesGrid *grid;
	wxString videoName;
	int w,h;
	int frame_n;
	int length;
	bool loaded;
	bool IsPlaying;
	double fps;
	double zoomValue;

	bool bTrackerEditing;
	int MovementEdit;
	double TrackerEdit;
	int MouseDownX, MouseDownY;

	VideoSlider *ControlSlider;
	wxComboBox *zoomBox;
	wxTextCtrl *PositionDisplay;
	wxTextCtrl *SubsPosition;
	AssDialogue *curLine;
	AudioDisplay *audio;

	VideoDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~VideoDisplay();

	void SetVideo(const wxString &filename);
	void Reset();
	void Unload();
	void JumpToFrame(int n);
	void JumpToTime(int ms);
	void RefreshSubtitles();
	void RefreshVideo();
	void DrawText(wxPoint Pos, wxString Text);
	void UpdatePositionDisplay();
	void SetZoom(double value);
	void SetZoomPos(int pos);
	void UpdateSubsRelativeTime();
	void GetScriptSize(int &w,int &h);
	wxString GetTempWorkFile ();

	double GetARFromType(int type);
	void SetAspectRatio(int type,double value=1.0);
	int GetAspectRatioType() { return arType; }
	double GetAspectRatioValue() { return arValue; }

	void Play();
	void PlayLine();
	void Stop();

	DECLARE_EVENT_TABLE()
};


#endif
