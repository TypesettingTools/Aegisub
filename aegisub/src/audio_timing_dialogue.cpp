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

/// @file audio_timing_dialogue.cpp
/// @brief Default timing mode for dialogue subtitles
/// @ingroup audio_ui


#ifndef AGI_PRE
#include <stdint.h>
#include <wx/pen.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "main.h"
#include "selection_controller.h"
#include "audio_controller.h"
#include "audio_timing.h"
#include "utils.h"



/// @class AudioMarkerDialogueTiming
/// @brief AudioMarker implementation for AudioTimingControllerDialogue
///
/// Audio marker intended to live in pairs of two, taking styles depending
/// on which marker in the pair is to the left and which is to the right.
class AudioMarkerDialogueTiming : public AudioMarker {
	/// The other marker for the dialogue line's pair
	AudioMarkerDialogueTiming *other;

	/// Current sample position of this marker
	int64_t position;

	/// Draw style for the marker
	wxPen style;
	/// Foot style for the marker
	FeetStyle feet;

public:
	// AudioMarker interface
	int64_t GetPosition() const { return position; }
	wxPen GetStyle() const { return style; }
	FeetStyle GetFeet() const { return feet; }
	bool CanSnap() const { return true; }

public:
	// Specific interface

	/// @brief Move the marker to a new position
	/// @param new_position The position to move the marker to, in audio samples
	///
	/// If the marker moves to the opposite side of the other marker in the pair,
	/// the styles of the two markers will be changed to match the new start/end
	/// relationship of them.
	void SetPosition(int64_t new_position);


	/// @brief Constructor
	///
	/// Initialises the fields to default values.
	AudioMarkerDialogueTiming();


	/// @brief Initialise a pair of dialogue markers to be a pair
	/// @param marker1 The first marker in the pair to make
	/// @param marker2 The second marker in the pair to make
	///
	/// This checks that the markers aren't already part of a pair, and then
	/// sets their "other" field. Positions and styles aren't affected.
	static void InitPair(AudioMarkerDialogueTiming *marker1, AudioMarkerDialogueTiming *marker2);
};



/// @class AudioTimingControllerDialogue
/// @brief Default timing mode for dialogue subtitles
///
/// Displays a start and end marker for an active subtitle line, and allows
/// for those markers to be dragged. Dragging the start/end markers changes
/// the audio selection.
///
/// When the audio rendering code is expanded to support it, inactive lines
/// will also be shown as shaded lines that cannot be changed.
///
/// Another later expansion will be to affect the timing of multiple selected
/// lines at the same time, if they e.g. have end1==start2.
class AudioTimingControllerDialogue : public AudioTimingController, private SelectionListener<AssDialogue> {
	/// Start and end markers for the active line
	AudioMarkerDialogueTiming markers[2];

	/// Has the timing been modified by the user?
	/// If auto commit is enabled this will only be true very briefly following
	/// changes
	bool timing_modified;

	/// Commit id for coalescing purposes when in auto commit mode
	int commit_id;

	/// Get the leftmost of the markers
	AudioMarkerDialogueTiming *GetLeftMarker();
	const AudioMarkerDialogueTiming *GetLeftMarker() const;
	/// Get the rightmost of the markers
	AudioMarkerDialogueTiming *GetRightMarker();
	const AudioMarkerDialogueTiming *GetRightMarker() const;

	/// The owning audio controller
	AudioController *audio_controller;

	/// Update the audio controller's selection
	void UpdateSelection();

	/// @brief Set the position of a marker and announce the change to the world
	/// @param marker Marker to move
	/// @param sample New position of the marker
	void SetMarker(AudioMarkerDialogueTiming *marker, int64_t sample);

	/// Autocommit option
	const agi::OptionValue *auto_commit;

	/// Selection controller managing the set of lines currently being timed
	SelectionController<AssDialogue> *selection_controller;

	agi::signal::Connection commit_slot;

	/// @todo Dealing with the AssFile directly is probably not the best way to
	///       handle committing, but anything better probably needs to wait for
	///       the subtitle container work
	AssFile *ass;

	// SubtitleSelectionListener interface
	void OnActiveLineChanged(AssDialogue *new_line);
	void OnSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed);

	// AssFile events
	void OnFileChanged(int type);

public:
	// AudioMarkerProvider interface
	void GetMarkers(const SampleRange &range, AudioMarkerVector &out_markers) const;

	// AudioTimingController interface
	wxString GetWarningMessage() const;
	SampleRange GetIdealVisibleSampleRange() const;
	SampleRange GetPrimaryPlaybackRange() const;
	bool HasLabels() const;
	void Next();
	void Prev();
	void Commit();
	void Revert();
	bool IsNearbyMarker(int64_t sample, int sensitivity) const;
	AudioMarker * OnLeftClick(int64_t sample, int sensitivity);
	AudioMarker * OnRightClick(int64_t sample, int sensitivity);
	void OnMarkerDrag(AudioMarker *marker, int64_t new_position);

