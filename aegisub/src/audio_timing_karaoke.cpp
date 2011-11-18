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
#include "audio_marker_provider_keyframes.h"
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
	int64_t position;
	Pen *pen;
	FeetStyle style;
public:

	int64_t GetPosition() const { return position; }
	wxPen GetStyle() const { return *pen; }
	FeetStyle GetFeet() const { return style; }
	bool CanSnap() const { return false; }

	void Move(int64_t new_pos) { position = new_pos; }

	KaraokeMarker(int64_t position, Pen *pen, FeetStyle style)
	: position(position)
	, pen(pen)
	, style(style)
	{
	}

	KaraokeMarker(int64_t position) : position(position) { }
	operator int64_t() const { return position; }
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

	/// Labels containing the stripped text of each syllable
	std::vector<AudioLabel> labels;

	bool auto_commit; ///< Should changes be automatically commited?
	bool auto_next;   ///< Should user-initiated commits automatically go to the next?
	int commit_id;    ///< Last commit id used for an autocommit

	void OnAutoCommitChange(agi::OptionValue const& opt);
	void OnAutoNextChange(agi::OptionValue const& opt);

	int64_t ToMS(int64_t samples) const { return c->audioController->MillisecondsFromSamples(samples); }
	int64_t ToSamples(int64_t ms) const { return c->audioController->SamplesFromMilliseconds(ms); }

	void DoCommit();

public:
	// AudioTimingController implementation
	void GetMarkers(const SampleRange &range, AudioMarkerVector &out_markers) const;
	wxString GetWarningMessage() const { return ""; }
	SampleRange GetIdealVisibleSampleRange() const;
	void GetRenderingStyles(AudioRenderingStyleRanges &ranges) const;
	SampleRange GetPrimaryPlaybackRange() const;
	bool HasLabels() const { return true; }
	void GetLabels(const SampleRange &range, std::vector<AudioLabel> &out_labels) const;
	void Next();
	void Prev();
	void Commit();
	void Revert();
	bool IsNearbyMarker(int64_t sample, int sensitivity) const;
	AudioMarker * OnLeftClick(int64_t sample, int sensitivity);
	AudioMarker * OnRightClick(int64_t sample, int sensitivity) { return 0; }
	void OnMarkerDrag(AudioMarker *marker, int64_t new_position);

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
, start_marker(ToSamples(active_line->Start.GetMS()), &start_pen, AudioMarker::Feet_Right)
, end_marker(ToSamples(active_line->End.GetMS()), &end_pen, AudioMarker::Feet_Left)
, keyframes_provider(c, "Audio/Display/Draw/Keyframes in Karaoke Mode")
, auto_commit(OPT_GET("Audio/Auto/Commit")->GetBool())
, auto_next(OPT_GET("Audio/Next Line on Commit")->GetBool())
, commit_id(-1)
{
	slots.push_back(kara->AddSyllablesChangedListener(&AudioTimingControllerKaraoke::Revert, this));
	slots.push_back(OPT_SUB("Audio/Auto/Commit", &AudioTimingControllerKaraoke::OnAutoCommitChange, this));
	slots.push_back(OPT_SUB("Audio/Next Line on Commit", &AudioTimingControllerKaraoke::OnAutoNextChange, this));

	keyframes_provider.AddMarkerMovedListener(std::tr1::bind(std::tr1::ref(AnnounceMarkerMoved)));

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
}

void AudioTimingControllerKaraoke::GetRenderingStyles(AudioRenderingStyleRanges &ranges) const
{
	SampleRange sr = GetPrimaryPlaybackRange();
	ranges.AddRange(sr.begin(), sr.end(), AudioStyle_Selected);
}

SampleRange AudioTimingControllerKaraoke::GetPrimaryPlaybackRange() const {
	return SampleRange(
		cur_syl > 0 ? markers[cur_syl - 1] : start_marker,
		cur_syl < markers.size() ? markers[cur_syl] : end_marker);
}

