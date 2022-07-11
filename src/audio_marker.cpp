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

/// @file audio_marker.cpp
/// @see audio_marker.h
/// @ingroup audio_ui
///

#include "audio_marker.h"

#include "include/aegisub/context.h"
#include "options.h"
#include "pen.h"
#include "project.h"
#include "video_controller.h"

#include <libaegisub/make_unique.h>

#include <algorithm>

class AudioMarkerKeyframe final : public AudioMarker {
	Pen *style;
	int position;
public:
	AudioMarkerKeyframe(Pen *style, int position) : style(style), position(position) { }
	int GetPosition() const override { return position; }
	FeetStyle GetFeet() const override { return Feet_None; }
	wxPen GetStyle() const override { return *style; }
	operator int() const { return position; }
};

AudioMarkerProviderKeyframes::AudioMarkerProviderKeyframes(agi::Context *c, const char *opt_name)
: p(c->project.get())
, keyframe_slot(p->AddKeyframesListener(&AudioMarkerProviderKeyframes::Update, this))
, timecode_slot(p->AddTimecodesListener(&AudioMarkerProviderKeyframes::Update, this))
, enabled_slot(OPT_SUB(opt_name, &AudioMarkerProviderKeyframes::Update, this))
, enabled_opt(OPT_GET(opt_name))
, style(agi::make_unique<Pen>("Colour/Audio Display/Keyframe"))
{
	Update();
}

AudioMarkerProviderKeyframes::~AudioMarkerProviderKeyframes() { }

void AudioMarkerProviderKeyframes::Update() {
	auto const& keyframes = p->Keyframes();
	auto const& timecodes = p->Timecodes();

	if (keyframes.empty() || !timecodes.IsLoaded() || !enabled_opt->GetBool()) {
		if (!markers.empty()) {
			markers.clear();
			AnnounceMarkerMoved();
		}
		return;
	}

	markers.clear();
	markers.reserve(keyframes.size());
	for (int frame : keyframes)
		markers.emplace_back(style.get(), timecodes.TimeAtFrame(frame, agi::vfr::START));
	AnnounceMarkerMoved();
}

void AudioMarkerProviderKeyframes::GetMarkers(TimeRange const& range, AudioMarkerVector &out) const {
	// Find first and last keyframes inside the range
	auto a = lower_bound(markers.begin(), markers.end(), range.begin());
	auto b = upper_bound(markers.begin(), markers.end(), range.end());

	// Place pointers to the markers in the output vector
	for (; a != b; ++a)
		out.push_back(&*a);
}

class VideoPositionMarker final : public AudioMarker {
	Pen style{"Colour/Audio Display/Play Cursor"};
	int position = -1;

public:
	void SetPosition(int new_pos) { position = new_pos; }

	int GetPosition() const override { return position; }
	FeetStyle GetFeet() const override { return Feet_None; }
	wxPen GetStyle() const override { return style; }
	operator int() const { return position; }
};

VideoPositionMarkerProvider::VideoPositionMarkerProvider(agi::Context *c)
: vc(c->videoController.get())
, video_seek_slot(vc->AddSeekListener(&VideoPositionMarkerProvider::Update, this))
, enable_opt_changed_slot(OPT_SUB("Audio/Display/Draw/Video Position", &VideoPositionMarkerProvider::OptChanged, this))
{
	OptChanged(*OPT_GET("Audio/Display/Draw/Video Position"));
}

VideoPositionMarkerProvider::~VideoPositionMarkerProvider() { }

void VideoPositionMarkerProvider::Update(int frame_number) {
	marker->SetPosition(vc->TimeAtFrame(frame_number));
	AnnounceMarkerMoved();
}

void VideoPositionMarkerProvider::OptChanged(agi::OptionValue const& opt) {
	if (opt.GetBool()) {
		video_seek_slot.Unblock();
		marker = agi::make_unique<VideoPositionMarker>();
		marker->SetPosition(vc->GetFrameN());
	}
	else {
		video_seek_slot.Block();
		marker.reset();
	}
}

void VideoPositionMarkerProvider::GetMarkers(const TimeRange &range, AudioMarkerVector &out) const {
	if (marker && range.contains(*marker))
		out.push_back(marker.get());
}

SecondsMarkerProvider::SecondsMarkerProvider()
: pen(agi::make_unique<Pen>("Colour/Audio Display/Seconds Line", 1, wxPENSTYLE_DOT))
, enabled(OPT_GET("Audio/Display/Draw/Seconds"))
, enabled_opt_changed(OPT_SUB("Audio/Display/Draw/Seconds", &SecondsMarkerProvider::EnabledOptChanged, this))
{
}

void SecondsMarkerProvider::EnabledOptChanged() {
	AnnounceMarkerMoved();
}

void SecondsMarkerProvider::GetMarkers(TimeRange const& range, AudioMarkerVector &out) const {
	if (!enabled->GetBool()) return;

	if ((range.length() + 999) / 1000 > (int)markers.size())
		markers.resize((range.length() + 999) / 1000, Marker(pen.get()));

	size_t i = 0;
	for (int time = ((range.begin() + 999) / 1000) * 1000; time < range.end(); time += 1000) {
		markers[i].position = time;
		out.push_back(&markers[i++]);
	}
}

wxPen SecondsMarkerProvider::Marker::GetStyle() const {
	return *style;
}
