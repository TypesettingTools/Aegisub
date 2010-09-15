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

/// @file threaded_frame_source.cpp
/// @see threaded_frame_source.h
/// @ingroup video
///

#include "threaded_frame_source.h"

#ifndef AGI_PRE
#include <iterator>
#include <functional>
#endif

#include "ass_dialogue.h"
#include "ass_exporter.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/subtitles_provider.h"
#include "video_provider_manager.h"

// Test if a line is a dialogue line which is not visible at the given time
struct invisible_line : public std::unary_function<const AssEntry*, bool> {
	double time;
	invisible_line(double time) : time(time * 1000.) { }
	bool operator()(const AssEntry *entry) const {
		const AssDialogue *diag = dynamic_cast<const AssDialogue*>(entry);
		return diag && (diag->Start.GetMS() > time || diag->End.GetMS() <= time);
	}
};

static void delete_frame(AegiVideoFrame *frame) {
	frame->Clear();
	delete frame;
}

std::tr1::shared_ptr<AegiVideoFrame> ThreadedFrameSource::ProcFrame(int frameNum, double time, bool raw) {
	std::tr1::shared_ptr<AegiVideoFrame> frame(new AegiVideoFrame, delete_frame);
	{
		wxMutexLocker locker(providerMutex);
		try {
			frame->CopyFrom(videoProvider->GetFrame(frameNum));
		}
		catch (VideoProviderError const& err) { throw VideoProviderErrorEvent(err); }
	}

	// This deliberately results in a call to LoadSubtitles while a render
	// is pending making the queued render use the new file
	if (!raw) {
		try {
			wxMutexLocker locker(fileMutex);
			if (subs.get() && singleFrame != frameNum) {
				// Generally edits and seeks come in groups; if the last thing done
				// was seek it is more likely that the user will seek again and
				// vice versa. As such, if this is the first frame requested after
				// an edit, only export the currently visible lines (because the
				// other lines will probably not be viewed before the file changes
				// again), and if it's a different frame, export the entire file.
				if (singleFrame == -1) {
					singleFrame = frameNum;
					// Copying a nontrivially sized AssFile is fairly slow, so
					// instead muck around with its innards to just temporarily
					// remove the non-visible lines without deleting them
					std::list<AssEntry*> visible;
					std::remove_copy_if(subs->Line.begin(), subs->Line.end(),
						std::back_inserter(visible),
						invisible_line(time));
					try {
						std::swap(subs->Line, visible);
						provider->LoadSubtitles(subs.get());
					}
					catch(...) {
						std::swap(subs->Line, visible);
						throw;
					}
				}
				else {
					provider->LoadSubtitles(subs.get());
					subs.reset();
				}
			}
		}
		catch (wxString const& err) { throw SubtitlesProviderErrorEvent(err); }

		provider->DrawSubtitles(*frame, time);
	}
	return frame;
}

void *ThreadedFrameSource::Entry() {
	while (!TestDestroy() && run) {
		double time;
		int frameNum;
		{
			wxMutexLocker jobLocker(jobMutex);

			if (nextTime == -1.) {
				jobReady.Wait();
				continue;
			}

			if (nextSubs.get()) {
				wxMutexLocker fileLocker(fileMutex);
				subs = nextSubs;
				singleFrame = -1;
			}

			time = nextTime;
			frameNum = nextFrame;
			nextTime = -1.;
		}

		try {
			FrameReadyEvent *evt = new FrameReadyEvent(ProcFrame(frameNum, time), time);
			evt->SetEventType(EVT_FRAME_READY);
			parent->QueueEvent(evt);
		}
		catch (wxEvent const& err) {
			// Pass error back to parent thread
			parent->QueueEvent(err.Clone());
		}
	}

	return EXIT_SUCCESS;
}

ThreadedFrameSource::ThreadedFrameSource(wxString videoFileName, wxEvtHandler *parent)
: wxThread(wxTHREAD_JOINABLE)
, provider(SubtitlesProviderFactory::GetProvider())
, videoProvider(VideoProviderFactory::GetProvider(videoFileName))
, parent(parent)
, nextTime(-1.)
, jobReady(jobMutex)
, run(true)
{
	Create();
	Run();
}
ThreadedFrameSource::~ThreadedFrameSource() {
	run = false;
	jobReady.Signal();
	Wait();
}

void ThreadedFrameSource::LoadSubtitles(AssFile *subs) throw() {
	AssExporter exporter(subs);
	exporter.AddAutoFilters();
	AssFile *exported = exporter.ExportTransform();
	wxMutexLocker locker(jobMutex);
	// Set nextSubs and let the worker thread move it to subs so that we don't
	// have to lock fileMutex on the GUI thread, as that can be locked for
	// extended periods of time with slow-rendering subtitles
	nextSubs.reset(exported);
}

void ThreadedFrameSource::RequestFrame(int frame, double time) throw() {
	wxMutexLocker locker(jobMutex);
	nextTime  = time;
	nextFrame = frame;
	jobReady.Signal();
}

std::tr1::shared_ptr<AegiVideoFrame> ThreadedFrameSource::GetFrame(int frame, double time, bool raw) {
	return ProcFrame(frame, time, raw);
}

wxDEFINE_EVENT(EVT_FRAME_READY, FrameReadyEvent);
wxDEFINE_EVENT(EVT_VIDEO_ERROR, VideoProviderErrorEvent);
wxDEFINE_EVENT(EVT_SUBTITLES_ERROR, SubtitlesProviderErrorEvent);

VideoProviderErrorEvent::VideoProviderErrorEvent(VideoProviderError const& err)
: agi::Exception(err.GetMessage(), &err)
{
	SetEventType(EVT_VIDEO_ERROR);
}
SubtitlesProviderErrorEvent::SubtitlesProviderErrorEvent(wxString err)
: agi::Exception(STD_STR(err), NULL)
{
	SetEventType(EVT_SUBTITLES_ERROR);
}
