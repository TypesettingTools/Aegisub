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

/// @file audio_timing_karaoke.cpp
/// @brief Timing mode for karaoke
/// @ingroup audio_ui
///

#include "config.h"

#include <libaegisub/signal.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_karaoke.h"
#include "audio_controller.h"
#include "audio_renderer.h"
#include "audio_timing.h"
#include "include/aegisub/context.h"
#include "main.h"
#include "pen.h"
#include "utils.h"

#ifndef AGI_PRE
#include <deque>
#endif

/// @class KaraokeMarker
/// @brief AudioMarker implementation for AudioTimingControllerKaraoke
class KaraokeMarker : public AudioMarker {
	int position;
	Pen *pen;
	FeetStyle style;
public:

	int GetPosition() const { return position; }
	wxPen GetStyle() const { return *pen; }
	FeetStyle GetFeet() const { return style; }
	bool CanSnap() const { return false; }

	void Move(int new_pos) { position = new_pos; }

	KaraokeMarker(int position, Pen *pen, FeetStyle style)
	: position(position)
	, pen(pen)
	, style(style)
	{
	}

	KaraokeMarker(int position) : position(position) { }
	operator int() const { return position; }
};

/// @class AudioTimingControllerKaraoke
/// @brief Karaoke timing mode for timing subtitles
///
/// Displays the active line with draggable markers between each pair of
/// adjacent syllables, along with the text of each syllable.
///
/// This does not support \kt, as it inherently requires that the end time of
/// one syllable be the same as the start time of the next one.
class AudioTimingControllerKaraoke : public AudioTimingController {
	std::deque<agi::signal::Connection> slots;
	agi::signal::Connection& file_changed_slot;

	agi::Context *c;          ///< Project context
	AssDialogue *active_line; ///< Currently active line
	AssKaraoke *kara;         ///< Parsed karaoke model provided by karaoke controller

	size_t cur_syl; ///< Index of currently selected syllable in the line

	/// Pen used for the mid-syllable markers
	Pen separator_pen;
	/// Pen used for the start-of-line marker
	Pen start_pen;
	/// Pen used for the end-of-line marker
	Pen end_pen;

	/// Immobile marker for the beginning of the line
	KaraokeMarker start_marker;
	/// Immobile marker for the end of the line
	KaraokeMarker end_marker;
	/// Mobile markers between each pair of syllables
	std::vector<KaraokeMarker> markers;

	/// Marker provider for video keyframes
	AudioMarkerProviderKeyframes keyframes_provider;

	/// Marker provider for video playback position
	VideoPositionMarkerProvider video_position_provider;

	/// Labels containing the stripped text of each syllable
	std::vector<AudioLabel> labels;

	bool auto_commit; ///< Should changes be automatically commited?
	bool auto_next;   ///< Should user-initiated commits automatically go to the next?
	int commit_id;    ///< Last commit id used for an autocommit

	void OnAutoCommitChange(agi::OptionValue const& opt);
	void OnAutoNextChange(agi::OptionValue const& opt);

	void DoCommit();
	void ApplyLead(bool announce_primary);

public:
	// AudioTimingController implementation
	void GetMarkers(const TimeRange &range, AudioMarkerVector &out_markers) const;
	wxString GetWarningMessage() const { return ""; }
	TimeRange GetIdealVisibleTimeRange() const;
	void GetRenderingStyles(AudioRenderingStyleRanges &ranges) const;
	TimeRange GetPrimaryPlaybackRange() const;
	void GetLabels(const TimeRange &range, std::vector<AudioLabel> &out_labels) const;
	void Next();
	void Prev();
	void Commit();
	void Revert();
	void AddLeadIn();
	void AddLeadOut();
	bool IsNearbyMarker(int ms, int sensitivity) const;
	std::vector<AudioMarker*> OnLeftClick(int ms, bool, int sensitivity, int);
	std::vector<AudioMarker*> OnRightClick(int, bool, int, int) { return std::vector<AudioMarker*>(); }
	void OnMarkerDrag(std::vector<AudioMarker*> const& marker, int new_position, int);

	AudioTimingControllerKaraoke(agi::Context *c, AssKaraoke *kara, agi::signal::Connection& file_changed);
};

AudioTimingController *CreateKaraokeTimingController(agi::Context *c, AssKaraoke *kara, agi::signal::Connection& file_changed)
{
	return new AudioTimingControllerKaraoke(c, kara, file_changed);
}

