// Copyright (c) 2006, Niels Martin Hansen
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file auto4_lua_assfile.cpp
/// @brief Lua 5.1-based scripting engine (interface to subtitle files)
/// @ingroup scripting
///

#include "config.h"

#ifdef WITH_AUTO4_LUA

#ifndef AGI_PRE
#include <assert.h>

#include <algorithm>

#include <wx/log.h>
#endif

#include <libaegisub/log.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "ass_style.h"
#include "auto4_lua.h"
#include "utils.h"

// This must be below the headers above.
#ifdef __WINDOWS__
#include "../../contrib/lua51/src/lualib.h"
#include "../../contrib/lua51/src/lauxlib.h"
#else
#include <lualib.h>
#include <lauxlib.h>
#endif


/// DOCME
namespace Automation4 {

	// LuaAssFile


	/// @brief DOCME
	/// @return 
	///
	void LuaAssFile::CheckAllowModify()
	{
		if (can_modify)
			return;
		lua_pushstring(L, "Attempt to modify subtitles in read-only feature context.");
		lua_error(L);
	}


	/// @brief DOCME
	/// @param L 
	/// @param e 
	///
	void LuaAssFile::AssEntryToLua(lua_State *L, AssEntry *e)
	{
		lua_newtable(L);

		wxString section(e->group);
		lua_pushstring(L, section.mb_str(wxConvUTF8));
		lua_setfield(L, -2, "section");

		wxString raw(e->GetEntryData());
		lua_pushstring(L, raw.mb_str(wxConvUTF8));
		lua_setfield(L, -2, "raw");

		if (StringEmptyOrWhitespace(raw)) {
			lua_pushstring(L, "clear");

		} else if (raw[0] == _T(';')) {
			// "text" field, same as "raw" but with semicolon stripped
			wxString text(raw, 1, raw.size()-1);
			lua_pushstring(L, text.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "text");

			lua_pushstring(L, "comment");

		} else if (raw[0] == _T('[')) {
			lua_pushstring(L, "head");

		} else if (section.Lower() == _T("[script info]")) {
			// assumed "info" class

			// first "key"
			wxString key = raw.BeforeFirst(_T(':'));
			lua_pushstring(L, key.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "key");

			// then "value"
			wxString value = raw.AfterFirst(_T(':'));
			lua_pushstring(L, value.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "value");

			lua_pushstring(L, "info");

		} else if (raw.Left(7).Lower() == _T("format:")) {

			// TODO: parse the format line; just use a tokenizer

			lua_pushstring(L, "format");

		} else if (e->GetType() == ENTRY_DIALOGUE) {
			AssDialogue *dia = static_cast<AssDialogue*>(e);

			lua_pushboolean(L, (int)dia->Comment);
			lua_setfield(L, -2, "comment");

			lua_pushnumber(L, dia->Layer);
			lua_setfield(L, -2, "layer");

			lua_pushnumber(L, dia->Start.GetMS());
			lua_setfield(L, -2, "start_time");
			lua_pushnumber(L, dia->End.GetMS());
			lua_setfield(L, -2, "end_time");

			lua_pushstring(L, dia->Style.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "style");
			lua_pushstring(L, dia->Actor.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "actor");

			lua_pushnumber(L, dia->Margin[0]);
			lua_setfield(L, -2, "margin_l");
			lua_pushnumber(L, dia->Margin[1]);
			lua_setfield(L, -2, "margin_r");
			lua_pushnumber(L, dia->Margin[2]);
			lua_setfield(L, -2, "margin_t");
			lua_pushnumber(L, dia->Margin[3]);
			lua_setfield(L, -2, "margin_b");

			lua_pushstring(L, dia->Effect.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "effect");

			lua_pushstring(L, ""); // tentative AS5 field
			lua_setfield(L, -2, "userdata");

			lua_pushstring(L, dia->Text.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "text");

			lua_pushstring(L, "dialogue");

		} else if (e->GetType() == ENTRY_STYLE) {
			AssStyle *sty = static_cast<AssStyle*>(e);

			lua_pushstring(L, sty->name.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "name");

			lua_pushstring(L, sty->font.mb_str(wxConvUTF8));
			lua_setfield(L, -2, "fontname");
			lua_pushnumber(L, sty->fontsize);
			lua_setfield(L, -2, "fontsize");

			lua_pushstring(L, sty->primary.GetASSFormatted(true).mb_str(wxConvUTF8));
			lua_setfield(L, -2, "color1");
			lua_pushstring(L, sty->secondary.GetASSFormatted(true).mb_str(wxConvUTF8));
			lua_setfield(L, -2, "color2");
			lua_pushstring(L, sty->outline.GetASSFormatted(true).mb_str(wxConvUTF8));
			lua_setfield(L, -2, "color3");
			lua_pushstring(L, sty->shadow.GetASSFormatted(true).mb_str(wxConvUTF8));
			lua_setfield(L, -2, "color4");

			lua_pushboolean(L, (int)sty->bold);
			lua_setfield(L, -2, "bold");
			lua_pushboolean(L, (int)sty->italic);
			lua_setfield(L, -2, "italic");
			lua_pushboolean(L, (int)sty->underline);
			lua_setfield(L, -2, "underline");
			lua_pushboolean(L, (int)sty->strikeout);
			lua_setfield(L, -2, "strikeout");

			lua_pushnumber(L, sty->scalex);
			lua_setfield(L, -2, "scale_x");
			lua_pushnumber(L, sty->scaley);
			lua_setfield(L, -2, "scale_y");

			lua_pushnumber(L, sty->spacing);
			lua_setfield(L, -2, "spacing");

			lua_pushnumber(L, sty->angle);
			lua_setfield(L, -2, "angle");

			lua_pushnumber(L, sty->borderstyle);
			lua_setfield(L, -2, "borderstyle");
			lua_pushnumber(L, sty->outline_w);
			lua_setfield(L, -2, "outline");
			lua_pushnumber(L, sty->shadow_w);
			lua_setfield(L, -2, "shadow");

			lua_pushnumber(L, sty->alignment);
			lua_setfield(L, -2, "align");

			lua_pushnumber(L, sty->Margin[0]);
			lua_setfield(L, -2, "margin_l");
			lua_pushnumber(L, sty->Margin[1]);
			lua_setfield(L, -2, "margin_r");
			lua_pushnumber(L, sty->Margin[2]);
			lua_setfield(L, -2, "margin_t");
			lua_pushnumber(L, sty->Margin[3]);
			lua_setfield(L, -2, "margin_b");

			lua_pushnumber(L, sty->encoding);
			lua_setfield(L, -2, "encoding");

			lua_pushnumber(L, 2); // From STS.h: "0: window, 1: video, 2: undefined (~window)"
			lua_setfield(L, -2, "relative_to");

			lua_pushboolean(L, false); // vertical writing, tentative AS5 field
			lua_setfield(L, -2, "vertical");

			lua_pushstring(L, "style");

		} else {
			lua_pushstring(L, "unknown");
		}
		// store class of item; last thing done for each class specific code must be pushing the class name
		lua_setfield(L, -2, "class");
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	AssEntry *LuaAssFile::LuaToAssEntry(lua_State *L)
	{
		// assume an assentry table is on the top of the stack
		// convert it to a real AssEntry object, and pop the table from the stack

		if (!lua_istable(L, -1)) {
			lua_pushstring(L, "Can't convert a non-table value to AssEntry");
			lua_error(L);
			return 0;
		}

		lua_getfield(L, -1, "class");
		if (!lua_isstring(L, -1)) {
			lua_pushstring(L, "Table lacks 'class' field, can't convert to AssEntry");
			lua_error(L);
			return 0;
		}
		wxString lclass(lua_tostring(L, -1), wxConvUTF8);
		lclass.MakeLower();
		lua_pop(L, 1);

		AssEntry *result;


/// DOCME
#define GETSTRING(varname, fieldname, lineclass)		\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isstring(L, -1)) {							\
		lua_pushstring(L, "Invalid string '" fieldname "' field in '" lineclass "' class subtitle line"); \
		lua_error(L);									\
		return 0;										\
	}													\
	wxString varname (lua_tostring(L, -1), wxConvUTF8);	\
	lua_pop(L, 1);

/// DOCME
#define GETFLOAT(varname, fieldname, lineclass)			\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isnumber(L, -1)) {							\
		lua_pushstring(L, "Invalid number '" fieldname "' field in '" lineclass "' class subtitle line"); \
		lua_error(L);									\
		return 0;										\
	}													\
	float varname = lua_tonumber(L, -1);				\
	lua_pop(L, 1);

/// DOCME
#define GETINT(varname, fieldname, lineclass)			\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isnumber(L, -1)) {							\
		lua_pushstring(L, "Invalid number '" fieldname "' field in '" lineclass "' class subtitle line"); \
		lua_error(L);									\
		return 0;										\
	}													\
	int varname = lua_tointeger(L, -1);					\
	lua_pop(L, 1);

