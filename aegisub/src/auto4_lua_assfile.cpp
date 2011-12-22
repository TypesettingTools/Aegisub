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

#include <libaegisub/exception.h>
#include <libaegisub/log.h>
#include <libaegisub/scoped_ptr.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_karaoke.h"
#include "ass_override.h"
#include "ass_style.h"
#include "auto4_lua.h"
#include "utils.h"

// This must be below the headers above.
#ifdef __WINDOWS__
#include "../../contrib/lua51/src/lualib.h"
#include "../../contrib/lua51/src/lauxlib.h"
#else
#include <lua.hpp>
#endif

namespace {
	void set_field(lua_State *L, const char *name, wxString const& value)
	{
		lua_pushstring(L, value.utf8_str());
		lua_setfield(L, -2, name);
	}

	void set_field(lua_State *L, const char *name, const char *value)
	{
		lua_pushstring(L, value);
		lua_setfield(L, -2, name);
	}

	// Userdata object must be just above the target table
	void set_field(lua_State *L, const char *name, lua_CFunction value)
	{
		assert(lua_type(L, -2) == LUA_TUSERDATA);
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, value, 1);
		lua_setfield(L, -2, name);
	}

	void set_field(lua_State *L, const char *name, bool value)
	{
		lua_pushboolean(L, value);
		lua_setfield(L, -2, name);
	}

	void set_field(lua_State *L, const char *name, double value)
	{
		lua_pushnumber(L, value);
		lua_setfield(L, -2, name);
	}

	void set_field(lua_State *L, const char *name, int value)
	{
		lua_pushinteger(L, value);
		lua_setfield(L, -2, name);
	}

	DEFINE_SIMPLE_EXCEPTION_NOINNER(BadField, Automation4::MacroRunError, "automation/macro/bad_field")
	BadField bad_field(const char *expected_type, const char *name, const char *line_clasee)
	{
		return BadField(std::string("Invalid or missing field '") + name + "' in '" + line_clasee + "' class subtitle line (expected " + expected_type + ")");
	}

	wxString get_string_field(lua_State *L, const char *name, const char *line_class)
	{
		lua_getfield(L, -1, name);
		if (!lua_isstring(L, -1))
			throw bad_field("string", name, line_class);
		wxString ret(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 1);
		return ret;
	}

	double get_double_field(lua_State *L, const char *name, const char *line_class)
	{
		lua_getfield(L, -1, name);
		if (!lua_isnumber(L, -1))
			throw bad_field("number", name, line_class);
		double ret = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	int get_int_field(lua_State *L, const char *name, const char *line_class)
	{
		lua_getfield(L, -1, name);
		if (!lua_isnumber(L, -1))
			throw bad_field("number", name, line_class);
		int ret = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	bool get_bool_field(lua_State *L, const char *name, const char *line_class)
	{
		lua_getfield(L, -1, name);
		if (!lua_isboolean(L, -1))
			throw bad_field("boolean", name, line_class);
		bool ret = !!lua_toboolean(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	using namespace Automation4;
	template<int (LuaAssFile::*closure)(lua_State *)>
	int closure_wrapper(lua_State *L)
	{
		return (LuaAssFile::GetObjPointer(L, lua_upvalueindex(1))->*closure)(L);
	}

	template<void (LuaAssFile::*closure)(lua_State *)>
	int closure_wrapper_v(lua_State *L)
	{
		(LuaAssFile::GetObjPointer(L, lua_upvalueindex(1))->*closure)(L);
		return 0;
	}

	int modification_mask(AssEntry *e)
	{
		switch (e->GetType())
		{
			case ENTRY_DIALOGUE: return AssFile::COMMIT_DIAG_ADDREM;
			case ENTRY_STYLE: return AssFile::COMMIT_STYLES;
			case ENTRY_ATTACHMENT: return AssFile::COMMIT_ATTACHMENT;
			default: return AssFile::COMMIT_SCRIPTINFO;
		}
	}
}

namespace Automation4 {
	void LuaAssFile::CheckAllowModify()
	{
		if (!can_modify)
			luaL_error(L, "Attempt to modify subtitles in read-only feature context.");
	}

	void LuaAssFile::AssEntryToLua(lua_State *L, AssEntry *e)
	{
		lua_newtable(L);

		wxString raw(e->GetEntryData());

		set_field(L, "section", e->group);
		set_field(L, "raw", raw);

		if (StringEmptyOrWhitespace(raw)) {
			set_field(L, "class", "clear");
		}
		else if (raw[0] == ';') {
			// "text" field, same as "raw" but with semicolon stripped
			set_field(L, "text", raw.Mid(1));
			set_field(L, "class", "comment");
		}
		else if (raw[0] == '[') {
			set_field(L, "class", "head");
		}
		else if (e->group.Lower() == "[script info]") {
			set_field(L, "key", raw.BeforeFirst(':'));
			set_field(L, "value", raw.AfterFirst(':'));
			set_field(L, "class", "info");
		}
		else if (raw.Left(7).Lower() == "format:") {
			// TODO: parse the format line; just use a tokenizer
			set_field(L, "class", "format");
		}
		else if (AssDialogue *dia = dynamic_cast<AssDialogue*>(e)) {
			set_field(L, "comment", dia->Comment);

			set_field(L, "layer", dia->Layer);

			set_field(L, "start_time", dia->Start);
			set_field(L, "end_time", dia->End);

			set_field(L, "style", dia->Style);
			set_field(L, "actor", dia->Actor);
			set_field(L, "effect", dia->Effect);

			set_field(L, "margin_l", dia->Margin[0]);
			set_field(L, "margin_r", dia->Margin[1]);
			set_field(L, "margin_t", dia->Margin[2]);
			set_field(L, "margin_b", dia->Margin[3]);

			set_field(L, "text", dia->Text);

			set_field(L, "class", "dialogue");
		}
		else if (AssStyle *sty = dynamic_cast<AssStyle*>(e)) {
			set_field(L, "name", sty->name);

			set_field(L, "fontname", sty->font);
			set_field(L, "fontsize", sty->fontsize);

			set_field(L, "color1", sty->primary.GetASSFormatted(true));
			set_field(L, "color2", sty->secondary.GetASSFormatted(true));
			set_field(L, "color3", sty->outline.GetASSFormatted(true));
			set_field(L, "color4", sty->shadow.GetASSFormatted(true));

			set_field(L, "bold", sty->bold);
			set_field(L, "italic", sty->italic);
			set_field(L, "underline", sty->underline);
			set_field(L, "strikeout", sty->strikeout);

			set_field(L, "scale_x", sty->scalex);
			set_field(L, "scale_y", sty->scaley);

			set_field(L, "spacing", sty->spacing);

			set_field(L, "angle", sty->angle);

			set_field(L, "borderstyle", sty->borderstyle);
			set_field(L, "outline", sty->outline_w);
			set_field(L, "shadow", sty->shadow_w);

			set_field(L, "align", sty->alignment);

			set_field(L, "margin_l", sty->Margin[0]);
			set_field(L, "margin_r", sty->Margin[1]);
			set_field(L, "margin_t", sty->Margin[2]);
			set_field(L, "margin_b", sty->Margin[3]);

			set_field(L, "encoding", sty->encoding);

			set_field(L, "relative_to", 2);// From STS.h: "0: window, 1: video, 2: undefined (~window)"

			set_field(L, "class", "style");
		}
		else {
			set_field(L, "class", "unknown");
		}
	}

	AssEntry *LuaAssFile::LuaToAssEntry(lua_State *L)
	{
		// assume an assentry table is on the top of the stack
		// convert it to a real AssEntry object, and pop the table from the stack

		if (!lua_istable(L, -1))
			luaL_error(L, "Can't convert a non-table value to AssEntry");

		lua_getfield(L, -1, "class");
		if (!lua_isstring(L, -1))
			luaL_error(L, "Table lacks 'class' field, can't convert to AssEntry");

		wxString lclass(lua_tostring(L, -1), wxConvUTF8);
		lclass.MakeLower();
		lua_pop(L, 1);

		AssEntry *result = 0;

		try {
			wxString section = get_string_field(L, "section", "common");

			if (lclass == "clear")
				result = new AssEntry("");
			else if (lclass == "comment")
				result = new AssEntry(";" + get_string_field(L, "text", "comment"));
			else if (lclass == "head")
				result = new AssEntry(section);
			else if (lclass == "info") {
				result = new AssEntry(wxString::Format("%s: %s", get_string_field(L, "key", "info"), get_string_field(L, "value", "info")));
				result->group = "[Script Info]"; // just so it can be read correctly back
			}
			else if (lclass == "format") {
				// ohshi- ...
				// *FIXME* maybe ignore the actual data and just put some default stuff based on section?
				result = new AssEntry("Format: Auto4,Is,Broken");
			}
			else if (lclass == "style") {
				AssStyle *sty = new AssStyle;
				result = sty;
				sty->name = get_string_field(L, "name", "style");
				sty->font = get_string_field(L, "fontname", "style");
				sty->fontsize = get_double_field(L, "fontsize", "style");
				sty->primary.Parse(get_string_field(L, "color1", "style"));
				sty->secondary.Parse(get_string_field(L, "color2", "style"));
				sty->outline.Parse(get_string_field(L, "color3", "style"));
				sty->shadow.Parse(get_string_field(L, "color4", "style"));
				sty->bold = get_bool_field(L, "bold", "style");
				sty->italic = get_bool_field(L, "italic", "style");
				sty->underline = get_bool_field(L, "underline", "style");
				sty->strikeout = get_bool_field(L, "strikeout", "style");
				sty->scalex = get_double_field(L, "scale_x", "style");
				sty->scaley = get_double_field(L, "scale_y", "style");
				sty->spacing = get_double_field(L, "spacing", "style");
				sty->angle = get_double_field(L, "angle", "style");
				sty->borderstyle = get_int_field(L, "borderstyle", "style");
				sty->outline_w = get_double_field(L, "outline", "style");
				sty->shadow_w = get_double_field(L, "shadow", "style");
				sty->alignment = get_int_field(L, "align", "style");
				sty->Margin[0] = get_int_field(L, "margin_l", "style");
				sty->Margin[1] = get_int_field(L, "margin_r", "style");
				sty->Margin[2] = get_int_field(L, "margin_t", "style");
				sty->Margin[3] = get_int_field(L, "margin_b", "style");
				sty->encoding = get_int_field(L, "encoding", "style");
				sty->UpdateData();
			}
			else if (lclass == "dialogue") {
				AssDialogue *dia = new AssDialogue;
				result = dia;

				dia->Comment = get_bool_field(L, "comment", "dialogue");
				dia->Layer = get_int_field(L, "layer", "dialogue");
				dia->Start = get_int_field(L, "start_time", "dialogue");
				dia->End = get_int_field(L, "end_time", "dialogue");
				dia->Style = get_string_field(L, "style", "dialogue");
				dia->Actor = get_string_field(L, "actor", "dialogue");
				dia->Margin[0] = get_int_field(L, "margin_l", "dialogue");
				dia->Margin[1] = get_int_field(L, "margin_r", "dialogue");
				dia->Margin[2] = get_int_field(L, "margin_t", "dialogue");
				dia->Margin[3] = get_int_field(L, "margin_b", "dialogue");
				dia->Effect = get_string_field(L, "effect", "dialogue");
				dia->Text = get_string_field(L, "text", "dialogue");
			}
			else
				luaL_error(L, "Found line with unknown class: %s", lclass.utf8_str().data());

			if (result->group.empty())
				result->group = section;

			return result;
		}
		catch (agi::Exception const& e) {
			delete result;
			luaL_error(L, e.GetMessage().c_str());
			return 0;
		}
	}

	void LuaAssFile::SeekCursorTo(int n)
	{
		if (n <= 0 || n > (int)lines.size())
			luaL_error(L, "Requested out-of-range line from subtitle file: %d", n);

		if (n < last_entry_id - n) {
			// fastest to search from start
			last_entry_ptr = lines.begin();
			last_entry_id = 1;
		}
		else if ((int)lines.size() - last_entry_id < abs(last_entry_id - n)) {
			// fastest to search from end
			last_entry_ptr = lines.end();
			last_entry_id = lines.size() + 1;
		}
		// otherwise fastest to search from cursor

		advance(last_entry_ptr, n - last_entry_id);
		last_entry_id = n;
	}

	int LuaAssFile::ObjectIndexRead(lua_State *L)
	{
		switch (lua_type(L, 2)) {
			case LUA_TNUMBER:
				// read an indexed AssEntry
				SeekCursorTo(lua_tointeger(L, 2));
				AssEntryToLua(L, *last_entry_ptr);
				return 1;

			case LUA_TSTRING:
			{
				// either return n or a function doing further stuff
				const char *idx = lua_tostring(L, 2);

				if (strcmp(idx, "n") == 0) {
					// get number of items
					lua_pushnumber(L, lines.size());
					return 1;
				}

				lua_pushvalue(L, 1);
				if (strcmp(idx, "delete") == 0)
					lua_pushcclosure(L, closure_wrapper_v<&LuaAssFile::ObjectDelete>, 1);
				else if (strcmp(idx, "deleterange") == 0)
					lua_pushcclosure(L, closure_wrapper_v<&LuaAssFile::ObjectDeleteRange>, 1);
				else if (strcmp(idx, "insert") == 0)
					lua_pushcclosure(L, closure_wrapper_v<&LuaAssFile::ObjectInsert>, 1);
				else if (strcmp(idx, "append") == 0)
					lua_pushcclosure(L, closure_wrapper_v<&LuaAssFile::ObjectAppend>, 1);
				else {
					// idiot
					lua_pop(L, 1);
					return luaL_error(L, "Invalid indexing in Subtitle File object: '%s'", idx);
				}

				return 1;
			}

			default:
				// crap, user is stupid!
				return luaL_error(L, "Attempt to index a Subtitle File object with value of type '%s'.", lua_typename(L, lua_type(L, 2)));
		}

		assert(false);
		return 0;
	}

	void LuaAssFile::ObjectIndexWrite(lua_State *L)
	{
		// instead of implementing everything twice, just call the other modification-functions from here
		// after modifying the stack to match their expectations

		CheckAllowModify();

		int n = luaL_checkint(L, 2);
		if (n < 0) {
			// insert line so new index is n
			lua_remove(L, 1);
			lua_pushinteger(L, -n);
			lua_replace(L, 1);
			ObjectInsert(L);
		}
		else if (n == 0) {
			// append line to list
			lua_remove(L, 1);
			lua_remove(L, 1);
			ObjectAppend(L);
		}
		else {
			// replace line at index n or delete
			if (!lua_isnil(L, 3)) {
				// insert
				AssEntry *e = LuaToAssEntry(L);
				modification_type |= modification_mask(e);
				SeekCursorTo(n);
				lines_to_delete.push_back(*last_entry_ptr);
				*last_entry_ptr = e;
			}
			else {
				// delete
				lua_remove(L, 1);
				lua_remove(L, 1);
				ObjectDelete(L);
			}
		}
	}

	int LuaAssFile::ObjectGetLen(lua_State *L)
	{
		lua_pushnumber(L, lines.size());
		return 1;
	}

	void LuaAssFile::ObjectDelete(lua_State *L)
	{
		CheckAllowModify();

		// get number of items to delete
		int itemcount = lua_gettop(L);
		std::vector<int> ids;
		ids.reserve(itemcount);

		while (itemcount > 0) {
			int n = luaL_checkint(L, itemcount);
			luaL_argcheck(L, n > 0 && n <= (int)lines.size(), itemcount, "Out of range line index");
			ids.push_back(n);
			--itemcount;
		}

		// sort the item id's so we can delete from last to first to preserve original numbering
		sort(ids.begin(), ids.end());

		// now delete the id's backwards
		SeekCursorTo(ids.back());
		for (size_t i = ids.size(); i > 0; --i) {
			while (last_entry_id > ids[i - 1]) {
				--last_entry_ptr;
				--last_entry_id;
			}
			modification_type |= modification_mask(*last_entry_ptr);
			lines_to_delete.push_back(*last_entry_ptr);
			lines.erase(last_entry_ptr++);
		}
	}

	void LuaAssFile::ObjectDeleteRange(lua_State *L)
	{
		CheckAllowModify();

		int a = std::max<int>(luaL_checkinteger(L, 1), 1);
		int b = std::min<int>(luaL_checkinteger(L, 2), lines.size());

		SeekCursorTo(a);

		while (a++ <= b) {
			modification_type |= modification_mask(*last_entry_ptr);
			lines_to_delete.push_back(*last_entry_ptr);
			lines.erase(last_entry_ptr++);
		}
	}

	void LuaAssFile::ObjectAppend(lua_State *L)
	{
		CheckAllowModify();

		int n = lua_gettop(L);

		if (last_entry_ptr != lines.begin()) {
			last_entry_ptr--;
			last_entry_id--;
		}

		for (int i = 1; i <= n; i++) {
			lua_pushvalue(L, i);
			AssEntry *e = LuaToAssEntry(L);
			modification_type |= modification_mask(e);

			// Find the appropriate place to put it
			std::list<AssEntry*>::iterator it = lines.end();
			if (!lines.empty()) {
				do {
					--it;
				}
				while (it != lines.begin() && (*it)->group != e->group);
			}

			if (it == lines.end() || (*it)->group != e->group) {
				// The new entry belongs to a group that doesn't exist yet, so
				// create it at the end of the file
				if (e->GetEntryData() != e->group) {
					// Add the header if the entry being added isn't a header
					AssEntry *header = new AssEntry(e->group);
					header->group = e->group;
					lines.push_back(header);
				}

				lines.push_back(e);
			}
			else {
				// Append the entry to the end of the existing group
				++it;
				lines.insert(it, e);
			}
		}

		// If last_entry_ptr is end, the file was empty but no longer is, so
		// last_entry_id is wrong
		if (last_entry_ptr == lines.end())
			last_entry_id = lines.size() + 1;
	}

	void LuaAssFile::ObjectInsert(lua_State *L)
	{
		CheckAllowModify();

		int before = luaL_checkinteger(L, 1);

		// + 1 to allow appending at the end of the file
		luaL_argcheck(L, before > 0 && before <= (int)lines.size() + 1, 1,
			"Out of range line index");

		if (before == (int)lines.size() + 1) {
			lua_remove(L, 1);
			ObjectAppend(L);
			return;
		}

		SeekCursorTo(before);

		int n = lua_gettop(L);
		for (int i = 2; i <= n; i++) {
			lua_pushvalue(L, i);
			AssEntry *e = LuaToAssEntry(L);
			modification_type |= modification_mask(e);
			lines.insert(last_entry_ptr, e);
			lua_pop(L, 1);
		}

		last_entry_id += n - 1;
	}

	void LuaAssFile::ObjectGarbageCollect(lua_State *L)
	{
		references--;
		if (!references) delete this;
		LOG_D("automation/lua") << "Garbage collected LuaAssFile";
	}

	int LuaAssFile::LuaParseKaraokeData(lua_State *L)
	{
		agi::scoped_ptr<AssEntry> e(LuaToAssEntry(L));
		AssDialogue *dia = dynamic_cast<AssDialogue*>(e.get());
		luaL_argcheck(L, dia, 1, "Subtitle line must be a dialogue line");

		int idx = 0;
		AssKaraoke kara(dia);
		for (AssKaraoke::iterator it = kara.begin(); it != kara.end(); ++it) {
			lua_newtable(L);
			set_field(L, "duration", it->duration);
			set_field(L, "start_time", it->start_time);
			set_field(L, "end_time", it->start_time + it->duration);
			set_field(L, "tag", it->tag_type);
			set_field(L, "text", it->GetText(false));
			set_field(L, "text_stripped", it->text);
			lua_rawseti(L, -2, idx++);
		}

		return 1;
	}

	void LuaAssFile::LuaSetUndoPoint(lua_State *L)
	{
		if (!can_set_undo)
			luaL_error(L, "Attempt to set an undo point in a context where it makes no sense to do so.");

		if (modification_type) {
			pending_commits.push_back(PendingCommit());
			PendingCommit& back = pending_commits.back();

			back.modification_type = modification_type;
			back.mesage = wxString(luaL_checkstring(L, 1), wxConvUTF8);
			back.lines = lines;
			modification_type = 0;
		}
	}

	LuaAssFile *LuaAssFile::GetObjPointer(lua_State *L, int idx)
	{
		assert(lua_type(L, idx) == LUA_TUSERDATA);
		void *ud = lua_touserdata(L, idx);
		return *((LuaAssFile**)ud);
	}

	void LuaAssFile::ProcessingComplete(wxString const& undo_description)
	{
		// Apply any pending commits
		for (std::deque<PendingCommit>::iterator it = pending_commits.begin(); it != pending_commits.end(); ++it) {
			swap(ass->Line, it->lines);
			ass->Commit(it->mesage, it->modification_type);
		}

		// Commit any changes after the last undo point was set
		if (modification_type)
			swap(ass->Line, lines);
		if (modification_type && can_set_undo && !undo_description.empty())
			ass->Commit(undo_description, modification_type);

		delete_clear(lines_to_delete);

		references--;
		if (!references) delete this;
	}

	void LuaAssFile::Cancel()
	{
		references--;
		if (!references) delete this;
	}

	LuaAssFile::LuaAssFile(lua_State *L, AssFile *ass, bool can_modify, bool can_set_undo)
	: ass(ass)
	, L(L)
	, can_modify(can_modify)
	, can_set_undo(can_set_undo)
	, modification_type(0)
	, references(2)
	, lines(ass->Line)
	, last_entry_ptr(lines.begin())
	, last_entry_id(1)
	{
		// prepare userdata object
		*static_cast<LuaAssFile**>(lua_newuserdata(L, sizeof(LuaAssFile**))) = this;

		// make the metatable
		lua_newtable(L);
		set_field(L, "__index", closure_wrapper<&LuaAssFile::ObjectIndexRead>);
		set_field(L, "__newindex", closure_wrapper_v<&LuaAssFile::ObjectIndexWrite>);
		set_field(L, "__len", closure_wrapper<&LuaAssFile::ObjectGetLen>);
		set_field(L, "__gc", closure_wrapper_v<&LuaAssFile::ObjectGarbageCollect>);
		lua_setmetatable(L, -2);

		// register misc functions
		// assume the "aegisub" global table exists
		lua_getglobal(L, "aegisub");

		set_field(L, "parse_karaoke_data", closure_wrapper<&LuaAssFile::LuaParseKaraokeData>);
		set_field(L, "set_undo_point", closure_wrapper_v<&LuaAssFile::LuaSetUndoPoint>);

		lua_pop(L, 1); // pop "aegisub" table

		// Leaves userdata object on stack
	}
};

#endif // WITH_AUTO4_LUA
