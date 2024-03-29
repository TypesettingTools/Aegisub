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

#include "libaegisub/lua/modules.h"

#include "libaegisub/lua/ffi.h"
#include "libaegisub/lua/utils.h"

extern "C" int luaopen_luabins(lua_State *L);
extern "C" int luaopen_re_impl(lua_State *L);
extern "C" int luaopen_unicode_impl(lua_State *L);
extern "C" int luaopen_lfs_impl(lua_State *L);
extern "C" int luaopen_lpeg(lua_State *L);

namespace agi::lua {
int regex_init(lua_State *L);

void preload_modules(lua_State *L) {
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	set_field(L, "aegisub.__re_impl", luaopen_re_impl);
	set_field(L, "aegisub.__unicode_impl", luaopen_unicode_impl);
	set_field(L, "aegisub.__lfs_impl", luaopen_lfs_impl);
	set_field(L, "lpeg", luaopen_lpeg);
	set_field(L, "luabins", luaopen_luabins);

	lua_pop(L, 2);

	register_lib_functions(L); // silence an unused static function warning
}

void do_register_lib_function(lua_State *L, const char *name, const char *type_name, void *func) {
	lua_pushvalue(L, -2); // push cast function
	lua_pushstring(L, type_name);
	lua_pushlightuserdata(L, func);
	lua_call(L, 2, 1);
	lua_setfield(L, -2, name);
}

void do_register_lib_table(lua_State *L, std::initializer_list<const char *> types) {
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

	// leaves ffi.cast on the stack
}
}
