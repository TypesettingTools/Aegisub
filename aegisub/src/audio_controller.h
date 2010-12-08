// Copyright (c) 2009-2010, Niels Martin Hansen
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

/// @file audio_controller.h
/// @see audio_controller.cpp
/// @ingroup audio_ui


#ifndef AGI_PRE
#include <memory>
#include <vector>
#include <set>
#include <stdint.h>
#include <assert.h>
#include <wx/event.h>
#include <wx/string.h>
#include <wx/timer.h>
#include <wx/pen.h>
#include <wx/power.h>
#endif

#include <libaegisub/exception.h>

#define AGI_AUDIO_CONTROLLER_INCLUDED 1


class AudioPlayer;
class AudioProvider;

// Declared below
class AudioControllerAudioEventListener;
class AudioControllerTimingEventListener;
class AudioTimingController;
class AudioMarker;
class AudioMarkerProvider;


typedef std::vector<const AudioMarker*> AudioMarkerVector;


/// @class AudioController
/// @brief Manage an open audio stream and UI state for it
///
/// Keeps track of the UI interaction state of the open audio for a project, ie. what the current
/// selection is, what moveable markers are on the audio, and any secondary non-moveable markers
/// that are present.
///
/// Changes in interaction are broadcast to all managed audio displays so they can redraw, and
/// the audio displays report all interactions back to the controller. There is a one to many
/// relationship between controller and audio displays. There is at most one audio controller
/// for an open subtitling project.
///
/// Creates and destroys audio providers and players. This behaviour should at some point be moved
/// to a separate class, as it adds too many responsibilities to this class, but at the time of
/// writing, it would extend the scope of reworking components too much.
///
/// There is not supposed to be a way to get direct access to the audio providers or players owned
/// by a controller. If some operation that isn't possible in the existing design is needed, the
/// controller should be extended in some way to allow it.
class AudioController : public wxEvtHandler {
public:

	/// @class SampleRange
	/// @brief Represents an immutable range of audio samples
	class SampleRange {
		int64_t _begin;
		int64_t _end;

	public:
		/// @brief Constructor
		/// @param begin Index of the first sample to include in the range
		/// @param end   Index of one past the last sample to include in the range
		SampleRange(int64_t begin, int64_t end)
			: _begin(begin)
			, _end(end)
		{
			assert(end >= begin);
		}

		/// @brief Copy constructor, optionally adjusting the range
		/// @param src          The range to duplicate
		/// @param begin_adjust Number of samples to add to the start of the range
		/// @param end_adjust   Number of samples to add to the end of the range
		SampleRange(const SampleRange &src, int64_t begin_adjust = 0, int64_t end_adjust = 0)
		{
			_begin = src._begin + begin_adjust;
			_end = src._end + end_adjust;
			assert(_end >= _begin);
		}

		/// Get the number of samples in the range
		int64_t length() const { return _end - _begin; }
		/// Get the index of the first sample in the range
		int64_t begin() const { return _begin; }
		/// Get the index of one past the last sample in the range
		int64_t end() const { return _end; }

		/// Determine whether the range contains a given sample index
		bool contains(int64_t sample) const { return sample >= begin() && sample < end(); }

		/// Determine whether there is an overlap between two ranges
		bool overlaps(const SampleRange &other) const
		{
			return other.contains(_begin)
				|| other.contains(_end)
				|| contains(other._begin)
				|| contains(other._end);
		}
	};


private:

	/// Listeners for audio-related events
	std::set<AudioControllerAudioEventListener *> audio_event_listeners;

	/// Listeners for timing-related events
	std::set<AudioControllerTimingEventListener *> timing_event_listeners;

	/// The audio output object
	AudioPlayer *player;

	/// The audio provider
	AudioProvider *provider;

	/// The current timing mode, if any; owned by the audio controller
	AudioTimingController *timing_controller;

	/// Provide keyframe data for audio displays
	std::auto_ptr<AudioMarkerProvider> keyframes_marker_provider;


	enum PlaybackMode {
		PM_NotPlaying,
		PM_Range,
		PM_PrimaryRange,
		PM_ToEnd
	};
	/// The current playback mode
	PlaybackMode playback_mode;


	/// Timer used for playback position updates
	wxTimer playback_timer;

