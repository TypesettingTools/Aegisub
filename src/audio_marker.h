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

#pragma once

#include <libaegisub/signal.h>

#include <memory>
#include <vector>

#include <wx/string.h>

class AudioMarkerKeyframe;
class Pen;
class Project;
class VideoController;
class TimeRange;
class VideoPositionMarker;
class wxPen;

namespace agi {
	class OptionValue;
	struct Context;
}

#include "time_range.h"

/// @class AudioMarker
/// @brief A marker on the audio display
class AudioMarker {
public:
	/// Describe which directions a marker has feet in
	enum FeetStyle {
		Feet_None = 0,
		Feet_Left,
		Feet_Right,
		Feet_Both // Conveniently Feet_Left|Feet_Right
	};

	/// @brief Get the marker's position
	/// @return The marker's position in milliseconds
	virtual int GetPosition() const = 0;

	/// @brief Get the marker's drawing style
	/// @return A pen object describing the marker's drawing style
	virtual wxPen GetStyle() const = 0;

	/// @brief Get the marker's feet style
	/// @return The marker's feet style
	virtual FeetStyle GetFeet() const = 0;
};

typedef std::vector<const AudioMarker*> AudioMarkerVector;

/// Abstract interface for audio marker providers
class AudioMarkerProvider {
protected:
	/// One or more of the markers provided by this object have changed
	agi::signal::Signal<> AnnounceMarkerMoved;
public:
	/// Virtual destructor, does nothing
	virtual ~AudioMarkerProvider() = default;

	/// @brief Return markers in a time range
	virtual void GetMarkers(const TimeRange &range, AudioMarkerVector &out) const = 0;

	DEFINE_SIGNAL_ADDERS(AnnounceMarkerMoved, AddMarkerMovedListener)
};

/// @class AudioLabelProvider
/// @brief Abstract interface for audio label providers
class AudioLabelProvider {
protected:
	/// One or more of the labels provided by this object have changed
	agi::signal::Signal<> AnnounceLabelChanged;
public:
	/// A label for a range of time on the audio display
	struct AudioLabel {
		/// Text of the label
		wxString text;
		/// Range which this label applies to
		TimeRange range;
	};

	/// Virtual destructor, does nothing
	virtual ~AudioLabelProvider() = default;

	/// @brief Get labels in a time range
	/// @param range Range of times to get labels for
	/// @param[out] out Vector which should be filled with the labels
	virtual void GetLabels(TimeRange const& range, std::vector<AudioLabel> &out) const = 0;

	DEFINE_SIGNAL_ADDERS(AnnounceLabelChanged, AddLabelChangedListener)
};


/// Marker provider for video keyframes
class AudioMarkerProviderKeyframes final : public AudioMarkerProvider {
	/// Project to get keyframes from
	Project *p;

	agi::signal::Connection keyframe_slot;
	agi::signal::Connection timecode_slot;
	agi::signal::Connection enabled_slot;
	const agi::OptionValue *enabled_opt;

	/// Current set of markers for the keyframes
	std::vector<AudioMarkerKeyframe> markers;

	/// Pen used for all keyframe markers, stored here for performance reasons
	std::unique_ptr<Pen> style;

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
	/// @param range Time range to get markers for
	/// @param[out] out Vector to fill with markers in the range
	void GetMarkers(TimeRange const& range, AudioMarkerVector &out) const override;
};

/// Marker provider for the current video playback position
class VideoPositionMarkerProvider final : public AudioMarkerProvider {
	VideoController *vc;

	std::unique_ptr<VideoPositionMarker> marker;

	agi::signal::Connection video_seek_slot;
	agi::signal::Connection enable_opt_changed_slot;

	void Update(int frame_number);
	void OptChanged(agi::OptionValue const& opt);

public:
	VideoPositionMarkerProvider(agi::Context *c);
	~VideoPositionMarkerProvider();

	void GetMarkers(const TimeRange &range, AudioMarkerVector &out) const override;
};

/// Marker provider for lines every second
class SecondsMarkerProvider final : public AudioMarkerProvider {
	struct Marker final : public AudioMarker {
		Pen *style;
		int position = 0;

		Marker(Pen *style) : style(style) { }
		int GetPosition() const override { return position; }
		FeetStyle GetFeet() const override { return Feet_None; }
		wxPen GetStyle() const override;
		operator int() const { return position; }
	};

	/// Pen used by all seconds markers, here for performance
	std::unique_ptr<Pen> pen;

	/// Markers returned from last call to GetMarkers
	mutable std::vector<Marker> markers;

	/// Cached reference to the option to enable/disable seconds markers
	const agi::OptionValue *enabled;

	/// Enabled option change connection
	agi::signal::Connection enabled_opt_changed;

	void EnabledOptChanged();

public:
	SecondsMarkerProvider();
	void GetMarkers(TimeRange const& range, AudioMarkerVector &out) const override;
};