public:
	// Specific interface

	/// @brief Constructor
	AudioTimingControllerDialogue(AudioController *audio_controller, SelectionController<AssDialogue> *selection_controller, AssFile *ass);
	~AudioTimingControllerDialogue();
};



AudioTimingController *CreateDialogueTimingController(AudioController *audio_controller, SelectionController<AssDialogue> *selection_controller, AssFile *ass)
{
	return new AudioTimingControllerDialogue(audio_controller, selection_controller, ass);
}

// AudioMarkerDialogueTiming
void AudioMarkerDialogueTiming::SetPosition(int64_t new_position)
{
	position = new_position;

	if (other)
	{
		/// @todo Make this depend on configuration
		wxPen style_left = wxPen(*wxRED, 2);
		wxPen style_right = wxPen(*wxBLUE, 2);
		if (position < other->position)
		{
			feet = Feet_Right;
			other->feet = Feet_Left;
			style = style_left;
			other->style = style_right;
		}
		else if (position > other->position)
		{
			feet = Feet_Left;
			other->feet = Feet_Right;
			style = style_right;
			other->style = style_left;
		}
	}
}


AudioMarkerDialogueTiming::AudioMarkerDialogueTiming()
: other(0)
, position(0)
, style(*wxTRANSPARENT_PEN)
, feet(Feet_None)
{
	// Nothing more to do
}


void AudioMarkerDialogueTiming::InitPair(AudioMarkerDialogueTiming *marker1, AudioMarkerDialogueTiming *marker2)
{
	assert(marker1->other == 0);
	assert(marker2->other == 0);

	marker1->other = marker2;
	marker2->other = marker1;
}



// AudioTimingControllerDialogue

AudioTimingControllerDialogue::AudioTimingControllerDialogue(AudioController *audio_controller, SelectionController<AssDialogue> *selection_controller, AssFile *ass)
: timing_modified(false)
, commit_id(-1)
, ass(ass)
, auto_commit(OPT_GET("Audio/Auto/Commit"))
, audio_controller(audio_controller)
, selection_controller(selection_controller)
{
	assert(audio_controller != 0);

	AudioMarkerDialogueTiming::InitPair(&markers[0], &markers[1]);

	selection_controller->AddSelectionListener(this);
	commit_slot = ass->AddCommitListener(&AudioTimingControllerDialogue::OnFileChanged, this);
}


AudioTimingControllerDialogue::~AudioTimingControllerDialogue()
{
	selection_controller->RemoveSelectionListener(this);
}



AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetLeftMarker()
{
	return markers[0].GetPosition() < markers[1].GetPosition() ? &markers[0] : &markers[1];
}

const AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetLeftMarker() const
{
	return markers[0].GetPosition() < markers[1].GetPosition() ? &markers[0] : &markers[1];
}

AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetRightMarker()
{
	return markers[0].GetPosition() < markers[1].GetPosition() ? &markers[1] : &markers[0];
}

const AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetRightMarker() const
{
	return markers[0].GetPosition() < markers[1].GetPosition() ? &markers[1] : &markers[0];
}



void AudioTimingControllerDialogue::GetMarkers(const SampleRange &range, AudioMarkerVector &out_markers) const
{
	if (range.contains(markers[0].GetPosition()))
		out_markers.push_back(&markers[0]);
	if (range.contains(markers[1].GetPosition()))
		out_markers.push_back(&markers[1]);
}

void AudioTimingControllerDialogue::OnActiveLineChanged(AssDialogue *new_line)
{
	Revert();
}

void AudioTimingControllerDialogue::OnSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed)
{
	/// @todo Create new passive markers, perhaps
}

void AudioTimingControllerDialogue::OnFileChanged(int type) {
	if (type == AssFile::COMMIT_UNDO || type == AssFile::COMMIT_FULL) return;

	Revert();
}


wxString AudioTimingControllerDialogue::GetWarningMessage() const
{
	// We have no warning messages currently, maybe add the old "Modified" message back later?
	return wxString();
}



SampleRange AudioTimingControllerDialogue::GetIdealVisibleSampleRange() const
{
	return GetPrimaryPlaybackRange();
}



SampleRange AudioTimingControllerDialogue::GetPrimaryPlaybackRange() const
{
	return SampleRange(
		GetLeftMarker()->GetPosition(),
		GetRightMarker()->GetPosition());
}



bool AudioTimingControllerDialogue::HasLabels() const
{
	return false;
}



void AudioTimingControllerDialogue::Next()
{
	selection_controller->NextLine();
}



