// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "libaegisub/kana_table.h"
#include "libaegisub/util.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/locale/boundary.hpp>
#include <boost/locale/collator.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace {
int32_t next_codepoint(const char* str, size_t* i) {
	UChar32 c;
	U8_NEXT_UNSAFE(str, *i, c);
	return c;
}

bool is_whitespace(int32_t c) {
	return !!u_isUWhiteSpace(c);
}

bool is_whitespace(std::string const& str) {
	size_t i = 0;
	while(auto c = next_codepoint(str.c_str(), &i)) {
		if(!u_isUWhiteSpace(c)) return false;
	}
	return true;
}

// strcmp but ignoring case and accents
int compare(std::string const& a, std::string const& b) {
	using namespace boost::locale;
	return std::use_facet<collator<char>>(std::locale()).compare(collator_base::primary, a, b);
}

} // namespace

namespace agi {

karaoke_match_result auto_match_karaoke(std::vector<std::string> const& source_strings,
                                        std::string const& dest_string) {
	karaoke_match_result result = { 0, 0 };
	if(source_strings.empty()) return result;

	using namespace boost::locale::boundary;
	using boost::starts_with;

	result.source_length = 1;
	ssegment_index destination_characters(character, begin(dest_string), end(dest_string));
	auto src = boost::to_lower_copy(source_strings[0]);
	auto dst = destination_characters.begin();
	auto dst_end = destination_characters.end();

	// Eat all the whitespace at the beginning of the source and destination
	// syllables and exit if either ran out.
	auto eat_whitespace = [&]() -> bool {
		size_t i = 0, first_non_whitespace = 0;
		while(is_whitespace(next_codepoint(src.c_str(), &i)))
			first_non_whitespace = i;
		if(first_non_whitespace) src = src.substr(first_non_whitespace);

		while(dst != dst_end && is_whitespace(dst->str())) {
			++dst;
			++result.destination_length;
		}

		// If we ran out of dest then this needs to match the rest of the
		// source syllables (this probably means the user did something wrong)
		if(dst == dst_end) {
			result.source_length = source_strings.size();
			return true;
		}

		return src.empty();
	};

	if(eat_whitespace()) return result;

	// We now have a non-whitespace character at the beginning of both source
	// and destination. Check if the source starts with a romanized kana, and
	// if it does then check if the destination also has the appropriate
	// character. If it does, match them and repeat.
	while(!src.empty()) {
		// First check for a basic match of the first character of the source and dest
		auto first_src_char = ssegment_index(character, begin(src), end(src)).begin()->str();
		if(compare(first_src_char, dst->str()) == 0) {
			++dst;
			++result.destination_length;
			src.erase(0, first_src_char.size());
			if(eat_whitespace()) return result;
			continue;
		}

		auto check = [&](kana_pair const& kp) -> bool {
			if(!starts_with(&*dst->begin(), kp.kana)) return false;

			src = src.substr(strlen(kp.romaji));
			for(size_t i = 0; kp.kana[i];) {
				i += dst->length();
				++result.destination_length;
				++dst;
			}
			return true;
		};

		bool matched = false;
		for(auto const& match : romaji_to_kana(src)) {
			if(check(match)) {
				if(eat_whitespace()) return result;
				matched = true;
				break;
			}
		}
		if(!matched) break;
	}

	// Source and dest are now non-empty and start with non-whitespace.
	// If there's only one character left in the dest, it obviously needs to
	// match all of the source syllables left.
	if(std::distance(dst, dst_end) == 1) {
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

	for(size_t lookahead = 0; lookahead < dst_lookahead_max; ++lookahead) {
		if(++dst == dst_end) break;

		// Transliterate this character if it's a known hiragana or katakana character
		std::vector<const char*> translit;
		auto next = std::next(dst);
		if(next != dst_end)
			boost::copy(kana_to_romaji(dst->str() + next->str()), back_inserter(translit));
		boost::copy(kana_to_romaji(dst->str()), back_inserter(translit));

		// Search for it and the transliterated version in the source
		int src_lookahead_max = (lookahead + 1) * max_character_length;
		int src_lookahead_pos = 0;
		for(auto const& syl : source_strings) {
			// Don't count blank syllables in the max search distance
			if(is_whitespace(syl)) continue;
			if(++src_lookahead_pos == 1) continue;
			if(src_lookahead_pos > src_lookahead_max) break;

			std::string lsyl = boost::to_lower_copy(syl);
			if(!(starts_with(syl, dst->str()) ||
			     util::any_of(translit, [&](const char* str) { return starts_with(lsyl, str); })))
				continue;

			// The syllable immediately after the current one matched, so
			// everything up to the match must go with the current syllable.
			if(src_lookahead_pos == 2) {
				result.destination_length += lookahead + 1;
				return result;
			}

			// The match was multiple syllables ahead, so just divide the
			// destination characters evenly between the source syllables
			result.destination_length += 1;
			result.source_length =
			    static_cast<size_t>((src_lookahead_pos - 1.0) / (lookahead + 1.0) + .5);
			return result;
		}
	}

	// We wouldn't have gotten here if the dest was empty, so make sure at
	// least one character is selected
	result.destination_length = std::max<size_t>(result.destination_length, 1u);

	return result;
}
} // namespace agi
