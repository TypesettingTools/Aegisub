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

/// @file audio_timing_dialogue.cpp
/// @brief Default timing mode for dialogue subtitles
/// @ingroup audio_ui


#include <cstdint>
#include <wx/pen.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "audio_renderer.h"
#include "audio_timing.h"
#include "command/command.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "pen.h"
#include "selection_controller.h"
#include "utils.h"

class TimeableLine;

/// @class DialogueTimingMarker
/// @brief AudioMarker implementation for AudioTimingControllerDialogue
///
/// Audio marker intended to live in pairs of two, taking styles depending
/// on which marker in the pair is to the left and which is to the right.
class DialogueTimingMarker : public AudioMarker {
	/// Current ms position of this marker
	int position;

	/// Draw style for the marker
	const Pen *style;

	/// Feet style for the marker
	FeetStyle feet;

	/// Rendering style of the owning line, needed for sorting
	AudioRenderingStyle type;

	/// The line which owns this marker
	TimeableLine *line;

public:
	int       GetPosition() const { return position; }
	wxPen     GetStyle()    const { return *style; }
	FeetStyle GetFeet()     const { return feet; }
	bool      CanSnap()     const { return true; }

	/// Move the marker to a new position
	/// @param new_position The position to move the marker to, in milliseconds
	///
	/// This notifies the owning line of the change, so that it can ensure that
	/// this marker has the appropriate rendering style.
	void SetPosition(int new_position);

	/// Constructor
	/// @param position Initial position of this marker
	/// @param style Rendering style of this marker
	/// @param feet Foot style of this marker
	/// @param type Type of this marker, used only for sorting
	/// @param line Line which this is a marker for
	DialogueTimingMarker(int position, const Pen *style, FeetStyle feet, AudioRenderingStyle type, TimeableLine *line)
	: position(position)
	, style(style)
	, feet(feet)
	, type(type)
	, line(line)
	{
	}

	DialogueTimingMarker(DialogueTimingMarker const& other, TimeableLine *line)
	: position(other.position)
	, style(other.style)
	, feet(other.feet)
	, type(other.type)
	, line(line)
	{
	}

	/// Get the line which this is a marker for
	TimeableLine *GetLine() const { return line; }

	/// Implicit decay to the position of the marker
	operator int() const { return position; }

	/// Comparison operator
	///
	/// Compares first on position, then on audio rendering style so that the
	/// markers for the active line end up after those for the inactive lines.
	bool operator<(DialogueTimingMarker const& other) const
	{
		if (position < other.position) return true;
		if (position > other.position) return false;
		return type < other.type;
	}

	/// Swap the rendering style of this marker with that of the passed marker
	void SwapStyles(DialogueTimingMarker &other)
	{
		std::swap(style, other.style);
		std::swap(feet, other.feet);
	}
};

/// A comparison predicate for pointers to dialogue markers and millisecond positions
struct marker_ptr_cmp
{
	bool operator()(const DialogueTimingMarker *lft, const DialogueTimingMarker *rgt) const
	{
		return *lft < *rgt;
	}

	bool operator()(const DialogueTimingMarker *lft, int rgt) const
	{
		return *lft < rgt;
	}

	bool operator()(int lft, const DialogueTimingMarker *rgt) const
	{
		return lft < *rgt;
	}
};

/// @class TimeableLine
/// @brief A single dialogue line which can be timed via AudioTimingControllerDialogue
///
/// This class provides markers and styling ranges for a single dialogue line,
/// both active and inactive. In addition, it can apply changes made via those
/// markers to the tracked dialogue line.
class TimeableLine {
	/// The current tracked dialogue line
	AssDialogue *line;
	/// The rendering style of this line
	AudioRenderingStyle style;

	/// One of the markers. Initially the left marker, but the user may change this.
	DialogueTimingMarker marker1;
	/// One of the markers. Initially the right marker, but the user may change this.
	DialogueTimingMarker marker2;

