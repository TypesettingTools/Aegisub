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

#include "libaegisub/lua/utils.h"

#include <boost/locale/conversion.hpp>

namespace {
using namespace agi::lua;

int unicode_upper(lua_State *L) {
	push_value(L, boost::locale::to_upper(check_string(L, 1)));
	return 1;
}

int unicode_lower(lua_State *L) {
	push_value(L, boost::locale::to_lower(check_string(L, 1)));
	return 1;
}

int unicode_fold(lua_State *L) {
	push_value(L, boost::locale::fold_case(check_string(L, 1)));
	return 1;
}
}

extern "C" int luaopen_unicode_impl(lua_State *L) {
	lua_createtable(L, 0, 3);
	set_field<unicode_upper>(L, "to_upper_case");
	set_field<unicode_lower>(L, "to_lower_case");
	set_field<unicode_fold>(L, "to_fold_case");
	return 1;
}
