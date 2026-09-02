// Copyright (c) 2026, Aegisub Project http://www.aegisub.org/
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

#include <main.h>

#include <ass_file.h>
#include <auto4_lua.h>

#include <libaegisub/mru.h>
#include <libaegisub/option.h>
#include <libaegisub/path.h>

#include <lua.hpp>
#include <string>

// The application-wide state these name is set up by the entry point, which a test binary cannot
// link. Only paths this test never takes reach them — loading a default subtitle file, resolving a
// style catalog, recording a recently used file — so definitions are all the linker wants.
namespace config {
	agi::Options *opt = nullptr;
	agi::MRUManager *mru = nullptr;
	agi::Path *path = nullptr;
}

namespace {
// `parse_karaoke_data` is registered onto the aegisub table by LuaAssFile's constructor, so a
// LuaAssFile has to exist for the whole call and the table has to be there before it is built.
class lua_parse_karaoke_data : public ::testing::Test {
protected:
	lua_State *L = nullptr;
	AssFile file;

	void SetUp() override {
		L = lua_open();
		ASSERT_NE(nullptr, L);
		luaL_openlibs(L);

		lua_newtable(L);
		lua_setglobal(L, "aegisub");
	}

	void TearDown() override {
		if (L) lua_close(L);
	}

	/// Runs a chunk that calls parse_karaoke_data and concatenates the `text_stripped` of every
	/// syllable it returned past the index-zero filler. Fails the test if the call raises or hands
	/// back something other than a table.
	std::string syllable_text(const char *chunk) {
		new Automation4::LuaAssFile(L, &file, true, false);

		if (luaL_loadstring(L, chunk)) {
			ADD_FAILURE() << "failed to compile chunk: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			return {};
		}

		if (lua_pcall(L, 0, 1, 0)) {
			ADD_FAILURE() << "parse_karaoke_data failed: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			return {};
		}

		if (lua_type(L, -1) != LUA_TTABLE) {
			ADD_FAILURE() << "expected a table, got " << lua_typename(L, lua_type(L, -1));
			lua_pop(L, 1);
			return {};
		}

		std::string ret;
		for (int i = 1; i <= (int)lua_objlen(L, -1); ++i) {
			lua_rawgeti(L, -1, i);
			lua_getfield(L, -1, "text_stripped");
			if (lua_type(L, -1) == LUA_TSTRING)
				ret += lua_tostring(L, -1);
			lua_pop(L, 2);
		}
		lua_pop(L, 1);
		return ret;
	}
};

// Everything LuaToAssEntry's dialogue branch reads, with the optional extradata left to the caller.
const char line_fields[] =
	"class = 'dialogue', comment = false, layer = 0, "
	"start_time = 0, end_time = 5000, style = 'Default', actor = '', "
	"margin_l = 0, margin_r = 0, margin_t = 0, effect = '', ";
}

TEST_F(lua_parse_karaoke_data, splits_syllables) {
	std::string chunk = std::string("return aegisub.parse_karaoke_data{") + line_fields +
		"text = '{\\\\k20}ka{\\\\k30}ra', extra = {}}";
	EXPECT_EQ("kara", syllable_text(chunk.c_str()));
}

// A previous bug in LuaToAssEntry's dialogue branch left a nil on the stack when no `extra` field
// was present on a line table passed to `aegisub.parse_karaoke_data(line)`, e.g. via the public
// `karaskel.preproc_line_text(meta, styles, line)` API, which in turn caused LuaParseKaraokeData
// to try writing syllables into a nil rather than a table, resulting in a crash.
TEST_F(lua_parse_karaoke_data, accepts_a_line_without_extradata) {
	std::string chunk = std::string("return aegisub.parse_karaoke_data{") + line_fields +
		"text = 'Hello world'}";
	EXPECT_EQ("Hello world", syllable_text(chunk.c_str()));
}

TEST_F(lua_parse_karaoke_data, splits_syllables_without_extradata) {
	std::string chunk = std::string("return aegisub.parse_karaoke_data{") + line_fields +
		"text = '{\\\\k20}ka{\\\\k30}ra'}";
	EXPECT_EQ("kara", syllable_text(chunk.c_str()));
}
