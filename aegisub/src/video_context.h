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

#ifndef AGI_PRE
#include <time.h>

#include <list>
#include <memory>

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
#endif

#include <libaegisub/vfr.h>
#include "video_frame.h"

class SubtitlesGrid;
class AudioProvider;
class AudioDisplay;
class AssDialogue;
class KeyFrameFile;
class SubtitlesProvider;
class VideoProvider;
class VideoDisplay;

namespace agi {
	class OptionValue;
}

/// DOCME
/// @class VideoContext
/// @brief DOCME
///
/// DOCME
class VideoContext : public wxEvtHandler {
	friend class AudioProvider;
	friend class KeyFrameFile;

private:
	/// DOCME
	std::list<VideoDisplay*> displayList;

	/// DOCME
	AegiVideoFrame tempFrame;

	/// DOCME
	std::auto_ptr<VideoProvider> provider;

	/// DOCME
	std::auto_ptr<SubtitlesProvider> subsProvider;

	/// DOCME
	std::vector<int> keyFrames;

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
	bool isPlaying;

	/// DOCME
	bool keepAudioSync;

	/// DOCME
	int frame_n;

	/// DOCME
	int length;

	/// DOCME
	double arValue;

	/// DOCME
	int arType;

	bool hasSubtitles;

	wxString ovrTimecodeFile;

	agi::OptionValue* playAudioOnStep;

	void OnPlayTimer(wxTimerEvent &event);

	agi::vfr::Framerate videoFPS;
	agi::vfr::Framerate ovrFPS;

	bool singleFrame;

public:
	/// DOCME
	SubtitlesGrid *grid;

	/// File name of currently open video, if any
	wxString videoName;

	/// DOCME
	AudioDisplay *audio;

	const agi::vfr::Framerate &VFR_Input;
	const agi::vfr::Framerate &VFR_Output;

	VideoContext();
	~VideoContext();

	void AddDisplay(VideoDisplay *display);
	void RemoveDisplay(VideoDisplay *display);


	/// @brief Get the video provider used for the currently open video
	VideoProvider *GetProvider() const { return provider.get(); }
	AegiVideoFrame GetFrame(int n,bool raw=false);

	/// @brief Save the currently displayed frame as an image
	/// @param raw Should the frame have subtitles?
	void SaveSnapshot(bool raw);

	/// @brief Is there a video loaded?
	bool IsLoaded() const { return !!provider.get(); }

	/// @brief Is the video currently playing?
	bool IsPlaying() const { return isPlaying; }

	/// @brief Does the video file loaded have muxed subtitles that we can load?
	bool HasSubtitles() const { return hasSubtitles; }

	/// @brief DOCME
	/// @param sync 
	/// @return 
	void EnableAudioSync(bool sync = true) { keepAudioSync = sync; }

	/// @brief Get the width of the currently open video
	int GetWidth() const;

	/// @brief Get the height of the currently open video
	int GetHeight() const;

	/// @brief Get the length in frames of the currently open video
	int GetLength() const { return length; }

	/// @brief Get the current frame number
	int GetFrameN() const { return frame_n; }

	double GetARFromType(int type) const;
	void SetAspectRatio(int type,double value=1.0);

	/// @brief DOCME
	/// @return 
	int GetAspectRatioType() const { return arType; }

	/// @brief DOCME
	/// @return 
	double GetAspectRatioValue() const { return arValue; }

	/// @brief Open a new video
	/// @param filename Video to open, or empty to close the current video
	void SetVideo(const wxString &filename);
	/// @brief Close the video, keyframes and timecodes
	void Reset();

	/// @brief Jump to the beginning of a frame
	/// @param n Frame number to jump to
	void JumpToFrame(int n);
	/// @brief Jump to a time
	/// @param ms Time to jump to in milliseconds
	/// @param end Type of time
	void JumpToTime(int ms, agi::vfr::Time end = agi::vfr::START);

	/// @brief Refresh the subtitle provider
	/// @param full Send the entire subtitle file to the renderer rather than
	///             just the lines visible on the current frame
	void Refresh(bool full = false);

	/// @brief Update the video display
	/// @param full Recalculate size and slider lengths
	/// @param seek Update is just a seek and file has not changed
	void UpdateDisplays(bool full, bool seek = false);

	/// @brief Get the height and width of the current script
	/// @param[out] w Width
	/// @param[out] h Height
	///
	/// This probably shouldn't be in VideoContext
	void GetScriptSize(int &w,int &h);

	/// Starting playing the video
	void Play();
	/// Play the next frame then stop
	void PlayNextFrame();
	/// Play the previous frame then stop
	void PlayPrevFrame();
	/// Seek to the beginning of the current line, then play to the end of it
	void PlayLine();
	/// Stop playing
	void Stop();

	const std::vector<int>& GetKeyFrames() const { return keyFrames; };
	wxString GetKeyFramesName() const { return keyFramesFilename; }
	void LoadKeyframes(wxString filename);
	void SaveKeyframes(wxString filename);
	void CloseKeyframes();
	bool OverKeyFramesLoaded() const { return !keyFramesFilename.empty(); }
	bool KeyFramesLoaded() const { return !keyFrames.empty(); }

	wxString GetTimecodesName() const { return ovrTimecodeFile; }
	void LoadTimecodes(wxString filename);
	void SaveTimecodes(wxString filename);
	void CloseTimecodes();
	bool OverTimecodesLoaded() const { return ovrFPS.IsLoaded(); }
	bool TimecodesLoaded() const { return videoFPS.IsLoaded() || ovrFPS.IsLoaded(); };

	const agi::vfr::Framerate& FPS() const { return ovrFPS.IsLoaded() ? ovrFPS : videoFPS; }

	int TimeAtFrame(int frame, agi::vfr::Time type = agi::vfr::EXACT) const;
	int FrameAtTime(int time, agi::vfr::Time type = agi::vfr::EXACT) const;

	static VideoContext *Get();

	DECLARE_EVENT_TABLE()
};