AudioTimingControllerKaraoke::AudioTimingControllerKaraoke(agi::Context *c, AssKaraoke *kara, agi::signal::Connection& file_changed)
: file_changed_slot(file_changed)
, c(c)
, active_line(c->selectionController->GetActiveLine())
, kara(kara)
, cur_syl(0)
, separator_pen("Colour/Audio Display/Syllable Boundaries", "Audio/Line Boundaries Thickness", wxPENSTYLE_DOT)
, start_pen("Colour/Audio Display/Line boundary Start", "Audio/Line Boundaries Thickness")
, end_pen("Colour/Audio Display/Line boundary End", "Audio/Line Boundaries Thickness")
, start_marker(active_line->Start, &start_pen, AudioMarker::Feet_Right)
, end_marker(active_line->End, &end_pen, AudioMarker::Feet_Left)
, keyframes_provider(c, "Audio/Display/Draw/Keyframes in Karaoke Mode")
, video_position_provider(c)
, auto_commit(OPT_GET("Audio/Auto/Commit")->GetBool())
, auto_next(OPT_GET("Audio/Next Line on Commit")->GetBool())
, commit_id(-1)
{
	slots.push_back(kara->AddSyllablesChangedListener(&AudioTimingControllerKaraoke::Revert, this));
	slots.push_back(OPT_SUB("Audio/Auto/Commit", &AudioTimingControllerKaraoke::OnAutoCommitChange, this));
	slots.push_back(OPT_SUB("Audio/Next Line on Commit", &AudioTimingControllerKaraoke::OnAutoNextChange, this));

	keyframes_provider.AddMarkerMovedListener(std::tr1::bind(std::tr1::ref(AnnounceMarkerMoved)));
	video_position_provider.AddMarkerMovedListener(std::tr1::bind(std::tr1::ref(AnnounceMarkerMoved)));

	Revert();

}

void AudioTimingControllerKaraoke::OnAutoCommitChange(agi::OptionValue const& opt) {
	auto_commit = opt.GetBool();
}

void AudioTimingControllerKaraoke::OnAutoNextChange(agi::OptionValue const& opt) {
	auto_next = opt.GetBool();
}

void AudioTimingControllerKaraoke::Next() {
	++cur_syl;
	if (cur_syl > markers.size()) {
		--cur_syl;
		c->selectionController->NextLine();
	}
	else {
		AnnounceUpdatedPrimaryRange();
		AnnounceUpdatedStyleRanges();
	}

	c->audioController->PlayPrimaryRange();
}

void AudioTimingControllerKaraoke::Prev() {
	if (cur_syl == 0) {
		AssDialogue *old_line = active_line;
		c->selectionController->PrevLine();
		if (old_line != active_line) {
			cur_syl = markers.size();
			AnnounceUpdatedPrimaryRange();
			AnnounceUpdatedStyleRanges();
		}
	}
	else {
		--cur_syl;
		AnnounceUpdatedPrimaryRange();
		AnnounceUpdatedStyleRanges();
	}

	c->audioController->PlayPrimaryRange();
}

void AudioTimingControllerKaraoke::GetRenderingStyles(AudioRenderingStyleRanges &ranges) const
{
	TimeRange sr = GetPrimaryPlaybackRange();
	ranges.AddRange(sr.begin(), sr.end(), AudioStyle_Primary);
	ranges.AddRange(start_marker, end_marker, AudioStyle_Selected);
}

TimeRange AudioTimingControllerKaraoke::GetPrimaryPlaybackRange() const {
	return TimeRange(
		cur_syl > 0 ? markers[cur_syl - 1] : start_marker,
		cur_syl < markers.size() ? markers[cur_syl] : end_marker);
}

TimeRange AudioTimingControllerKaraoke::GetIdealVisibleTimeRange() const {
	return TimeRange(start_marker, end_marker);
}

void AudioTimingControllerKaraoke::GetMarkers(TimeRange const& range, AudioMarkerVector &out) const {
	size_t i;
	for (i = 0; i < markers.size() && markers[i] < range.begin(); ++i) ;
	for (; i < markers.size() && markers[i] < range.end(); ++i)
		out.push_back(&markers[i]);

	if (range.contains(start_marker)) out.push_back(&start_marker);
	if (range.contains(end_marker)) out.push_back(&end_marker);

	keyframes_provider.GetMarkers(range, out);
	video_position_provider.GetMarkers(range, out);
}

void AudioTimingControllerKaraoke::DoCommit() {
	active_line->Text = kara->GetText();
	file_changed_slot.Block();
	commit_id = c->ass->Commit(_("karaoke timing"), AssFile::COMMIT_DIAG_TEXT, commit_id, active_line);
	file_changed_slot.Unblock();
}

