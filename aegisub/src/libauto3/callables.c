// Copyright (c) 2005, 2006, 2007, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//

// DO NOT compile this file separately! It's included as part of auto3.c
#ifndef BUILDING_AUTO3_C
# error callables.c can not be compiled separately. It is included as part of auto3.c.
#endif


static struct Auto3Interpreter* GetScriptObject(lua_State *L)
{
	return (struct Auto3Interpreter *)lua_touserdata(L, lua_upvalueindex(1));
}


static int LuaInclude(lua_State *L)
{
	filename_t filename;
	const char *incname;
	char *error;
	struct Auto3Interpreter *script;
	script = GetScriptObject(L);
	
	if (!script->cb.resolve_include) {
		lua_pushstring(L, "Attempt to use include, but not implemented by host application");
		lua_error(L);
	}

	incname = luaL_checkstring(L, 1);

	filename = script->cb.resolve_include(script->cb.rundata, incname);

	if (filename) {
		// Load include
		if (Auto3LuaLoad(L, filename, incname, &error)) {
			free(filename);
			lua_pushfstring(L, "Failed to include file '%s', error: %s", incname, error);
			lua_error(L);
		}
		// Run include (don't protect, we're already in a protected environment, we'd just propagate it anyway)
		lua_call(L, 0, 0);
		free(filename);
	} else {
		lua_pushfstring(L, "Failed to resolve include file '%s'", incname);
		lua_error(L);
	}

	// Not really compatible, but I don't think anyone have ever exploited that includes can return stuff
	return 0;
}


static int LuaTextExtents(lua_State *L)
{
	struct Auto3Interpreter *script;
	const char *text, *fontname;
	int fontsize, bold, italic, spacing, encoding;
	float scale_x, scale_y;
	float out_width, out_height, out_descent, out_extlead;

	script = GetScriptObject(L);

	if (!script->cb.text_extents) return 0;

	// get text
	text = luaL_checkstring(L, 2);

	// check we have style table
	if (!lua_istable(L, 1)) {
		lua_pushstring(L, "First argument to text_extents must be style table");
		lua_error(L);
	}

	// get style def
	lua_pushstring(L, "fontname"); lua_gettable(L, 1);
	fontname = lua_tostring(L, -1);

	lua_pushstring(L, "fontsize"); lua_gettable(L, 1);
	fontsize = (int)lua_tonumber(L, -1);

	lua_pushstring(L, "bold"); lua_gettable(L, 1);
	bold = lua_toboolean(L, -1);

	lua_pushstring(L, "italic"); lua_gettable(L, 1);
	italic = lua_toboolean(L, -1);

	lua_pushstring(L, "scale_x"); lua_gettable(L, 1);
	scale_x = (float)lua_tonumber(L, -1);

	lua_pushstring(L, "scale_y"); lua_gettable(L, 1);
	scale_y = (float)lua_tonumber(L, -1);

	lua_pushstring(L, "spacing"); lua_gettable(L, 1);
	spacing = (int)lua_tonumber(L, -1);

	lua_pushstring(L, "encoding"); lua_gettable(L, 1);
	encoding = (int)lua_tonumber(L, -1);

	// get measurements
	script->cb.text_extents(script->cb.rundata, text, fontname, fontsize, bold, italic,
		spacing, scale_x, scale_y, encoding, &out_width, &out_height, &out_descent, &out_extlead);

	// remove strings and stuff
	lua_pop(L, 8);

	// return result
	lua_pushnumber(L, out_width);
	lua_pushnumber(L, out_height);
	lua_pushnumber(L, out_descent);
	lua_pushnumber(L, out_extlead);
	return 4;
}


static int LuaFrameFromMs(lua_State *L)
{
	int frame;
	struct Auto3Interpreter *script;
	script = GetScriptObject(L);

	if (!script->cb.frame_from_ms) return 0;

	frame = script->cb.frame_from_ms(script->cb.rundata, luaL_checkint(L, 1));
	lua_pushnumber(L, frame);
	return 1;
}


static int LuaMsFromFrame(lua_State *L)
{
	int ms;
	struct Auto3Interpreter *script;
	script = GetScriptObject(L);

	if (!script->cb.ms_from_frame) return 0;

	ms = script->cb.ms_from_frame(script->cb.rundata, luaL_checkint(L, 1));
	lua_pushnumber(L, ms);
	return 1;
}


static int LuaReportProgress(lua_State *L)
{
	struct Auto3Interpreter *script;
	script = GetScriptObject(L);
	
	if (script->cb.set_progress) script->cb.set_progress(script->cb.logdata, (float)luaL_checknumber(L, 1));

	return 0;
}


static int LuaOutputDebug(lua_State *L)
{
	struct Auto3Interpreter *script;
	script = GetScriptObject(L);
	
	if (script->cb.log_message) script->cb.log_message(script->cb.logdata, luaL_checkstring(L, 1));

	return 0;
}


static int LuaSetStatus(lua_State *L)
{
	struct Auto3Interpreter *script;
	script = GetScriptObject(L);
	
	if (script->cb.set_status) script->cb.set_status(script->cb.logdata, luaL_checkstring(L, 1));

	return 0;
}


