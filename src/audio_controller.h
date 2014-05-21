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

#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>
#include <libaegisub/signal.h>

#include <cstdint>
#include <wx/event.h>
#include <wx/power.h>
#include <wx/timer.h>

class AudioPlayer;
class AudioProvider;
class AudioTimingController;
namespace agi { struct Context; }
class TimeRange;

/// @class AudioController
/// @brief Manage playback of an open audio stream
///
/// AudioController owns an AudioPlayer and uses it to play audio from the
/// project's current audio provider.
class AudioController final : public wxEvtHandler {
	/// Project context this controller belongs to
	agi::Context *context;

	/// Slot for subtitles save signal
	agi::signal::Connection subtitle_save_slot;

	/// Playback is in progress and the current position was updated
	agi::signal::Signal<int> AnnouncePlaybackPosition;

	/// Playback has stopped
	agi::signal::Signal<> AnnouncePlaybackStop;

	/// The timing controller was replaced
	agi::signal::Signal<> AnnounceTimingControllerChanged;

	/// The audio output object
	std::unique_ptr<AudioPlayer> player;

	/// The current timing mode, if any; owned by the audio controller
	std::unique_ptr<AudioTimingController> timing_controller;

	enum PlaybackMode {
		PM_NotPlaying,
		PM_Range,
		PM_PrimaryRange,
		PM_ToEnd
	};
	/// The current playback mode
	PlaybackMode playback_mode = PM_NotPlaying;

	/// Timer used for playback position updates
	wxTimer playback_timer;

	/// The audio provider
	AudioProvider *provider = nullptr;
	agi::signal::Connection provider_connection;

	void OnAudioProvider(AudioProvider *new_provider);

	/// Event handler for the playback timer
	void OnPlaybackTimer(wxTimerEvent &event);

	/// @brief Timing controller signals primary playback range changed
	void OnTimingControllerUpdatedPrimaryRange();

	/// @brief Timing controller signals that the rendering style ranges have changed
	void OnTimingControllerUpdatedStyleRanges();

	/// Handler for the current audio player changing
	void OnAudioPlayerChanged();

#ifdef wxHAS_POWER_EVENTS
	/// Handle computer going into suspend mode by stopping audio and closing device
	void OnComputerSuspending(wxPowerEvent &event);
	/// Handle computer resuming from suspend by re-opening the audio device
	void OnComputerResuming(wxPowerEvent &event);
#endif

	/// @brief Convert a count of audio samples to a time in milliseconds
	/// @param samples Sample count to convert
	/// @return The number of milliseconds equivalent to the sample-count, rounded down
	int64_t MillisecondsFromSamples(int64_t samples) const;

	/// @brief Convert a time in milliseconds to a count of audio samples
	/// @param ms Time in milliseconds to convert
	/// @return The index of the first sample that is wholly inside the millisecond
	int64_t SamplesFromMilliseconds(int64_t ms) const;

public:
	AudioController(agi::Context *context);
	~AudioController();

	/// @brief Start or restart audio playback, playing a range
	/// @param range The range of audio to play back
	///
	/// The end of the played back range may be requested changed, but is not
	/// changed automatically from any other operations.
	void PlayRange(const TimeRange &range);

	/// @brief Start or restart audio playback, playing the primary playback range
	///
	/// If the primary playback range is updated during playback, the end of
	/// the active playback range will be updated to match the new selection.
	/// The playback end can not be changed in any other way.
	void PlayPrimaryRange();

	/// @brief Start or restart audio playback, playing from a point to the end of of the primary playback range
	/// @param start_ms Time in milliseconds to start playback at
	///
	/// This behaves like PlayPrimaryRange, but the start point can differ from
	/// the beginning of the primary range.
	void PlayToEndOfPrimary(int start_ms);

	/// @brief Start or restart audio playback, playing from a point to the end of stream
	/// @param start_ms Time in milliseconds to start playback at
	///
	/// Playback to end cannot be converted to a range playback like range
	/// playback can, it will continue until the end is reached, it is stopped,
	/// or restarted.
	void PlayToEnd(int start_ms);

	/// @brief Stop all audio playback
	void Stop();

	/// @brief Determine whether playback is ongoing
	/// @return True if audio is being played back
	bool IsPlaying();

	/// @brief Get the current playback position
	/// @return Approximate current time in milliseconds being heard by the user
	///
	/// Returns 0 if playback is stopped. The return value is only approximate.
	int GetPlaybackPosition();

	/// Get the duration of the currently open audio in milliseconds, or 0 if none
	/// @return Duration in milliseconds
	int GetDuration() const;

	/// @brief Get the primary playback range
	/// @return An immutable TimeRange object
	TimeRange GetPrimaryPlaybackRange() const;

	/// @brief Set the playback audio volume
	/// @param volume The new amplification factor for the audio
	void SetVolume(double volume);

	/// @brief Return the current timing controller
	/// @return The current timing controller or 0
	AudioTimingController *GetTimingController() const { return timing_controller.get(); }

	/// @brief Change the current timing controller
	/// @param new_mode The new timing controller or nullptr
	void SetTimingController(std::unique_ptr<AudioTimingController> new_controller);

	DEFINE_SIGNAL_ADDERS(AnnouncePlaybackPosition,        AddPlaybackPositionListener)
	DEFINE_SIGNAL_ADDERS(AnnouncePlaybackStop,            AddPlaybackStopListener)
	DEFINE_SIGNAL_ADDERS(AnnounceTimingControllerChanged, AddTimingControllerListener)
};

namespace agi {
	/// Base class for all audio-related errors
	DEFINE_BASE_EXCEPTION(AudioError, Exception)

	/// Opening the audio failed for any reason
	DEFINE_SIMPLE_EXCEPTION(AudioOpenError, AudioError, "audio/open")

	/// There are no audio providers available to open audio files
	DEFINE_SIMPLE_EXCEPTION(NoAudioProvidersError, AudioOpenError, "audio/open/no_providers")

	/// The file exists, but no providers could find any audio tracks in it
	DEFINE_SIMPLE_EXCEPTION(AudioDataNotFoundError, AudioOpenError, "audio/open/no_tracks")

	/// There are audio tracks, but no provider could actually read them
	DEFINE_SIMPLE_EXCEPTION(AudioProviderOpenError, AudioOpenError, "audio/open/provider")

	/// The audio cache failed to initialize
	DEFINE_SIMPLE_EXCEPTION(AudioCacheOpenError, AudioOpenError, "audio/open/cache")

	/// There are no audio players available
	DEFINE_SIMPLE_EXCEPTION(NoAudioPlayersError, AudioOpenError, "audio/open/no_players")

	/// The audio player failed to initialize
	DEFINE_SIMPLE_EXCEPTION(AudioPlayerOpenError, AudioOpenError, "audio/open/player")
}
