// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file threaded_frame_source.h
/// @see threaded_frame_source.cpp
/// @ingroup video
///

#ifndef AGI_PRE
#include <tr1/memory>

#include <wx/event.h>
#include <wx/thread.h>
#endif

#include <libaegisub/exception.h>
#include "video_frame.h"

class AssFile;
class SubtitlesProvider;
class VideoProvider;

/// @class ThreadedFrameSource
/// @brief An asynchronous video decoding and subtitle rendering wrapper
class ThreadedFrameSource : public wxThread {
	/// Subtitles provider
	std::auto_ptr<SubtitlesProvider> provider;
	/// Video provider
	std::tr1::shared_ptr<VideoProvider> videoProvider;
	/// Event handler to send FrameReady events to
	wxEvtHandler *parent;

	int nextFrame;   ///< Next queued frame, or -1 for none
	double nextTime; ///< Next queued time
	std::auto_ptr<AssFile> nextSubs; ///< Next queued AssFile

	/// Subtitles to be loaded the next time a frame is requested
	std::auto_ptr<AssFile> subs;
	/// If subs is set and this is not -1, frame which subs was limited to when
	/// it was last sent to the subtitle provider
	int singleFrame;

	wxMutex fileMutex;     ///< Mutex for subtitle file usage
	wxMutex jobMutex;      ///< Mutex for nextFrame/nextTime
	wxMutex providerMutex; ///< Mutex for video provider
	wxMutex evtMutex;      ///< Mutex for FrameReadyEvents associated with this

	wxCondition jobReady;  ///< Signal for indicating that a frame has be requested

	/// Frame buffers to render subtitles into
	/// Two are needed so that a frame can be decoded/rendered while the GUI is
	/// doing stuff with the other
	AegiVideoFrame frameBuffer[2];
	/// Next frame buffer to use
	int frameBufferIdx;

	bool run; ///< Should the thread continue to run

	void *Entry();
	AegiVideoFrame const& ProcFrame(int frameNum, double time, bool raw = false);
public:
	/// @brief Load the passed subtitle file
	/// @param subs File to load
	///
	/// This function blocks until is it is safe for the calling thread to
	/// modify subs
	void LoadSubtitles(AssFile *subs) throw();

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
	AegiVideoFrame const& GetFrame(int frame, double time, bool raw = false);

	/// @brief Non-blocking Delete
	///
	/// Needed due to that calling Delete while the thread is waiting on
	/// jobReady results in a deadlock; as such this is the only safe way to
	/// destroy a ThreadedFrameSource
	void End();

	std::tr1::shared_ptr<VideoProvider> GetVideoProvider() const { return videoProvider; }

	/// @brief Constructor
	/// @param videoFileName File to open
	/// @param parent Event handler to send FrameReady events to
	ThreadedFrameSource(wxString videoFileName, wxEvtHandler *parent);
	~ThreadedFrameSource();
};

/// @class FrameReadyEvent
/// @brief Event which signals that a requested frame is ready
class FrameReadyEvent : public wxEvent {
	/// Externally passed mutex that is kept locked as long as this or a copy
	/// of this exists. Used to ensure that the next FrameReadyEvent is not
	/// announced until this one is fully processed.
	///
	/// Although tr1 does not require that shared_ptr be thread safe (due to
	/// the standard having no concept of threads), all implementations have
	/// at least a thread safe reference counter, which is all we happen to
	/// need here.
	std::tr1::shared_ptr<wxMutexLocker> mutex;
public:
	/// Frame which is ready; only guaranteed to be valid as long as this
	/// event object exists
	const AegiVideoFrame *frame;
	/// Time which was used for subtitle rendering
	double time;
	wxEvent *Clone() const { return new FrameReadyEvent(*this); };
	FrameReadyEvent(const AegiVideoFrame *frame, double time, std::tr1::shared_ptr<wxMutexLocker> mutex)
		: mutex(mutex), frame(frame), time(time) {
	}
};

// These exceptions are wxEvents so that they can be passed directly back to
// the parent thread as events
class VideoProviderErrorEvent : public wxEvent, public agi::Exception {
public:
	const char * GetName() const { return "video/error"; }
	wxEvent *Clone() const { return new VideoProviderErrorEvent(*this); };
	agi::Exception *Copy() const { return new VideoProviderErrorEvent(*this); };
	VideoProviderErrorEvent(wxString msg);
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
