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

#include "overlua.h"
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#endif


struct FileScriptReader {
	static const size_t datasize = 0x10000;
	char *data;
	FILE *file;
	FileScriptReader() { data = new char[datasize]; }
	~FileScriptReader() { delete[] data; }
	static const char *reader(lua_State *L, void *data, size_t *size)
	{
		FileScriptReader *self = (FileScriptReader*)data;
		*size = fread(self->data, 1, self->datasize, self->file);
		if (*size) return self->data;
		else return 0;
	}
};

OverLuaScript::OverLuaScript(const char *filename)
{
	FileScriptReader reader;
#ifdef WIN32
	wchar_t *filenamew = new wchar_t[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, filename, -1, filenamew, MAX_PATH);
	reader.file = _wfopen(filenamew, L"r");
	delete[] filenamew;
#else
	reader.file = fopen(filename, "r");
#endif

	Create(reader, filename);

	fclose(reader.file);
}


struct MemScriptReader {
	const void *memdata;
	size_t memdatasize;
	static const char *reader(lua_State *L, void *data, size_t *size)
	{
		MemScriptReader *self = (MemScriptReader*)data;
		*size = self->memdatasize;
		self->memdatasize = 0;
		if (*size) return (const char*)self->memdata;
		else return 0;
	}
};

int OverLuaScript::lua_debug_print(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
#ifdef WIN32
	OutputDebugStringA(str);
#else
	printf(str);
#endif
	return 0;
}

OverLuaScript::OverLuaScript(const void *data, size_t length)
{
	MemScriptReader reader;
	reader.memdata = data;
	reader.memdatasize = length;

	Create(reader, "Memory script");

	int err;

	L = luaL_newstate();

	err = lua_load(L, reader.reader, &reader, "Memory script");

	// todo: better error handling
	if (err == LUA_ERRSYNTAX) throw "Syntax error";
	if (err = LUA_ERRMEM) throw "Memory error";

	err = lua_pcall(L, 0, 0, 0);

	if (err == LUA_ERRRUN) throw "Runtime error";
	if (err == LUA_ERRMEM) throw "Memory error";
	if (err == LUA_ERRERR) throw "Error-handler error";
}

OverLuaScript::~OverLuaScript()
{
	lua_close(L);
}

void OverLuaScript::RenderFrameRGB(OverLuaFrameAggregate &frame, double time)
{
	OutputDebugStringW(L"RenderFrameRGB: get frame func\n");
	lua_getglobal(L, "render_frame");
	OutputDebugStringW(L"RenderFrameRGB: CreateLuaObject\n");
	frame.CreateLuaObject(L);
	lua_pushnumber(L, time);
	OutputDebugStringW(L"RenderFrameRGB: call\n");
	if (lua_pcall(L, 2, 0, 0)) {
		const char *err = lua_tostring(L, -1);
		//MessageBoxA(0, err, "OverLua", MB_ICONERROR);
		throw err;
	}
	OutputDebugStringW(L"RenderFrameRGB: garbage collect\n");
	lua_gc(L, LUA_GCCOLLECT, 0);
	OutputDebugStringW(L"RenderFrameRGB: done rendering frame\n");
}