SampleRange AudioTimingControllerKaraoke::GetIdealVisibleSampleRange() const {
	return SampleRange(start_marker, end_marker);
}

void AudioTimingControllerKaraoke::GetMarkers(SampleRange const& range, AudioMarkerVector &out) const {
	size_t i;
	for (i = 0; i < markers.size() && markers[i] < range.begin(); ++i) ;
	for (; i < markers.size() && markers[i] < range.end(); ++i)
		out.push_back(&markers[i]);

	if (range.contains(start_marker)) out.push_back(&start_marker);
	if (range.contains(end_marker)) out.push_back(&end_marker);

	keyframes_provider.GetMarkers(range, out);
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

	start_marker.Move(ToSamples(active_line->Start.GetMS()));
	end_marker.Move(ToSamples(active_line->End.GetMS()));

	markers.clear();
	labels.clear();

	markers.reserve(kara->size());
	labels.reserve(kara->size());

	for (AssKaraoke::iterator it = kara->begin(); it != kara->end(); ++it) {
		int64_t sample = ToSamples(it->start_time);
		if (it != kara->begin())
			markers.push_back(KaraokeMarker(sample, &separator_pen, AudioMarker::Feet_None));
		labels.push_back(AudioLabel(it->text, SampleRange(sample, ToSamples(it->start_time + it->duration))));
	}

	AnnounceUpdatedPrimaryRange();
	AnnounceUpdatedStyleRanges();
	AnnounceMarkerMoved();
}

bool AudioTimingControllerKaraoke::IsNearbyMarker(int64_t sample, int sensitivity) const {
	SampleRange range(sample - sensitivity, sample + sensitivity);

	for (size_t i = 0; i < markers.size(); ++i)
		if (range.contains(markers[i]))
			return true;

	return false;
}

AudioMarker *AudioTimingControllerKaraoke::OnLeftClick(int64_t sample, int sensitivity) {
	SampleRange range(sample - sensitivity, sample + sensitivity);

	size_t syl = distance(markers.begin(), lower_bound(markers.begin(), markers.end(), sample));
	if (syl < markers.size() && range.contains(markers[syl]))
		return &markers[syl];
	if (syl > 0 && range.contains(markers[syl - 1]))
		return &markers[syl - 1];

	cur_syl = syl;

	AnnounceUpdatedPrimaryRange();
	AnnounceUpdatedStyleRanges();

	return 0;
}

void AudioTimingControllerKaraoke::OnMarkerDrag(AudioMarker *m, int64_t new_position) {
	KaraokeMarker *marker = static_cast<KaraokeMarker*>(m);
	// No rearranging of syllables allowed
	new_position = mid(
		marker == &markers.front() ? start_marker.GetPosition() : (marker - 1)->GetPosition(),
		new_position,
		marker == &markers.back() ? end_marker.GetPosition() : (marker + 1)->GetPosition());

	marker->Move(new_position);

	size_t syl = marker - &markers.front() + 1;
	kara->SetStartTime(syl, ToMS(new_position));

	if (syl == cur_syl || syl + 1 == cur_syl) {
		AnnounceUpdatedPrimaryRange();
		AnnounceUpdatedStyleRanges();
	}

	AnnounceMarkerMoved();

	labels[syl - 1].range = SampleRange(labels[syl - 1].range.begin(), new_position);
	labels[syl].range = SampleRange(new_position, labels[syl].range.end());
	AnnounceLabelChanged();

	if (auto_commit)
		DoCommit();
	else
		commit_id = -1;
}

void AudioTimingControllerKaraoke::GetLabels(SampleRange const& range, std::vector<AudioLabel> &out) const {
	for (size_t i = 0; i < labels.size(); ++i) {
		if (range.overlaps(labels[i].range))
			out.push_back(labels[i]);
	}
}
