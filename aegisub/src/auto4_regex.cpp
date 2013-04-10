// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "config.h"

#ifdef WITH_AUTO4_LUA
#include "auto4_lua_utils.h"

#include <boost/regex/icu.hpp>

namespace {
boost::u32regex& get_regex(lua_State *L) {
	return *static_cast<boost::u32regex*>(luaL_checkudata(L, 1, "aegisub.regex"));
}

boost::smatch& get_smatch(lua_State *L) {
	return *static_cast<boost::smatch*>(luaL_checkudata(L, 1, "aegisub.smatch"));
}

int regex_matches(lua_State *L) {
	lua_pushboolean(L, u32regex_match(luaL_checkstring(L, 2), get_regex(L)));
	return 1;
}

int regex_match(lua_State *L) {
	auto re = get_regex(L);
	std::string str = luaL_checkstring(L, 2);
	int start = lua_tointeger(L, 3);

	auto result = static_cast<boost::smatch*>(lua_newuserdata(L, sizeof(boost::smatch)));
	new(result) boost::smatch;
	luaL_getmetatable(L, "aegisub.smatch");
	lua_setmetatable(L, -2);

	if (!u32regex_search(str.cbegin() + start, str.cend(), *result, re,
		start > 0 ? boost::match_prev_avail | boost::match_not_bob : boost::match_default))
	{
		lua_pop(L, 1);
		lua_pushnil(L);
	}

	return 1;
}

int regex_get_match(lua_State *L) {
	auto match = get_smatch(L);
	int idx = luaL_checkinteger(L, 2) - 1;
	if (static_cast<size_t>(idx) > match.size() || !match[idx].matched) {
		lua_pushnil(L);
		return 1;
	}

	push_value(L, distance(match.prefix().first, match[idx].first + 1));
	push_value(L, distance(match.prefix().first, match[idx].second));
	return 2;
}

int regex_search(lua_State *L) {
	auto re = get_regex(L);
	std::string str = luaL_checkstring(L, 2);
	int start = luaL_checkinteger(L, 3) - 1;
	boost::smatch result;
	if (!u32regex_search(str.cbegin() + start, str.cend(), result, re,
		start > 0 ? boost::match_prev_avail | boost::match_not_bob : boost::match_default))
	{
		lua_pushnil(L);
		return 1;
	}

	push_value(L, start + result.position() + 1);
	push_value(L, start + result.position() + result.length());
	return 2;
}

int regex_replace(lua_State *L) {
	auto re = get_regex(L);
	const auto replacement = luaL_checkstring(L, 2);
	const std::string str = luaL_checkstring(L, 3);
	int max_count = luaL_checkinteger(L, 4);

	// Can't just use regex_replace here since it can only do one or infinite replacements
	auto match = boost::u32regex_iterator<std::string::const_iterator>(begin(str), end(str), re);
	auto end_it = boost::u32regex_iterator<std::string::const_iterator>();

	auto suffix = begin(str);

	std::string ret;
	auto out = back_inserter(ret);
	while (match != end_it && max_count > 0) {
		copy(suffix, match->prefix().second, out);
		match->format(out, replacement);
		suffix = match->suffix().first;
		++match;
		--max_count;
	}

	copy(suffix, end(str), out);

	push_value(L, ret);
	return 1;
}

int regex_compile(lua_State *L) {
	std::string pattern(luaL_checkstring(L, 1));
	int flags = luaL_checkinteger(L, 2);
	boost::u32regex *re = static_cast<boost::u32regex*>(lua_newuserdata(L, sizeof(boost::u32regex)));

	try {
		new(re) boost::u32regex;
		*re = boost::make_u32regex(pattern, boost::u32regex::perl | flags);
	}
	catch (std::exception const& e) {
		lua_pop(L, 1);
		push_value(L, e.what());
		return 1;
		// Do the actual triggering of the error in the Lua code as that code
		// can report the original call site
	}

	luaL_getmetatable(L, "aegisub.regex");
	lua_setmetatable(L, -2);

	return 1;
}

int regex_gc(lua_State *L) {
	using boost::u32regex;
	get_regex(L).~u32regex();
	return 0;
}

int smatch_gc(lua_State *L) {
	using boost::smatch;
	get_smatch(L).~smatch();
	return 0;
}

int regex_process_flags(lua_State *L) {
	int ret = 0;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; ++i) {
		if (!lua_islightuserdata(L, i)) {
			push_value(L, "Flags must follow all non-flag arguments");
			return 1;
		}
		ret |= (int)(intptr_t)lua_touserdata(L, i);
	}

	push_value(L, ret);
	return 1;
}

int regex_init_flags(lua_State *L) {
	lua_newtable(L);

	set_field(L, "ICASE", (void*)boost::u32regex::icase);
	set_field(L, "NOSUB", (void*)boost::u32regex::nosubs);
	set_field(L, "COLLATE", (void*)boost::u32regex::collate);
	set_field(L, "NEWLINE_ALT", (void*)boost::u32regex::newline_alt);
	set_field(L, "NO_MOD_M", (void*)boost::u32regex::no_mod_m);
	set_field(L, "NO_MOD_S", (void*)boost::u32regex::no_mod_s);
	set_field(L, "MOD_S", (void*)boost::u32regex::mod_s);
	set_field(L, "MOD_X", (void*)boost::u32regex::mod_x);
	set_field(L, "NO_EMPTY_SUBEXPRESSIONS", (void*)boost::u32regex::no_empty_expressions);

	return 1;
}

}

namespace Automation4 {
int regex_init(lua_State *L) {
	if (luaL_newmetatable(L, "aegisub.regex")) {
		set_field(L, "__gc", regex_gc);
		lua_pop(L, 1);
	}

	if (luaL_newmetatable(L, "aegisub.smatch")) {
		set_field(L, "__gc", smatch_gc);
		lua_pop(L, 1);
	}

	lua_newtable(L);
	set_field(L, "matches", regex_matches);
	set_field(L, "search", regex_search);
	set_field(L, "match", regex_match);
	set_field(L, "get_match", regex_get_match);
	set_field(L, "replace", regex_replace);
	set_field(L, "compile", regex_compile);
	set_field(L, "process_flags", regex_process_flags);
	set_field(L, "init_flags", regex_init_flags);
	return 1;
}
}
#endif
