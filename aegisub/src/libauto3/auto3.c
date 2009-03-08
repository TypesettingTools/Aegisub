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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "auto3.h"


// Win32 DLL entry point
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	// TODO: Destroy any still-alive scripts/interpreters here on unload?
    return TRUE;
}
#endif


struct script_reader_data {
	FILE *f;
	int isfirst;
#define SCRIPT_READER_BUFSIZE 512
	char databuf[SCRIPT_READER_BUFSIZE];
};
static const char *script_reader_func(lua_State *L, void *data, size_t *size)
{
	struct script_reader_data *self;
	unsigned char *b;
	FILE *f;

	self = (struct script_reader_data *)(data);
	b = (unsigned char *)self->databuf;
	f = self->f;

	if (feof(f)) {
		*size = 0;
		return NULL;
	}

	if (self->isfirst) {
		self->isfirst = 0;
		// check if file is sensible and maybe skip bom
		if ((*size = fread(b, 1, 4, f)) == 4) {
			if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) {
				// got an utf8 file with bom
				// nothing further to do, already skipped the bom
				fseek(f, -1, SEEK_CUR);
			} else {
				// oops, not utf8 with bom
				// check if there is some other BOM in place and complain if there is...
				if ((b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) || // utf32be
					(b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) || // utf32le
					(b[0] == 0xFF && b[1] == 0xFE) || // utf16be
					(b[0] == 0xFE && b[1] == 0xFF) || // utf16le
					(b[0] == 0x2B && b[1] == 0x2F && b[2] == 0x76) || // utf7
					(b[0] == 0x00 && b[2] == 0x00) || // looks like utf16be
					(b[1] == 0x00 && b[3] == 0x00)) { // looks like utf16le
						// can't support these files
						*size = 0;
						self->isfirst = -1;
						strcpy(self->databuf, "File is an unsupported UTF");
						return NULL;
				}
				// assume utf8 without bom, and rewind file
				fseek(f, 0, SEEK_SET);
			}
		} else {
			// hmm, rather short file this...
			// doesn't have a bom, assume it's just ascii/utf8 without bom
			return self->databuf; // *size is already set
		}
	}

	*size = fread(b, 1, SCRIPT_READER_BUFSIZE, f);

	return self->databuf;
}
static int Auto3LuaLoad(lua_State *L, filename_t filename, const char *prettyname, char **error)
{
	struct script_reader_data script_reader;
	int res;

	script_reader.f =
#ifdef WIN32
		_wfopen(filename, L"rb");
#else
		fopen(filename, "rb");
#endif
	if (!script_reader.f) return -1;

	script_reader.isfirst = 1;

	res = lua_load(L, script_reader_func, &script_reader, prettyname);

	fclose(script_reader.f);

	if (res) {
		*error = strdup(lua_tostring(L, -1));
		return res;
	}
	if (script_reader.isfirst == -1) {
		// Signals we got a bad UTF
		*error = strdup(script_reader.databuf);
		return -1;
	}

	return 0;
}


