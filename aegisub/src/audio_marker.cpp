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
//
// $Id$

/// @file audio_marker.cpp
/// @see audio_marker.h
/// @ingroup audio_ui
///

#include "config.h"

#include "audio_marker.h"

#include "include/aegisub/context.h"
#include "main.h"
#include "pen.h"
#include "video_context.h"

#ifndef AGI_PRE
#include <algorithm>
#endif

class AudioMarkerKeyframe : public AudioMarker {
	Pen *style;
	int position;
public:
	AudioMarkerKeyframe(Pen *style, int position) : style(style), position(position) { }
	int GetPosition() const { return position; }
	FeetStyle GetFeet() const { return Feet_None; }
	bool CanSnap() const { return true; }
	wxPen GetStyle() const { return *style; }
	operator int() const { return position; }
};

AudioMarkerProviderKeyframes::AudioMarkerProviderKeyframes(agi::Context *c, const char *opt_name)
: vc(c->videoController)
, keyframe_slot(vc->AddKeyframesListener(&AudioMarkerProviderKeyframes::Update, this))
, timecode_slot(vc->AddTimecodesListener(&AudioMarkerProviderKeyframes::Update, this))
, enabled_slot(OPT_SUB(opt_name, &AudioMarkerProviderKeyframes::Update, this))
, enabled_opt(OPT_GET(opt_name))
, style(new Pen("Colour/Audio Display/Keyframe"))
{
	Update();
}

AudioMarkerProviderKeyframes::~AudioMarkerProviderKeyframes() { }

void AudioMarkerProviderKeyframes::Update() {
	std::vector<int> const& keyframes = vc->GetKeyFrames();
	agi::vfr::Framerate const& timecodes = vc->FPS();

	if (keyframes.empty() || !timecodes.IsLoaded() || !enabled_opt->GetBool()) {
		if (!markers.empty()) {
			markers.clear();
			AnnounceMarkerMoved();
		}
		return;
	}

	markers.clear();
	markers.reserve(keyframes.size());
	for (size_t i = 0; i < keyframes.size(); ++i) {
		markers.push_back(AudioMarkerKeyframe(style.get(), timecodes.TimeAtFrame(keyframes[i])));
	}
	AnnounceMarkerMoved();
}

void AudioMarkerProviderKeyframes::GetMarkers(TimeRange const& range, AudioMarkerVector &out) const {
	// Find first and last keyframes inside the range
	std::vector<AudioMarkerKeyframe>::const_iterator
		a = lower_bound(markers.begin(), markers.end(), range.begin()),
		b = upper_bound(markers.begin(), markers.end(), range.end());

	// Place pointers to the markers in the output vector
	for (; a != b; ++a)
		out.push_back(&*a);
}

class VideoPositionMarker : public AudioMarker {
	Pen style;
	int position;

public:
	VideoPositionMarker()
	: style("Colour/Audio Display/Play Cursor")
	, position(-1)
	{
	}

	void SetPosition(int new_pos) { position = new_pos; }

	int GetPosition() const { return position; }
	FeetStyle GetFeet() const { return Feet_None; }
	bool CanSnap() const { return true; }
	wxPen GetStyle() const { return style; }
	operator int() const { return position; }
};

VideoPositionMarkerProvider::VideoPositionMarkerProvider(agi::Context *c)
: vc(c->videoController)
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
		marker.reset(new VideoPositionMarker);
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