	/// Event handler for the playback timer
	void OnPlaybackTimer(wxTimerEvent &event);


#ifdef wxHAS_POWER_EVENTS
	/// Handle computer going into suspend mode by stopping audio and closing device
	void OnComputerSuspending(wxPowerEvent &event);
	/// Handle computer resuming from suspend by re-opening the audio device
	void OnComputerResuming(wxPowerEvent &event);
#endif


public:

	/// @brief Constructor
	AudioController();

	/// @brief Destructor
	~AudioController();


	/// @brief Open an audio stream
	/// @param url URL of the stream to open
	///
	/// The URL can either be a plain filename (with no qualifiers) or one
	/// recognised by various providers.
	void OpenAudio(const wxString &url);

	/// @brief Closes the current audio stream
	void CloseAudio();

	/// @brief Determine whether audio is currently open
	/// @return True if an audio stream is open and can be played back
	bool IsAudioOpen() const;

	/// @brief Get the URL for the current open audio stream
	/// @return The URL for the audio stream
	///
	/// The returned URL can be passed into OpenAudio() later to open the same stream again.
	wxString GetAudioURL() const;


	/// @brief Add an audio event listener
	/// @param listener The listener to add
	void AddAudioListener(AudioControllerAudioEventListener *listener);

	/// @brief Remove an audio event listener
	/// @param listener The listener to remove
	void RemoveAudioListener(AudioControllerAudioEventListener *listener);

	/// @brief Add a timing event listener
	/// @param listener The listener to add
	void AddTimingListener(AudioControllerTimingEventListener *listener);

	/// @brief Remove a timing event listener
	/// @param listener The listener to remove
	void RemoveTimingListener(AudioControllerTimingEventListener *listener);


	/// @brief Start or restart audio playback, playing a range
	/// @param range The range of audio to play back
	///
	/// The end of the played back range may be requested changed, but is not changed
	/// automatically from any other operations.
	void PlayRange(const SampleRange &range);

	/// @brief Start or restart audio playback, playing the primary playback range
	///
	/// If the primary playback range is updated during playback, the end of the 
	/// active playback range will be updated to match the new selection. The playback
	/// end can not be changed in any other way.
	void PlayPrimaryRange();

	/// @brief Start or restart audio playback, playing from a point to the end of stream
	/// @param start_sample Index of the sample to start playback at
	///
	/// Playback to end cannot be converted to a range playback like range playback can,
	/// it will continue until the end is reached, it is stopped, or restarted.
	void PlayToEnd(int64_t start_sample);

	/// @brief Stop all audio playback
	void Stop();

	/// @brief Determine whether playback is ongoing
	/// @return True if audio is being played back
	bool IsPlaying();

	/// @brief Get the current playback position
	/// @return Approximate current sample index being heard by the user
	///
	/// Returns 0 if playback is stopped. The return value is only approximate.
	int64_t GetPlaybackPosition();

	/// @brief If playing, restart playback from the specified position
	/// @param new_position Sample index to restart playback from
	///
	/// This function can be used to re-synchronise audio playback to another source that
	/// might not be able to keep up with the full speed, such as video playback in high
	/// resolution or with complex subtitles.
	///
	/// This function only does something if audio is already playing.
	void ResyncPlaybackPosition(int64_t new_position);


	/// @brief Get the primary playback range
	/// @return An immutable SampleRange object
	SampleRange GetPrimaryPlaybackRange() const;

	/// @brief Get all static markers inside a range
	/// @param range   The sample range to retrieve markers for
	/// @param markers Vector to fill found markers into
	///
	/// The markers retrieved are static markers the user can't interact with.
	/// Markers for user interaction are obtained through the timing controller.
	void GetMarkers(const SampleRange &range, AudioMarkerVector &markers) const;


	/// @brief Get the playback audio volume
	/// @return The amplification factor for the audio
	double GetVolume() const;

	/// @brief Set the playback audio volume
	/// @param volume The new amplification factor for the audio
	void SetVolume(double volume);


	/// @brief Return the current audio provider
	/// @return A const pointer to the current audio provider
	const AudioProvider * GetAudioProvider() const { return provider; }