// Read the 'config' global and create config struct
static int Auto3ParseConfigData(lua_State *L, struct Auto3Interpreter *script, char **error)
{
	struct Auto3ConfigOption *opt;
	int i, n;
	const char *tmp;

	if (!lua_istable(L, -1)) {
		// No 'config' table at all, just make the sentinel option
		script->config = calloc(1, sizeof(struct Auto3ConfigOption));
		return 0;
	}
	
	// Get expected number of elements in table
	n = luaL_getn(L, -1);
	// Allocate memory for max number of elements + 1
	script->config = calloc(n+1, sizeof(struct Auto3ConfigOption));

	// Prepare traversal
	lua_pushnil(L);
	opt = script->config;

	// Get at most n options
	i = 1;
	while (lua_next(L, -2)) {
		if (i > n) {
			// More options than we have space for...
			// Just ignore the extra options for now
			lua_pop(L, 2);
			break;
		}

		// Top of stack should be the next option living in a table
		if (lua_istable(L, -1)) {

			// get the "kind"
			lua_pushstring(L, "kind");
			lua_gettable(L, -2);
			if (lua_isstring(L, -1)) {
				// use C standard lib functions here, as it's probably faster than messing around with unicode
				// lua is known to always properly null-terminate strings, and the strings are known to be pure ascii
				tmp = lua_tostring(L, -1);
				if (strcmp(tmp, "label") == 0) {
					opt->kind = COK_LABEL;
				} else if (strcmp(tmp, "text") == 0) {
					opt->kind = COK_TEXT;
				} else if (strcmp(tmp, "int") == 0) {
					opt->kind = COK_INT;
				} else if (strcmp(tmp, "float") == 0) {
					opt->kind = COK_FLOAT;
				} else if (strcmp(tmp, "bool") == 0) {
					opt->kind = COK_BOOL;
				} else if (strcmp(tmp, "colour") == 0) {
					opt->kind = COK_COLOUR;
				} else if (strcmp(tmp, "style") == 0) {
					opt->kind = COK_STYLE;
				} else {
					opt->kind = COK_INVALID;
				}
			} else {
				opt->kind = COK_INVALID;
			}

			// remove "kind" string from stack again
			lua_pop(L, 1);

			// no need to check for rest if this one is already deemed invalid
			if (opt->kind != COK_INVALID) {
				// name
				lua_pushstring(L, "name");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					opt->name = strdup(lua_tostring(L, -1));
				} else {
					// name is required to be valid
					opt->kind = COK_INVALID;
				}
				lua_pop(L, 1);

				// label
				lua_pushstring(L, "label");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					opt->label = strdup(lua_tostring(L, -1));
				} else {
					// label is also required
					opt->kind = COK_INVALID;
				}
				lua_pop(L, 1);

				// hint
				lua_pushstring(L, "hint");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					opt->hint = strdup(lua_tostring(L, -1));
				} else {
					opt->hint = strdup("");
				}
				lua_pop(L, 1);

				// min
				lua_pushstring(L, "min");
				lua_gettable(L, -2);
				if (lua_isnumber(L, -1)) {
					opt->min.valid = 1;
					opt->min.floatval = (float)lua_tonumber(L, -1);
					opt->min.intval = (int)opt->min.floatval;
				} else {
					opt->min.valid = 0;
				}
				lua_pop(L, 1);

				// max
				lua_pushstring(L, "max");
				lua_gettable(L, -2);
				if (lua_isnumber(L, -1)) {
					opt->max.valid = 1;
					opt->max.floatval = (float)lua_tonumber(L, -1);
					opt->max.intval = (int)opt->max.floatval;
				} else {
					opt->max.valid = 0;
				}
				lua_pop(L, 1);

				// default
				lua_pushstring(L, "default");
				lua_gettable(L, -2);
				switch (opt->kind) {
					case COK_LABEL:
						// nothing to do, nothing expected
						break;
					case COK_TEXT:
					case COK_STYLE:
					case COK_COLOUR:
						// expect it to be a string
						if (lua_isstring(L, -1)) {
							opt->default_val.stringval = strdup(lua_tostring(L, -1));
							opt->value.stringval = strdup(opt->default_val.stringval);
						} else {
							// not a string, baaaad scripter
							opt->kind = COK_INVALID;
						}
						break;
					case COK_INT:
						// expect it to be a number
						if (lua_isnumber(L, -1)) {
							opt->default_val.intval = (int)lua_tonumber(L, -1);
							opt->value.intval = opt->default_val.intval;
						} else {
							opt->kind = COK_INVALID;
						}
						break;
					case COK_FLOAT:
						// expect it to be a number
						if (lua_isnumber(L, -1)) {
							opt->default_val.floatval = (float)lua_tonumber(L, -1);
							opt->value.floatval = opt->default_val.floatval;
						} else {
							opt->kind = COK_INVALID;
						}
						break;
					case COK_BOOL:
						// expect it to be a bool
						if (lua_isboolean(L, -1)) {
							opt->default_val.intval = lua_toboolean(L, -1);
							opt->value.intval = opt->default_val.intval;
						} else {
							opt->kind = COK_INVALID;
						}
						break;
					case COK_INVALID:
						break;
				}
				lua_pop(L, 1);
			}

			// On to next structure to be filled
			opt++;
		}

		// Remove option table from stack
		lua_pop(L, 1);
		// Such that the current key is on top, and we can get the next
	}


	// Remove 'config' table from stack
	lua_pop(L, 1);

	return 0;
}


