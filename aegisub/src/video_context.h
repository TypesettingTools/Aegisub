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

/// @file video_context.h
/// @see video_context.cpp
/// @ingroup video
///

#ifndef AGI_PRE
#include <time.h>

#include <list>
#include <memory>

#include <wx/timer.h>
#include <wx/stopwatch.h>
#endif

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>
#include <libaegisub/vfr.h>

class AegiVideoFrame;
class SubtitlesProviderErrorEvent;
class ThreadedFrameSource;
class VideoProvider;
class VideoProviderErrorEvent;

namespace agi {
	struct Context;
	class OptionValue;
}

/// @class VideoContext
/// @brief Manage a bunch of things vaguely related to video playback
///
/// VideoContext's core responsibility is opening and playing videos. Along
/// with that, it also manages video timecodes and keyframes, and some
/// video-related UI properties
class VideoContext : public wxEvtHandler {
	/// Current frame number changed (new frame number)
	agi::signal::Signal<int> Seek;
	/// A new video was opened
	agi::signal::Signal<> VideoOpen;
	/// New keyframes opened (new keyframe data)
	agi::signal::Signal<std::vector<int> const&> KeyframesOpen;
	/// New timecodes opened (new timecode data)
	agi::signal::Signal<agi::vfr::Framerate const&> TimecodesOpen;
	/// Aspect ratio was changed (type, value)
	agi::signal::Signal<int, double> ARChange;

	agi::Context *context;

	/// The video provider owned by the threaded frame source, or nullptr if no
	/// video is open
	VideoProvider *videoProvider;

	/// Asynchronous provider of video frames
	agi::scoped_ptr<ThreadedFrameSource> provider;

	/// Filename of currently open video
	wxString videoFile;

	/// List of frame numbers which are keyframes
	std::vector<int> keyFrames;

	/// File name of the currently open keyframes or empty if keyframes are not overridden
	wxString keyFramesFilename;

	/// Playback timer used to periodically check if we should go to the next
	/// frame while playing video
	wxTimer playback;

	/// Time since playback was last started
	wxStopWatch playTime;

	/// The start time of the first frame of the current playback; undefined if
	/// video is not currently playing
	int startMS;

	/// The last frame to play if video is currently playing
	int endFrame;

	/// The frame number which was last requested from the video provider,
	/// which may not be the same thing as the currently displayed frame
	int frame_n;

	/// The picture aspect ratio of the video if the aspect ratio has been
	/// overridden by the user
	double arValue;

	/// @brief The current AR type
	///
	/// 0 is square pixels; 1-3 are predefined ARs; 4 is custom, where the real
	/// AR is in arValue
	int arType;

	/// Does the currently loaded video file have subtitles muxed into it?
	bool hasSubtitles;

	/// Filename of the currently loaded timecodes file, or empty if timecodes
	/// have not been overridden
	wxString ovrTimecodeFile;

	/// Cached option for audio playing when frame stepping
	const agi::OptionValue* playAudioOnStep;

	void OnPlayTimer(wxTimerEvent &event);

	/// The timecodes from the video file
	agi::vfr::Framerate videoFPS;
	/// External timecode which have been loaded, if any
	agi::vfr::Framerate ovrFPS;

	void OnVideoError(VideoProviderErrorEvent const& err);
	void OnSubtitlesError(SubtitlesProviderErrorEvent const& err);

	void OnSubtitlesCommit();
	void OnSubtitlesSave();

	/// Close the video, keyframes and timecodes
	void Reset();

public:
	VideoContext();
	~VideoContext();

	/// @brief Set the context that this is the video controller for
	/// @param context Initialized project context
	///
	/// Once this is no longer a singleton this can probably be moved into
	/// the constructor
	void SetContext(agi::Context *context);

	/// @brief Get the video provider used for the currently open video
	VideoProvider *GetProvider() const { return videoProvider; }

	/// Synchronously get a video frame
	/// @param n Frame number to get
	/// @param raw If true, subtitles are not rendered on the frame
	/// @return The requested frame
	std::shared_ptr<AegiVideoFrame> GetFrame(int n, bool raw = false);

	/// Asynchronously get a video frame, triggering a EVT_FRAME_READY event when it's ready
	/// @param n Frame number to get
	void GetFrameAsync(int n);

	/// Is there a video loaded?
	bool IsLoaded() const { return !!videoProvider; }

	/// Get the file name of the currently open video, if any
	wxString GetVideoName() const { return videoFile; }

	/// Is the video currently playing?
	bool IsPlaying() const { return playback.IsRunning(); }

	/// Does the video file loaded have muxed subtitles that we can load?
	bool HasSubtitles() const { return hasSubtitles; }

	/// Get the width of the currently open video
	int GetWidth() const;

	/// Get the height of the currently open video
	int GetHeight() const;

	/// Get the length in frames of the currently open video
	int GetLength() const;

	/// Get the current frame number
	int GetFrameN() const { return frame_n; }

	/// Get the actual aspect ratio from a predefined AR type
	double GetARFromType(int type) const;

	/// Override the aspect ratio of the currently loaded video
	/// @param type Aspect ratio type from 0-4
	/// @param value If type is 4 (custom), the aspect ratio to use
	void SetAspectRatio(int type, double value=1.0);

	/// Get the current AR type
	int GetAspectRatioType() const { return arType; }

	/// Get the current aspect ratio of the video
	double GetAspectRatioValue() const { return arValue; }

	/// @brief Open a new video
	/// @param filename Video to open, or empty to close the current video
	void SetVideo(const wxString &filename);
	/// @brief Close and reopen the current video
	void Reload();

	/// @brief Jump to the beginning of a frame
	/// @param n Frame number to jump to
	void JumpToFrame(int n);
	/// @brief Jump to a time
	/// @param ms Time to jump to in milliseconds
	/// @param end Type of time
	void JumpToTime(int ms, agi::vfr::Time end = agi::vfr::START);

	/// Starting playing the video
	void Play();
	/// Play the next frame then stop
	void NextFrame();
	/// Play the previous frame then stop
	void PrevFrame();
	/// Seek to the beginning of the current line, then play to the end of it
	void PlayLine();
	/// Stop playing
	void Stop();

	DEFINE_SIGNAL_ADDERS(Seek, AddSeekListener)
	DEFINE_SIGNAL_ADDERS(VideoOpen, AddVideoOpenListener)
	DEFINE_SIGNAL_ADDERS(KeyframesOpen, AddKeyframesListener)
	DEFINE_SIGNAL_ADDERS(TimecodesOpen, AddTimecodesListener)
	DEFINE_SIGNAL_ADDERS(ARChange, AddARChangeListener)

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
	const agi::vfr::Framerate& VideoFPS() const { return videoFPS; }

	int TimeAtFrame(int frame, agi::vfr::Time type = agi::vfr::EXACT) const;
	int FrameAtTime(int time, agi::vfr::Time type = agi::vfr::EXACT) const;

	static VideoContext *Get();
	static void OnExit();
};
