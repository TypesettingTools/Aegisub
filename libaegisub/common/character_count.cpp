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

#include <unicode/uchar.h>
#include <unicode/utf8.h>

#include <mutex>
#include <unicode/brkiter.h>

namespace {
struct utext_deleter {
	void operator()(UText *ut) { if (ut) utext_close(ut); }
};
using utext_ptr = std::unique_ptr<UText, utext_deleter>;

template<typename Iterator>
utext_ptr to_utext(Iterator begin, Iterator end) {
	UErrorCode err = U_ZERO_ERROR;
	utext_ptr ret(utext_openUTF8(nullptr, &*begin, end - begin, &err));
	if (U_FAILURE(err)) throw agi::InternalError("Failed to open utext", nullptr);
	return ret;
}

template <typename Iterator>
size_t count_in_range(Iterator begin, Iterator end, bool ignore_whitespace) {
	if (begin == end) return 0;

	static std::unique_ptr<icu::BreakIterator> character_bi;
	static std::once_flag token;
	std::call_once(token, [&] {
		UErrorCode status = U_ZERO_ERROR;
		character_bi.reset(BreakIterator::createCharacterInstance(Locale::getDefault(), status));
		if (U_FAILURE(status)) throw agi::InternalError("Failed to create character iterator", nullptr);
	});

	UErrorCode err = U_ZERO_ERROR;

	utext_ptr ut = to_utext(begin, end);
	character_bi->setText(ut.get(), err);
	if (U_FAILURE(err)) throw agi::InternalError("Failed to set break iterator text", nullptr);

	size_t count = 0;
	auto pos = character_bi->first();
	for (auto end = character_bi->next(); end != BreakIterator::DONE; pos = end, end = character_bi->next()) {
		if (!ignore_whitespace)
			++count;
		else {
			UChar32 c;
			int i = 0;
			U8_NEXT_UNSAFE(begin + pos, i, c);
			if (!u_isUWhiteSpace(c))
				++count;
		}

	}
	return count;
}
}

namespace agi {
size_t CharacterCount(std::string const& str) {
	size_t characters = 0;
	auto pos = begin(str);
	do {
		auto it = std::find(pos, end(str), '{');
		characters += count_in_range(pos, it, true);
		if (it == end(str)) break;

		pos = std::find(pos, end(str), '}');
		if (pos == end(str)) {
			characters += count_in_range(it, pos, true);
			break;
		}
	} while (++pos != end(str));

	return characters;
}

size_t MaxLineLength(std::string const& text, bool ignore_whitespace) {
	auto tokens = agi::ass::TokenizeDialogueBody(text);
	agi::ass::MarkDrawings(text, tokens);

	size_t pos = 0;
	size_t max_line_length = 0;
	size_t current_line_length = 0;
	for (auto token : tokens) {
		if (token.type == agi::ass::DialogueTokenType::LINE_BREAK) {
			if (text[pos + 1] == 'h') {
				if (!ignore_whitespace)
					current_line_length += 1;
			}
			else { // N or n
				max_line_length = std::max(max_line_length, current_line_length);
				current_line_length = 0;
			}
		}
		else if (token.type == agi::ass::DialogueTokenType::TEXT)
			current_line_length += count_in_range(begin(text) + pos, begin(text) + pos + token.length, ignore_whitespace);

		pos += token.length;
	}

	return std::max(max_line_length, current_line_length);
}
}