// Keeping this file a bit shorter: put all functions called from Lua into a separate file
#include "callables.c"


// Create a new interpreter
AUTO3_API struct Auto3Interpreter *CreateAuto3Script(const filename_t filename, const char *prettyname, struct Auto3Callbacks *cb, char **error)
{
	struct Auto3Interpreter *script;
	lua_State *L;

	script = malloc(sizeof(struct Auto3Interpreter));
	if (!script) return NULL;
	// Copy in callbacks
	memcpy(&script->cb, cb, sizeof(struct Auto3Callbacks));


	// Init Lua
	script->L = lua_open();
	if (!script->L) goto failearly;
	L = script->L;


	// register standard libs
	lua_pushcfunction(L, luaopen_base); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_string); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_table); lua_call(L, 0, 0);
	lua_pushcfunction(L, luaopen_math); lua_call(L, 0, 0);
	// dofile and loadfile are replaced with include
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");
	lua_pushlightuserdata(L, script);
	lua_pushcclosure(L, LuaInclude, 1);
	lua_setglobal(L, "include");

	// reference to the script object
	lua_pushlightuserdata(L, script);

	// make "aegisub" table
	lua_newtable(L);

	// put helper functions in it
	// colorstring_to_rgb is moved to utils.auto3
	lua_pushstring(L, "text_extents");
	lua_pushvalue(L, -3);
	lua_pushcclosure(L, LuaTextExtents, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "frame_from_ms");
	lua_pushvalue(L, -3);
	lua_pushcclosure(L, LuaFrameFromMs, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "ms_from_frame");
	lua_pushvalue(L, -3);
	lua_pushcclosure(L, LuaMsFromFrame, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "report_progress");
	lua_pushvalue(L, -3);
	lua_pushcclosure(L, LuaReportProgress, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "output_debug");
	lua_pushvalue(L, -3);
	lua_pushcclosure(L, LuaOutputDebug, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "set_status");
	lua_pushvalue(L, -3);
	lua_pushcclosure(L, LuaSetStatus, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "lua_automation_version");
	lua_pushnumber(L, 3);
	lua_settable(L, -3);

	// store table
	lua_setglobal(L, "aegisub");
	// remove ref to script object
	lua_pop(L, 1);


	// Read the script
	if (Auto3LuaLoad(L, filename, prettyname, error)) {
		// error is already filled
		goto faillua;
	}

	// Execute the script
	if (lua_pcall(L, 0, 0, 0)) {
		*error = strdup(lua_tostring(L, -1));
		goto faillua;
	}


	// Script has been run, stuff exists in the global environment
	lua_getglobal(L, "version");
	if (!lua_isnumber(L, -1)) {
		*error = strdup("'version' value not found or not a number");
		goto faillua;
	}
	if ((int)lua_tonumber(L, -1) != 3) {
		// invalid version
		*error = strdup("'version' must be 3 for Automation 3 scripts");
		goto faillua;
	}
	// skip 'kind', it's useless
	// name
	lua_getglobal(L, "name");
	if (!lua_isstring(L, -1)) {
		script->name = strdup(prettyname);
	} else {
		script->name = strdup(lua_tostring(L, -1));
	}
	// description (optional)
	lua_getglobal(L, "description");
	if (lua_isstring(L, -1)) {
		script->description = strdup(lua_tostring(L, -1));
	} else {
		script->description = strdup("");
	}
	lua_pop(L, 3);


	// Parse the config data
	lua_getglobal(L, "configuration");
	if (Auto3ParseConfigData(L, script, error)) {
		goto faildescription;
	}


	return script;


	// Various fail-cases
faildescription:
	free(script->description);
	free(script->name);
faillua:
	lua_close(script->L);
failearly:
	free(script);
	return NULL;
}


// Release an interpreter
AUTO3_API void DestroyAuto3Script(struct Auto3Interpreter *script)
{
	struct Auto3ConfigOption *opt;

	// free the config data
	opt = script->config;
	while (opt->name) {
		free(opt->name);
		free(opt->label);
		free(opt->hint);
		if (opt->kind == COK_TEXT || opt->kind == COK_COLOUR || opt->kind == COK_STYLE) {
			free(opt->default_val.stringval);
			free(opt->value.stringval);
		}

		opt++;
	}
	free(script->config);

	// free the rest
	free(script->description);
	free(script->name);
	lua_close(script->L);
	free(script);
}


