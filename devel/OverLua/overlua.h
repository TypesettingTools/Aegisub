/*
 * C++ interface for OverLua
 *

    Copyright 2007  Niels Martin Hansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Contact:
    E-mail: <jiifurusu@gmail.com>
    IRC: jfs in #aegisub on irc.rizon.net

 */

#ifndef OVERLUA_H
#define OVERLUA_H

// assume we're in aegisub's svn tree and
// are building with the aegisub patched lua
#include "../lua51/src/lua.h"
#include "../lua51/src/lualib.h"
#include "../lua51/src/lauxlib.h"

#include <stddef.h>
#include "image.h"
#include "cairo_wrap.h"
#include "raster_ops.h"


class OverLuaScript {
private:
	lua_State *L;

	// No default constructor
	OverLuaScript() { }

	template <class ScriptReaderClass>
	void Create(ScriptReaderClass &reader, const char *chunkname, const char *datastring)
	{
		int err;

		L = luaL_newstate();

		// Base Lua libs
		luaL_openlibs(L);
		// Cairo lib
		lua_pushcfunction(L, luaopen_cairo); lua_call(L, 0, 0);
		// Raster library
		lua_pushcfunction(L, luaopen_raster); lua_call(L, 0, 0);
		// Debug print
		lua_pushcclosure(L, lua_debug_print, 0);
		lua_setglobal(L, "dprint");
		// Datastring
		if (datastring)
			lua_pushstring(L, datastring);
		else
			lua_pushnil(L);
		lua_setglobal(L, "overlua_datastring");

		err = lua_load(L, reader.reader, &reader, chunkname);

		// todo: better error handling
		if (err == LUA_ERRSYNTAX) throw lua_tostring(L, -1);
		if (err == LUA_ERRMEM) throw "Memory error";

		err = lua_pcall(L, 0, 0, 0);

		if (err == LUA_ERRRUN) throw lua_tostring(L, -1);
		if (err == LUA_ERRMEM) throw "Memory error";
		if (err == LUA_ERRERR) throw "Error-handler error";
	}

	static int lua_debug_print(lua_State *L);

public:
	OverLuaScript(const char *filename, const char *_datastring = 0);
	OverLuaScript(const void *data, size_t length, const char *_datastring = 0);
	virtual ~OverLuaScript();

	void RenderFrameRGB(BaseImageAggregate &frame, double time);

};

#endif
