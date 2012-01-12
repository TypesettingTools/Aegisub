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
#include "audio_controller.h"
#include "audio_marker_provider_keyframes.h"
#include "audio_renderer.h"
#include "audio_timing.h"
#include "include/aegisub/context.h"
#include "main.h"
#include "pen.h"
#include "selection_controller.h"
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

	/// Draw style for the left marker
	Pen style_left;
	/// Draw style for the right marker
	Pen style_right;


public:
	// AudioMarker interface
	int64_t GetPosition() const { return position; }
	wxPen GetStyle() const { return style; }
	FeetStyle GetFeet() const { return feet; }
	bool CanSnap() const { return false; }

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

	/// Implicit decay to the position of the marker
	operator int64_t() const { return position; }
};

/// @class InactiveLineMarker
/// @brief Markers for the beginning and ends of inactive lines
class InactiveLineMarker : public AudioMarker {
	int64_t position;
	Pen style;
	FeetStyle feet;

public:
	int64_t GetPosition() const { return position; }
	wxPen GetStyle() const { return style; }
	FeetStyle GetFeet() const { return feet; }
	bool CanSnap() const { return true; }

	InactiveLineMarker(int64_t position, bool start)
	: position(position)
	, style("Colour/Audio Display/Line Boundary Inactive Line", "Audio/Line Boundaries Thickness")
	, feet(start ? Feet_Right : Feet_Left)
	{
	}

	/// Implicit decay to the position of the marker
	operator int64_t() const { return position; }
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
	AudioMarkerDialogueTiming active_markers[2];

	/// Markers for inactive lines
	std::vector<InactiveLineMarker> inactive_markers;

	/// Marker provider for video keyframes
	AudioMarkerProviderKeyframes keyframes_provider;

	/// Has the timing been modified by the user?
	/// If auto commit is enabled this will only be true very briefly following
	/// changes
	bool timing_modified;

	/// Commit id for coalescing purposes when in auto commit mode
	int commit_id;

	/// The owning project context
	agi::Context *context;

	/// Autocommit option
	const agi::OptionValue *auto_commit;
	const agi::OptionValue *inactive_line_mode;
	const agi::OptionValue *inactive_line_comments;

	agi::signal::Connection commit_connection;
	agi::signal::Connection audio_open_connection;
	agi::signal::Connection inactive_line_mode_connection;
	agi::signal::Connection inactive_line_comment_connection;

	/// Get the leftmost of the markers
	AudioMarkerDialogueTiming *GetLeftMarker();
	const AudioMarkerDialogueTiming *GetLeftMarker() const;

	/// Get the rightmost of the markers
	AudioMarkerDialogueTiming *GetRightMarker();
	const AudioMarkerDialogueTiming *GetRightMarker() const;

	/// Update the audio controller's selection
	void UpdateSelection();

	/// Regenerate markers for inactive lines
	void RegenerateInactiveLines();

	/// Add the inactive line markers for a single line
	/// @param line Line to add markers for. May be NULL.
	void AddInactiveMarkers(AssDialogue *line);

	/// @brief Set the position of a marker and announce the change to the world
	/// @param marker Marker to move
	/// @param sample New position of the marker
	void SetMarker(AudioMarkerDialogueTiming *marker, int64_t sample);

	/// Snap a position to a nearby marker, if any
	/// @param position Position to snap
	/// @param snap_range Maximum distance to snap in samples
	int64_t SnapPosition(int64_t position, int64_t snap_range) const;

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
	void GetRenderingStyles(AudioRenderingStyleRanges &ranges) const;
	void GetLabels(SampleRange const& range, std::vector<AudioLabel> &out) const { }
	void Next();
	void Prev();
	void Commit();
	void Revert();
	bool IsNearbyMarker(int64_t sample, int sensitivity) const;
	AudioMarker * OnLeftClick(int64_t sample, int sensitivity, int64_t snap_range);
	AudioMarker * OnRightClick(int64_t sample, int sensitivity, int64_t snap_range);
	void OnMarkerDrag(AudioMarker *marker, int64_t new_position, int64_t snap_range);

public:
	// Specific interface

	/// @brief Constructor
	AudioTimingControllerDialogue(agi::Context *c);
	~AudioTimingControllerDialogue();
};

