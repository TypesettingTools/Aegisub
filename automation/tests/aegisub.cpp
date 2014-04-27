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

#include <libaegisub/lua/modules.h>
#include <libaegisub/lua/script_reader.h>
#include <libaegisub/lua/utils.h>

#include <cstdio>
#include <cstdlib>

using namespace agi::lua;

namespace {
void check(lua_State *L, int status) {
	if (status && !lua_isnil(L, -1)) {
		fprintf(stderr, "%s\n", get_string_or_default(L, -1).c_str());
		exit(status);
	}
}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: aegisub-lua <script> [args]\n");
		return 1;
	}

	// Init lua state
	lua_State *L = lua_open();
	agi::lua::preload_modules(L);
	Install(L, {"include"});
	push_value(L, luaopen_debug); lua_call(L, 0, 0);

	// Build arg table for scripts
	lua_createtable(L, argc - 1, 0);
	for (int i = 1; i < argc; ++i) {
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i - 1);
	}
	lua_setglobal(L, "arg");

	// Stack needs to be error handler -> function -> args
	lua_pushcfunction(L, add_stack_trace);

	try {
		check(L, !LoadFile(L, argv[1]));
	} catch (agi::Exception const& e) {
		fprintf(stderr, "%s\n", e.GetChainedMessage().c_str());
	}

	for (int i = 2; i < argc; ++i)
		lua_pushstring(L, argv[i]);

	int base = lua_gettop(L) - argc + 1;
	check(L, lua_pcall(L, argc - 2, LUA_MULTRET, base));
}