// Our "malloc" function, allocate memory for strings with this
AUTO3_API void *Auto3Malloc(size_t amount)
{
	return malloc(amount);
}

// Convenience function, use this for duplicating strings this lib should own
AUTO3_API char *Auto3Strdup(const char *str)
{
	return strdup(str);
}

// Our "free" function, free generated error messages with this
AUTO3_API void Auto3Free(void *ptr)
{
	free(ptr);
}


static void MakeMetaInfoTable(lua_State *L, struct Auto3Interpreter *script)
{
	int res_x, res_y;

	script->cb.get_meta_info(script->cb.rwdata, &res_x, &res_y);

	lua_newtable(L);

	lua_pushstring(L, "res_x");
	lua_pushnumber(L, res_x);
	lua_settable(L, -3);

	lua_pushstring(L, "res_y");
	lua_pushnumber(L, res_y);
	lua_settable(L, -3);
}


static void MakeStylesTable(lua_State *L, struct Auto3Interpreter *script)
{
	char *name, *fontname, *color1, *color2, *color3, *color4;
	int fontsize, bold, italic, underline, strikeout, borderstyle, align, margin_l, margin_r, margin_v, encoding;
	float scale_x, scale_y, spacing, angle, outline, shadow;
	int n;

	lua_newtable(L);
	n = -1;

	script->cb.reset_style_pointer(script->cb.rwdata);
	while (script->cb.get_next_style(script->cb.rwdata, &name, &fontname, &fontsize, &color1, &color2, &color3, &color4,
			&bold, &italic, &underline, &strikeout, &scale_x, &scale_y, &spacing, &angle, &borderstyle, &outline,
			&shadow, &align, &margin_l, &margin_r, &margin_v, &encoding)) {
		n++;

		// Got a style...
		lua_pushstring(L, name); // name for table index
		lua_newtable(L);

		// Set properties

		lua_pushstring(L, "name");
		lua_pushstring(L, name);
		lua_settable(L, -3);

		lua_pushstring(L, "fontname");
		lua_pushstring(L, fontname);
		lua_settable(L, -3);

		lua_pushstring(L, "fontsize");
		lua_pushnumber(L, fontsize);
		lua_settable(L, -3);

		lua_pushstring(L, "color1");
		lua_pushstring(L, color1);
		lua_settable(L, -3);

		lua_pushstring(L, "color2");
		lua_pushstring(L, color2);
		lua_settable(L, -3);

		lua_pushstring(L, "color3");
		lua_pushstring(L, color3);
		lua_settable(L, -3);

		lua_pushstring(L, "color4");
		lua_pushstring(L, color4);
		lua_settable(L, -3);

		lua_pushstring(L, "bold");
		lua_pushboolean(L, bold);
		lua_settable(L, -3);

		lua_pushstring(L, "italic");
		lua_pushboolean(L, italic);
		lua_settable(L, -3);

		lua_pushstring(L, "underline");
		lua_pushboolean(L, underline);
		lua_settable(L, -3);

		lua_pushstring(L, "strikeout");
		lua_pushboolean(L, strikeout);
		lua_settable(L, -3);

		lua_pushstring(L, "scale_x");
		lua_pushnumber(L, scale_x);
		lua_settable(L, -3);

		lua_pushstring(L, "scale_y");
		lua_pushnumber(L, scale_y);
		lua_settable(L, -3);

		lua_pushstring(L, "spacing");
		lua_pushnumber(L, spacing);
		lua_settable(L, -3);

		lua_pushstring(L, "angle");
		lua_pushnumber(L, angle);
		lua_settable(L, -3);

		lua_pushstring(L, "borderstyle");
		lua_pushnumber(L, borderstyle);
		lua_settable(L, -3);

		lua_pushstring(L, "outline");
		lua_pushnumber(L, outline);
		lua_settable(L, -3);

		lua_pushstring(L, "shadow");
		lua_pushnumber(L, shadow);
		lua_settable(L, -3);

		lua_pushstring(L, "align");
		lua_pushnumber(L, align);
		lua_settable(L, -3);

		lua_pushstring(L, "margin_l");
		lua_pushnumber(L, margin_l);
		lua_settable(L, -3);

		lua_pushstring(L, "margin_r");
		lua_pushnumber(L, margin_r);
		lua_settable(L, -3);

		lua_pushstring(L, "margin_v");
		lua_pushnumber(L, margin_v);
		lua_settable(L, -3);

		lua_pushstring(L, "encoding");
		lua_pushnumber(L, encoding);
		lua_settable(L, -3);

		// Store to numeric index
		lua_pushnumber(L, n);
		lua_pushvalue(L, -2); // extra copy of table
		lua_settable(L, -5);
		// And named index
		lua_settable(L, -3);
	}

	// Finally, make -1 key in table (because the name 'n' might clash with a style name)
	lua_pushnumber(L, -1);
	lua_pushnumber(L, n);
	lua_settable(L, -3);
}


