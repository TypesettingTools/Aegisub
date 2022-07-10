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

#include <boost/locale/conversion.hpp>

namespace {
template <std::string (*func)(const char*, std::locale const&)>
char* wrap(const char* str, char** err) {
	try {
		return agi::lua::strndup(func(str, std::locale()));
	} catch(std::exception const& e) {
		*err = strdup(e.what());
		return nullptr;
	}
}
} // namespace

extern "C" int luaopen_unicode_impl(lua_State* L) {
	agi::lua::register_lib_table(L, {}, "to_upper_case", wrap<boost::locale::to_upper<char>>,
	                             "to_lower_case", wrap<boost::locale::to_lower<char>>,
	                             "to_fold_case", wrap<boost::locale::fold_case<char>>);
	return 1;
}
