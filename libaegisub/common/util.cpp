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

#include "libaegisub/util.h"
#include "libaegisub/util_osx.h"

#include "libaegisub/exception.h"

#include <boost/locale/boundary.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/util.hpp>
#include <boost/range/distance.hpp>
#include <ctime>
#include <unicode/edits.h>
#include <unicode/casemap.h>
#include <unicode/ucasemap.h>
#include <unicode/bytestream.h>
#include <unicode/locid.h>

namespace {
const size_t bad_pos = (size_t)-1;
const std::pair bad_match(bad_pos, bad_pos);

std::pair<size_t, size_t> find_range(std::string const& haystack, std::string const& needle, size_t start = 0) {
	const size_t match_start = haystack.find(needle, start);
	if (match_start == std::string::npos)
		return bad_match;
	return {match_start, match_start + needle.size()};
}

void parse_blocks(std::vector<std::pair<size_t, size_t>>& blocks, std::string const& str) {
	blocks.clear();

	size_t ovr_start = bad_pos;
	size_t i = 0;
	for (auto const& c : str) {
		if (c == '{' && ovr_start == bad_pos)
			ovr_start = i;
		else if (c == '}' && ovr_start != bad_pos) {
			blocks.emplace_back(ovr_start, i + 1);
			ovr_start = bad_pos;
		}
		++i;
	}
}

} // anonymous namespace

namespace agi { namespace util {

std::string strftime(const char *fmt, const tm *tmptr) {
	if (!tmptr) {
		time_t t = time(nullptr);
		tmptr = localtime(&t);
	}

	char buff[65536];
	::strftime(buff, sizeof buff, fmt, tmptr);
	return buff;
}

std::string fold_case(std::string_view str, icu::Edits *edits) {
	if (str.size() > std::numeric_limits<int32_t>::max())
		throw InvalidInputException("String is too long for case folding");
	auto size = static_cast<int32_t>(str.size());
	UErrorCode err = U_ZERO_ERROR;
	std::string ret;
	icu::StringByteSink sink(&ret, size);
	icu::CaseMap::utf8Fold(0, icu::StringPiece(str.data(), size), sink, edits, err);
	if (U_FAILURE(err)) throw InvalidInputException(u_errorName(err));
	return ret;
}

std::pair<size_t, size_t> ifind(std::string const& haystack, std::string const& needle) {
	icu::Edits edits;
	const auto folded_hs = fold_case(haystack, &edits);
	const auto folded_n = fold_case(needle, nullptr);
	auto it = edits.getFineIterator();
	size_t pos = 0;
	while (true) {
		auto match = find_range(folded_hs, folded_n, pos);
		if (match == bad_match || !edits.hasChanges())
			return match;
		UErrorCode err = U_ZERO_ERROR;

		int32_t first_raw = it.sourceIndexFromDestinationIndex(static_cast<int32_t>(match.first), err);
		int32_t second_raw = it.sourceIndexFromDestinationIndex(static_cast<int32_t>(match.second), err);

		bool good_match = it.destinationIndexFromSourceIndex(first_raw, err) == static_cast<int32_t>(match.first) && it.destinationIndexFromSourceIndex(second_raw, err) == static_cast<int32_t>(match.second);

		if (U_FAILURE(err)) throw InvalidInputException(u_errorName(err));

		if (good_match)
			return {first_raw, second_raw};

		pos = match.first + 1;
	}
}

std::string tagless_find_helper::strip_tags(std::string const& str, size_t s) {
	parse_blocks(blocks, str);

	std::string out;

	size_t last = s;
	for (auto const& block : blocks) {
		if (block.second <= s) continue;
		if (block.first > last)
			out.append(str.begin() + last, str.begin() + block.first);
		last = block.second;
	}

	if (last < str.size())
		out.append(str.begin() + last, str.end());

	start = s;
	return out;
}

void tagless_find_helper::map_range(size_t &s, size_t &e) {
	s += start;
	e += start;

	// Shift the start and end of the match to be relative to the unstripped
	// match
	for (auto const& block : blocks) {
		// Any blocks before start are irrelevant as they're included in `start`
		if (block.second <= start) continue;
		// Skip over blocks at the very beginning of the match
		// < should only happen if the cursor was within an override block
		// when the user started a search
		if (block.first <= s) {
			size_t len = block.second - std::max(block.first, start);
			s += len;
			e += len;
			continue;
		}

		assert(block.first > s);
		// Blocks after the match are irrelevant
		if (block.first >= e) break;

		// Extend the match to include blocks within the match
		// Note that blocks cannot be partially within the match
		e += block.second - block.first;
	}
}

void InitLocale() {
	// FIXME: need to verify we actually got a utf-8 locale
	auto id = boost::locale::util::get_system_locale(true);
	UErrorCode err = U_ZERO_ERROR;
	icu::Locale::setDefault(icu::Locale::createCanonical(id.c_str()), err);
	if (U_FAILURE(err)) throw InternalError(u_errorName(err));
	std::locale::global(boost::locale::generator().generate(""));
}
} // namespace util

#ifndef __APPLE__
namespace osx {
AppNapDisabler::AppNapDisabler(std::string reason) { }
AppNapDisabler::~AppNapDisabler() { }
}
#endif
} // namespace agi