static void MakeEventsTable(lua_State *L, struct Auto3Interpreter *script)
{
	int layer, start_time, end_time, margin_l, margin_r, margin_v, comment;
	char *style, *actor, *effect, *text;
	int n;

	lua_newtable(L);
	n = -1;

	script->cb.reset_subs_pointer(script->cb.rwdata);
	while (script->cb.get_next_sub(script->cb.rwdata, &layer, &start_time, &end_time, &style, &actor,
			&margin_l, &margin_r, &margin_v, &effect, &text, &comment)) {
		n++;

		// Got a line...
		lua_pushnumber(L, n);
		lua_newtable(L);

		lua_pushstring(L, "kind");
		if (comment)
			lua_pushstring(L, "comment");
		else
			lua_pushstring(L, "dialogue");
		lua_settable(L, -3);

		lua_pushstring(L, "layer");
		lua_pushnumber(L, layer);
		lua_settable(L, -3);

		lua_pushstring(L, "start_time");
		lua_pushnumber(L, start_time);
		lua_settable(L, -3);

		lua_pushstring(L, "end_time");
		lua_pushnumber(L, end_time);
		lua_settable(L, -3);

		lua_pushstring(L, "style");
		lua_pushstring(L, style);
		lua_settable(L, -3);

		lua_pushstring(L, "name");
		lua_pushstring(L, actor);
		lua_settable(L, -3);

		lua_pushstring(L, "margin_l");
		lua_pushnumber(L, margin_l);
		lua_settable(L, -3);

		lua_pushstring(L, "margin_r");
		lua_pushnumber(L, margin_r);
		lua_settable(L, -3);

		lua_pushstring(L, "margin_v");
		lua_pushnumber(L, margin_v);
		lua_settable(L, -3);

		lua_pushstring(L, "effect");
		lua_pushstring(L, effect);
		lua_settable(L, -3);

		lua_pushstring(L, "text");
		lua_pushstring(L, text);
		lua_settable(L, -3);

		// No parsing karaoke data here, that can just as well be done in Lua code

		// Store at numeric index
		lua_settable(L, -3);
	}

	// Finally, make 'n' key in table
	lua_pushstring(L, "n");
	lua_pushnumber(L, n);
	lua_settable(L, -3);
}


static void MakeConfigSettingsTable(lua_State *L, struct Auto3Interpreter *script)
{
	struct Auto3ConfigOption *opt;

	lua_newtable(L);

	opt = script->config;
	while (opt->name) {
		lua_pushstring(L, opt->name);

		switch (opt->kind) {
			case COK_TEXT:
			case COK_STYLE:
			case COK_COLOUR:
				lua_pushstring(L, opt->value.stringval);
				break;

			case COK_INT:
				lua_pushnumber(L, opt->value.intval);
				break;

			case COK_FLOAT:
				lua_pushnumber(L, opt->value.floatval);
				break;

			case COK_BOOL:
				lua_pushboolean(L, opt->value.intval);
				break;

			default:
				lua_pushnil(L);
				break;
		}

		lua_settable(L, -3);

		opt++;
	}
}


