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

#include "libaegisub/karaoke_matcher.h"

#include "libaegisub/ass/karaoke.h"
#include "libaegisub/exception.h"
#include "libaegisub/kana_table.h"
#include "libaegisub/scoped_ptr.h"
#include "libaegisub/util.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <unicode/brkiter.h>
#include <unicode/coll.h>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace {
int32_t next_codepoint(const char *str, size_t *i) {
	UChar32 c;
	U8_NEXT_UNSAFE(str, *i, c);
	return c;
}

bool is_whitespace(int32_t c) {
	return !!u_isUWhiteSpace(c);
}

bool is_whitespace(std::string_view str) {
	size_t i = 0;
	while (i < str.size()) {
		UChar32 c;
		U8_NEXT(str.data(), i, str.size(), c);
		if (!u_isUWhiteSpace(c)) return false;
	}
	return true;
}

// strcmp but ignoring case and accents
int compare(std::string_view a, std::string_view b) {
	UErrorCode err = U_ZERO_ERROR;
	thread_local std::unique_ptr<icu::Collator> collator(icu::Collator::createInstance(err));
	collator->setStrength(icu::Collator::PRIMARY);
	int result = collator->compareUTF8(a, b, err);
	if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));
	return result;
}
} // namespace

agi::KaraokeMatchResult agi::AutoMatchKaraoke(std::vector<std::string_view> const& source_strings,
                                              std::string_view dest_string) {
	KaraokeMatchResult result = {0, 0};
	if (source_strings.empty()) return result;
	result.source_length = 1;

	// Constructing an icu::BreakIterator is very expensive, so reuse them between calls
	thread_local BreakIterator dst, src;
	dst.set_text(dest_string);
	std::string src_str(source_strings[0]);
	boost::to_lower(src_str);
	src.set_text(src_str);

	// Eat all the whitespace at the beginning of the source and destination
	// syllables and exit if either ran out.
	auto eat_whitespace = [&]() -> bool {
		while (!src.done() && is_whitespace(src.current()))
			src.next();
		while (!dst.done() && is_whitespace(dst.current())) {
			dst.next();
			++result.destination_length;
		}

		// If we ran out of dest then this needs to match the rest of the
		// source syllables (this probably means the user did something wrong)
		if (dst.done()) {
			result.source_length = source_strings.size();
			return true;
		}

		return src.done();
	};

	if (eat_whitespace()) return result;

	// We now have a non-whitespace character at the beginning of both source
	// and destination. Check if the source starts with a romanized kana, and
	// if it does then check if the destination also has the appropriate
	// character. If it does, match them and repeat.
	while (!src.done()) {
		// First check for a basic match of the start of the source and dest
		// Checking for src/dst done is handled by eat_whitespace()
		while (compare(src.current(), dst.current()) == 0) {
			src.next();
			dst.next();
			++result.destination_length;
			if (eat_whitespace()) return result;
		}

		// Check if the start of src is now a known romaji, and if so check
		// if dst starts with any of the possible corresponding kana
		bool matched = false;
		for (auto [kana, romaji] : romaji_to_kana(src.current_to_end())) {
			if (!dst.current_to_end().starts_with(kana)) continue;

			// romaji is always one byte per character
			for (size_t i = 0; i < romaji.size(); ++i)
				src.next();
			// FIXME: Valid for all locales?
			size_t count = kana.size() / 3;
			result.destination_length += count;
			for (size_t i = 0; i < count; ++i)
				dst.next();

			if (eat_whitespace()) return result;
			matched = true;
			break;
		}
		if (!matched) break;
	}

	// Source and dest are now non-empty and start with non-whitespace.
	// If there's only one character left in the dest, it obviously needs to
	// match all of the source syllables left.
	if (dst.is_last()) {
		result.source_length = source_strings.size();
		++result.destination_length;
		return result;
	}

	// We couldn't match the current character, but if we can match the *next*
	// syllable then we know that everything in between must belong to the
	// current syllable. Do this by looking up to KANA_SEARCH_DISTANCE
	// characters ahead in destination and seeing if we can match them against
	// the beginning of a syllable after this syllable.
	// If a match is found, make a guess at how much source and destination
	// should be selected based on the distances it was found at.

	// The longest kanji are 'uketamawa.ru' and 'kokorozashi', each with a
	// reading consisting of five kana. This means each each character from
	// the destination can match at most five syllables from the source.
	static const int max_character_length = 5;

	// Arbitrarily chosen limit on the number of dest characters to try
	// skipping. Higher numbers probably increase false-positives.
	static const int dst_lookahead_max = 3;

	for (size_t lookahead = 0; lookahead < dst_lookahead_max; ++lookahead) {
		dst.next();
		if (dst.done()) break;
		auto cur = dst.current();

		// Transliterate this character if it's a known hiragana or katakana character
		// Kana can be either one or two characters long, and both characters must
		// be exactly 3 bytes long and start with 0xE3.
		if (cur.size() != 3 || cur[0] != '\xE3') continue;

		std::vector<std::string_view> translit;
		if (dest_string.size() >= cur.data() - dest_string.data() + 6)
			boost::copy(kana_to_romaji(std::string_view(cur.data(), 6)), back_inserter(translit));
		boost::copy(kana_to_romaji(dst.current()), back_inserter(translit));

		// Search for it and the transliterated version in the source
		size_t src_lookahead_max = (lookahead + 1) * max_character_length;
		size_t src_lookahead_pos = 0;
		for (auto const& syl : source_strings) {
			// Don't count blank syllables in the max search distance
			if (is_whitespace(syl)) continue;
			if (++src_lookahead_pos == 1) continue;
			if (src_lookahead_pos > src_lookahead_max) break;

			std::string lsyl(syl);
			boost::to_lower(lsyl);
			if (!(syl.starts_with(cur) || util::any_of(translit, [&](auto s) { return lsyl.starts_with(s); })))
				continue;

			// The syllable immediately after the current one matched, so
			// everything up to the match must go with the current syllable.
			if (src_lookahead_pos == 2) {
				result.destination_length += lookahead + 1;
				return result;
			}

			// The match was multiple syllables ahead, so just divide the
			// destination characters evenly between the source syllables
			result.destination_length += 1;
			result.source_length = static_cast<size_t>((src_lookahead_pos - 1.0) / (lookahead + 1.0) + .5);
			return result;
		}
	}

	// We wouldn't have gotten here if the dest was empty, so make sure at
	// least one character is selected
	result.destination_length = std::max<size_t>(result.destination_length, 1u);

	return result;
}

