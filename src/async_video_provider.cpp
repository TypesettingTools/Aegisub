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
#include "video_frame.h"
#include "video_provider_manager.h"

#include <libaegisub/dispatch.h>

#include <algorithm>
#include <boost/gil.hpp>

enum {
	NEW_SUBS_FILE = -1,
	SUBS_FILE_ALREADY_LOADED = -2
};

std::shared_ptr<VideoFrame> AsyncVideoProvider::ProcFrame(int frame_number, double time, bool raw) {
	// Find an unused buffer to use or allocate a new one if needed
	auto& pool = raw ? raw_buffers : buffers;
	std::shared_ptr<VideoFrame> frame;
	for (auto& buffer : pool) {
		if (buffer.use_count() == 1) {
			frame = buffer;
			break;
		}
	}

	if (!frame) {
		frame = std::make_shared<VideoFrame>();
		if (!raw && subs_provider)
			frame->data.SetAllocator(subs_provider->GetFrameAllocator());
		pool.push_back(frame);
	}

	try {
		source_provider->GetFrame(frame_number, *frame);
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
				AssFixStylesFilter::ProcessSubs(subs.get());
				single_frame = frame_number;
				subs_provider->LoadSubtitles(subs.get(), time);
			}
		}
	}
	catch (agi::Exception const& err) {
		// Still show the frame, just without subtitles
		parent->QueueEvent(new SubtitlesProviderErrorEvent(err.GetMessage()));
		return frame;
	}

	try {
		subs_provider->DrawSubtitles(*frame, time / 1000.);
	}
	catch (agi::UserCancelException const&) { }
	catch (agi::Exception const& err) {
		// Still show the frame, just without subtitles
		parent->QueueEvent(new SubtitlesProviderErrorEvent(err.GetMessage()));
	}

	return frame;
}

VideoFrame AsyncVideoProvider::GetBlankFrame(bool white) {
	VideoFrame result;
	result.width = GetWidth();
	result.height = GetHeight();
	result.pitch = result.width * 4;
	result.flipped = false;
	result.data.resize(result.pitch * result.height, white ? 255 : 0);
	return result;
}

VideoFrame AsyncVideoProvider::GetSubtitles(double time) {
	// Run on the worker so that this can't interleave with loading a new
	// file, which would let a sandboxed renderer see both files
	if (subs_provider)
		subs_provider->WaitUntilReady();
	VideoFrame ret;
	worker->Sync([&]{ ret = ProcSubtitles(time); });
	return ret;
}

VideoFrame AsyncVideoProvider::ProcSubtitles(double time) {
	// We are looking for an RGBA image for which blending that image is equivalent
	// to blending the current frame's subtitles (however the subtitle provider does this,
	// usually by blending many separate small images with alpha).
	// To do this, draw the subtitles once on a black frame and once on a white frame
	// and solve for the color and alpha.
	VideoFrame frame_black = GetBlankFrame(false);
	if (!subs || !subs_provider) return frame_black;
	VideoFrame frame_white = GetBlankFrame(true);

	try {
		subs_provider->LoadSubtitles(subs.get());
		subs_provider->DrawSubtitles(frame_black, time / 1000.);
		subs_provider->DrawSubtitles(frame_white, time / 1000.);
	}
	catch (agi::UserCancelException const&) { throw; }
	catch (agi::Exception const& err) {
		parent->QueueEvent(new SubtitlesProviderErrorEvent(err.GetMessage()));
		// Fully transparent
		return GetBlankFrame(false);
	}

	using namespace boost::gil;
	auto blackview = interleaved_view(frame_black.width, frame_black.height, (bgra8_pixel_t*) frame_black.data.data(), frame_black.width * 4);
	auto whiteview = interleaved_view(frame_white.width, frame_white.height, (bgra8_pixel_t*) frame_white.data.data(), frame_white.width * 4);

	transform_pixels(blackview, whiteview, blackview, [=](const bgra8_pixel_t black, const bgra8_pixel_t white) -> bgra8_pixel_t {
		// The renderer may not be trustworthy, so the frames may not have
		// been rendered consistently and these may be out of range
		int a = std::clamp(255 - (white[0] - black[0]), 0, 255);

		bgra8_pixel_t ret;
		if (a == 0) {
			ret[0] = 0;
			ret[1] = 0;
			ret[2] = 0;
			ret[3] = 0;
		} else {
			ret[0] = std::min(255., black[0] / (a / 255.));
			ret[1] = std::min(255., black[1] / (a / 255.));
			ret[2] = std::min(255., black[2] / (a / 255.));
			ret[3] = a;
		}
		return ret;
	});

	return frame_black;
}

