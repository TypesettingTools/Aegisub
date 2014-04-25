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

#include "libaegisub/log.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/regex.hpp>

namespace agi { namespace lua {
std::string get_string_or_default(lua_State *L, int idx) {
	size_t len = 0;
	const char *str = lua_tolstring(L, idx, &len);
	if (!str)
		return "<not a string>";
	return std::string(str, len);
}

std::string get_string(lua_State *L, int idx) {
	size_t len = 0;
	const char *str = lua_tolstring(L, idx, &len);
	return std::string(str ? str : "", len);
}

std::string check_string(lua_State *L, int idx) {
	size_t len = 0;
	const char *str = luaL_checklstring(L, idx, &len);
	return std::string(str ? str : "", len);
}

std::string get_global_string(lua_State *L, const char *name) {
	lua_getglobal(L, name);
	std::string ret;
	if (lua_isstring(L, -1))
		ret = lua_tostring(L, -1);
	lua_pop(L, 1);
	return ret;
}

static int moon_line(lua_State *L, int lua_line, std::string const& file) {
	if (luaL_dostring(L, "return require 'moonscript.line_tables'")) {
		lua_pop(L, 1); // pop error message
		return lua_line;
	}

	push_value(L, file);
	lua_rawget(L, -2);

	if (!lua_istable(L, -1)) {
		lua_pop(L, 2);
		return lua_line;
	}

	lua_rawgeti(L, -1, lua_line);
	if (!lua_isnumber(L, -1)) {
		lua_pop(L, 3);
		return lua_line;
	}

	auto char_pos = static_cast<size_t>(lua_tonumber(L, -1));
	lua_pop(L, 3);

	// The moonscript line tables give us a character offset into the file,
	// so now we need to map that to a line number
	lua_getfield(L, LUA_REGISTRYINDEX, ("raw moonscript: " + file).c_str());
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return lua_line;
	}

	size_t moon_len;
	auto moon = lua_tolstring(L, -1, &moon_len);
	return std::count(moon, moon + std::min(moon_len, char_pos), '\n') + 1;
}

int add_stack_trace(lua_State *L) {
	int level = 1;
	if (lua_isnumber(L, 2)) {
		level = (int)lua_tointeger(L, 2);
		lua_pop(L, 1);
	}

	const char *err = lua_tostring(L, 1);
	if (!err) return 1;

	std::string message = err;
	if (lua_gettop(L))
		lua_pop(L, 1);

	// Strip the location from the error message since it's redundant with
	// the stack trace
	boost::regex location(R"(^\[string ".*"\]:[0-9]+: )");
	message = regex_replace(message, location, "", boost::format_first_only);

	std::vector<std::string> frames;
	frames.emplace_back(std::move(message));

	lua_Debug ar;
	while (lua_getstack(L, level++, &ar)) {
		lua_getinfo(L, "Snl", &ar);

		if (ar.what[0] == 't')
			frames.emplace_back("(tail call)");
		else {
			bool is_moon = false;
			std::string file = ar.source;
			if (file == "=[C]")
				file = "<C function>";
			else if (boost::ends_with(file, ".moon"))
				is_moon = true;

			auto real_line = [&](int line) {
				return is_moon ? moon_line(L, line, file) : line;
			};

			std::string function = ar.name ? ar.name : "";
			if (*ar.what == 'm')
				function = "<main>";
			else if (*ar.what == 'C')
				function = '?';
			else if (!*ar.namewhat)
				function = str(boost::format("<anonymous function at lines %d-%d>") % real_line(ar.linedefined) % real_line(ar.lastlinedefined - 1));

			frames.emplace_back(str(boost::format("    File \"%s\", line %d\n%s") % file % real_line(ar.currentline) % function));
		}
	}

	push_value(L, join(frames | boost::adaptors::reversed, "\n"));

	return 1;
}


#ifdef _DEBUG
void LuaStackcheck::check_stack(int additional) {
	int top = lua_gettop(L);
	if (top - additional != startstack) {
		LOG_D("automation/lua") << "lua stack size mismatch.";
		dump();
		assert(top - additional == startstack);
	}
}

void LuaStackcheck::dump() {
	int top = lua_gettop(L);
	LOG_D("automation/lua/stackdump") << "--- dumping lua stack...";
	for (int i = top; i > 0; i--) {
		lua_pushvalue(L, i);
		std::string type(lua_typename(L, lua_type(L, -1)));
		if (lua_isstring(L, i))
			LOG_D("automation/lua/stackdump") << type << ": " << lua_tostring(L, -1);
		else
			LOG_D("automation/lua/stackdump") << type;
		lua_pop(L, 1);
	}
	LOG_D("automation/lua") << "--- end dump";
}
#endif

}
}
