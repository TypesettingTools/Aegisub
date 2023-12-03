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

#include <libaegisub/ass/karaoke.h>
#include <libaegisub/unicode.h>

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace agi {

struct KaraokeMatchResult {
	/// The number of strings in the source matched
	size_t source_length;
	/// The number of characters in the destination string matched
	size_t destination_length;

	bool operator==(KaraokeMatchResult const&) const = default;
};

/// Try to automatically select the portion of dst which corresponds to the first string in src
KaraokeMatchResult AutoMatchKaraoke(std::vector<std::string_view> const& src, std::string_view dst);

class KaraokeMatcher {
public:
	struct MatchGroup {
		std::span<const ass::KaraokeSyllable> src;
		std::string_view dst;
	};

private:
	std::vector<ass::KaraokeSyllable> syllables;
	std::string destination_str;

	std::vector<MatchGroup> matched_groups;

	std::vector<size_t> character_positions;
	size_t src_start = 0, dst_start = 0;
	size_t src_len = 0, dst_len = 0;

	agi::BreakIterator bi;

public:
	/// Start processing a new line pair
	void SetInputData(std::vector<ass::KaraokeSyllable>&& src, std::string&& dst);
	/// Build and return the output line from the matched syllables
	std::string GetOutputLine() const;

	std::span<const MatchGroup> MatchedGroups() const { return matched_groups; }
	std::span<const ass::KaraokeSyllable> CurrentSourceSelection() const;
	std::span<const ass::KaraokeSyllable> UnmatchedSource() const;
	std::string_view CurrentDestinationSelection() const;
	std::string_view UnmatchedDestination() const;

	// Adjust source and destination match lengths
	bool IncreaseSourceMatch();
	bool DecreaseSourceMatch();
	bool IncreaseDestinationMatch();
	bool DecreaseDestinationMatch();
	/// Attempt to treat source as Japanese romaji, destination as Japanese kana+kanji, and make an automatic match
	void AutoMatchJapanese();
	/// Accept current selection and save match
	bool AcceptMatch();
	/// Undo last match, adding it back to the unmatched input
	bool UndoMatch();
};
} // namespace agi
