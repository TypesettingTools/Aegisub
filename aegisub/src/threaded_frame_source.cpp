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

/// @file threaded_frame_source.cpp
/// @see threaded_frame_source.h
/// @ingroup video
///

#include "threaded_frame_source.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "export_fixstyle.h"
#include "include/aegisub/subtitles_provider.h"
#include "video_frame.h"
#include "video_provider_manager.h"

#include <libaegisub/address_of_adaptor.h>
#include <libaegisub/dispatch.h>

#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>

enum {
	NEW_SUBS_FILE = -1,
	SUBS_FILE_ALREADY_LOADED = -2
};

std::shared_ptr<VideoFrame> ThreadedFrameSource::ProcFrame(int frame_number, double time, bool raw) {
	std::shared_ptr<VideoFrame> frame;

	try {
		frame = video_provider->GetFrame(frame_number);
	}
	catch (VideoProviderError const& err) { throw VideoProviderErrorEvent(err); }

	if (raw || !subs_provider || !subs) return frame;

	try {
		if (single_frame != frame_number && single_frame != SUBS_FILE_ALREADY_LOADED) {
			// Generally edits and seeks come in groups; if the last thing done
			// was seek it is more likely that the user will seek again and
			// vice versa. As such, if this is the first frame requested after
			// an edit, only export the currently visible lines (because the
			// other lines will probably not be viewed before the file changes
			// again), and if it's a different frame, export the entire file.
			if (single_frame != NEW_SUBS_FILE) {
				subs_provider->LoadSubtitles(subs.get());
				single_frame = SUBS_FILE_ALREADY_LOADED;
			}
			else {
				AssFixStylesFilter().ProcessSubs(subs.get(), nullptr);

				single_frame = frame_number;
				// Copying a nontrivially sized AssFile is fairly slow, so
				// instead muck around with its innards to just temporarily
				// remove the non-visible lines without deleting them
				std::deque<AssDialogue*> full;
				boost::push_back(full, subs->Events | agi::address_of);
				subs->Events.remove_if([=](AssDialogue const& diag) {
					return diag.Start > time || diag.End <= time;
				});

				try {
					subs_provider->LoadSubtitles(subs.get());

					subs->Events.clear();
					boost::push_back(subs->Events, full | boost::adaptors::indirected);
				}
				catch (...) {
					subs->Events.clear();
					boost::push_back(subs->Events, full | boost::adaptors::indirected);
					throw;
				}
			}
		}
	}
	catch (std::string const& err) { throw SubtitlesProviderErrorEvent(err); }

	subs_provider->DrawSubtitles(*frame, time / 1000.);

	return frame;
}

static std::unique_ptr<SubtitlesProvider> get_subs_provider(wxEvtHandler *parent) {
	try {
		return SubtitlesProviderFactory::GetProvider();
	}
	catch (std::string const& err) {
		parent->AddPendingEvent(SubtitlesProviderErrorEvent(err));
		return nullptr;
	}
}

ThreadedFrameSource::ThreadedFrameSource(agi::fs::path const& video_filename, std::string const& colormatrix, wxEvtHandler *parent)
: worker(agi::dispatch::Create())
, subs_provider(get_subs_provider(parent))
, video_provider(VideoProviderFactory::GetProvider(video_filename, colormatrix))
, parent(parent)
{
}

ThreadedFrameSource::~ThreadedFrameSource() {
	// Block until all currently queued jobs are complete
	worker->Sync([]{});
}

void ThreadedFrameSource::LoadSubtitles(const AssFile *new_subs) throw() {
	uint_fast32_t req_version = ++version;

	auto copy = new AssFile(*new_subs);
	worker->Async([=]{
		subs.reset(copy);
		single_frame = NEW_SUBS_FILE;
		ProcAsync(req_version);
	});
}

void ThreadedFrameSource::UpdateSubtitles(const AssFile *new_subs, std::set<const AssDialogue*> const& changes) throw() {
	uint_fast32_t req_version = ++version;

	// Copy just the lines which were changed, then replace the lines at the
	// same indices in the worker's copy of the file with the new entries
	std::deque<std::pair<size_t, AssDialogue*>> changed;
	size_t i = 0;
	for (auto const& e : new_subs->Events) {
		if (changes.count(&e))
			changed.emplace_back(i, new AssDialogue(e));
		++i;
	}

	worker->Async([=]{
		size_t i = 0;
		auto it = subs->Events.begin();
		for (auto& update : changed) {
			advance(it, update.first - i);
			i = update.first;
			subs->Events.insert(it, *update.second);
			delete &*it--;
		}

		single_frame = NEW_SUBS_FILE;
		ProcAsync(req_version);
	});
}

void ThreadedFrameSource::RequestFrame(int new_frame, double new_time) throw() {
	uint_fast32_t req_version = ++version;

	worker->Async([=]{
		time = new_time;
		frame_number = new_frame;
		ProcAsync(req_version);
	});
}

void ThreadedFrameSource::ProcAsync(uint_fast32_t req_version) {
	// Only actually produce the frame if there's no queued changes waiting
	if (req_version < version || frame_number < 0) return;

	try {
		FrameReadyEvent *evt = new FrameReadyEvent(ProcFrame(frame_number, time), time);
		evt->SetEventType(EVT_FRAME_READY);
		parent->QueueEvent(evt);
	}
	catch (wxEvent const& err) {
		// Pass error back to parent thread
		parent->QueueEvent(err.Clone());
	}
}

std::shared_ptr<VideoFrame> ThreadedFrameSource::GetFrame(int frame, double time, bool raw) {
	std::shared_ptr<VideoFrame> ret;
	worker->Sync([&]{
		ret = ProcFrame(frame, time, raw);
	});
	return ret;
}

wxDEFINE_EVENT(EVT_FRAME_READY, FrameReadyEvent);
wxDEFINE_EVENT(EVT_VIDEO_ERROR, VideoProviderErrorEvent);
wxDEFINE_EVENT(EVT_SUBTITLES_ERROR, SubtitlesProviderErrorEvent);

VideoProviderErrorEvent::VideoProviderErrorEvent(VideoProviderError const& err)
: agi::Exception(err.GetMessage(), &err)
{
	SetEventType(EVT_VIDEO_ERROR);
}
SubtitlesProviderErrorEvent::SubtitlesProviderErrorEvent(std::string const& err)
: agi::Exception(err, nullptr)
{
	SetEventType(EVT_SUBTITLES_ERROR);
}
