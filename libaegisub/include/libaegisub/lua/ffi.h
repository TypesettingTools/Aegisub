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

#include <cstdlib>
#include <lua.hpp>

namespace agi {
namespace lua {
void do_register_lib_function(lua_State* L, const char* name, const char* type_name, void* func);
void do_register_lib_table(lua_State* L, std::initializer_list<const char*> types);

static void register_lib_functions(lua_State*) {
	// Base case of recursion; nothing to do
}

template <typename Func, typename... Rest>
void register_lib_functions(lua_State* L, const char* name, Func* func, Rest... rest) {
	// This cast isn't legal, but LuaJIT internally requires that it work, so we can rely on it too
	do_register_lib_function(L, name, type_name<Func*>::name().c_str(), (void*)func);
	register_lib_functions(L, rest...);
}

template <typename... Args>
void register_lib_table(lua_State* L, std::initializer_list<const char*> types, Args... functions) {
	static_assert((sizeof...(functions) & 1) == 0,
	              "Functions must be alternating names and function pointers");

	do_register_lib_table(L, types); // leaves ffi.cast on the stack
	lua_createtable(L, 0, sizeof...(functions) / 2);
	register_lib_functions(L, functions...);
	lua_remove(L, -2); // ffi.cast function
	                   // Leaves lib table on the stack
}

template <typename T> char* strndup(T const& str) {
	char* ret = static_cast<char*>(malloc(str.size() + 1));
	memcpy(ret, str.data(), str.size());
	ret[str.size()] = 0;
	return ret;
}

} // namespace lua
} // namespace agi