static void ReadBackSubs(lua_State *L, struct Auto3Interpreter *script)
{
	int layer, start_time, end_time, margin_l, margin_r, margin_v, comment;
	const char *style, *actor, *effect, *text, *kind;
	int i, n;

	script->cb.start_subs_write(script->cb.rwdata);

	i = 0;
	n = luaL_getn(L, -1);
	while (i <= n) {
		// Assume n is always correct, this is not entirely compatible
		lua_rawgeti(L, -1, i);
		if (script->cb.set_progress) script->cb.set_progress(script->cb.logdata, 100.f * i / n);
		i++;
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		lua_pushstring(L, "kind");
		lua_gettable(L, -2);
		if (!lua_isstring(L, -1)) {
			lua_pop(L, 2);
			continue;
		}
		kind = lua_tostring(L, -1);
		// leave kind on stack so it won't be gc'd

		// Make comment rather tell if this is a "non-dialogue" line
		comment = strcmp(kind, "dialogue");
		// then test if it's a dialogue line (not non-dialogue) or is an actual comment
		if (!comment || strcmp(kind, "comment")) {

			lua_pushstring(L, "layer");
			lua_gettable(L, -3);
			lua_pushstring(L, "start_time");
			lua_gettable(L, -4);
			lua_pushstring(L, "end_time");
			lua_gettable(L, -5);
			lua_pushstring(L, "style");
			lua_gettable(L, -6);
			lua_pushstring(L, "name");
			lua_gettable(L, -7);
			lua_pushstring(L, "margin_l");
			lua_gettable(L, -8);
			lua_pushstring(L, "margin_r");
			lua_gettable(L, -9);
			lua_pushstring(L, "margin_v");
			lua_gettable(L, -10);
			lua_pushstring(L, "effect");
			lua_gettable(L, -11);
			lua_pushstring(L, "text");
			lua_gettable(L, -12);

			if (lua_isnumber(L, -10) && lua_isnumber(L, -9) && lua_isnumber(L, -8) &&
					lua_isstring(L, -7) && lua_isstring(L, -6) && lua_isnumber(L, -5) &&
					lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isstring(L, -2) &&
					lua_isstring(L, -1)) {
				layer = (int)lua_tonumber(L, -10);
				start_time = (int)lua_tonumber(L, -9);
				end_time = (int)lua_tonumber(L, -8);
				style = lua_tostring(L, -7);
				actor = lua_tostring(L, -6);
				margin_l = (int)lua_tonumber(L, -5);
				margin_r = (int)lua_tonumber(L, -4);
				margin_v = (int)lua_tonumber(L, -3);
				effect = lua_tostring(L, -2);
				text = lua_tostring(L, -1);

				script->cb.write_sub(script->cb.rwdata, layer, start_time, end_time, style, actor,
					margin_l, margin_r, margin_v, effect, text, comment);

			} else {
				if (script->cb.log_error) script->cb.log_error(script->cb.logdata, "Skipping output line with invalid fields");
			}

			lua_pop(L, 10);
		}

		lua_pop(L, 2); // pop line and 'kind'
	}
}


// Start the script execution
AUTO3_API int RunAuto3Script(struct Auto3Interpreter *script)
{
	lua_State *L;

	L = script->L;

	if (script->cb.set_status)	script->cb.set_status(script->cb.logdata, "Preparing subtitle data");
	if (script->cb.set_progress) script->cb.set_progress(script->cb.logdata, 0);

	// first put the function itself on the stack
	lua_getglobal(L, "process_lines");

	// now put the four arguments on the stack
	MakeMetaInfoTable(L, script);
	MakeStylesTable(L, script);
	MakeEventsTable(L, script);
	MakeConfigSettingsTable(L, script);

	// do the actual call
	if (script->cb.set_status)	script->cb.set_status(script->cb.logdata, "Running script");
	if (lua_pcall(L, 4, 1, 0)) {
		if (script->cb.log_error) {
			script->cb.log_error(script->cb.logdata, "Runtime error in script:");
			script->cb.log_error(script->cb.logdata, lua_tostring(L, -1));
		}
		return -1;
	}

	// Check for initial sanity
	if (!lua_istable(L, -1)) {
		if (script->cb.log_error) script->cb.log_error(script->cb.logdata, "Script did not return a table, unable to process result");
	}

	// Read back subtitles
	if (script->cb.set_status)	script->cb.set_status(script->cb.logdata, "Reading back subtitle data");
	if (script->cb.set_progress) script->cb.set_progress(script->cb.logdata, 0);
	ReadBackSubs(L, script);

	// Finished
	if (script->cb.set_progress) script->cb.set_progress(script->cb.logdata, 100);

	return 0;
}



