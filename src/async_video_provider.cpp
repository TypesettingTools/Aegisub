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

#include "async_video_provider.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "export_fixstyle.h"
#include "include/aegisub/subtitles_provider.h"
#include "video_provider_manager.h"

#include <libaegisub/dispatch.h>

enum {
	NEW_SUBS_FILE = -1,
	SUBS_FILE_ALREADY_LOADED = -2
};

std::shared_ptr<VideoFrame> AsyncVideoProvider::ProcFrame(int frame_number, double time, bool raw) {
	std::shared_ptr<VideoFrame> frame;

	try {
		frame = source_provider->GetFrame(frame_number);
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
				subs_provider->LoadSubtitles(subs.get(), time);
			}
		}
	}
	catch (agi::Exception const& err) { throw SubtitlesProviderErrorEvent(err.GetMessage()); }

	try {
		subs_provider->DrawSubtitles(*frame, time / 1000.);
	}
	catch (agi::UserCancelException const&) { }

	return frame;
}

static std::unique_ptr<SubtitlesProvider> get_subs_provider(wxEvtHandler *evt_handler, agi::BackgroundRunner *br) {
	try {
		return SubtitlesProviderFactory::GetProvider(br);
	}
	catch (agi::Exception const& err) {
		evt_handler->AddPendingEvent(SubtitlesProviderErrorEvent(err.GetMessage()));
		return nullptr;
	}
}

AsyncVideoProvider::AsyncVideoProvider(agi::fs::path const& video_filename, std::string const& colormatrix, wxEvtHandler *parent, agi::BackgroundRunner *br)
: worker(agi::dispatch::Create())
, subs_provider(get_subs_provider(parent, br))
, source_provider(VideoProviderFactory::GetProvider(video_filename, colormatrix, br))
, parent(parent)
{
}

AsyncVideoProvider::~AsyncVideoProvider() {
	// Block until all currently queued jobs are complete
	worker->Sync([]{});
}

void AsyncVideoProvider::LoadSubtitles(const AssFile *new_subs) throw() {
	uint_fast32_t req_version = ++version;

	auto copy = new AssFile(*new_subs);
	worker->Async([=]{
		subs.reset(copy);
		single_frame = NEW_SUBS_FILE;
		ProcAsync(req_version);
	});
}

void AsyncVideoProvider::UpdateSubtitles(const AssFile *new_subs, std::set<const AssDialogue*> const& changes) throw() {
	uint_fast32_t req_version = ++version;

	// Copy just the lines which were changed, then replace the lines at the
	// same indices in the worker's copy of the file with the new entries
	std::vector<AssDialogue *> changed;
	for (auto d : changes)
		changed.push_back(new AssDialogue(*d));

	worker->Async([=]{
		int i = 0;
		auto it = subs->Events.begin();
		for (auto& update : changed) {
			std::advance(it, update->Row - i);
			i = update->Row;
			subs->Events.insert(it, *update);
			delete &*it--;
		}

		single_frame = NEW_SUBS_FILE;
		ProcAsync(req_version);
	});
}

void AsyncVideoProvider::RequestFrame(int new_frame, double new_time) throw() {
	uint_fast32_t req_version = ++version;

	worker->Async([=]{
		time = new_time;
		frame_number = new_frame;
		ProcAsync(req_version);
	});
}

bool AsyncVideoProvider::NeedUpdate(std::vector<AssDialogueBase const*> const& visible_lines) {
	// Always need to render after a seek
	if (single_frame != NEW_SUBS_FILE || frame_number != last_rendered)
		return true;

	// Obviously need to render if the number of visible lines has changed
	if (visible_lines.size() != last_lines.size())
		return true;

	for (size_t i = 0; i < last_lines.size(); ++i) {
		auto const& last = last_lines[i];
		auto const& cur = *visible_lines[i];
		if (last.Layer  != cur.Layer)  return true;
		if (last.Margin != cur.Margin) return true;
		if (last.Style  != cur.Style)  return true;
		if (last.Effect != cur.Effect) return true;
		if (last.Text   != cur.Text)   return true;

		// Changing the start/end time effects the appearance only if the
		// line is animated. This is obviously not a very accurate check for
		// animated lines, but false positives aren't the end of the world
		if ((last.Start != cur.Start || last.End != cur.End) &&
			(!cur.Effect.get().empty() || cur.Text.get().find('\\') != std::string::npos))
			return true;
	}

	return false;
}

void AsyncVideoProvider::ProcAsync(uint_fast32_t req_version) {
	// Only actually produce the frame if there's no queued changes waiting
	if (req_version < version || frame_number < 0) return;

	std::vector<AssDialogueBase const*> visible_lines;
	for (auto const& line : subs->Events) {
		if (!line.Comment && !(line.Start > time || line.End <= time))
			visible_lines.push_back(&line);
	}

	if (!NeedUpdate(visible_lines)) return;

	last_lines.clear();
	last_lines.reserve(visible_lines.size());
	for (auto line : visible_lines)
		last_lines.push_back(*line);
	last_rendered = frame_number;

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

std::shared_ptr<VideoFrame> AsyncVideoProvider::GetFrame(int frame, double time, bool raw) {
	std::shared_ptr<VideoFrame> ret;
	worker->Sync([&]{ ret = ProcFrame(frame, time, raw); });
	return ret;
}

void AsyncVideoProvider::SetColorSpace(std::string const& matrix) {
	worker->Async([=] { source_provider->SetColorSpace(matrix); });
}

wxDEFINE_EVENT(EVT_FRAME_READY, FrameReadyEvent);
wxDEFINE_EVENT(EVT_VIDEO_ERROR, VideoProviderErrorEvent);
wxDEFINE_EVENT(EVT_SUBTITLES_ERROR, SubtitlesProviderErrorEvent);

VideoProviderErrorEvent::VideoProviderErrorEvent(VideoProviderError const& err)
: agi::Exception(err.GetMessage())
{
	SetEventType(EVT_VIDEO_ERROR);
}
SubtitlesProviderErrorEvent::SubtitlesProviderErrorEvent(std::string const& err)
: agi::Exception(err)
{
	SetEventType(EVT_SUBTITLES_ERROR);
}
