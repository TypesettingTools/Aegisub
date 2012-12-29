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

/// @file threaded_frame_source.cpp
/// @see threaded_frame_source.h
/// @ingroup video
///

#include "threaded_frame_source.h"

#include <deque>
#include <functional>

#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/algorithm_ext.hpp>

#include "ass_dialogue.h"
#include "ass_exporter.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "include/aegisub/subtitles_provider.h"
#include "video_frame.h"
#include "video_provider_manager.h"

enum {
	NEW_SUBS_FILE = -1,
	SUBS_FILE_ALREADY_LOADED = -2
};

// Test if a line is a dialogue line which is not visible at the given time
struct invisible_line : public std::unary_function<AssEntry const&, bool> {
	double time;
	invisible_line(double time) : time(time * 1000.) { }
	bool operator()(AssEntry const& entry) const {
		const AssDialogue *diag = dynamic_cast<const AssDialogue*>(&entry);
		return diag && (diag->Start > time || diag->End <= time);
	}
};

std::shared_ptr<AegiVideoFrame> ThreadedFrameSource::ProcFrame(int frameNum, double time, bool raw) {
	std::shared_ptr<AegiVideoFrame> frame(new AegiVideoFrame, [](AegiVideoFrame *frame) {
		frame->Clear();
		delete frame;
	});

	{
		wxMutexLocker locker(providerMutex);
		try {
			frame->CopyFrom(videoProvider->GetFrame(frameNum));
		}
		catch (VideoProviderError const& err) { throw VideoProviderErrorEvent(err); }
	}

	// This deliberately results in a call to LoadSubtitles while a render
	// is pending making the queued render use the new file
	if (!raw && provider) {
		try {
			wxMutexLocker locker(fileMutex);
			if (subs && singleFrame != frameNum && singleFrame != SUBS_FILE_ALREADY_LOADED) {
				// Generally edits and seeks come in groups; if the last thing done
				// was seek it is more likely that the user will seek again and
				// vice versa. As such, if this is the first frame requested after
				// an edit, only export the currently visible lines (because the
				// other lines will probably not be viewed before the file changes
				// again), and if it's a different frame, export the entire file.
				if (singleFrame != NEW_SUBS_FILE) {
					provider->LoadSubtitles(subs.get());
					singleFrame = SUBS_FILE_ALREADY_LOADED;
				}
				else {
					// This will crash if any of the export filters try to use
					// anything but the subtitles, but that wouldn't be safe to
					// do anyway
					agi::Context c;
					memset(&c, 0, sizeof c);
					c.ass = subs.get();

					AssExporter exporter(&c);
					exporter.AddAutoFilters();
					exporter.ExportTransform();

					singleFrame = frameNum;
					// Copying a nontrivially sized AssFile is fairly slow, so
					// instead muck around with its innards to just temporarily
					// remove the non-visible lines without deleting them
					std::deque<AssEntry*> full;
					for (auto& line : subs->Line)
						full.push_back(&line);
					subs->Line.remove_if(invisible_line(time));

					try {
						provider->LoadSubtitles(subs.get());

						subs->Line.clear();
						boost::push_back(subs->Line, full | boost::adaptors::indirected);
					}
					catch (...) {
						subs->Line.clear();
						boost::push_back(subs->Line, full | boost::adaptors::indirected);
						throw;
					}
				}
			}
		}
		catch (wxString const& err) { throw SubtitlesProviderErrorEvent(err); }

		provider->DrawSubtitles(*frame, time);
	}
	return frame;
}

void *ThreadedFrameSource::Entry() {
	while (!TestDestroy()) {
		double time;
		int frameNum;
		std::unique_ptr<AssFile> newSubs;
		std::deque<std::pair<size_t, std::unique_ptr<AssEntry>>> updates;
		{
			wxMutexLocker jobLocker(jobMutex);

			if (!run)
				return EXIT_SUCCESS;

			if (nextTime == -1.) {
				jobReady.Wait();
				continue;
			}

			time = nextTime;
			frameNum = nextFrame;
			nextTime = -1.;
			newSubs = move(nextSubs);
			updates = move(changedSubs);
		}

		if (newSubs || updates.size()) {
			wxMutexLocker fileLocker(fileMutex);

			if (newSubs)
				subs = move(newSubs);

			if (updates.size()) {
				size_t i = 0;
				auto it = subs->Line.begin();
				// Replace each changed line in subs with the updated version
				for (auto& update : updates) {
					advance(it, update.first - i);
					i = update.first;
					subs->Line.insert(it, *update.second.release());
					delete &*it--;
				}
			}

			singleFrame = NEW_SUBS_FILE;
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

static SubtitlesProvider *get_subs_provider(wxEvtHandler *parent) {
	try {
		return SubtitlesProviderFactory::GetProvider();
	}
	catch (wxString const& err) {
		parent->AddPendingEvent(SubtitlesProviderErrorEvent(err));
		return 0;
	}
}

ThreadedFrameSource::ThreadedFrameSource(wxString videoFileName, wxEvtHandler *parent)
: wxThread(wxTHREAD_JOINABLE)
, provider(get_subs_provider(parent))
, videoProvider(VideoProviderFactory::GetProvider(videoFileName))
, parent(parent)
, nextFrame(-1)
, nextTime(-1.)
, singleFrame(-1)
, jobReady(jobMutex)
, run(true)
{
	Create();
	Run();
}

ThreadedFrameSource::~ThreadedFrameSource() {
	{
		wxMutexLocker locker(jobMutex);
		run = false;
		jobReady.Signal();
	}
	Wait();
}

void ThreadedFrameSource::LoadSubtitles(const AssFile *subs) throw() {
	AssFile *copy = new AssFile(*subs);
	wxMutexLocker locker(jobMutex);
	// Set nextSubs and let the worker thread move it to subs so that we don't
	// have to lock fileMutex on the GUI thread, as that can be locked for
	// extended periods of time with slow-rendering subtitles
	nextSubs.reset(copy);
	changedSubs.clear();
}

void ThreadedFrameSource::UpdateSubtitles(const AssFile *subs, std::set<const AssEntry*> changes) throw() {
	std::deque<std::pair<size_t, std::unique_ptr<AssEntry>>> changed;
	size_t i = 0;
	for (auto const& e : subs->Line) {
		if (changes.count(&e))
			changed.emplace_back(i, std::unique_ptr<AssEntry>(e.Clone()));
		++i;
	}

	wxMutexLocker locker(jobMutex);
	changedSubs = std::move(changed);
}

void ThreadedFrameSource::RequestFrame(int frame, double time) throw() {
	wxMutexLocker locker(jobMutex);
	nextTime  = time;
	nextFrame = frame;
	jobReady.Signal();
}

std::shared_ptr<AegiVideoFrame> ThreadedFrameSource::GetFrame(int frame, double time, bool raw) {
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
: agi::Exception(from_wx(err), nullptr)
{
	SetEventType(EVT_SUBTITLES_ERROR);
}