	/// Pointer to whichever marker happens to be on the left
	DialogueTimingMarker *left_marker;
	/// Pointer to whichever marker happens to be on the right
	DialogueTimingMarker *right_marker;

public:
	/// Constructor
	/// @param style Rendering style to use for this line's time range
	/// @param style_left The rendering style for the start marker
	/// @param style_right The rendering style for the end marker
	TimeableLine(AudioRenderingStyle style, const Pen *style_left, const Pen *style_right)
		: line(0)
		, style(style)
		, marker1(0, style_left, AudioMarker::Feet_Right, style, this)
		, marker2(0, style_right, AudioMarker::Feet_Left, style, this)
		, left_marker(&marker1)
		, right_marker(&marker2)
	{
	}

	/// Explicit copy constructor needed due to that the markers have a pointer to this
	TimeableLine(TimeableLine const& other)
		: line(other.line)
		, style(other.style)
		, marker1(*other.left_marker, this)
		, marker2(*other.right_marker, this)
		, left_marker(&marker1)
		, right_marker(&marker2)
	{
	}

	/// Get the tracked dialogue line
	AssDialogue *GetLine() const { return line; }

	/// Get the time range for this line
	operator TimeRange() const { return TimeRange(*left_marker, *right_marker); }

	/// Add this line's style to the style ranges
	void GetStyleRange(AudioRenderingStyleRanges *ranges) const
	{
		ranges->AddRange(*left_marker, *right_marker, style);
	}

	/// Get this line's markers
	/// @param c Vector to add the markers to
	void GetMarkers(std::vector<DialogueTimingMarker*> *c) const
	{
		c->push_back(left_marker);
		c->push_back(right_marker);
	}

	/// Get the leftmost of the markers
	DialogueTimingMarker *GetLeftMarker() { return left_marker; }

	/// Get the rightmost of the markers
	DialogueTimingMarker *GetRightMarker() { return right_marker; }

	/// Does this line have a marker in the given range?
	bool ContainsMarker(TimeRange const& range) const
	{
		return range.contains(marker1) || range.contains(marker2);
	}

	/// Check if the markers have the correct styles, and correct them if needed
	void CheckMarkers()
	{
		if (*right_marker < *left_marker)
		{
			marker1.SwapStyles(marker2);
			std::swap(left_marker, right_marker);
		}
	}

	/// Apply any changes made here to the tracked dialogue line
	void Apply()
	{
		if (line)
		{
			line->Start = left_marker->GetPosition();
			line->End = right_marker->GetPosition();
		}
	}

	/// Set the dialogue line which this is tracking and reset the markers to
	/// the line's time range
	/// @return Were the markers actually set to the line's time?
	bool SetLine(AssDialogue *new_line)
	{
		if (!line || new_line->End > 0)
		{
			line = new_line;
			marker1.SetPosition(new_line->Start);
			marker2.SetPosition(new_line->End);
			return true;
		}
		else
		{
			line = new_line;
			return false;
		}
	}
};

void DialogueTimingMarker::SetPosition(int new_position) {
	position = new_position;
	line->CheckMarkers();
}

/// @class AudioTimingControllerDialogue
/// @brief Default timing mode for dialogue subtitles
///
/// Displays a start and end marker for an active subtitle line, and possibly
/// some of the inactive lines. The markers for the active line can be dragged,
/// updating the audio selection and the start/end time of that line. In
/// addition, any markers for inactive lines that start/end at the same time
/// as the active line starts/ends can optionally be dragged along with the
/// active line's markers, updating those lines as well.
class AudioTimingControllerDialogue : public AudioTimingController {
	/// The rendering style for the active line's start marker
	Pen style_left;
	/// The rendering style for the active line's end marker
	Pen style_right;
	/// The rendering style for the start and end markers of inactive lines
	Pen style_inactive;

	/// The currently active line
	TimeableLine active_line;

	/// Inactive lines which are currently modifiable
	std::list<TimeableLine> inactive_lines;

	/// Selected lines which are currently modifiable
	std::list<TimeableLine> selected_lines;

	/// All audio markers for active and inactive lines, sorted by position
	std::vector<DialogueTimingMarker*> markers;

	/// Marker provider for video keyframes
	AudioMarkerProviderKeyframes keyframes_provider;

	/// Marker provider for video playback position
	VideoPositionMarkerProvider video_position_provider;

	/// Marker provider for seconds lines
	SecondsMarkerProvider seconds_provider;

