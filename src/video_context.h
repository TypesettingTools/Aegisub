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

#include <libaegisub/fs_fwd.h>
#include <libaegisub/signal.h>
#include <libaegisub/vfr.h>

#include <boost/filesystem/path.hpp>
#include <chrono>
#include <ctime>
#include <list>
#include <memory>
#include <set>

#include <wx/timer.h>

class AssDialogue;
class DialogProgress;
class ThreadedFrameSource;
class VideoProvider;
struct SubtitlesProviderErrorEvent;
struct VideoFrame;
struct VideoProviderErrorEvent;

namespace agi {
	struct Context;
	class OptionValue;
}

enum class AspectRatio {
	Default = 0,
	Fullscreen,
	Widescreen,
	Cinematic,
	Custom
};

/// @class VideoContext
/// @brief Manage a bunch of things vaguely related to video playback
///
/// VideoContext's core responsibility is opening and playing videos. Along
/// with that, it also manages video timecodes and keyframes, and some
/// video-related UI properties
class VideoContext final : public wxEvtHandler {
	/// Current frame number changed (new frame number)
	agi::signal::Signal<int> Seek;
	/// A new video was opened
	agi::signal::Signal<> VideoOpen;
	/// New keyframes opened (new keyframe data)
	agi::signal::Signal<std::vector<int> const&> KeyframesOpen;
	/// New timecodes opened (new timecode data)
	agi::signal::Signal<agi::vfr::Framerate const&> TimecodesOpen;
	/// Aspect ratio was changed (type, value)
	agi::signal::Signal<AspectRatio, double> ARChange;

	agi::Context *context;

	DialogProgress *progress = nullptr;

	/// The video provider owned by the threaded frame source, or nullptr if no
	/// video is open
	VideoProvider *video_provider;

	/// Asynchronous provider of video frames
	std::unique_ptr<ThreadedFrameSource> provider;

	/// Filename of currently open video
	agi::fs::path video_filename;

	/// List of frame numbers which are keyframes
	std::vector<int> keyframes;

	/// File name of the currently open keyframes or empty if keyframes are not overridden
	agi::fs::path keyframes_filename;

	/// Playback timer used to periodically check if we should go to the next
	/// frame while playing video
	wxTimer playback;

	/// Time when playback was last started
	std::chrono::steady_clock::time_point playback_start_time;

	/// The start time of the first frame of the current playback; undefined if
	/// video is not currently playing
	int start_ms = 0;

	/// The last frame to play if video is currently playing
	int end_frame = 0;

	/// The frame number which was last requested from the video provider,
	/// which may not be the same thing as the currently displayed frame
	int frame_n = 0;

	/// The picture aspect ratio of the video if the aspect ratio has been
	/// overridden by the user
	double ar_value = 1.;

	/// The current AR type
	AspectRatio ar_type = AspectRatio::Default;

	/// Does the currently loaded video file have subtitles muxed into it?
	bool has_subtitles = false;

	/// Filename of the currently loaded timecodes file, or empty if timecodes
	/// have not been overridden
	agi::fs::path timecodes_filename;

	/// Cached option for audio playing when frame stepping
	const agi::OptionValue* playAudioOnStep;

	/// Amending the frame source's copy of the subtitle file requires that it
	/// be kept in perfect sync. Saving the file can add lines to the file
	/// without a commit, breaking this sync, so force a non-amend after each
	/// save.
	bool no_amend = false;

	void OnPlayTimer(wxTimerEvent &event);

	/// The timecodes from the video file
	agi::vfr::Framerate video_fps;
	/// External timecode which have been loaded, if any
	agi::vfr::Framerate ovr_fps;

	void OnVideoError(VideoProviderErrorEvent const& err);
	void OnSubtitlesError(SubtitlesProviderErrorEvent const& err);

	void OnSubtitlesCommit(int type, std::set<const AssDialogue *> const& changed);
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
	VideoProvider *GetProvider() const { return video_provider; }

	/// Synchronously get a video frame
	/// @param n Frame number to get
	/// @param raw If true, subtitles are not rendered on the frame
	/// @return The requested frame
	std::shared_ptr<VideoFrame> GetFrame(int n, bool raw = false);

	/// Asynchronously get a video frame, triggering a EVT_FRAME_READY event when it's ready
	/// @param n Frame number to get
	void GetFrameAsync(int n);

	/// Is there a video loaded?
	bool IsLoaded() const { return !!video_provider; }

	/// Get the file name of the currently open video, if any
	agi::fs::path GetVideoName() const { return video_filename; }

	/// Is the video currently playing?
	bool IsPlaying() const { return playback.IsRunning(); }

	/// Does the video file loaded have muxed subtitles that we can load?
	bool HasSubtitles() const { return has_subtitles; }

	/// Get the width of the currently open video
	int GetWidth() const;

	/// Get the height of the currently open video
	int GetHeight() const;

	/// Get the length in frames of the currently open video
	int GetLength() const;

	/// Get the current frame number
	int GetFrameN() const { return frame_n; }

	/// Get the actual aspect ratio from a predefined AR type
	double GetARFromType(AspectRatio type) const;

	/// Override the aspect ratio of the currently loaded video
	void SetAspectRatio(double value);

	/// Override the aspect ratio of the currently loaded video
	/// @param type Predefined type to set the AR to. Must not be Custom.
	void SetAspectRatio(AspectRatio type);

	/// Get the current AR type
	AspectRatio GetAspectRatioType() const { return ar_type; }

	/// Get the current aspect ratio of the video
	double GetAspectRatioValue() const { return ar_value; }

	/// @brief Open a new video
	/// @param filename Video to open, or empty to close the current video
	void SetVideo(const agi::fs::path &filename);
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

	const std::vector<int>& GetKeyFrames() const { return keyframes; };
	agi::fs::path GetKeyFramesName() const { return keyframes_filename; }
	void LoadKeyframes(agi::fs::path const& filename);
	void SaveKeyframes(agi::fs::path const& filename);
	void CloseKeyframes();
	bool OverKeyFramesLoaded() const { return !keyframes_filename.empty(); }
	bool KeyFramesLoaded() const { return !keyframes.empty(); }

	agi::fs::path GetTimecodesName() const { return timecodes_filename; }
	void LoadTimecodes(agi::fs::path const& filename);
	void SaveTimecodes(agi::fs::path const& filename);
	void CloseTimecodes();
	bool OverTimecodesLoaded() const { return ovr_fps.IsLoaded(); }
	bool TimecodesLoaded() const { return video_fps.IsLoaded() || ovr_fps.IsLoaded(); };

	const agi::vfr::Framerate& FPS() const { return ovr_fps.IsLoaded() ? ovr_fps : video_fps; }
	const agi::vfr::Framerate& VideoFPS() const { return video_fps; }

	int TimeAtFrame(int frame, agi::vfr::Time type = agi::vfr::EXACT) const;
	int FrameAtTime(int time, agi::vfr::Time type = agi::vfr::EXACT) const;

	static VideoContext *Get();
};
