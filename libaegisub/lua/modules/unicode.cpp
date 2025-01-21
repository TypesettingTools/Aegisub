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

#include <libaegisub/lua/ffi.h>

#include <unicode/unistr.h>

#include <cstring>

namespace {
char *wrap(void (*fn)(icu::UnicodeString&), const char *str, char **err) {
	auto ustr = icu::UnicodeString::fromUTF8(str);
	if (ustr.isBogus()) {
		*err = strdup((std::string("Converting \"") + str + "\" to a unicode string failed.").c_str());
	}
	fn(ustr);
	std::string out;
	ustr.toUTF8String(out);
	return agi::lua::strndup(out);
}

template<void (*fn)(icu::UnicodeString&)>
char *wrap(const char *str, char **err) {
	return wrap(fn, str, err);
}

void to_upper(icu::UnicodeString& str) { str.toUpper(); }
void to_lower(icu::UnicodeString& str) { str.toLower(); }
void to_fold(icu::UnicodeString& str) { str.foldCase(); }
}

extern "C" int luaopen_unicode_impl(lua_State *L) {
	agi::lua::register_lib_table(L, {},
		"to_upper_case", wrap<to_upper>,
		"to_lower_case", wrap<to_lower>,
		"to_fold_case", wrap<to_fold>);
	return 1;
}