	/// The set of lines which have been modified and need to have their
	/// changes applied on commit
	std::set<TimeableLine*> modified_lines;

	/// Commit id for coalescing purposes when in auto commit mode
	int commit_id;

	/// The owning project context
	agi::Context *context;

	/// Autocommit option
	const agi::OptionValue *auto_commit;
	const agi::OptionValue *inactive_line_mode;
	const agi::OptionValue *inactive_line_comments;
	const agi::OptionValue *drag_timing;

	agi::signal::Connection commit_connection;
	agi::signal::Connection audio_open_connection;
	agi::signal::Connection inactive_line_mode_connection;
	agi::signal::Connection inactive_line_comment_connection;
	agi::signal::Connection active_line_connection;
	agi::signal::Connection selection_connection;

	/// Update the audio controller's selection
	void UpdateSelection();

	/// Regenerate the list of timeable inactive lines
	void RegenerateInactiveLines();

	/// Regenerate the list of timeable selected lines
	void RegenerateSelectedLines();

	/// Add a line to the list of timeable inactive lines
	void AddInactiveLine(SubtitleSelection const& sel, AssDialogue *diag);

	/// Regenerate the list of active and inactive line markers
	void RegenerateMarkers();

	/// Get the start markers for the active line and all selected lines
	std::vector<AudioMarker*> GetLeftMarkers();

	/// Get the end markers for the active line and all selected lines
	std::vector<AudioMarker*> GetRightMarkers();

	/// @brief Set the position of markers and announce the change to the world
	/// @param upd_markers Markers to move
	/// @param ms New position of the markers
	void SetMarkers(std::vector<AudioMarker*> const& upd_markers, int ms);

	/// Snap a position to a nearby marker, if any
	/// @param position   Position to snap
	/// @param snap_range Maximum distance to snap in milliseconds
	/// @param exclude    Markers which should be excluded from the potential snaps
	int SnapPosition(int position, int snap_range, std::vector<AudioMarker*> const& exclude) const;

	/// Commit all pending changes to the file
	/// @param user_triggered Is this a user-initiated commit or an autocommit
	void DoCommit(bool user_triggered);

	// SubtitleSelectionListener interface
	void OnActiveLineChanged(AssDialogue *new_line);
	void OnSelectedSetChanged(const SubtitleSelection &lines_added, const SubtitleSelection &lines_removed);

	// AssFile events
	void OnFileChanged(int type);

public:
	// AudioMarkerProvider interface
	void GetMarkers(const TimeRange &range, AudioMarkerVector &out_markers) const;

	// AudioTimingController interface
	wxString GetWarningMessage() const;
	TimeRange GetIdealVisibleTimeRange() const;
	TimeRange GetPrimaryPlaybackRange() const;
	TimeRange GetActiveLineRange() const;
	void GetRenderingStyles(AudioRenderingStyleRanges &ranges) const;
	void GetLabels(TimeRange const& range, std::vector<AudioLabel> &out) const { }
	void Next(NextMode mode);
	void Prev();
	void Commit();
	void Revert();
	void AddLeadIn();
	void AddLeadOut();
	void ModifyLength(int delta, bool shift_following);
	void ModifyStart(int delta);
	bool IsNearbyMarker(int ms, int sensitivity) const;
	std::vector<AudioMarker*> OnLeftClick(int ms, bool ctrl_down, int sensitivity, int snap_range);
	std::vector<AudioMarker*> OnRightClick(int ms, bool, int sensitivity, int snap_range);
	void OnMarkerDrag(std::vector<AudioMarker*> const& markers, int new_position, int snap_range);

	/// Constructor
	/// @param c Project context
	AudioTimingControllerDialogue(agi::Context *c);
};

AudioTimingController *CreateDialogueTimingController(agi::Context *c)
{
	return new AudioTimingControllerDialogue(c);
}

