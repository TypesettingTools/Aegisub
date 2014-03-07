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

/// @file auto4_lua_assfile.cpp
/// @brief Lua 5.1-based scripting engine (interface to subtitle files)
/// @ingroup scripting
///

#include "config.h"

#include "auto4_lua.h"

#include "auto4_lua_utils.h"
#include "ass_dialogue.h"
#include "ass_info.h"
#include "ass_file.h"
#include "ass_karaoke.h"
#include "ass_style.h"
#include "utils.h"

#include <libaegisub/exception.h>
#include <libaegisub/util.h>

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <cassert>
#include <memory>

namespace {
	DEFINE_SIMPLE_EXCEPTION_NOINNER(BadField, Automation4::MacroRunError, "automation/macro/bad_field")
	BadField bad_field(const char *expected_type, const char *name, const char *line_clasee)
	{
		return BadField(std::string("Invalid or missing field '") + name + "' in '" + line_clasee + "' class subtitle line (expected " + expected_type + ")");
	}

	std::string get_string_field(lua_State *L, const char *name, const char *line_class)
	{
		lua_getfield(L, -1, name);
		if (!lua_isstring(L, -1))
			throw bad_field("string", name, line_class);
		std::string ret(lua_tostring(L, -1));
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
		switch (e->Group())
		{
			case AssEntryGroup::DIALOGUE: return AssFile::COMMIT_DIAG_ADDREM;
			case AssEntryGroup::STYLE:    return AssFile::COMMIT_STYLES;
			default:                      return AssFile::COMMIT_SCRIPTINFO;
		}
	}
}

namespace Automation4 {
	LuaAssFile::~LuaAssFile() { }

	void LuaAssFile::CheckAllowModify()
	{
		if (!can_modify)
			luaL_error(L, "Attempt to modify subtitles in read-only feature context.");
	}

	void LuaAssFile::CheckBounds(int idx)
	{
		if (idx <= 0 || idx > (int)lines.size())
			luaL_error(L, "Requested out-of-range line from subtitle file: %d", idx);
	}

	void LuaAssFile::AssEntryToLua(lua_State *L, AssEntry *e)
	{
		lua_newtable(L);

		set_field(L, "section", e->GroupHeader());
		set_field(L, "raw", e->GetEntryData());

		if (AssInfo *info = dynamic_cast<AssInfo*>(e)) {
			set_field(L, "key", info->Key());
			set_field(L, "value", info->Value());
			set_field(L, "class", "info");
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
			set_field(L, "margin_b", dia->Margin[2]);

			set_field(L, "text", dia->Text);

			set_field(L, "class", "dialogue");
		}
		else if (AssStyle *sty = dynamic_cast<AssStyle*>(e)) {
			set_field(L, "name", sty->name);

			set_field(L, "fontname", sty->font);
			set_field(L, "fontsize", sty->fontsize);

			set_field(L, "color1", sty->primary.GetAssStyleFormatted() + "&");
			set_field(L, "color2", sty->secondary.GetAssStyleFormatted() + "&");
			set_field(L, "color3", sty->outline.GetAssStyleFormatted() + "&");
			set_field(L, "color4", sty->shadow.GetAssStyleFormatted() + "&");

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
			set_field(L, "margin_b", sty->Margin[2]);

			set_field(L, "encoding", sty->encoding);

			set_field(L, "relative_to", 2);// From STS.h: "0: window, 1: video, 2: undefined (~window)"

			set_field(L, "class", "style");
		}
		else {
			assert(false);
		}
	}

	std::unique_ptr<AssEntry> LuaAssFile::LuaToAssEntry(lua_State *L)
	{
		// assume an assentry table is on the top of the stack
		// convert it to a real AssEntry object, and pop the table from the stack

		if (!lua_istable(L, -1))
			luaL_error(L, "Can't convert a non-table value to AssEntry");

		lua_getfield(L, -1, "class");
		if (!lua_isstring(L, -1))
			luaL_error(L, "Table lacks 'class' field, can't convert to AssEntry");

		std::string lclass(lua_tostring(L, -1));
		boost::to_lower(lclass);
		lua_pop(L, 1);

		std::unique_ptr<AssEntry> result;

		try {
			if (lclass == "info")
				result = agi::util::make_unique<AssInfo>(get_string_field(L, "key", "info"), get_string_field(L, "value", "info"));
			else if (lclass == "style") {
				auto sty = new AssStyle;
				result.reset(sty);
				sty->name = get_string_field(L, "name", "style");
				sty->font = get_string_field(L, "fontname", "style");
				sty->fontsize = get_double_field(L, "fontsize", "style");
				sty->primary = get_string_field(L, "color1", "style");
				sty->secondary = get_string_field(L, "color2", "style");
				sty->outline = get_string_field(L, "color3", "style");
				sty->shadow = get_string_field(L, "color4", "style");
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
				sty->encoding = get_int_field(L, "encoding", "style");
				sty->UpdateData();
			}
			else if (lclass == "dialogue") {
				auto dia = new AssDialogue;
				result.reset(dia);

				dia->Comment = get_bool_field(L, "comment", "dialogue");
				dia->Layer = get_int_field(L, "layer", "dialogue");
				dia->Start = get_int_field(L, "start_time", "dialogue");
				dia->End = get_int_field(L, "end_time", "dialogue");
				dia->Style = get_string_field(L, "style", "dialogue");
				dia->Actor = get_string_field(L, "actor", "dialogue");
				dia->Margin[0] = get_int_field(L, "margin_l", "dialogue");
				dia->Margin[1] = get_int_field(L, "margin_r", "dialogue");
				dia->Margin[2] = get_int_field(L, "margin_t", "dialogue");
				dia->Effect = get_string_field(L, "effect", "dialogue");
				dia->Text = get_string_field(L, "text", "dialogue");
			}
			else {
				luaL_error(L, "Found line with unknown class: %s", lclass.c_str());
				return nullptr;
			}

			return result;
		}
		catch (agi::Exception const& e) {
			luaL_error(L, e.GetMessage().c_str());
			return nullptr;
		}
	}

