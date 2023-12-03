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

#include "libaegisub/unicode.h"

#include "libaegisub/exception.h"

using namespace agi;

BreakIterator::BreakIterator() {
	UErrorCode err = U_ZERO_ERROR;
	bi.reset(icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), err));
	if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));
}

void BreakIterator::set_text(std::string_view new_str) {
	UErrorCode err = U_ZERO_ERROR;
	UTextPtr ut(utext_openUTF8(nullptr, new_str.data(), new_str.size(), &err));
	bi->setText(ut.get(), err);
	if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));

	str = new_str;
	begin = 0;
	end = bi->next();
}
