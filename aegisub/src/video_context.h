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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_context.h
/// @see video_context.cpp
/// @ingroup video
///


#pragma once


///////////
// Headers
#ifndef AGI_PRE
#include <time.h>

#include <list>

#include <wx/glcanvas.h>
#include <wx/timer.h>
#include <wx/stopwatch.h>
#endif

#if ! wxUSE_GLCANVAS
#error "Aegisub requires wxWidgets to be compiled with OpenGL support."
#endif

#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>

/// DOCME
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



/// DOCME
/// @class VideoContext
/// @brief DOCME
///
/// DOCME
class VideoContext : public wxEvtHandler {
	friend class AudioProvider;
	friend class VisualTool;
	friend class VideoContextThread;

private:

	/// DOCME
	static VideoContext *instance;

	/// DOCME
	std::list<VideoDisplay*> displayList;


	/// DOCME
	GLuint lastTex;

	/// DOCME
	int lastFrame;

	/// DOCME
	bool ownGlContext;

	/// DOCME
	wxGLContext *glContext;

	/// DOCME
	VideoFrameFormat vidFormat;

	/// DOCME
	AegiVideoFrame tempFrame;


	/// DOCME
	wxString tempfile;

	/// DOCME
	VideoProvider *provider;

	/// DOCME
	SubtitlesProvider *subsProvider;


	/// DOCME
	bool keyFramesLoaded;

	/// DOCME
	bool overKeyFramesLoaded;

	/// DOCME
	wxArrayInt KeyFrames;

	/// DOCME
	wxArrayInt overKeyFrames;

	/// DOCME
	wxString keyFramesFilename;


	/// DOCME
	wxMutex playMutex;

	/// DOCME
	wxTimer playback;

	/// DOCME
	wxStopWatch playTime;

	/// DOCME
	int startFrame;

	/// DOCME
	int endFrame;

	/// DOCME
	int playNextFrame;

	/// DOCME
	int nextFrame;


	/// DOCME
	bool threaded;

	/// DOCME
	bool threadLocked;

	/// DOCME
	int threadNextFrame;

	/// DOCME
	wxMutex vidMutex;

	/// DOCME
	wxThread *thread;


	/// DOCME
	bool loaded;

	/// DOCME
	bool isInverted;

	/// DOCME
	bool isPlaying;

	/// DOCME
	bool keepAudioSync;


	/// DOCME

	/// DOCME
	float texW,texH;

	/// DOCME

	/// DOCME
	int w,h;

	/// DOCME
	int frame_n;

	/// DOCME
	int length;

	/// DOCME
	double fps;


	/// DOCME
	double arValue;

	/// DOCME
	int arType;

	void UnloadTexture();
	void OnPlayTimer(wxTimerEvent &event);

public:

	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	wxString videoName;


	/// DOCME
	AssDialogue *curLine;

	/// DOCME
	AudioDisplay *audio;

	VideoContext();
	~VideoContext();

	void AddDisplay(VideoDisplay *display);
	void RemoveDisplay(VideoDisplay *display);


	/// @brief DOCME
	/// @return 
	///
	VideoProvider *GetProvider() { return provider; }
	AegiVideoFrame GetFrame(int n,bool raw=false);

	void SaveSnapshot(bool raw);

	wxGLContext *GetGLContext(wxGLCanvas *canvas);
	GLuint GetFrameAsTexture(int n);

	/// @brief DOCME
	/// @return 
	///
	float GetTexW() { return texW; }

	/// @brief DOCME
	/// @return 
	///
	float GetTexH() { return texH; }

	/// @brief DOCME
	/// @return 
	///
	VideoFrameFormat GetFormat() { return vidFormat; }


	/// @brief DOCME
	/// @return 
	///
	bool IsLoaded() { return loaded; }

	/// @brief DOCME
	/// @return 
	///
	bool IsPlaying() { return isPlaying; }

	/// @brief DOCME
	/// @return 
	///
	bool IsInverted() { return isInverted; }


	/// @brief DOCME
	/// @param sync 
	/// @return 
	///
	void EnableAudioSync(bool sync = true) { keepAudioSync = sync; }


	/// @brief DOCME
	/// @return 
	///
	int GetWidth() { return w; }

	/// @brief DOCME
	/// @return 
	///
	int GetHeight() { return h; }

	/// @brief DOCME
	/// @return 
	///
	int GetLength() { return length; }

	/// @brief DOCME
	/// @return 
	///
	int GetFrameN() { return frame_n; }

	/// @brief DOCME
	/// @return 
	///
	double GetFPS() { return fps; }

	/// @brief DOCME
	/// @param _fps 
	/// @return 
	///
	void SetFPS(double _fps) { fps = _fps; }

	double GetARFromType(int type);
	void SetAspectRatio(int type,double value=1.0);

	/// @brief DOCME
	/// @return 
	///
	int GetAspectRatioType() { return arType; }

	/// @brief DOCME
	/// @return 
	///
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

	wxArrayInt GetKeyFrames();
	void SetKeyFrames(wxArrayInt frames);
	void SetOverKeyFrames(wxArrayInt frames);
	void CloseOverKeyFrames();
	bool OverKeyFramesLoaded();
	bool KeyFramesLoaded();

	/// @brief DOCME
	/// @return 
	///
	wxString GetKeyFramesName() { return keyFramesFilename; }

	/// @brief DOCME
	/// @param name 
	///
	void SetKeyFramesName(wxString name) { keyFramesFilename = name; }

	static VideoContext *Get();
	static void Clear();

	DECLARE_EVENT_TABLE()
};



/// DOCME
/// @class VideoContextThread
/// @brief DOCME
///
/// DOCME
class VideoContextThread : public wxThread {
private:

	/// DOCME
	VideoContext *parent;

public:
	VideoContextThread(VideoContext *parent);
	wxThread::ExitCode Entry();
};