	int LuaAssFile::ObjectIndexRead(lua_State *L)
	{
		switch (lua_type(L, 2)) {
			case LUA_TNUMBER:
			{
				// read an indexed AssEntry
				int idx = lua_tointeger(L, 2);
				CheckBounds(idx);
				AssEntryToLua(L, lines[idx - 1]);
				return 1;
			}

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
				auto e = LuaToAssEntry(L);
				modification_type |= modification_mask(e.get());
				CheckBounds(n);
				lines_to_delete.emplace_back(lines[n - 1]);
				lines[n - 1] = e.release();
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
			ids.push_back(n - 1);
			--itemcount;
		}

		sort(ids.begin(), ids.end());

		size_t id_idx = 0, out = 0;
		for (size_t i = 0; i < lines.size(); ++i) {
			if (id_idx < ids.size() && ids[id_idx] == i) {
				modification_type |= modification_mask(lines[i]);
				lines_to_delete.emplace_back(lines[i]);
				++id_idx;
			}
			else {
				lines[out++] = lines[i];
			}
		}

		lines.erase(lines.begin() + out, lines.end());
	}

	void LuaAssFile::ObjectDeleteRange(lua_State *L)
	{
		CheckAllowModify();

		size_t a = std::max<size_t>(luaL_checkinteger(L, 1), 1) - 1;
		size_t b = std::min<size_t>(luaL_checkinteger(L, 2), lines.size());

		if (a >= b) return;

		for (size_t i = a; i < b; ++i) {
			modification_type |= modification_mask(lines[i]);
			lines_to_delete.emplace_back(lines[i]);
		}

		lines.erase(lines.begin() + a, lines.begin() + b);
	}

	void LuaAssFile::ObjectAppend(lua_State *L)
	{
		CheckAllowModify();

		int n = lua_gettop(L);

		for (int i = 1; i <= n; i++) {
			lua_pushvalue(L, i);
			auto e = LuaToAssEntry(L);
			modification_type |= modification_mask(e.get());

			// Find the appropriate place to put it
			auto it = lines.end();
			if (!lines.empty()) {
				do {
					--it;
				}
				while (it != lines.begin() && (*it)->Group() != e->Group());
			}

			if (it == lines.end() || (*it)->Group() != e->Group()) {
				// The new entry belongs to a group that doesn't exist yet, so
				// create it at the end of the file
				lines.push_back(e.release());
			}
			else {
				// Append the entry to the end of the existing group
				lines.insert(++it, e.release());
			}
		}
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

		int n = lua_gettop(L);
		std::vector<AssEntry *> new_entries(n - 1, nullptr);
		for (int i = 2; i <= n; i++) {
			lua_pushvalue(L, i);
			auto e = LuaToAssEntry(L);
			modification_type |= modification_mask(e.get());
			new_entries[i - 2] = e.release();
			lua_pop(L, 1);
		}
		lines.insert(lines.begin() + before - 1, new_entries.begin(), new_entries.end());
	}

	void LuaAssFile::ObjectGarbageCollect(lua_State *L)
	{
		references--;
		if (!references) delete this;
		LOG_D("automation/lua") << "Garbage collected LuaAssFile";
	}

	int LuaAssFile::ObjectIPairs(lua_State *L)
	{
		lua_pushvalue(L, lua_upvalueindex(1)); // push 'this' as userdata
		lua_pushcclosure(L, closure_wrapper<&LuaAssFile::IterNext>, 1);
		lua_pushnil(L);
		push_value(L, 0);
		return 3;
	}