/// DOCME
#define GETBOOL(varname, fieldname, lineclass)			\
	lua_getfield(L, -1, fieldname);						\
	if (!lua_isboolean(L, -1)) {						\
		lua_pushstring(L, "Invalid boolean '" fieldname "' field in '" lineclass "' class subtitle line"); \
		lua_error(L);									\
		return 0;										\
	}													\
	bool varname = !!lua_toboolean(L, -1);				\
	lua_pop(L, 1);

		GETSTRING(section, "section", "common")

		if (lclass == _T("clear")) {
			result = new AssEntry(_T(""));
			result->group = section;

		} else if (lclass == _T("comment")) {
			GETSTRING(raw, "text", "comment")
			raw.Prepend(_T(";"));
			result = new AssEntry(raw);
			result->group = section;

		} else if (lclass == _T("head")) {
			result = new AssEntry(section);
			result->group = section;

		} else if (lclass == _T("info")) {
			GETSTRING(key, "key", "info")
			GETSTRING(value, "value", "info")
			result = new AssEntry(wxString::Format(_T("%s: %s"), key.c_str(), value.c_str()));
			result->group = _T("[Script Info]"); // just so it can be read correctly back

		} else if (lclass == _T("format")) {
			// ohshi- ...
			// *FIXME* maybe ignore the actual data and just put some default stuff based on section?
			result = new AssEntry(_T("Format: Auto4,Is,Broken"));
			result->group = section;

		} else if (lclass == _T("style")) {
			GETSTRING(name, "name", "style")
			GETSTRING(fontname, "fontname", "style")
			GETFLOAT(fontsize, "fontsize", "style")
			GETSTRING(color1, "color1", "style")
			GETSTRING(color2, "color2", "style")
			GETSTRING(color3, "color3", "style")
			GETSTRING(color4, "color4", "style")
			GETBOOL(bold, "bold", "style")
			GETBOOL(italic, "italic", "style")
			GETBOOL(underline, "underline", "style")
			GETBOOL(strikeout, "strikeout", "style")
			GETFLOAT(scale_x, "scale_x", "style")
			GETFLOAT(scale_y, "scale_y", "style")
			GETFLOAT(spacing, "spacing", "style")
			GETFLOAT(angle, "angle", "style")
			GETINT(borderstyle, "borderstyle", "style")
			GETFLOAT(outline, "outline", "style")
			GETFLOAT(shadow, "shadow", "style")
			GETINT(align, "align", "style")
			GETINT(margin_l, "margin_l", "style")
			GETINT(margin_r, "margin_r", "style")
			GETINT(margin_t, "margin_t", "style")
			GETINT(margin_b, "margin_b", "style")
			GETINT(encoding, "encoding", "style")
			// leaving out relative_to and vertical

			AssStyle *sty = new AssStyle();
			sty->name = name;
			sty->font = fontname;
			sty->fontsize = fontsize;
			sty->primary.Parse(color1);
			sty->secondary.Parse(color2);
			sty->outline.Parse(color3);
			sty->shadow.Parse(color4);
			sty->bold = bold;
			sty->italic = italic;
			sty->underline = underline;
			sty->strikeout = strikeout;
			sty->scalex = scale_x;
			sty->scaley = scale_y;
			sty->spacing = spacing;
			sty->angle = angle;
			sty->borderstyle = borderstyle;
			sty->outline_w = outline;
			sty->shadow_w = shadow;
			sty->alignment = align;
			sty->Margin[0] = margin_l;
			sty->Margin[1] = margin_r;
			sty->Margin[2] = margin_t;
			sty->Margin[3] = margin_b;
			sty->encoding = encoding;
			sty->UpdateData();

			result = sty;

		} else if (lclass == _T("styleex")) {
			lua_pushstring(L, "Found line with class 'styleex' which is not supported. Wait until AS5 is a reality.");
			lua_error(L);
			return 0;

		} else if (lclass == _T("dialogue")) {
			GETBOOL(comment, "comment", "dialogue")
			GETINT(layer, "layer", "dialogue")
			GETINT(start_time, "start_time", "dialogue")
			GETINT(end_time, "end_time", "dialogue")
			GETSTRING(style, "style", "dialogue")
			GETSTRING(actor, "actor", "dialogue")
			GETINT(margin_l, "margin_l", "dialogue")
			GETINT(margin_r, "margin_r", "dialogue")
			GETINT(margin_t, "margin_t", "dialogue")
			GETINT(margin_b, "margin_b", "dialogue")
			GETSTRING(effect, "effect", "dialogue")
			//GETSTRING(userdata, "userdata", "dialogue")
			GETSTRING(text, "text", "dialogue")

			AssDialogue *dia = new AssDialogue();
			dia->Comment = comment;
			dia->Layer = layer;
			dia->Start.SetMS(start_time);
			dia->End.SetMS(end_time);
			dia->Style = style;
			dia->Actor = actor;
			dia->Margin[0] = margin_l;
			dia->Margin[1] = margin_r;
			dia->Margin[2] = margin_t;
			dia->Margin[3] = margin_b;
			dia->Effect = effect;
			dia->Text = text;

			result = dia;

		} else {
			lua_pushfstring(L, "Found line with unknown class: %s", lclass.mb_str(wxConvUTF8).data());
			lua_error(L);
			return 0;
		}


