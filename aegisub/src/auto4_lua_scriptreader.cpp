// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file auto4_lua_scriptreader.cpp
/// @brief Script-file reader for Lua 5.1-based scripting engine
/// @ingroup scripting
///

#include "config.h"

#include "auto4_lua_scriptreader.h"

#include <libaegisub/io.h>
#include <libaegisub/fs.h>

#include <fstream>
#include <lua.hpp>
#include <memory>

namespace Automation4 {
	bool LoadFile(lua_State *L, agi::fs::path const& filename) {
		std::unique_ptr<std::istream> file(agi::io::Open(filename, true));
		file->seekg(0, std::ios::end);
		size_t size = file->tellg();
		file->seekg(0, std::ios::beg);

		std::string buff;
		buff.resize(size);

		// Discard the BOM if present
		file->read(&buff[0], 3);
		size_t start = file->gcount();
		if (start == 3 && buff[0] == -17 && buff[1] == -69 && buff[2] == -65) {
			buff.resize(size - 3);
			start = 0;
		}

		file->read(&buff[start], size - start);

		if (!agi::fs::HasExtension(filename, "moon"))
			return luaL_loadbuffer(L, &buff[0], buff.size(), filename.string().c_str()) == 0;

		// We have a MoonScript file, so we need to load it with that
		// It might be nice to have a dedicated lua state for compiling
		// MoonScript to Lua
		if (luaL_dostring(L, "return require('moonscript').loadstring"))
			return false; // Leaves error message on stack
		lua_pushlstring(L, &buff[0], buff.size());
		lua_pushstring(L, filename.string().c_str());
		if (lua_pcall(L, 2, 2, 0))
			return false; // Leaves error message on stack

		// loadstring returns nil, error on error or a function on success
		if (lua_isnil(L, 1)) {
			lua_remove(L, 1);
			return false;
		}

		lua_pop(L, 1); // Remove the extra nil for the stackchecker
		return true;
	}
}
