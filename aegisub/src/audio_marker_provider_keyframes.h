// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file audio_marker_provider_keyframes.h
/// @see audio_marker_provider_keyframes.cpp
/// @ingroup audio_ui
///

#include "audio_controller.h"

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>

#ifndef AGI_PRE
#include <vector>
#endif

class AudioMarkerKeyframe;
class Pen;
class VideoContext;
namespace agi {
	class OptionValue;
	struct Context;
}

/// Marker provider for video keyframes
class AudioMarkerProviderKeyframes : public AudioMarkerProvider {
	/// Audio controller for time -> sample conversions
	AudioController *controller;
	/// Video controller to get keyframes from
	VideoContext *vc;

	agi::signal::Connection keyframe_slot;
	agi::signal::Connection audio_open_slot;
	agi::signal::Connection timecode_slot;
	agi::signal::Connection enabled_slot;
	const agi::OptionValue *enabled_opt;

	/// Current set of markers for the keyframes
	std::vector<AudioMarkerKeyframe> markers;

	/// Pen used for all keyframe markers, stored here for performance reasons
	agi::scoped_ptr<Pen> style;

	/// Regenerate the list of markers
	void Update();

public:
	/// Constructor
	/// @param c Project context; must have audio and video controllers initialized
	/// @param opt_name Name of the option to use to decide whether or not this provider is enabled
	AudioMarkerProviderKeyframes(agi::Context *c, const char *opt_name);
	/// Explicit destructor needed due to members with incomplete types
	~AudioMarkerProviderKeyframes();

	/// Get all keyframe markers within a range
	/// @param range Range of samples to get markers for
	/// @param[out] out Vector to fill with markers in the range
	void GetMarkers(SampleRange const& range, AudioMarkerVector &out) const;
};