AudioTimingControllerDialogue::AudioTimingControllerDialogue(agi::Context *c)
: style_left("Colour/Audio Display/Line boundary Start", "Audio/Line Boundaries Thickness")
, style_right("Colour/Audio Display/Line boundary End", "Audio/Line Boundaries Thickness")
, style_inactive("Colour/Audio Display/Line Boundary Inactive Line", "Audio/Line Boundaries Thickness")
, active_line(AudioStyle_Primary, &style_left, &style_right)
, keyframes_provider(c, "Audio/Display/Draw/Keyframes in Dialogue Mode")
, video_position_provider(c)
, commit_id(-1)
, context(c)
, auto_commit(OPT_GET("Audio/Auto/Commit"))
, inactive_line_mode(OPT_GET("Audio/Inactive Lines Display Mode"))
, inactive_line_comments(OPT_GET("Audio/Display/Draw/Inactive Comments"))
, drag_timing(OPT_GET("Audio/Drag Timing"))
, commit_connection(c->ass->AddCommitListener(&AudioTimingControllerDialogue::OnFileChanged, this))
, inactive_line_mode_connection(OPT_SUB("Audio/Inactive Lines Display Mode", &AudioTimingControllerDialogue::RegenerateInactiveLines, this))
, inactive_line_comment_connection(OPT_SUB("Audio/Display/Draw/Inactive Comments", &AudioTimingControllerDialogue::RegenerateInactiveLines, this))
, active_line_connection(c->selectionController->AddActiveLineListener(&AudioTimingControllerDialogue::OnActiveLineChanged, this))
, selection_connection(c->selectionController->AddSelectionListener(&AudioTimingControllerDialogue::OnSelectedSetChanged, this))
{
	keyframes_provider.AddMarkerMovedListener(std::bind(std::ref(AnnounceMarkerMoved)));
	video_position_provider.AddMarkerMovedListener(std::bind(std::ref(AnnounceMarkerMoved)));
	seconds_provider.AddMarkerMovedListener(std::bind(std::ref(AnnounceMarkerMoved)));

	Revert();
}

void AudioTimingControllerDialogue::GetMarkers(const TimeRange &range, AudioMarkerVector &out_markers) const
{
	// The order matters here; later markers are painted on top of earlier
	// markers, so the markers that we want to end up on top need to appear last

	seconds_provider.GetMarkers(range, out_markers);

	// Copy inactive line markers in the range
	copy(
		lower_bound(markers.begin(), markers.end(), range.begin(), marker_ptr_cmp()),
		upper_bound(markers.begin(), markers.end(), range.end(), marker_ptr_cmp()),
		back_inserter(out_markers));

	keyframes_provider.GetMarkers(range, out_markers);
	video_position_provider.GetMarkers(range, out_markers);
}

void AudioTimingControllerDialogue::OnActiveLineChanged(AssDialogue *new_line)
{
	Revert();
}

void AudioTimingControllerDialogue::OnSelectedSetChanged(SubtitleSelection const&, SubtitleSelection const&)
{
	RegenerateSelectedLines();
	RegenerateInactiveLines();
}

void AudioTimingControllerDialogue::OnFileChanged(int type) {
	if (type & AssFile::COMMIT_DIAG_TIME)
		Revert();
	else if (type & AssFile::COMMIT_DIAG_ADDREM)
		RegenerateInactiveLines();
}

wxString AudioTimingControllerDialogue::GetWarningMessage() const
{
	// We have no warning messages currently, maybe add the old "Modified" message back later?
	return wxString();
}

TimeRange AudioTimingControllerDialogue::GetIdealVisibleTimeRange() const
{
	return GetPrimaryPlaybackRange();
}

TimeRange AudioTimingControllerDialogue::GetPrimaryPlaybackRange() const
{
	return active_line;
}

TimeRange AudioTimingControllerDialogue::GetActiveLineRange() const
{
	return active_line;
}

void AudioTimingControllerDialogue::GetRenderingStyles(AudioRenderingStyleRanges &ranges) const
{
	active_line.GetStyleRange(&ranges);
	for_each(selected_lines.begin(), selected_lines.end(),
		std::bind(&TimeableLine::GetStyleRange, std::placeholders::_1, &ranges));
	for_each(inactive_lines.begin(), inactive_lines.end(),
		std::bind(&TimeableLine::GetStyleRange, std::placeholders::_1, &ranges));
}

