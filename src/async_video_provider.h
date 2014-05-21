// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "include/aegisub/video_provider.h"

#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>

#include <atomic>
#include <memory>
#include <set>
#include <wx/event.h>

class AssDialogue;
class AssFile;
class SubtitlesProvider;
class VideoProvider;
class VideoProviderError;
struct VideoFrame;
namespace agi {
	class BackgroundRunner;
	namespace dispatch { class Queue; }
}

/// An asynchronous video decoding and subtitle rendering wrapper
class AsyncVideoProvider {
	/// Asynchronous work queue
	std::unique_ptr<agi::dispatch::Queue> worker;

	/// Subtitles provider
	std::unique_ptr<SubtitlesProvider> subs_provider;
	/// Video provider
	std::unique_ptr<VideoProvider> source_provider;
	/// Event handler to send FrameReady events to
	wxEvtHandler *parent;

	int frame_number = -1; ///< Last frame number requested
	double time = -1.; ///< Time of the frame to pass to the subtitle renderer

	/// Copy of the subtitles file to avoid having to touch the project context
	std::unique_ptr<AssFile> subs;

	/// If >= 0, the subtitles provider current has just the lines visible on
	/// that frame loaded. If -1, the entire file is loaded. If -2, the
	/// currently loaded file is out of date.
	int single_frame = -1;

	std::shared_ptr<VideoFrame> ProcFrame(int frame, double time, bool raw = false);

	/// Produce a frame if req_version is still the current version
	void ProcAsync(uint_fast32_t req_version);

	/// Monotonic counter used to drop frames when changes arrive faster than
	/// they can be rendered
	std::atomic<uint_fast32_t> version{ 0 };

public:
	/// @brief Load the passed subtitle file
	/// @param subs File to load
	///
	/// This function blocks until is it is safe for the calling thread to
	/// modify subs
	void LoadSubtitles(const AssFile *subs) throw();

	/// @brief Update a previously loaded subtitle file
	/// @param subs Subtitle file which was last passed to LoadSubtitles
	/// @param changes Set of lines which have changed
	///
	/// This function only supports changes to existing lines, and not
	/// insertions or deletions.
	void UpdateSubtitles(const AssFile *subs, std::set<const AssDialogue *> const& changes) throw();

	/// @brief Queue a request for a frame
	/// @brief frame Frame number
	/// @brief time  Exact start time of the frame in seconds
	///
	/// This merely queues up a request and deletes any pending requests; there
	/// is no guarantee that the requested frame will ever actually be produced
	void RequestFrame(int frame, double time) throw();

	/// @brief Synchronously get a frame
	/// @brief frame Frame number
	/// @brief time  Exact start time of the frame in seconds
	/// @brief raw   Get raw frame without subtitles
	std::shared_ptr<VideoFrame> GetFrame(int frame, double time, bool raw = false);

	/// Ask the video provider to change YCbCr matricies
	void SetColorSpace(std::string const& matrix);

	int GetFrameCount() const             { return source_provider->GetFrameCount(); }
	int GetWidth() const                  { return source_provider->GetWidth(); }
	int GetHeight() const                 { return source_provider->GetHeight(); }
	double GetDAR() const                 { return source_provider->GetDAR(); }
	agi::vfr::Framerate GetFPS() const    { return source_provider->GetFPS(); }
	std::vector<int> GetKeyFrames() const { return source_provider->GetKeyFrames(); }
	std::string GetColorSpace() const     { return source_provider->GetColorSpace(); }
	std::string GetRealColorSpace() const { return source_provider->GetRealColorSpace(); }
	std::string GetWarning() const        { return source_provider->GetWarning(); }
	std::string GetDecoderName() const    { return source_provider->GetDecoderName(); }
	bool ShouldSetVideoProperties() const { return source_provider->ShouldSetVideoProperties(); }

	/// @brief Constructor
	/// @param videoFileName File to open
	/// @param parent Event handler to send FrameReady events to
	AsyncVideoProvider(agi::fs::path const& filename, std::string const& colormatrix, wxEvtHandler *parent, agi::BackgroundRunner *br);
	~AsyncVideoProvider();
};

/// Event which signals that a requested frame is ready
struct FrameReadyEvent final : public wxEvent {
	/// Frame which is ready
	std::shared_ptr<VideoFrame> frame;
	/// Time which was used for subtitle rendering
	double time;
	wxEvent *Clone() const override { return new FrameReadyEvent(*this); };
	FrameReadyEvent(std::shared_ptr<VideoFrame> frame, double time)
	: frame(std::move(frame)), time(time) { }
};

// These exceptions are wxEvents so that they can be passed directly back to
// the parent thread as events
struct VideoProviderErrorEvent final : public wxEvent, public agi::Exception {
	const char * GetName() const override { return "video/error"; }
	wxEvent *Clone() const override { return new VideoProviderErrorEvent(*this); };
	agi::Exception *Copy() const override { return new VideoProviderErrorEvent(*this); };
	VideoProviderErrorEvent(VideoProviderError const& err);
};

struct SubtitlesProviderErrorEvent final : public wxEvent, public agi::Exception {
	const char * GetName() const override { return "subtitles/error"; }
	wxEvent *Clone() const override { return new SubtitlesProviderErrorEvent(*this); };
	agi::Exception *Copy() const override { return new SubtitlesProviderErrorEvent(*this); };
	SubtitlesProviderErrorEvent(std::string const& msg);
};

wxDECLARE_EVENT(EVT_FRAME_READY, FrameReadyEvent);
wxDECLARE_EVENT(EVT_VIDEO_ERROR, VideoProviderErrorEvent);
wxDECLARE_EVENT(EVT_SUBTITLES_ERROR, SubtitlesProviderErrorEvent);
