// Copyright (c) 2010, Niels Martin Hansen
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

/// @file audio_timing.h
/// @brief Construction-functions for timing controller objects
/// @ingroup audio_ui

class AssDialogue;
class AssFile;
class AudioController;

#include <libaegisub/signal.h>

/// @class AudioTimingController
/// @brief Base class for objects controlling audio timing
///
/// There is just one active audio timing controller at a time per audio
/// controller. The timing controller manages the timing mode and supplies
/// markers that can be manipulated to the audio display, as well as the
/// current selection.
///
/// The timing controller must then be sent the marker drag events as well as
/// clicks in empty areas of the audio display.
class AudioTimingController : public AudioMarkerProvider {
protected:
	/// The primary playback range has changed, usually as a result of user interaction.
	agi::signal::Signal<> AnnounceUpdatedPrimaryRange;

	/// One or more rendering style ranges have changed in the timing controller.
	agi::signal::Signal<> AnnounceUpdatedStyleRanges;

	/// A marker has been updated in some way.
	agi::signal::Signal<AudioMarker*> AnnounceMarkerMoved;
public:
	/// @brief Get any warning message to show in the audio display
	/// @return The warning message to show, may be empty if there is none
	virtual wxString GetWarningMessage() const = 0;

	/// @brief Get the sample range the user is most likely to want to see for the current state
	/// @return A sample range
	///
	/// This is used for "bring working area into view" operations.
	virtual SampleRange GetIdealVisibleSampleRange() const = 0;

	/// @brief Get the primary playback range
	/// @return A sample range
	///
	/// Get the sample range the user is most likely to want to play back
	/// currently.
	virtual SampleRange GetPrimaryPlaybackRange() const = 0;

	/// @brief Does this timing mode have labels on the audio display?
	/// @return True if this timing mode needs labels on the audio display.
	///
	/// This is labels for things such as karaoke syllables. When labels are
	/// required, some vertical space is set off for them in the drawing of the
	/// audio display.
	virtual bool HasLabels() const = 0;

	/// @brief Go to next timing unit
	/// 
	/// Advances the timing controller cursor to the next timing unit, for
	/// example the next dialogue line or the next karaoke syllable.
	virtual void Next() = 0;

	/// @brief Go to the previous timing unit
	///
	/// Rewinds the timing controller to the previous timing unit.
	virtual void Prev() = 0;

	/// @brief Commit all changes
	///
	/// Stores all changes permanently.
	virtual void Commit() = 0;

	/// @brief Revert all changes
	///
	/// Revert all changes to the last committed state.
	virtual void Revert() = 0;

	/// @brief Determine if a position is close to a draggable marker
	/// @param sample      The audio sample index to test
	/// @param sensitivity Distance in samples to consider markers as nearby
	/// @return True if a marker is close by the given sample, as defined by sensitivity
	///
	/// This is solely for hit-testing against draggable markers, for
	/// controlling the mouse cursor.
	virtual bool IsNearbyMarker(int64_t sample, int sensitivity) const = 0;

	/// @brief The user pressed the left button down at an empty place in the audio
	/// @param sample      The audio sample index the user clicked
	/// @param sensitivity Distance in samples to consider existing markers
	/// @return An audio marker or 0. If a marker is returned and the user
	/// starts dragging the mouse after pressing down the button, the returned
	/// marker is being dragged.
	virtual AudioMarker * OnLeftClick(int64_t sample, int sensitivity) = 0;

	/// @brief The user pressed the right button down at an empty place in the audio
	/// @param sample      The audio sample index the user clicked
	/// @param sensitivity Distance in samples to consider existing markers
	/// @return An audio marker or 0. If a marker is returned and the user
	/// starts dragging the mouse after pressing down the button, the returned
	/// marker is being dragged.
	virtual AudioMarker * OnRightClick(int64_t sample, int sensitivity) = 0;

	/// @brief The user dragged a timing marker
	/// @param marker       The marker being dragged
	/// @param new_position Sample position the marker was dragged to
	virtual void OnMarkerDrag(AudioMarker *marker, int64_t new_position) = 0;

	/// @brief Destructor
	virtual ~AudioTimingController() { }

	DEFINE_SIGNAL_ADDERS(AnnounceUpdatedPrimaryRange, AddUpdatedPrimaryRangeListener)
	DEFINE_SIGNAL_ADDERS(AnnounceUpdatedStyleRanges, AddUpdatedStyleRangesListener)
	DEFINE_SIGNAL_ADDERS(AnnounceMarkerMoved, AddMarkerMovedListener)
};


/// @brief Create a standard dialogue audio timing controller
/// @param audio_controller     The audio controller to own the timing controller
/// @param selection_controller The selection controller to manage the set of
/// lines being timed
/// @param ass                  The file being timed
AudioTimingController *CreateDialogueTimingController(AudioController *audio_controller, SelectionController<AssDialogue> *selection_controller, AssFile *ass);