/// DOCME
#undef GETSTRING

/// DOCME
#undef GETFLOAT

/// DOCME
#undef GETINT

/// DOCME
#undef GETBOOL

		//lua_pop(L, 1); // the function shouldn't eat the table it converted
		return result;
	}


	/// @brief DOCME
	/// @param n 
	///
	void LuaAssFile::GetAssEntry(int n)
	{
		entryIter e;
		if (n < last_entry_id/2) {
			// fastest to search from start
			e = ass->Line.begin();
			last_entry_id = n;
			while (n-- > 0) e++;
			last_entry_ptr = e;

		} else if (last_entry_id + n > last_entry_id + ((int)ass->Line.size() - last_entry_id)/2) {
			// fastest to search from end
			int i = (int)ass->Line.size();
			e = ass->Line.end();
			last_entry_id = n;
			while (i-- > n) e--;
			last_entry_ptr = e;

		} else if (last_entry_id > n) {
			// search backwards from last_entry_id
			e = last_entry_ptr;
			while (n < last_entry_id) e--, last_entry_id--;
			last_entry_ptr = e;
			
		} else {
			// search forwards from last_entry_id
			e = last_entry_ptr;
			// reqid and last_entry_id might be equal here, make sure the loop will still work
			while (n > last_entry_id) e++, last_entry_id++;
			last_entry_ptr = e;
		}
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectIndexRead(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, 1);

		switch (lua_type(L, 2)) {

			case LUA_TNUMBER:
				{
					// read an indexed AssEntry

					// get requested index
					int reqid = lua_tointeger(L, 2);
					if (reqid <= 0 || reqid > (int)laf->ass->Line.size()) {
						lua_pushfstring(L, "Requested out-of-range line from subtitle file: %d", reqid);
						lua_error(L);
						return 0;
					}

					laf->GetAssEntry(reqid-1);
					laf->AssEntryToLua(L, *laf->last_entry_ptr);
					return 1;
				}

			case LUA_TSTRING:
				{
					// either return n or a function doing further stuff
					const char *idx = lua_tostring(L, 2);

					if (strcmp(idx, "n") == 0) {
						// get number of items
						lua_pushnumber(L, laf->ass->Line.size());
						return 1;

					} else if (strcmp(idx, "delete") == 0) {
						// make a "delete" function
						lua_pushvalue(L, 1);
						lua_pushcclosure(L, ObjectDelete, 1);
						return 1;

					} else if (strcmp(idx, "deleterange") == 0) {
						// make a "deleterange" function
						lua_pushvalue(L, 1);
						lua_pushcclosure(L, ObjectDeleteRange, 1);
						return 1;

					} else if (strcmp(idx, "insert") == 0) {
						// make an "insert" function
						lua_pushvalue(L, 1);
						lua_pushcclosure(L, ObjectInsert, 1);
						return 1;

					} else if (strcmp(idx, "append") == 0) {
						// make an "append" function
						lua_pushvalue(L, 1);
						lua_pushcclosure(L, ObjectAppend, 1);
						return 1;

					} else {
						// idiot
						lua_pushfstring(L, "Invalid indexing in Subtitle File object: '%s'", idx);
						lua_error(L);
						// should never return
					}
					assert(false);
				}

			default:
				{
					// crap, user is stupid!
					lua_pushfstring(L, "Attempt to index a Subtitle File object with value of type '%s'.", lua_typename(L, lua_type(L, 2)));
					lua_error(L);
				}
		}

		assert(false);
		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectIndexWrite(lua_State *L)
	{
		// instead of implementing everything twice, just call the other modification-functions from here
		// after modifying the stack to match their expectations

		if (!lua_isnumber(L, 2)) {
			lua_pushstring(L, "Attempt to write to non-numeric index in subtitle index");
			lua_error(L);
			return 0;
		}

		LuaAssFile *laf = GetObjPointer(L, 1);
		laf->CheckAllowModify();

		int n = lua_tointeger(L, 2);

		if (n < 0) {
			// insert line so new index is n
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, ObjectInsert, 1);
			lua_pushinteger(L, -n);
			lua_pushvalue(L, 3);
			lua_call(L, 2, 0);
			return 0;

		} else if (n == 0) {
			// append line to list
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, ObjectAppend, 1);
			lua_pushvalue(L, 3);
			lua_call(L, 1, 0);
			return 0;

		} else {
			// replace line at index n or delete
			if (!lua_isnil(L, 3)) {
				// insert
				AssEntry *e = LuaToAssEntry(L);
				laf->GetAssEntry(n-1);
				delete *laf->last_entry_ptr;
				*laf->last_entry_ptr = e;
				return 0;

			} else {
				// delete
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, ObjectDelete, 1);
				lua_pushvalue(L, 2);
				lua_call(L, 1, 0);
				return 0;

			}
		}
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectGetLen(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, 1);
		lua_pushnumber(L, laf->ass->Line.size());
		return 1;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectDelete(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		// get number of items to delete
		int itemcount = lua_gettop(L);
		std::vector<int> ids;
		ids.reserve(itemcount);

		// sort the item id's so we can delete from last to first to preserve original numbering
		while (itemcount > 0) {
			if (!lua_isnumber(L, itemcount)) {
				lua_pushstring(L, "Attempt to delete non-numeric line id from Subtitle Object");
				lua_error(L);
				return 0;
			}
			int n = lua_tointeger(L, itemcount);
			if (n > (int)laf->ass->Line.size() || n < 1) {
				lua_pushstring(L, "Attempt to delete out of range line id from Subtitle Object");
				lua_error(L);
				return 0;
			}
			ids.push_back(n-1); // make C-style line ids
			--itemcount;
		}
		std::sort(ids.begin(), ids.end());

		// now delete the id's backwards
		// start with the last one, to initialise things
		laf->GetAssEntry(ids.back());
		// get an iterator to it, and increase last_entry_ptr so it'll still be valid after deletion, and point to the right index
		entryIter e = laf->last_entry_ptr++;
		laf->ass->Line.erase(e);
		int n = laf->last_entry_id;
		for (int i = (int)ids.size()-2; i >= 0; --i) {
			int id = ids[i];
			while (id > n--) laf->last_entry_ptr--;
			e = laf->last_entry_ptr++;
			delete *e;
			laf->ass->Line.erase(e);
		}
		laf->last_entry_id = n;

		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectDeleteRange(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
			lua_pushstring(L, "Non-numeric argument given to deleterange");
			lua_error(L);
			return 0;
		}

		int a = lua_tointeger(L, 1), b = lua_tointeger(L, 2);

		if (a < 1) a = 1;
		if (b > (int)laf->ass->Line.size()) b = (int)laf->ass->Line.size();

		if (b < a) return 0;

		if (a == b) {
			laf->GetAssEntry(a-1);
			entryIter e = laf->last_entry_ptr++;
			delete *e;
			laf->ass->Line.erase(e);
			return 0;
		}

		entryIter ai, bi;
		laf->GetAssEntry(a-1);
		ai = laf->last_entry_ptr;
		laf->GetAssEntry(b-1);
		bi = laf->last_entry_ptr;
		laf->last_entry_ptr++;

		laf->ass->Line.erase(ai, bi);

		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectAppend(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		int n = lua_gettop(L);

		if (laf->last_entry_ptr != laf->ass->Line.begin()) {
			laf->last_entry_ptr--;
			laf->last_entry_id--;
		}

		for (int i = 1; i <= n; i++) {
			lua_pushvalue(L, i);
			AssEntry *e = LuaToAssEntry(L);
			if (e->GetType() == ENTRY_DIALOGUE) {
				// find insertion point, looking backwards
				std::list<AssEntry*>::iterator it = laf->ass->Line.end();
				do { --it; } while ((*it)->GetType() != ENTRY_DIALOGUE);
				// found last dialogue entry in file, move one past
				++it;
				laf->ass->Line.insert(it, e);
			}
			else {
				laf->ass->Line.push_back(e);
			}
		}

		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectInsert(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, lua_upvalueindex(1));

		laf->CheckAllowModify();
		
		if (!lua_isnumber(L, 1)) {
			lua_pushstring(L, "Can't insert at non-numeric index");
			lua_error(L);
			return 0;
		}

		int n = lua_gettop(L);

		laf->GetAssEntry(int(lua_tonumber(L, 1)-1));

		for (int i = 2; i <= n; i++) {
			lua_pushvalue(L, i);
			AssEntry *e = LuaToAssEntry(L);
			lua_pop(L, 1);
			laf->ass->Line.insert(laf->last_entry_ptr, e);
			laf->last_entry_id++;
		}

		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::ObjectGarbageCollect(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, 1);
		delete laf;
		LOG_D("automation/lua") << "Garbage collected LuaAssFile";
		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::LuaParseTagData(lua_State *L)
	{
		lua_newtable(L);
		// TODO
		return 1;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::LuaUnparseTagData(lua_State *L)
	{
		lua_pushstring(L, "");
		// TODO
		return 1;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::LuaParseKaraokeData(lua_State *L)
	{
		AssEntry *e = LuaToAssEntry(L);
		AssDialogue *dia = dynamic_cast<AssDialogue*>(e);
		if (!dia) {
			delete e;
			lua_pushstring(L, "Attempt to create karaoke table from non-dialogue subtitle line");
			lua_error(L);
			return 0;
		}

		dia->ParseASSTags();

		int kcount = 0;
		int kdur = 0;
		int ktime = 0;
		wxString ktag = _T("");
		wxString ktext = _T("");
		wxString ktext_stripped = _T("");

		lua_newtable(L);

		for (int i = 0; i < (int)dia->Blocks.size(); i++) {
			AssDialogueBlock *block = dia->Blocks[i];

			switch (block->GetType()) {

				case BLOCK_BASE:
					break;

				case BLOCK_PLAIN:
					ktext += block->text;
					ktext_stripped += block->text;
					break;

				case BLOCK_DRAWING:
					// a drawing is regarded as a kind of control code here, so it's just stripped away
					ktext += block->text;
					break;

				case BLOCK_OVERRIDE: {
					bool brackets_open = false;
					AssDialogueBlockOverride *ovr = dynamic_cast<AssDialogueBlockOverride*>(block);

					for (int j = 0; j < (int)ovr->Tags.size(); j++) {
						AssOverrideTag *tag = ovr->Tags[j];

						if (tag->IsValid() && tag->Name.Mid(0,2).CmpNoCase(_T("\\k")) == 0) {
							// karaoke tag
							if (brackets_open) {
								ktext += _T("}");
								brackets_open = false;
							}

							// store to lua
							lua_newtable(L);
							lua_pushnumber(L, kdur);
							lua_setfield(L, -2, "duration");
							lua_pushnumber(L, ktime);
							lua_setfield(L, -2, "start_time");
							lua_pushnumber(L, ktime+kdur);
							lua_setfield(L, -2, "end_time");
							lua_pushstring(L, ktag.mb_str(wxConvUTF8));
							lua_setfield(L, -2, "tag");
							lua_pushstring(L, ktext.mb_str(wxConvUTF8));
							lua_setfield(L, -2, "text");
							lua_pushstring(L, ktext_stripped.mb_str(wxConvUTF8));
							lua_setfield(L, -2, "text_stripped");
							lua_rawseti(L, -2, kcount);

							// prepare new syllable
							kcount++;
							ktag = tag->Name.Mid(1);
							// check if it's a "set time" tag, special handling for that (depends on previous syllable duration)
							if (ktag == _T("kt")) {
								ktime = tag->Params[0]->Get<int>() * 10;
								kdur = 0;
							} else {
								ktime += kdur; // duration of previous syllable
								kdur = tag->Params[0]->Get<int>() * 10;
							}
							ktext.clear();
							ktext_stripped.clear();

						} else {
							// not karaoke tag
							if (!brackets_open) {
								ktext += _T("{");
								brackets_open = true;
							}
							ktext += *tag;
						}

					}

					if (brackets_open) {
						ktext += _T("}");
						brackets_open = false;
					}

					break;
				}

			}

		}

		dia->ClearBlocks();

		// store final syllable/block to lua
		lua_newtable(L);
		lua_pushnumber(L, kdur);
		lua_setfield(L, -2, "duration");
		lua_pushnumber(L, ktime);
		lua_setfield(L, -2, "start_time");
		lua_pushnumber(L, ktime+kdur);
		lua_setfield(L, -2, "end_time");
		lua_pushstring(L, ktag.mb_str(wxConvUTF8));
		lua_setfield(L, -2, "tag");
		lua_pushstring(L, ktext.mb_str(wxConvUTF8));
		lua_setfield(L, -2, "text");
		lua_pushstring(L, ktext_stripped.mb_str(wxConvUTF8));
		lua_setfield(L, -2, "text_stripped");
		lua_rawseti(L, -2, kcount);

		delete dia;
		return 1;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaAssFile::LuaSetUndoPoint(lua_State *L)
	{
		LuaAssFile *laf = GetObjPointer(L, lua_upvalueindex(1));
		if (!laf->can_set_undo) {
			lua_pushstring(L, "Attempt to set an undo point in a context without undo-support.");
			lua_error(L);
			return 0;
		}

		wxString description;
		if (lua_isstring(L, 1)) {
			description = wxString(lua_tostring(L, 1), wxConvUTF8);
			lua_pop(L, 1);
		}
		AssFile::top->FlagAsModified(description);

		laf->ass = AssFile::top; // make sure we're still working on the most recent undo point
		return 0;
	}


	/// @brief DOCME
	/// @param L   
	/// @param idx 
	/// @return 
	///
	LuaAssFile *LuaAssFile::GetObjPointer(lua_State *L, int idx)
	{
		assert(lua_type(L, idx) == LUA_TUSERDATA);
		void *ud = lua_touserdata(L, idx);
		return *((LuaAssFile**)ud);
	}


	/// @brief DOCME
	///
	LuaAssFile::~LuaAssFile()
	{
	}


	/// @brief DOCME
	/// @param _L            
	/// @param _ass          
	/// @param _can_modify   
	/// @param _can_set_undo 
	///
	LuaAssFile::LuaAssFile(lua_State *_L, AssFile *_ass, bool _can_modify, bool _can_set_undo)
		: ass(_ass)
		, L(_L)
		, can_modify(_can_modify)
		, can_set_undo(_can_set_undo)
	{
		// prepare cursor
		last_entry_ptr = ass->Line.begin();
		last_entry_id = 0;

		// prepare userdata object
		void *ud = lua_newuserdata(L, sizeof(LuaAssFile*));
		*((LuaAssFile**)ud) = this;

		// make the metatable
		lua_newtable(L);
		lua_pushcfunction(L, ObjectIndexRead);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, ObjectIndexWrite);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, ObjectGetLen);
		lua_setfield(L, -2, "__len");
		lua_pushcfunction(L, ObjectGarbageCollect);
		lua_setfield(L, -2, "__gc");
		lua_setmetatable(L, -2);

		// register misc functions
		// assume the "aegisub" global table exists
		lua_getglobal(L, "aegisub");
		assert(lua_type(L, -2) == LUA_TUSERDATA);
		lua_pushvalue(L, -2); // the userdata object
		lua_pushcclosure(L, LuaParseTagData, 1);
		lua_setfield(L, -2, "parse_tag_data");
		assert(lua_type(L, -2) == LUA_TUSERDATA);
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaUnparseTagData, 1);
		lua_setfield(L, -2, "unparse_tag_data");
		assert(lua_type(L, -2) == LUA_TUSERDATA);
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaParseKaraokeData, 1);
		lua_setfield(L, -2, "parse_karaoke_data");
		assert(lua_type(L, -2) == LUA_TUSERDATA);
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaSetUndoPoint, 1);
		lua_setfield(L, -2, "set_undo_point");
		lua_pop(L, 1);
	}

};

#endif // WITH_AUTO4_LUA



