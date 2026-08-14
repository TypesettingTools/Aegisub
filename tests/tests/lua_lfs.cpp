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

#include <libaegisub/fs.h>
#include <libaegisub/lua/modules.h>

#include <lua.hpp>
#include <optional>
#include <string>

namespace {
// The lfs entry points are static in lua/modules/lfs.cpp and are only reachable
// through the FFI table `luaopen_lfs_impl()` builds, so call `get_mode()` the way
// aegisub.lfs's `attributes()` does.
const char get_mode_chunk[] = R"LUA(
local path = ...
local ffi = require 'ffi'
local impl = require 'aegisub.__lfs_impl'
local err = ffi.new('char *[1]')
local mode = impl.get_mode(path, err)
if err[0] ~= nil then error(ffi.string(err[0]), 0) end
if mode == nil then return false end
return ffi.string(mode)
)LUA";

// make the test data independent of the encoding of this source file
const char nonascii_dir[] = "data/lfs_M\xC3\xBCller";
const char nonascii_file[] = "data/lfs_M\xC3\xBCller/file";

class lagi_lua_lfs : public ::testing::Test {
protected:
	lua_State *L = nullptr;

	void SetUp() override {
		L = lua_open();
		ASSERT_NE(nullptr, L);
		agi::lua::preload_modules(L);

		agi::fs::CreateDirectory(agi::fs::path(nonascii_dir));
		agi::fs::Touch(agi::fs::path(nonascii_file));
	}

	void TearDown() override {
		agi::fs::Remove(agi::fs::path(nonascii_file));
		agi::fs::Remove(agi::fs::path(nonascii_dir));

		if (L) lua_close(L);
	}

	/// The return value the lfs `get_mode()` implementation reports for a given path,
	// or an empty optional in case of `nil`. Fails the test if `get_mode()` itself errors.
	std::optional<std::string> get_mode(const char *path) {
		if (luaL_loadstring(L, get_mode_chunk)) {
			ADD_FAILURE() << "failed to compile helper chunk: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			return std::nullopt;
		}

		lua_pushstring(L, path);
		if (lua_pcall(L, 1, 1, 0)) {
			ADD_FAILURE() << "get_mode(" << path << ") failed: " << lua_tostring(L, -1);
			lua_pop(L, 1);
			return std::nullopt;
		}

		std::optional<std::string> ret;
		if (lua_type(L, -1) == LUA_TSTRING)
			ret = lua_tostring(L, -1);
		lua_pop(L, 1);
		return ret;
	}
};
}

TEST_F(lagi_lua_lfs, get_mode_reports_files_and_directories) {
	EXPECT_EQ("file", get_mode("data/file"));
	EXPECT_EQ("directory", get_mode("data/dir"));
	EXPECT_EQ(std::nullopt, get_mode("data/nonexistent"));
}

// A previous bug had `get_mode()` hand off the UTF-8 bytes it received via
// the `path` argument straight to std::filesystem instead of going through
// agi::fs::path, causing `lfs.attributes(nonAsciiPath, "mode")` to report as
// nonexistent on Windows, unless the OS happened to be running with the UTF-8
// pseudo ANSI code page (65001).
TEST_F(lagi_lua_lfs, get_mode_handles_non_ascii_paths) {
	EXPECT_EQ("directory", get_mode(nonascii_dir));
	EXPECT_EQ("file", get_mode(nonascii_file));
}
