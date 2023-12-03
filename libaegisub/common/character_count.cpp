// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "libaegisub/character_count.h"

#include "libaegisub/ass/dialogue_parser.h"
#include "libaegisub/exception.h"
#include "libaegisub/unicode.h"

#include <unicode/uchar.h>
#include <unicode/utf8.h>

#include <mutex>

namespace {
const std::basic_string_view<char32_t> ass_special_chars = U"nNh";

size_t count_in_range(std::string_view str, int mask) {
	if (str.empty()) return 0;

	thread_local agi::BreakIterator bi;
	bi.set_text(str);

	size_t count = 0;
	if (!mask) {
		for (; !bi.done(); bi.next())
			++count;
		return count;
	}

	UChar32 prev = 0;
	for (; !bi.done(); bi.next()) {
		// Getting the character category only requires the first codepoint of a character
		UChar32 c;
		int i = 0;
		U8_NEXT(bi.current().data(), i, bi.current().size(), c);
		UChar32 p = prev;
		prev = c;

		if ((U_GET_GC_MASK(c) & mask) != 0) // if character is an ignored category
			continue;

		// If previous character was a backslash and we're ignoring whitespace,
		// check if this is an ass whitespace character (e.g. \h)
		if (mask & U_GC_Z_MASK && p == '\\' && ass_special_chars.find(c) != ass_special_chars.npos) {
			// If we're ignoring punctuation we didn't count the slash, but
			// otherwise we need to uncount it
			if (!(mask & U_GC_P_MASK))
				--count;
			continue;
		}

		++count;
	}
	return count;
}

int ignore_mask_to_icu_mask(int mask) {
	int ret = 0;
	if (mask & agi::IGNORE_PUNCTUATION)
		ret |= U_GC_P_MASK;
	if (mask & agi::IGNORE_WHITESPACE)
		ret |= U_GC_Z_MASK;
	return ret;
}
}

namespace agi {
size_t CharacterCount(std::string_view str, int ignore) {
	int mask = ignore_mask_to_icu_mask(ignore);
	if ((ignore & agi::IGNORE_BLOCKS) == 0)
		return count_in_range(str, mask);

	size_t characters = 0;
	while (!str.empty()) {
		auto pos = str.find('{');
		if (pos == str.npos) break;

		// if there's no trailing }, the rest of the string counts as characters,
		// including the leading {
		auto end = str.find('}');
		if (end == str.npos) break;

		if (pos > 0)
			characters += count_in_range(str.substr(0, pos), mask);
		str.remove_prefix(end + 1);
	}

	if (!str.empty())
		characters += count_in_range(str, mask);

	return characters;
}

size_t MaxLineLength(std::string_view text, int mask) {
	mask = ignore_mask_to_icu_mask(mask);
	auto tokens = agi::ass::TokenizeDialogueBody(text);
	agi::ass::MarkDrawings(text, tokens);

	size_t pos = 0;
	size_t max_line_length = 0;
	size_t current_line_length = 0;
	for (auto token : tokens) {
		if (token.type == agi::ass::DialogueTokenType::LINE_BREAK) {
			if (text[pos + 1] == 'h') {
				if (!(mask & U_GC_Z_MASK))
					current_line_length += 1;
			}
			else { // N or n
				max_line_length = std::max(max_line_length, current_line_length);
				current_line_length = 0;
			}
		}
		else if (token.type == agi::ass::DialogueTokenType::TEXT)
			current_line_length += count_in_range(text.substr(pos, token.length), mask);

		pos += token.length;
	}

	return std::max(max_line_length, current_line_length);
}

size_t IndexOfCharacter(std::string_view str, size_t n) {
	if (str.empty() || n == 0) return 0;
	thread_local BreakIterator bi;
	bi.set_text(str);

	for (; n > 0 && !bi.done(); --n)
		bi.next();
	if (bi.done())
		return str.size();
	return bi.current().data() - str.data();
}
}
