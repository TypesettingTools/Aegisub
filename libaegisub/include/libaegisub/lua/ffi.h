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

#include <lua.hpp>

namespace agi { namespace lua {
static void register_lib_functions(lua_State *) {
	// Base case of recursion; nothing to do
}

template<typename Func, typename... Rest>
void register_lib_functions(lua_State *L, const char *name, Func *func, Rest... rest) {
	lua_pushvalue(L, -2); // push cast function
	lua_pushstring(L, type_name<Func*>::name().c_str());
	// This cast isn't legal, but LuaJIT internally requires that it work, so we can rely on it too
	lua_pushlightuserdata(L, (void *)func);
	lua_call(L, 2, 1);
	lua_setfield(L, -2, name);

	register_lib_functions(L, rest...);
}

template<typename... Args>
void register_lib_table(lua_State *L, std::initializer_list<const char *> types, Args... functions) {
	static_assert((sizeof...(functions) & 1) == 0, "Functions must be alternating names and function pointers");

	lua_getglobal(L, "require");
	lua_pushstring(L, "ffi");
	lua_call(L, 1, 1);

	// Register all passed type with the ffi
	for (auto type : types) {
		lua_getfield(L, -1, "cdef");
		lua_pushfstring(L, "typedef struct %s %s;", type, type);
		lua_call(L, 1, 0);
	}

	lua_getfield(L, -1, "cast");
	lua_remove(L, -2); // ffi table

	lua_createtable(L, 0, sizeof...(functions) / 2);
	register_lib_functions(L, functions...);
	lua_remove(L, -2); // ffi.cast function
	// Leaves lib table on the stack
}

} }
