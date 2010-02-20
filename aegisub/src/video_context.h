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


#pragma once


///////////
// Headers
#include <time.h>
#include <wx/wxprec.h>
#include <wx/glcanvas.h>
#include <wx/timer.h>
#include <wx/stopwatch.h>
#if ! wxUSE_GLCANVAS
#error "Aegisub requires wxWidgets to be compiled with OpenGL support."
#endif
#include <list>
#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
typedef GLuint GLhandleARB;
#endif
#include "video_frame.h"


//////////////
// Prototypes
class SubtitlesGrid;
class AudioProvider;
class AudioDisplay;
class AssDialogue;
class VideoProvider;
class VideoDisplay;
class SubtitlesProvider;
class VideoContextThread;


//////////////
// Main class
class VideoContext : public wxEvtHandler {
	friend class AudioProvider;
	friend class VisualTool;
	friend class VideoContextThread;

private:
	static VideoContext *instance;
	std::list<VideoDisplay*> displayList;

	GLuint lastTex;
	int lastFrame;
	bool ownGlContext;
	wxGLContext *glContext;
	VideoFrameFormat vidFormat;
	AegiVideoFrame tempFrame;

	wxString tempfile;
	VideoProvider *provider;
	SubtitlesProvider *subsProvider;

	bool keyFramesLoaded;
	bool overKeyFramesLoaded;
	wxArrayInt KeyFrames;
	wxArrayInt overKeyFrames;
	wxString keyFramesFilename;

	int keyframesRevision;

	wxMutex playMutex;
	wxTimer playback;
	wxStopWatch playTime;
	int startFrame;
	int endFrame;
	int playNextFrame;
	int nextFrame;

	bool threaded;
	bool threadLocked;
	int threadNextFrame;
	wxMutex vidMutex;
	wxThread *thread;

	bool loaded;
	bool isInverted;
	bool isPlaying;
	bool keepAudioSync;

	float texW,texH;
	int w,h;
	int frame_n;
	int length;
	double fps;

	double arValue;
	int arType;

	void UnloadTexture();
	void OnPlayTimer(wxTimerEvent &event);

public:
	SubtitlesGrid *grid;
	wxString videoName;

	AssDialogue *curLine;
	AudioDisplay *audio;

	VideoContext();
	~VideoContext();

	void AddDisplay(VideoDisplay *display);
	void RemoveDisplay(VideoDisplay *display);

	VideoProvider *GetProvider() { return provider; }
	AegiVideoFrame GetFrame(int n,bool raw=false);

	void SaveSnapshot(bool raw);

	wxGLContext *GetGLContext(wxGLCanvas *canvas);
	float GetTexW() { return texW; }
	float GetTexH() { return texH; }
	VideoFrameFormat GetFormat() { return vidFormat; }

	bool IsLoaded() { return loaded; }
	bool IsPlaying() { return isPlaying; }
	bool IsInverted() { return isInverted; }

	void EnableAudioSync(bool sync = true) { keepAudioSync = sync; }

	int GetWidth() { return w; }
	int GetHeight() { return h; }
	int GetLength() { return length; }
	int GetFrameN() { return frame_n; }
	double GetFPS() { return fps; }
	void SetFPS(double _fps) { fps = _fps; }

	double GetARFromType(int type);
	void SetAspectRatio(int type,double value=1.0);
	int GetAspectRatioType() { return arType; }
	double GetAspectRatioValue() { return arValue; }

	void SetVideo(const wxString &filename);
	void Reset();
	void Reload();

	void JumpToFrame(int n);
	void JumpToTime(int ms,bool exact=false);

	void Refresh(bool video,bool subtitles);
	void UpdateDisplays(bool full);

	void GetScriptSize(int &w,int &h);
	wxString GetTempWorkFile ();

	void Play();
	void PlayLine();
	void Stop();

	const wxArrayInt & GetKeyFrames();
	void SetKeyFrames(wxArrayInt frames);
	void SetOverKeyFrames(wxArrayInt frames);
	void CloseOverKeyFrames();
	bool OverKeyFramesLoaded();
	bool KeyFramesLoaded();
	int GetKeyframesRevision() const { return keyframesRevision; }
	wxString GetKeyFramesName() { return keyFramesFilename; }
	void SetKeyFramesName(wxString name) { keyFramesFilename = name; }

	static VideoContext *Get();
	static void Clear();

	DECLARE_EVENT_TABLE()
};


//////////
// Thread
class VideoContextThread : public wxThread {
private:
	VideoContext *parent;

public:
	VideoContextThread(VideoContext *parent);
	wxThread::ExitCode Entry();
};