void AudioTimingControllerDialogue::Next(NextMode mode)
{
	if (mode == TIMING_UNIT)
	{
		context->selectionController->NextLine();
		return;
	}

	int new_end_ms = *active_line.GetRightMarker();

	cmd::call("grid/line/next/create", context);

	if (mode == LINE_RESET_DEFAULT || active_line.GetLine()->End == 0) {
		const int default_duration = OPT_GET("Timing/Default Duration")->GetInt();
		// Setting right first here so that they don't get switched and the
		// same marker gets set twice
		active_line.GetRightMarker()->SetPosition(new_end_ms + default_duration);
		active_line.GetLeftMarker()->SetPosition(new_end_ms);
		sort(markers.begin(), markers.end(), marker_ptr_cmp());
		modified_lines.insert(&active_line);
		UpdateSelection();
	}
}

void AudioTimingControllerDialogue::Prev()
{
	context->selectionController->PrevLine();
}

void AudioTimingControllerDialogue::Commit()
{
	DoCommit(true);
}

void AudioTimingControllerDialogue::DoCommit(bool user_triggered)
{
	// Store back new times
	if (modified_lines.size())
	{
		for_each(modified_lines.begin(), modified_lines.end(),
			std::mem_fn(&TimeableLine::Apply));

		commit_connection.Block();
		if (user_triggered)
		{
			context->ass->Commit(_("timing"), AssFile::COMMIT_DIAG_TIME);
			commit_id = -1; // never coalesce with a manually triggered commit
		}
		else
		{
			AssDialogue *amend = modified_lines.size() == 1 ? (*modified_lines.begin())->GetLine() : 0;
			commit_id = context->ass->Commit(_("timing"), AssFile::COMMIT_DIAG_TIME, commit_id, amend);
		}

		commit_connection.Unblock();
		modified_lines.clear();
	}
}

void AudioTimingControllerDialogue::Revert()
{
	commit_id = -1;

	if (AssDialogue *line = context->selectionController->GetActiveLine())
	{
		modified_lines.clear();
		if (active_line.SetLine(line))
		{
			AnnounceUpdatedPrimaryRange();
			if (inactive_line_mode->GetInt() == 0)
				AnnounceUpdatedStyleRanges();
		}
		else
		{
			modified_lines.insert(&active_line);
		}
	}

	RegenerateInactiveLines();
	RegenerateSelectedLines();
}

void AudioTimingControllerDialogue::AddLeadIn()
{
	DialogueTimingMarker *m = active_line.GetLeftMarker();
	SetMarkers(std::vector<AudioMarker*>(1, m), *m - OPT_GET("Audio/Lead/IN")->GetInt());
}

void AudioTimingControllerDialogue::AddLeadOut()
{
	DialogueTimingMarker *m = active_line.GetRightMarker();
	SetMarkers(std::vector<AudioMarker*>(1, m), *m + OPT_GET("Audio/Lead/OUT")->GetInt());
}

void AudioTimingControllerDialogue::ModifyLength(int delta, bool) {
	DialogueTimingMarker *m = active_line.GetRightMarker();
	SetMarkers(std::vector<AudioMarker*>(1, m),
		std::max<int>(*m + delta * 10, *active_line.GetLeftMarker()));
}

void AudioTimingControllerDialogue::ModifyStart(int delta) {
	DialogueTimingMarker *m = active_line.GetLeftMarker();
	SetMarkers(std::vector<AudioMarker*>(1, m),
		std::min<int>(*m + delta * 10, *active_line.GetRightMarker()));
}

bool AudioTimingControllerDialogue::IsNearbyMarker(int ms, int sensitivity) const
{
	assert(sensitivity >= 0);
	return active_line.ContainsMarker(TimeRange(ms-sensitivity, ms+sensitivity));
}