AudioTimingController *CreateDialogueTimingController(agi::Context *c)
{
	return new AudioTimingControllerDialogue(c);
}

// AudioMarkerDialogueTiming
void AudioMarkerDialogueTiming::SetPosition(int64_t new_position)
{
	position = new_position;

	if (other)
	{
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
, style_left("Colour/Audio Display/Line boundary Start", "Audio/Line Boundaries Thickness")
, style_right("Colour/Audio Display/Line boundary End", "Audio/Line Boundaries Thickness")
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

AudioTimingControllerDialogue::AudioTimingControllerDialogue(agi::Context *c)
: keyframes_provider(c, "Audio/Display/Draw/Keyframes in Dialogue Mode")
, timing_modified(false)
, commit_id(-1)
, context(c)
, auto_commit(OPT_GET("Audio/Auto/Commit"))
, inactive_line_mode(OPT_GET("Audio/Inactive Lines Display Mode"))
, inactive_line_comments(OPT_GET("Audio/Display/Draw/Inactive Comments"))
, commit_connection(c->ass->AddCommitListener(&AudioTimingControllerDialogue::OnFileChanged, this))
, audio_open_connection(c->audioController->AddAudioOpenListener(&AudioTimingControllerDialogue::Revert, this))
, inactive_line_mode_connection(OPT_SUB("Audio/Inactive Lines Display Mode", &AudioTimingControllerDialogue::RegenerateInactiveLines, this))
, inactive_line_comment_connection(OPT_SUB("Audio/Display/Draw/Inactive Comments", &AudioTimingControllerDialogue::RegenerateInactiveLines, this))
{
	assert(c->audioController != 0);

	AudioMarkerDialogueTiming::InitPair(&active_markers[0], &active_markers[1]);

	c->selectionController->AddSelectionListener(this);
	keyframes_provider.AddMarkerMovedListener(std::tr1::bind(std::tr1::ref(AnnounceMarkerMoved)));

	Revert();
}


AudioTimingControllerDialogue::~AudioTimingControllerDialogue()
{
	context->selectionController->RemoveSelectionListener(this);
}

AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetLeftMarker()
{
	return active_markers[0] < active_markers[1] ? &active_markers[0] : &active_markers[1];
}

const AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetLeftMarker() const
{
	return &std::min(active_markers[0], active_markers[1]);
}

AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetRightMarker()
{
	return active_markers[0] < active_markers[1] ? &active_markers[1] : &active_markers[0];
}

const AudioMarkerDialogueTiming *AudioTimingControllerDialogue::GetRightMarker() const
{
	return &std::max(active_markers[0], active_markers[1]);
}

void AudioTimingControllerDialogue::GetMarkers(const SampleRange &range, AudioMarkerVector &out_markers) const
{
	// The order matters here; later markers are painted on top of earlier
	// markers, so the markers that we want to end up on top need to appear last

	// Copy inactive line markers in the range
	std::vector<InactiveLineMarker>::const_iterator
		a = lower_bound(inactive_markers.begin(), inactive_markers.end(), range.begin()),
		b = upper_bound(inactive_markers.begin(), inactive_markers.end(), range.end());

	for (; a != b; ++a)
		out_markers.push_back(&*a);

	if (range.contains(active_markers[0]))
		out_markers.push_back(&active_markers[0]);
	if (range.contains(active_markers[1]))
		out_markers.push_back(&active_markers[1]);

	keyframes_provider.GetMarkers(range, out_markers);
}

void AudioTimingControllerDialogue::OnActiveLineChanged(AssDialogue *new_line)
{
	if (context->audioController->IsAudioOpen())
	{
		Revert();
	}
}

void AudioTimingControllerDialogue::OnSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed)
{
	/// @todo Create new passive markers, perhaps
}

void AudioTimingControllerDialogue::OnFileChanged(int type) {
	if (type & AssFile::COMMIT_DIAG_TIME)
	{
		Revert();
	}
	else if (type & AssFile::COMMIT_DIAG_ADDREM)
	{
		RegenerateInactiveLines();
	}
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
	return SampleRange(*GetLeftMarker(), *GetRightMarker());
}

void AudioTimingControllerDialogue::GetRenderingStyles(AudioRenderingStyleRanges &ranges) const
{
	ranges.AddRange(*GetLeftMarker(), *GetRightMarker(), AudioStyle_Selected);
	for (size_t i = 0; i < inactive_markers.size(); i += 2)
	{
		ranges.AddRange(inactive_markers[i], inactive_markers[i + 1], AudioStyle_Inactive);
	}
}

void AudioTimingControllerDialogue::Next()
{
	context->selectionController->NextLine();
}

void AudioTimingControllerDialogue::Prev()
{
	context->selectionController->PrevLine();
}

void AudioTimingControllerDialogue::Commit()
{
	int new_start_ms = context->audioController->MillisecondsFromSamples(*GetLeftMarker());
	int new_end_ms = context->audioController->MillisecondsFromSamples(*GetRightMarker());

	// If auto committing is enabled, timing_modified will be true iif it is an
	// auto commit, as there is never pending changes to commit when the button
	// is clicked
	bool user_triggered = !(timing_modified && auto_commit->GetBool());

	// Store back new times
	if (timing_modified)
	{
		Selection sel;
		context->selectionController->GetSelectedSet(sel);
		for (Selection::iterator sub = sel.begin(); sub != sel.end(); ++sub)
		{
			(*sub)->Start = new_start_ms;
			(*sub)->End = new_end_ms;
		}

		commit_connection.Block();
		if (user_triggered)
		{
			context->ass->Commit(_("timing"), AssFile::COMMIT_DIAG_TIME);
			commit_id = -1; // never coalesce with a manually triggered commit
		}
		else
			commit_id = context->ass->Commit(_("timing"), AssFile::COMMIT_DIAG_TIME, commit_id, context->selectionController->GetActiveLine());

		commit_connection.Unblock();
		timing_modified = false;
	}

	if (user_triggered && OPT_GET("Audio/Next Line on Commit")->GetBool())
	{
		/// @todo Old audio display created a new line if there was no next,
		///       like the edit box, so maybe add a way to do that which both
		///       this and the edit box can use
		Next();
		if (context->selectionController->GetActiveLine()->End == 0) {
			const int default_duration = OPT_GET("Timing/Default Duration")->GetInt();
			active_markers[0].SetPosition(context->audioController->SamplesFromMilliseconds(new_end_ms));
			active_markers[1].SetPosition(context->audioController->SamplesFromMilliseconds(new_end_ms + default_duration));
			timing_modified = true;
			UpdateSelection();
		}
	}
}

void AudioTimingControllerDialogue::Revert()
{
	if (AssDialogue *line = context->selectionController->GetActiveLine())
	{
		if (line->Start != 0 || line->End != 0)
		{
			active_markers[0].SetPosition(context->audioController->SamplesFromMilliseconds(line->Start));
			active_markers[1].SetPosition(context->audioController->SamplesFromMilliseconds(line->End));
			timing_modified = false;
			AnnounceUpdatedPrimaryRange();
			if (inactive_line_mode->GetInt() == 0)
				AnnounceUpdatedStyleRanges();
		}
	}
	RegenerateInactiveLines();
}

bool AudioTimingControllerDialogue::IsNearbyMarker(int64_t sample, int sensitivity) const
{
	SampleRange range(sample-sensitivity, sample+sensitivity);

	return range.contains(active_markers[0]) || range.contains(active_markers[1]);
}

AudioMarker * AudioTimingControllerDialogue::OnLeftClick(int64_t sample, int sensitivity, int64_t snap_range)
{
	assert(sensitivity >= 0);

	int64_t dist_l, dist_r;

	AudioMarkerDialogueTiming *left = GetLeftMarker();
	AudioMarkerDialogueTiming *right = GetRightMarker();

	dist_l = tabs(*left - sample);
	dist_r = tabs(*right - sample);

	if (dist_l < dist_r && dist_l <= sensitivity)
	{
		// Clicked near the left marker:
		// Insta-move it and start dragging it
		SetMarker(left, SnapPosition(sample, snap_range));
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
	SetMarker(left, SnapPosition(sample, snap_range));
	return right;
}

AudioMarker * AudioTimingControllerDialogue::OnRightClick(int64_t sample, int sensitivity, int64_t snap_range)
{
	AudioMarkerDialogueTiming *right = GetRightMarker();
	SetMarker(right, SnapPosition(sample, snap_range));
	return right;
}

void AudioTimingControllerDialogue::OnMarkerDrag(AudioMarker *marker, int64_t new_position, int64_t snap_range)
{
	assert(marker == &active_markers[0] || marker == &active_markers[1]);

	SetMarker(static_cast<AudioMarkerDialogueTiming*>(marker), SnapPosition(new_position, snap_range));
}

void AudioTimingControllerDialogue::UpdateSelection()
{
	AnnounceUpdatedPrimaryRange();
	AnnounceUpdatedStyleRanges();
}

void AudioTimingControllerDialogue::SetMarker(AudioMarkerDialogueTiming *marker, int64_t sample)
{
	marker->SetPosition(sample);
	timing_modified = true;
	if (auto_commit->GetBool()) Commit();
	UpdateSelection();
}

static bool noncomment_dialogue(AssEntry *e)
{
	if (AssDialogue *diag = dynamic_cast<AssDialogue*>(e))
		return !diag->Comment;
	return false;
}

static bool dialogue(AssEntry *e)
{
	return !!dynamic_cast<AssDialogue*>(e);
}

void AudioTimingControllerDialogue::RegenerateInactiveLines()
{
	bool (*predicate)(AssEntry*) = inactive_line_comments->GetBool() ? dialogue : noncomment_dialogue;

	switch (int mode = inactive_line_mode->GetInt())
	{
	case 1: // Previous line only
	case 2: // Previous and next lines
		inactive_markers.clear();
		if (AssDialogue *line = context->selectionController->GetActiveLine())
		{
			std::list<AssEntry*>::iterator current_line =
				find(context->ass->Line.begin(), context->ass->Line.end(), line);

			std::list<AssEntry*>::iterator prev = current_line;
			while (--prev != context->ass->Line.begin() && !predicate(*prev)) ;
			AddInactiveMarkers(static_cast<AssDialogue*>(*prev));

			if (mode == 2)
			{
				std::list<AssEntry*>::iterator next =
					find_if(++current_line, context->ass->Line.end(), predicate);
				if (next != context->ass->Line.end())
					AddInactiveMarkers(static_cast<AssDialogue*>(*next));
			}
		}
		break;
	case 3: // All inactive lines
	{
		inactive_markers.clear();
		AssDialogue *active_line = context->selectionController->GetActiveLine();
		for (std::list<AssEntry*>::const_iterator it = context->ass->Line.begin(); it != context->ass->Line.end(); ++it)
		{
			if (*it != active_line && predicate(*it))
				AddInactiveMarkers(static_cast<AssDialogue*>(*it));
		}
		break;
	}
	default:
		if (inactive_markers.empty())
			return;
		inactive_markers.clear();
	}

	sort(inactive_markers.begin(), inactive_markers.end());
	AnnounceUpdatedStyleRanges();
}

int64_t AudioTimingControllerDialogue::SnapPosition(int64_t position, int64_t snap_range) const
{
	if (snap_range <= 0)
		return position;

	SampleRange snap_sample_range(position - snap_range, position + snap_range);
	const AudioMarker *snap_marker = 0;
	AudioMarkerVector potential_snaps;
	GetMarkers(snap_sample_range, potential_snaps);
	for (AudioMarkerVector::iterator mi = potential_snaps.begin(); mi != potential_snaps.end(); ++mi)
	{
		if ((*mi)->CanSnap())
		{
			if (!snap_marker)
				snap_marker = *mi;
			else if (tabs((*mi)->GetPosition() - position) < tabs(snap_marker->GetPosition() - position))
				snap_marker = *mi;
		}
	}

	if (snap_marker)
		return snap_marker->GetPosition();
	return position;
}

void AudioTimingControllerDialogue::AddInactiveMarkers(AssDialogue *line)
{
	inactive_markers.push_back(InactiveLineMarker(
		context->audioController->SamplesFromMilliseconds(line->Start), true));
	inactive_markers.push_back(InactiveLineMarker(
		context->audioController->SamplesFromMilliseconds(line->End), false));
}
