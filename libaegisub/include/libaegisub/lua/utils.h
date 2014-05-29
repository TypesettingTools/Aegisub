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

#include <libaegisub/fs.h>

#include <boost/exception/detail/attribute_noreturn.hpp>
#include <lua.hpp>
#include <string>
#include <vector>
#include <type_traits>

namespace agi { namespace lua {
// Exception type for errors where the error details are on the lua stack
struct error_tag {};

// Below are functionally equivalent to the luaL_ functions, but using a C++
// exception for stack unwinding
int BOOST_ATTRIBUTE_NORETURN error(lua_State *L, const char *fmt, ...);
int BOOST_ATTRIBUTE_NORETURN argerror(lua_State *L, int narg, const char *extramsg);
int BOOST_ATTRIBUTE_NORETURN typerror(lua_State *L, int narg, const char *tname);
void argcheck(lua_State *L, bool cond, int narg, const char *msg);

inline void push_value(lua_State *L, bool value) { lua_pushboolean(L, value); }
inline void push_value(lua_State *L, const char *value) { lua_pushstring(L, value); }
inline void push_value(lua_State *L, double value) { lua_pushnumber(L, value); }
inline void push_value(lua_State *L, int value) { lua_pushinteger(L, value); }
inline void push_value(lua_State *L, void *p) { lua_pushlightuserdata(L, p); }

template<typename Integer>
typename std::enable_if<std::is_integral<Integer>::value>::type
push_value(lua_State *L, Integer value) {
	lua_pushinteger(L, static_cast<lua_Integer>(value));
}

inline void push_value(lua_State *L, fs::path const& value) {
	lua_pushstring(L, value.string().c_str());
}

inline void push_value(lua_State *L, std::string const& value) {
	lua_pushlstring(L, value.c_str(), value.size());
}

inline void push_value(lua_State *L, lua_CFunction value) {
	if (lua_gettop(L) >= 2 && lua_type(L, -2) == LUA_TUSERDATA) {
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, value, 1);
	}
	else
		lua_pushcclosure(L, value, 0);
}

template<typename T>
void push_value(lua_State *L, std::vector<T> const& value) {
	lua_createtable(L, value.size(), 0);
	for (size_t i = 0; i < value.size(); ++i) {
		push_value(L, value[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

/// Wrap a function which may throw exceptions and make it trigger lua errors
/// whenever it throws
template<int (*func)(lua_State *L)>
int exception_wrapper(lua_State *L) {
	try {
		return func(L);
	}
	catch (agi::Exception const& e) {
		push_value(L, e.GetMessage());
		return lua_error(L);
	}
	catch (std::exception const& e) {
		push_value(L, e.what());
		return lua_error(L);
	}
	catch (error_tag) {
		// Error message is already on the stack
		return lua_error(L);
	}
	catch (...) {
		std::terminate();
	}
}

template<typename T>
void set_field(lua_State *L, const char *name, T value) {
	push_value(L, value);
	lua_setfield(L, -2, name);
}

template<int (*func)(lua_State *L)>
void set_field(lua_State *L, const char *name) {
	push_value(L, exception_wrapper<func>);
	lua_setfield(L, -2, name);
}

std::string get_string_or_default(lua_State *L, int idx);
std::string get_string(lua_State *L, int idx);
std::string get_global_string(lua_State *L, const char *name);

std::string check_string(lua_State *L, int idx);
int check_int(lua_State *L, int idx);
size_t check_uint(lua_State *L, int idx);
void *check_udata(lua_State *L, int idx, const char *mt);

template<typename T, typename... Args>
T *make(lua_State *L, const char *mt, Args&&... args) {
	auto obj = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
	new(obj) T(std::forward<Args>(args)...);
	luaL_getmetatable(L, mt);
	lua_setmetatable(L, -2);
	return obj;
}

template<typename T>
T& get(lua_State *L, int idx, const char *mt) {
	return *static_cast<T *>(check_udata(L, idx, mt));
}

struct LuaForEachBreak {};

template<typename Func>
void lua_for_each(lua_State *L, Func&& func) {
	lua_pushnil(L); // initial key
	while (lua_next(L, -2)) {
		try {
			func();
		}
		catch (LuaForEachBreak) {
			lua_pop(L, 1);
			break;
		}
		lua_pop(L, 1); // pop value, leave key
	}
	lua_pop(L, 1); // pop table
}

/// Lua error handler which adds the stack trace to the error message, with
/// moonscript line rewriting support
int add_stack_trace(lua_State *L);

#ifdef _DEBUG
struct LuaStackcheck {
	lua_State *L;
	int startstack;

	void check_stack(int additional);
	void dump();

	LuaStackcheck(lua_State *L) : L(L), startstack(lua_gettop(L)) { }
	~LuaStackcheck() { check_stack(0); }
};
#else
struct LuaStackcheck {
	void check_stack(int) { }
	void dump() { }
	LuaStackcheck(lua_State*) { }
};
#endif

} }
