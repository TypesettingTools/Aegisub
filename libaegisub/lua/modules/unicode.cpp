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

#include <libaegisub/type_name.h>

#include <boost/locale/conversion.hpp>
#include <lua.hpp>

namespace {
template<typename T>
void push_ffi_function(lua_State *L, const char *name, T *func) {
	lua_pushvalue(L, -2); // push cast function
	lua_pushstring(L, agi::type_name<T*>::name().c_str());
	// This cast isn't legal, but LuaJIT internally requires that it work
	lua_pushlightuserdata(L, (void *)func);
	lua_call(L, 2, 1);
	lua_setfield(L, -2, name);
}

template<std::string (*func)(const char *, std::locale const&)>
char *wrap(const char *str, char **err) {
	try {
		return strdup(func(str, std::locale()).c_str());
	} catch (std::exception const& e) {
		*err = strdup(e.what());
		return nullptr;
	}
}
}

extern "C" int luaopen_unicode_impl(lua_State *L) {
	lua_getglobal(L, "require");
	lua_pushstring(L, "ffi");
	lua_call(L, 1, 1);
	lua_getfield(L, -1, "cast");
	lua_remove(L, -2); // ffi table

	lua_createtable(L, 0, 3);
	push_ffi_function(L, "to_upper_case", wrap<boost::locale::to_upper<char>>);
	push_ffi_function(L, "to_lower_case", wrap<boost::locale::to_lower<char>>);
	push_ffi_function(L, "to_fold_case", wrap<boost::locale::fold_case<char>>);

	lua_remove(L, -2); // ffi.cast function
	return 1;
}