std::vector<AudioMarker*> AudioTimingControllerDialogue::OnLeftClick(int ms, bool ctrl_down, int sensitivity, int snap_range)
{
	assert(sensitivity >= 0);
	assert(snap_range >= 0);

	std::vector<AudioMarker*> ret;

	DialogueTimingMarker *left = active_line.GetLeftMarker();
	DialogueTimingMarker *right = active_line.GetRightMarker();

	int dist_l = tabs(*left - ms);
	int dist_r = tabs(*right - ms);

	if (dist_l > sensitivity && dist_r > sensitivity)
	{
		// Clicked far from either marker:
		// Insta-set the left marker to the clicked position and return the
		// right as the dragged one, such that if the user does start dragging,
		// he will create a new selection from scratch
		std::vector<AudioMarker*> jump = GetLeftMarkers();
		ret = drag_timing->GetBool() ? GetRightMarkers() : jump;
		// Get ret before setting as setting may swap left/right
		SetMarkers(jump, SnapPosition(ms, snap_range, jump));
		return ret;
	}

	DialogueTimingMarker *clicked = dist_l <= dist_r ? left : right;

	if (ctrl_down)
	{
		// The use of GetPosition here is important, as otherwise it'll start
		// after lines ending at the same time as the active line begins
		auto it = lower_bound(markers.begin(), markers.end(), clicked->GetPosition(), marker_ptr_cmp());
		for(; it != markers.end() && !(*clicked < **it); ++it)
			ret.push_back(*it);
	}
	else
		ret.push_back(clicked);

	// Left-click within drag range should still move the left marker to the
	// clicked position, but not the right marker
	if (clicked == left)
		SetMarkers(ret, SnapPosition(ms, snap_range, ret));

	return ret;
}

std::vector<AudioMarker*> AudioTimingControllerDialogue::OnRightClick(int ms, bool, int sensitivity, int snap_range)
{
	std::vector<AudioMarker*> ret = GetRightMarkers();
	SetMarkers(ret, SnapPosition(ms, snap_range, ret));
	return ret;
}

void AudioTimingControllerDialogue::OnMarkerDrag(std::vector<AudioMarker*> const& markers, int new_position, int snap_range)
{
	SetMarkers(markers, SnapPosition(new_position, snap_range, markers));
}

void AudioTimingControllerDialogue::UpdateSelection()
{
	AnnounceUpdatedPrimaryRange();
	AnnounceUpdatedStyleRanges();
}

void AudioTimingControllerDialogue::SetMarkers(std::vector<AudioMarker*> const& upd_markers, int ms)
{
	// Since we're moving markers, the sorted list of markers will need to be
	// resorted. To avoid resorting the entire thing, find the subrange that
	// is effected.
	int min_ms = ms;
	int max_ms = ms;
	for (AudioMarker *upd_marker : upd_markers)
	{
		DialogueTimingMarker *marker = static_cast<DialogueTimingMarker*>(upd_marker);
		min_ms = std::min<int>(*marker, min_ms);
		max_ms = std::max<int>(*marker, max_ms);
	}

	auto begin = lower_bound(markers.begin(), markers.end(), min_ms, marker_ptr_cmp());
	auto end = upper_bound(begin, markers.end(), max_ms, marker_ptr_cmp());

	// Update the markers
	for (AudioMarker *upd_marker : upd_markers)
	{
		DialogueTimingMarker *marker = static_cast<DialogueTimingMarker*>(upd_marker);
		marker->SetPosition(ms);
		modified_lines.insert(marker->GetLine());
	}

	// Resort the range
	sort(begin, end, marker_ptr_cmp());

	if (auto_commit->GetBool()) DoCommit(false);
	UpdateSelection();

	AnnounceMarkerMoved();
}

static bool noncomment_dialogue(AssEntry const& e)
{
	if (const AssDialogue *diag = dynamic_cast<const AssDialogue*>(&e))
		return !diag->Comment;
	return false;
}

static bool dialogue(AssEntry const& e)
{
	return !!dynamic_cast<const AssDialogue*>(&e);
}

