// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file threaded_frame_source.h
/// @see threaded_frame_source.cpp
/// @ingroup video
///

#include <deque>
#include <memory>
#include <set>

#include <wx/event.h>
#include <wx/thread.h>

#include <libaegisub/exception.h>
#include <libaegisub/scoped_ptr.h>

class AegiVideoFrame;
class AssEntry;
class AssFile;
class SubtitlesProvider;
class VideoProvider;
class VideoProviderError;

/// @class ThreadedFrameSource
/// @brief An asynchronous video decoding and subtitle rendering wrapper
class ThreadedFrameSource : public wxThread {
	/// Subtitles provider
	agi::scoped_ptr<SubtitlesProvider> provider;
	/// Video provider
	agi::scoped_ptr<VideoProvider> videoProvider;
	/// Event handler to send FrameReady events to
	wxEvtHandler *parent;

	int nextFrame;   ///< Next queued frame, or -1 for none
	double nextTime; ///< Next queued time
	std::unique_ptr<AssFile> nextSubs; ///< Next queued AssFile
	std::deque<std::pair<size_t, std::unique_ptr<AssEntry>>> changedSubs;

	/// Subtitles to be loaded the next time a frame is requested
	std::unique_ptr<AssFile> subs;
	/// If subs is set and this is not -1, frame which subs was limited to when
	/// it was last sent to the subtitle provider
	int singleFrame;

	wxMutex fileMutex;     ///< Mutex for subs and singleFrame
	wxMutex jobMutex;      ///< Mutex for nextFrame, nextTime and nextSubs
	wxMutex providerMutex; ///< Mutex for video provider
	wxMutex evtMutex;      ///< Mutex for FrameReadyEvents associated with this

	wxCondition jobReady;  ///< Signal for indicating that a frame has be requested

	bool run; ///< Should the thread continue to run

	void *Entry();
	std::shared_ptr<AegiVideoFrame> ProcFrame(int frameNum, double time, bool raw = false);
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
	void UpdateSubtitles(const AssFile *subs, std::set<const AssEntry *> changes) throw();

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
	std::shared_ptr<AegiVideoFrame> GetFrame(int frame, double time, bool raw = false);

	/// Get a reference to the video provider this is using
	VideoProvider *GetVideoProvider() const { return videoProvider.get(); }

	/// @brief Constructor
	/// @param videoFileName File to open
	/// @param parent Event handler to send FrameReady events to
	ThreadedFrameSource(wxString videoFileName, wxEvtHandler *parent);
	~ThreadedFrameSource();
};

/// @class FrameReadyEvent
/// @brief Event which signals that a requested frame is ready
class FrameReadyEvent : public wxEvent {
public:
	/// Frame which is ready
	std::shared_ptr<AegiVideoFrame> frame;
	/// Time which was used for subtitle rendering
	double time;
	wxEvent *Clone() const { return new FrameReadyEvent(*this); };
	FrameReadyEvent(std::shared_ptr<AegiVideoFrame> frame, double time)
		: frame(frame), time(time) {
	}
};

// These exceptions are wxEvents so that they can be passed directly back to
// the parent thread as events
class VideoProviderErrorEvent : public wxEvent, public agi::Exception {
public:
	const char * GetName() const { return "video/error"; }
	wxEvent *Clone() const { return new VideoProviderErrorEvent(*this); };
	agi::Exception *Copy() const { return new VideoProviderErrorEvent(*this); };
	VideoProviderErrorEvent(VideoProviderError const& err);
};

class SubtitlesProviderErrorEvent : public wxEvent, public agi::Exception {
public:
	const char * GetName() const { return "subtitles/error"; }
	wxEvent *Clone() const { return new SubtitlesProviderErrorEvent(*this); };
	agi::Exception *Copy() const { return new SubtitlesProviderErrorEvent(*this); };
	SubtitlesProviderErrorEvent(wxString msg);
};

wxDECLARE_EVENT(EVT_FRAME_READY, FrameReadyEvent);
wxDECLARE_EVENT(EVT_VIDEO_ERROR, VideoProviderErrorEvent);
wxDECLARE_EVENT(EVT_SUBTITLES_ERROR, SubtitlesProviderErrorEvent);