void AudioTimingControllerDialogue::Prev()
{
	selection_controller->PrevLine();
}



void AudioTimingControllerDialogue::Commit()
{
	int new_start_ms = audio_controller->MillisecondsFromSamples(GetLeftMarker()->GetPosition());
	int new_end_ms = audio_controller->MillisecondsFromSamples(GetRightMarker()->GetPosition());

	// If auto committing is enabled, timing_modified will be true iif it is an
	// auto commit, as there is never pending changes to commit when the button
	// is clicked
	bool user_triggered = !(timing_modified && auto_commit->GetBool());

	// Store back new times
	if (timing_modified)
	{
		Selection sel;
		selection_controller->GetSelectedSet(sel);
		for (Selection::iterator sub = sel.begin(); sub != sel.end(); ++sub)
		{
			(*sub)->Start.SetMS(new_start_ms);
			(*sub)->End.SetMS(new_end_ms);
		}

		commit_slot.Block();
		if (user_triggered)
		{
			ass->Commit(_("timing"), AssFile::COMMIT_TIMES);
			commit_id = -1; // never coalesce with a manually triggered commit
		}
		else
			commit_id = ass->Commit(_("timing"), AssFile::COMMIT_TIMES, commit_id);

		commit_slot.Unblock();
		timing_modified = false;
	}

	if (user_triggered && OPT_GET("Audio/Next Line on Commit")->GetBool())
	{
		/// @todo Old audio display created a new line if there was no next,
		///       like the edit box, so maybe add a way to do that which both
		///       this and the edit box can use
		Next();
		if (selection_controller->GetActiveLine()->End.GetMS() == 0) {
			const int default_duration = OPT_GET("Timing/Default Duration")->GetInt();
			markers[0].SetPosition(audio_controller->SamplesFromMilliseconds(new_end_ms));
			markers[1].SetPosition(audio_controller->SamplesFromMilliseconds(new_end_ms + default_duration));
			timing_modified = true;
			UpdateSelection();
		}
	}
}



void AudioTimingControllerDialogue::Revert()
{
	if (AssDialogue *line = selection_controller->GetActiveLine())
	{
		AssTime new_start = line->Start;
		AssTime new_end = line->End;

		if (new_start.GetMS() != 0 || new_end.GetMS() != 0)
		{
			markers[0].SetPosition(audio_controller->SamplesFromMilliseconds(new_start.GetMS()));
			markers[1].SetPosition(audio_controller->SamplesFromMilliseconds(new_end.GetMS()));
			timing_modified = false;
			UpdateSelection();
		}
	}
}



bool AudioTimingControllerDialogue::IsNearbyMarker(int64_t sample, int sensitivity) const
{
	SampleRange range(sample-sensitivity, sample+sensitivity);

	return range.contains(markers[0].GetPosition()) || range.contains(markers[1].GetPosition());
}


AudioMarker * AudioTimingControllerDialogue::OnLeftClick(int64_t sample, int sensitivity)
{
	assert(sensitivity >= 0);

	int64_t dist_l, dist_r;

	AudioMarkerDialogueTiming *left = GetLeftMarker();
	AudioMarkerDialogueTiming *right = GetRightMarker();

	dist_l = tabs(left->GetPosition() - sample);
	dist_r = tabs(right->GetPosition() - sample);

	if (dist_l < dist_r && dist_l <= sensitivity)
	{
		// Clicked near the left marker:
		// Insta-move it and start dragging it
		SetMarker(left, sample);
		return left;
	}

	if (dist_r < dist_l && dist_r <= sensitivity)
	{
		// Clicked near the right marker:
		// Only drag it. For insta-move, the user must right-click.
		return right;
	}

	// Clicked far from either marker:
	// Insta-set the left marker to the clicked position and return the right as the dragged one,
	// such that if the user does start dragging, he will create a new selection from scratch
	SetMarker(left, sample);
	return right;
}



AudioMarker * AudioTimingControllerDialogue::OnRightClick(int64_t sample, int sensitivity)
{
	AudioMarkerDialogueTiming *right = GetRightMarker();
	SetMarker(right, sample);
	return right;
}



void AudioTimingControllerDialogue::OnMarkerDrag(AudioMarker *marker, int64_t new_position)
{
	assert(marker == &markers[0] || marker == &markers[1]);

	SetMarker(static_cast<AudioMarkerDialogueTiming*>(marker), new_position);
}



void AudioTimingControllerDialogue::UpdateSelection()
{
	AnnounceUpdatedPrimaryRange();
}

void AudioTimingControllerDialogue::SetMarker(AudioMarkerDialogueTiming *marker, int64_t sample)
{
	marker->SetPosition(sample);
	AnnounceMarkerMoved(marker);
	timing_modified = true;
	if (auto_commit->GetBool()) Commit();
	UpdateSelection();
}