void AudioTimingControllerDialogue::RegenerateInactiveLines()
{
	bool (*predicate)(AssEntry const&) = inactive_line_comments->GetBool() ? dialogue : noncomment_dialogue;

	bool was_empty = inactive_lines.empty();
	inactive_lines.clear();

	SubtitleSelection sel = context->selectionController->GetSelectedSet();

	switch (int mode = inactive_line_mode->GetInt())
	{
	case 1: // Previous line only
	case 2: // Previous and next lines
		if (AssDialogue *line = context->selectionController->GetActiveLine())
		{
			entryIter current_line = context->ass->Line.iterator_to(*line);
			if (current_line == context->ass->Line.end())
				break;

			entryIter prev = current_line;
			while (--prev != context->ass->Line.begin() && !predicate(*prev)) ;
			if (prev != context->ass->Line.begin())
				AddInactiveLine(sel, static_cast<AssDialogue*>(&*prev));

			if (mode == 2)
			{
				entryIter next =
					find_if(++current_line, context->ass->Line.end(), predicate);
				if (next != context->ass->Line.end())
					AddInactiveLine(sel, static_cast<AssDialogue*>(&*next));
			}
		}
		break;
	case 3: // All inactive lines
	{
		AssDialogue *active_line = context->selectionController->GetActiveLine();
		for (auto& line : context->ass->Line)
		{
			if (&line != active_line && predicate(line))
				AddInactiveLine(sel, static_cast<AssDialogue*>(&line));
		}
		break;
	}
	default:
		if (was_empty)
		{
			RegenerateMarkers();
			return;
		}
	}

	AnnounceUpdatedStyleRanges();

	RegenerateMarkers();
}

void AudioTimingControllerDialogue::AddInactiveLine(SubtitleSelection const& sel, AssDialogue *diag)
{
	if (sel.count(diag)) return;

	inactive_lines.emplace_back(AudioStyle_Inactive, &style_inactive, &style_inactive);
	inactive_lines.back().SetLine(diag);
}

void AudioTimingControllerDialogue::RegenerateSelectedLines()
{
	bool was_empty = selected_lines.empty();
	selected_lines.clear();

	AssDialogue *active = context->selectionController->GetActiveLine();
	SubtitleSelection sel = context->selectionController->GetSelectedSet();
	for (auto line : sel)
	{
		if (line == active) continue;

		selected_lines.emplace_back(AudioStyle_Selected, &style_inactive, &style_inactive);
		selected_lines.back().SetLine(line);
	}

	if (!selected_lines.empty() || !was_empty)
	{
		AnnounceUpdatedStyleRanges();
		RegenerateMarkers();
	}
}

void AudioTimingControllerDialogue::RegenerateMarkers()
{
	markers.clear();

	active_line.GetMarkers(&markers);
	for_each(selected_lines.begin(), selected_lines.end(),
		std::bind(&TimeableLine::GetMarkers, std::placeholders::_1, &markers));
	for_each(inactive_lines.begin(), inactive_lines.end(),
		std::bind(&TimeableLine::GetMarkers, std::placeholders::_1, &markers));
	sort(markers.begin(), markers.end(), marker_ptr_cmp());

	AnnounceMarkerMoved();
}

std::vector<AudioMarker*> AudioTimingControllerDialogue::GetLeftMarkers()
{
	std::vector<AudioMarker*> ret;
	ret.reserve(selected_lines.size() + 1);
	ret.push_back(active_line.GetLeftMarker());
	transform(selected_lines.begin(), selected_lines.end(), back_inserter(ret),
		std::bind(&TimeableLine::GetLeftMarker, std::placeholders::_1));
	return ret;
}

std::vector<AudioMarker*> AudioTimingControllerDialogue::GetRightMarkers()
{
	std::vector<AudioMarker*> ret;
	ret.reserve(selected_lines.size() + 1);
	ret.push_back(active_line.GetRightMarker());
	transform(selected_lines.begin(), selected_lines.end(), back_inserter(ret),
		std::bind(&TimeableLine::GetRightMarker, std::placeholders::_1));
	return ret;
}

int AudioTimingControllerDialogue::SnapPosition(int position, int snap_range, std::vector<AudioMarker*> const& exclude) const
{
	if (position < 0)
		position = 0;

	if (snap_range <= 0)
		return position;

	TimeRange snap_time_range(position - snap_range, position + snap_range);
	const AudioMarker *snap_marker = 0;
	AudioMarkerVector potential_snaps;
	GetMarkers(snap_time_range, potential_snaps);
	for (auto marker : potential_snaps)
	{
		if (marker->CanSnap() && find(exclude.begin(), exclude.end(), marker) == exclude.end())
		{
			if (!snap_marker)
				snap_marker = marker;
			else if (tabs(marker->GetPosition() - position) < tabs(snap_marker->GetPosition() - position))
				snap_marker = marker;
		}
	}

	if (snap_marker)
		return snap_marker->GetPosition();
	return position;
}
