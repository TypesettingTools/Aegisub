// Copyright (c) 2022, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace agi::ass {
/// Parsed syllable data
struct KaraokeSyllable {
	int start_time; ///< Start time relative to time zero (not line start) in milliseconds
	int duration;   ///< Duration in milliseconds
	std::string text; ///< Stripped syllable text
	std::string tag_type; ///< \k, \kf or \ko
	/// Non-karaoke override tags in this syllable. Key is an index in text
	/// before which the value should be inserted
	std::map<size_t, std::string> ovr_tags;

	/// Get the text of this line with override tags and optionally the karaoke tag
	std::string GetText(bool k_tag) const;

	friend bool operator==(KaraokeSyllable const&, KaraokeSyllable const&) = default;
};

class Karaoke {
	std::vector<KaraokeSyllable> syls;
	agi::signal::Signal<> AnnounceSyllablesChanged;

	void DoAddSplit(size_t syl_idx, size_t pos);
	/// Normalize the syllables so that the total duration is equal to the line length
	void Normalize(int end_time);

	/// Add karaoke splits at each space
	void AutoSplit();

public:
	void SetLine(std::vector<KaraokeSyllable>&& syls, bool auto_split, std::optional<int> end_time);

	/// Add a split before character pos in syllable syl_idx
	void AddSplit(size_t syl_idx, size_t pos);
	/// Remove the split at the given index
	void RemoveSplit(size_t syl_idx);
	/// Set the start time of a syllable in ms
	void SetStartTime(size_t syl_idx, int time);
	/// Adjust the line's start and end times without shifting the syllables
	void SetLineTimes(int start_time, int end_time);

	using iterator = std::vector<KaraokeSyllable>::const_iterator;
	iterator begin() const { return syls.begin(); }
	iterator end() const { return syls.end(); }
	size_t size() const { return syls.size(); }

	/// Get the line's text with k tags
	std::string GetText() const;

	/// Get the karaoke tag type used, with leading slash
	/// @returns "\k", "\kf", or "\ko"
	std::string_view GetTagType() const;
	/// Set the tag type for all karaoke tags in this line
	void SetTagType(std::string_view new_type);

	DEFINE_SIGNAL_ADDERS(AnnounceSyllablesChanged, AddSyllablesChangedListener)
};

} // namespace agi::ass