void AudioTimingControllerKaraoke::Commit() {
	if (!auto_commit)
		DoCommit();
	if (auto_next)
		c->selectionController->NextLine();
}

void AudioTimingControllerKaraoke::Revert() {
	active_line = c->selectionController->GetActiveLine();

	cur_syl = 0;
	commit_id = -1;

	start_marker.Move(active_line->Start);
	end_marker.Move(active_line->End);

	markers.clear();
	labels.clear();

	markers.reserve(kara->size());
	labels.reserve(kara->size());

	for (AssKaraoke::iterator it = kara->begin(); it != kara->end(); ++it) {
		if (it != kara->begin())
			markers.push_back(KaraokeMarker(it->start_time, &separator_pen, AudioMarker::Feet_None));
		labels.push_back(AudioLabel(it->text, TimeRange(it->start_time, it->start_time + it->duration)));
	}

	AnnounceUpdatedPrimaryRange();
	AnnounceUpdatedStyleRanges();
	AnnounceMarkerMoved();
}

void AudioTimingControllerKaraoke::AddLeadIn() {
	start_marker.Move(start_marker - OPT_GET("Audio/Lead/IN")->GetInt());
	labels.front().range = TimeRange(start_marker, labels.front().range.end());
	ApplyLead(cur_syl == 0);
}

void AudioTimingControllerKaraoke::AddLeadOut() {
	end_marker.Move(end_marker + OPT_GET("Audio/Lead/OUT")->GetInt());
	labels.back().range = TimeRange(labels.back().range.begin(), end_marker);
	ApplyLead(cur_syl == markers.size());
}

void AudioTimingControllerKaraoke::ApplyLead(bool announce_primary) {
	kara->SetLineTimes(start_marker, end_marker);

	AnnounceUpdatedStyleRanges();
	AnnounceMarkerMoved();
	if (announce_primary)
		AnnounceUpdatedPrimaryRange();

	if (auto_commit)
		DoCommit();
	else
		commit_id = -1;
}

bool AudioTimingControllerKaraoke::IsNearbyMarker(int ms, int sensitivity) const {
	TimeRange range(ms - sensitivity, ms + sensitivity);

	for (size_t i = 0; i < markers.size(); ++i)
		if (range.contains(markers[i]))
			return true;

	return false;
}

std::vector<AudioMarker*> AudioTimingControllerKaraoke::OnLeftClick(int ms, bool, int sensitivity, int) {
	TimeRange range(ms - sensitivity, ms + sensitivity);

	size_t syl = distance(markers.begin(), lower_bound(markers.begin(), markers.end(), ms));
	if (syl < markers.size() && range.contains(markers[syl]))
		return std::vector<AudioMarker*>(1, &markers[syl]);
	if (syl > 0 && range.contains(markers[syl - 1]))
		return std::vector<AudioMarker*>(1, &markers[syl - 1]);

	cur_syl = syl;

	AnnounceUpdatedPrimaryRange();
	AnnounceUpdatedStyleRanges();

	return std::vector<AudioMarker*>();
}

void AudioTimingControllerKaraoke::OnMarkerDrag(std::vector<AudioMarker*> const& m, int new_position, int) {
	assert(m.size() == 1);
	KaraokeMarker *marker = static_cast<KaraokeMarker*>(m[0]);
	// No rearranging of syllables allowed
	new_position = mid(
		marker == &markers.front() ? start_marker.GetPosition() : (marker - 1)->GetPosition(),
		new_position,
		marker == &markers.back() ? end_marker.GetPosition() : (marker + 1)->GetPosition());

	marker->Move(new_position);

	size_t syl = marker - &markers.front() + 1;
	kara->SetStartTime(syl, new_position);

	if (syl == cur_syl || syl == cur_syl + 1) {
		AnnounceUpdatedPrimaryRange();
		AnnounceUpdatedStyleRanges();
	}

	AnnounceMarkerMoved();

	labels[syl - 1].range = TimeRange(labels[syl - 1].range.begin(), new_position);
	labels[syl].range = TimeRange(new_position, labels[syl].range.end());
	AnnounceLabelChanged();

	if (auto_commit)
		DoCommit();
	else
		commit_id = -1;
}

void AudioTimingControllerKaraoke::GetLabels(TimeRange const& range, std::vector<AudioLabel> &out) const {
	for (size_t i = 0; i < labels.size(); ++i) {
		if (range.overlaps(labels[i].range))
			out.push_back(labels[i]);
	}
}