	/// @brief Return the current timing controller
	/// @return The current timing controller or 0
	AudioTimingController * GetTimingController() const { return timing_controller; }

	/// @brief Change the current timing controller
	/// @param new_mode The new timing controller or 0. This may be the same object as
	/// the current timing controller, to signal that the timing controller has changed
	/// the object being timed, eg. changed to a new dialogue line.
	void SetTimingController(AudioTimingController *new_controller);


	/// @brief Timing controller signals primary playback range changed
	/// @param timing_controller The timing controller sending this notification
	///
	/// Only timing controllers should call this function. This function must be called
	/// when the primary playback range is changed in the timing controller, usually
	/// as a result of user interaction.
	void OnTimingControllerUpdatedPrimaryRange(AudioTimingController *timing_controller);

	/// @brief Timing controller signals that the rendering style ranges have changed
	/// @param timing_controller The timing controller sending this notification
	///
	/// Only timing controllers should call this function. This function must be called
	/// when one or more rendering style ranges have changed in the timing controller.
	void OnTimingControllerUpdatedStyleRanges(AudioTimingController *timing_controller);

	/// @brief Timing controller signals that an audio marker has moved
	/// @param timing_controller The timing controller sending this notification
	/// @param marker            The marker that was moved
	///
	/// Only timing controllers should call this function. This function must be called
	/// when a marker owned by the timing controller has been updated in some way.
	void OnTimingControllerMarkerMoved(AudioTimingController *timing_controller, AudioMarker *marker);


	/// @brief Convert a count of audio samples to a time in milliseconds
	/// @param samples Sample count to convert
	/// @return The number of milliseconds equivalent to the sample-count, rounded down
	int64_t MillisecondsFromSamples(int64_t samples) const;

	/// @brief Convert a time in milliseconds to a count of audio samples
	/// @param ms Time in milliseconds to convert
	/// @return The index of the first sample that is wholly inside the millisecond
	int64_t SamplesFromMilliseconds(int64_t ms) const;
};



/// @class AudioControllerAudioEventListener
/// @brief Abstract interface for objects that want audio events
class AudioControllerAudioEventListener {
public:
	/// A new audio stream was opened (and any previously open was closed)
	virtual void OnAudioOpen(AudioProvider *) = 0;

	/// The current audio stream was closed
	virtual void OnAudioClose() = 0;

	/// Playback is in progress and ths current position was updated
	virtual void OnPlaybackPosition(int64_t sample_position) = 0;

	/// Playback has stopped
	virtual void OnPlaybackStop() = 0;
};


/// @class AudioControllerTimingEventListener
/// @brief Abstract interface for objects that want audio timing events
class AudioControllerTimingEventListener {
public:
	/// One or more moveable markers were moved
	virtual void OnMarkersMoved() = 0;

	/// The selection was changed
	virtual void OnSelectionChanged() = 0;

	/// The timing controller was replaced
	virtual void OnTimingControllerChanged() = 0;
};



/// @class AudioMarkerProvider
/// @brief Abstract interface for audio marker providers
class AudioMarkerProvider {
public:
	/// Virtual destructor, does nothing
	virtual ~AudioMarkerProvider() { }

	/// @brief Return markers in a sample range
	virtual void GetMarkers(const AudioController::SampleRange &range, AudioMarkerVector &out) const = 0;
};



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
	/// @return The marker's position in samples
	virtual int64_t GetPosition() const = 0;

	/// @brief Get the marker's drawing style
	/// @return A pen object describing the marker's drawing style
	virtual wxPen GetStyle() const = 0;

	/// @brief Get the marker's feet style
	/// @return The marker's feet style
	virtual FeetStyle GetFeet() const = 0;

	/// @brief Retrieve whether this marker participates in snapping
	/// @return True if this marker may snap to other snappable markers
	///
	/// If a marker being dragged returns true from this method, and another marker which also
	/// returns true from this method is within range, the marker being dragged will be positioned
	/// at the position of the other marker if it is released while it is inside snapping range.
	virtual bool CanSnap() const = 0;
};



namespace agi {
	DEFINE_BASE_EXCEPTION(AudioControllerError, Exception);
	DEFINE_SIMPLE_EXCEPTION(AudioOpenError, AudioControllerError, "audio_controller/open_failed");
};