std::pair<int, int> AsyncVideoProvider::GetDisplayResolution() const {
	int width = GetWidth();
	int height = GetHeight();
	double sar = double(width) / double(height);

	double dar = GetDAR();
	if (dar == 0)
		dar = sar;

	return std::make_pair(
		std::round(width * std::max(1., dar / sar)),
		std::round(height * std::max(1., sar / dar))
	);
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

AsyncVideoProvider::AsyncVideoProvider(agi::fs::path const& video_filename, agi::ycbcr::Header colormatrix, wxEvtHandler *parent, agi::BackgroundRunner *br)
: worker(agi::dispatch::Create())
, subs_provider(get_subs_provider(parent, br))
, source_provider(VideoProviderFactory::GetProvider(video_filename, colormatrix, br))
, parent(parent)
{
	// Subtitles are drawn on the worker thread, which can't wait for the
	// provider to be ready as it can't show progress. If the user cancels,
	// frames are drawn without subtitles until it's ready.
	if (subs_provider) {
		try {
			subs_provider->WaitUntilReady();
		}
		catch (agi::UserCancelException const&) { }
	}
}

AsyncVideoProvider::~AsyncVideoProvider() {
	// Block until all currently queued jobs are complete
	worker->Sync([]{});
}

void AsyncVideoProvider::LoadSubtitles(const AssFile *new_subs, bool new_document) throw() {
	uint_fast32_t req_version = ++version;

	auto copy = new AssFile(*new_subs);
	worker->Async([=, this]{
		if (new_document && subs_provider) {
			subs_provider->NewDocument();
			// The frames may be shared with the previous renderer, which could
			// have hidden data in them for the next one, such as after the end
			// of the frame where it isn't overwritten, so new ones are needed.
			// Ones which are still in use are freed once they're done with.
			buffers.clear();
		}
		subs.reset(copy);
		single_frame = NEW_SUBS_FILE;
		ProcAsync(req_version, false);
	});
}

void AsyncVideoProvider::UpdateSubtitles(const AssDialogue *changed) throw() {
	uint_fast32_t req_version = ++version;

	// Copy just the line which were changed, then replace the line at the
	// same index in the worker's copy of the file with the new entry
	auto copy = new AssDialogue(*changed);
	worker->Async([=, this]{
		int i = 0;
		auto it = subs->Events.begin();
		std::advance(it, copy->Row - i);
		i = copy->Row;
		subs->Events.insert(it, *copy);
		delete &*it--;

		single_frame = NEW_SUBS_FILE;
		ProcAsync(req_version, true);
	});
}

void AsyncVideoProvider::RequestFrame(int new_frame, double new_time) throw() {
	uint_fast32_t req_version = ++version;

	worker->Async([=, this]{
		time = new_time;
		frame_number = new_frame;
		ProcAsync(req_version, false);
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

void AsyncVideoProvider::ProcAsync(uint_fast32_t req_version, bool check_updated) {
	// Only actually produce the frame if there's no queued changes waiting
	if (req_version < version || frame_number < 0) return;

	std::vector<AssDialogueBase const*> visible_lines;
	for (auto const& line : subs->Events) {
		if (!line.Comment && !(line.Start > time || line.End <= time))
			visible_lines.push_back(&line);
	}

	if (check_updated && !NeedUpdate(visible_lines)) return;

	last_lines.clear();
	last_lines.reserve(visible_lines.size());
	for (auto line : visible_lines)
		last_lines.push_back(*line);
	last_rendered = frame_number;

	try {
		auto evt = new FrameReadyEvent(ProcFrame(frame_number, time), time);
		evt->SetEventType(EVT_FRAME_READY);
		parent->QueueEvent(evt);
	}
	catch (wxEvent const& err) {
		// Pass error back to parent thread
		parent->QueueEvent(err.Clone());
	}
}

std::shared_ptr<VideoFrame> AsyncVideoProvider::GetFrame(int frame, double time, bool raw) {
	if (!raw && subs_provider) {
		try {
			subs_provider->WaitUntilReady();
		}
		catch (agi::UserCancelException const&) {
			// As when cancelling while drawing, return the frame without subtitles
			raw = true;
		}
	}
	std::shared_ptr<VideoFrame> ret;
	worker->Sync([&]{ ret = ProcFrame(frame, time, raw); });
	return ret;
}

void AsyncVideoProvider::SetColorSpace(agi::ycbcr::Header matrix) {
	worker->Async([this, matrix]() {
		source_provider->SetColorSpace(matrix);
	});
}

wxDEFINE_EVENT(EVT_FRAME_READY, FrameReadyEvent);
wxDEFINE_EVENT(EVT_VIDEO_ERROR, VideoProviderErrorEvent);
wxDEFINE_EVENT(EVT_SUBTITLES_ERROR, SubtitlesProviderErrorEvent);

VideoProviderErrorEvent::VideoProviderErrorEvent(VideoProviderError const& err)
: agi::Exception(std::string(err.GetMessage()))
{
	SetEventType(EVT_VIDEO_ERROR);
}
SubtitlesProviderErrorEvent::SubtitlesProviderErrorEvent(std::string const& err)
: agi::Exception(std::string(err))
{
	SetEventType(EVT_SUBTITLES_ERROR);
}