	int LuaAssFile::IterNext(lua_State *L)
	{
		int i = luaL_checkint(L, 2);
		if (i >= (int)lines.size()) {
			lua_pushnil(L);
			return 1;
		}

		push_value(L, i + 1);
		AssEntryToLua(L, lines[i]);
		return 2;
	}

	int LuaAssFile::LuaParseKaraokeData(lua_State *L)
	{
		auto e = LuaToAssEntry(L);
		AssDialogue *dia = dynamic_cast<AssDialogue*>(e.get());
		luaL_argcheck(L, dia, 1, "Subtitle line must be a dialogue line");

		int idx = 0;

		// 2.1.x stored everything before the first syllable at index zero
		// There's no longer any such thing with the new parser, but scripts
		// may rely on kara[0] existing so add an empty syllable
		lua_newtable(L);
		set_field(L, "duration", 0);
		set_field(L, "start_time", 0);
		set_field(L, "end_time", 0);
		set_field(L, "tag", "");
		set_field(L, "text", "");
		set_field(L, "text_stripped", "");
		lua_rawseti(L, -2, idx++);

		AssKaraoke kara(dia, false, false);
		for (auto const& syl : kara) {
			lua_newtable(L);
			set_field(L, "duration", syl.duration);
			set_field(L, "start_time", syl.start_time - dia->Start);
			set_field(L, "end_time", syl.start_time + syl.duration - dia->Start);
			set_field(L, "tag", syl.tag_type);
			set_field(L, "text", syl.GetText(false));
			set_field(L, "text_stripped", syl.text);
			lua_rawseti(L, -2, idx++);
		}

		return 1;
	}

	void LuaAssFile::LuaSetUndoPoint(lua_State *L)
	{
		if (!can_set_undo)
			luaL_error(L, "Attempt to set an undo point in a context where it makes no sense to do so.");

		if (modification_type) {
			pending_commits.emplace_back();
			PendingCommit& back = pending_commits.back();

			back.modification_type = modification_type;
			back.mesage = wxString::FromUTF8(luaL_checkstring(L, 1));
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

	std::vector<AssEntry *> LuaAssFile::ProcessingComplete(wxString const& undo_description)
	{
		auto apply_lines = [&](std::vector<AssEntry *> const& lines) {
			ass->Info.clear();
			ass->Styles.clear();
			ass->Events.clear();

			for (auto line : lines) {
				switch (line->Group()) {
					case AssEntryGroup::INFO:     ass->Info.push_back(*static_cast<AssInfo *>(line)); break;
					case AssEntryGroup::STYLE:    ass->Styles.push_back(*static_cast<AssStyle *>(line)); break;
					case AssEntryGroup::DIALOGUE: ass->Events.push_back(*static_cast<AssDialogue *>(line)); break;
					default: break;
				}
			}
		};
		// Apply any pending commits
		for (auto const& pc : pending_commits) {
			apply_lines(pc.lines);
			ass->Commit(pc.mesage, pc.modification_type);
		}

		// Commit any changes after the last undo point was set
		if (modification_type)
			apply_lines(lines);
		if (modification_type && can_set_undo && !undo_description.empty())
			ass->Commit(undo_description, modification_type);

		lines_to_delete.clear();

		auto ret = std::move(lines);
		references--;
		if (!references) delete this;
		return ret;
	}

	void LuaAssFile::Cancel()
	{
		for (auto& line : lines_to_delete) line.release();
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
	{
		for (auto& line : ass->Info)
			lines.push_back(&line);
		for (auto& line : ass->Styles)
			lines.push_back(&line);
		for (auto& line : ass->Events)
			lines.push_back(&line);

		// prepare userdata object
		*static_cast<LuaAssFile**>(lua_newuserdata(L, sizeof(LuaAssFile*))) = this;

		// make the metatable
		lua_newtable(L);
		set_field(L, "__index", closure_wrapper<&LuaAssFile::ObjectIndexRead>);
		set_field(L, "__newindex", closure_wrapper_v<&LuaAssFile::ObjectIndexWrite>);
		set_field(L, "__len", closure_wrapper<&LuaAssFile::ObjectGetLen>);
		set_field(L, "__gc", closure_wrapper_v<&LuaAssFile::ObjectGarbageCollect>);
		set_field(L, "__ipairs", closure_wrapper<&LuaAssFile::ObjectIPairs>);
		lua_setmetatable(L, -2);

		// register misc functions
		// assume the "aegisub" global table exists
		lua_getglobal(L, "aegisub");

		set_field(L, "parse_karaoke_data", closure_wrapper<&LuaAssFile::LuaParseKaraokeData>);
		set_field(L, "set_undo_point", closure_wrapper_v<&LuaAssFile::LuaSetUndoPoint>);

		lua_pop(L, 1); // pop "aegisub" table

		// Leaves userdata object on stack
	}
}