namespace agi {
void KaraokeMatcher::SetInputData(std::vector<ass::KaraokeSyllable>&& src, std::string&& dst) {
	syllables = std::move(src);
	matched_groups.clear();
	character_positions.clear();
	src_start = 0;
	src_len = syllables.size() ? 1 : 0;

	destination_str = std::move(dst);
	bi.set_text(destination_str);
	for (; !bi.done(); bi.next())
		character_positions.push_back(bi.current().data() - destination_str.data());
	character_positions.push_back(destination_str.size());
	dst_start = 0;
	dst_len = character_positions.size() > 1;
}

std::string KaraokeMatcher::GetOutputLine() const {
	std::string res;

	for (auto const& match : matched_groups) {
		int duration = 0;
		for (auto const& syl : match.src)
			duration += syl.duration;
		res += "{\\k" + std::to_string(duration / 10) + "}" + std::string(match.dst);
	}

	return res;
}

bool KaraokeMatcher::IncreaseSourceMatch() {
	if (src_start + src_len < syllables.size()) {
		++src_len;
		return true;
	}
	return false;
}

bool KaraokeMatcher::DecreaseSourceMatch() {
	if (src_len > 0) {
		--src_len;
		return true;
	}
	return false;
}

bool KaraokeMatcher::IncreaseDestinationMatch() {
	// +1 because there's one more entry in character_positions than there are
	// characters
	if (dst_start + dst_len + 1 < character_positions.size()) {
		++dst_len;
		return true;
	}
	return false;
}

bool KaraokeMatcher::DecreaseDestinationMatch() {
	if (dst_len > 0) {
		--dst_len;
		return true;
	}
	return false;
}

void KaraokeMatcher::AutoMatchJapanese() {
	std::vector<std::string_view> source;
	for (size_t i = src_start; i < syllables.size(); ++i)
		source.emplace_back(syllables[i].text);
	size_t dst_pos = character_positions[dst_start];
	auto [src, dst] = AutoMatchKaraoke(source, std::string_view(destination_str).substr(dst_pos));
	src_len = src;
	dst_len = dst;
}

bool KaraokeMatcher::AcceptMatch() {
	// Completely empty match
	if (src_len == 0 && dst_len == 0) return false;

	matched_groups.push_back({CurrentSourceSelection(), CurrentDestinationSelection()});

	src_start += src_len;
	dst_start += dst_len;
	src_len = 0;
	dst_len = 0;

	IncreaseSourceMatch();
	IncreaseDestinationMatch();

	return true;
}

bool KaraokeMatcher::UndoMatch() {
	if (matched_groups.empty()) return false;

	MatchGroup& group = matched_groups.back();
	src_start = group.src.data() - syllables.data();
	src_len = group.src.size();

	auto dst_pos = group.dst.data() - destination_str.data();
	auto it = std::find(character_positions.begin(), character_positions.end(), dst_pos);
	assert(it != character_positions.end());
	dst_start = it - character_positions.begin();

	dst_pos = group.dst.data() + group.dst.size() - destination_str.data();
	auto it2 = std::find(character_positions.begin(), character_positions.end(), dst_pos);
	assert(it2 != character_positions.end());
	dst_len = it2 - it;

	matched_groups.pop_back();

	return true;
}

std::span<const ass::KaraokeSyllable> KaraokeMatcher::CurrentSourceSelection() const {
	return std::span(syllables).subspan(src_start, src_len);
}
std::span<const ass::KaraokeSyllable> KaraokeMatcher::UnmatchedSource() const {
	return std::span(syllables).subspan(src_start + src_len);
}
std::string_view KaraokeMatcher::CurrentDestinationSelection() const {
	return std::string_view(destination_str)
	    .substr(character_positions[dst_start],
	            character_positions[dst_start + dst_len] - character_positions[dst_start]);
}
std::string_view KaraokeMatcher::UnmatchedDestination() const {
	return std::string_view(destination_str).substr(character_positions[dst_start + dst_len]);
}
} // namespace agi
