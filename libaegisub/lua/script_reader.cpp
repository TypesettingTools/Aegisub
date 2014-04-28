// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "libaegisub/lua/script_reader.h"

#include "libaegisub/file_mapping.h"
#include "libaegisub/log.h"
#include "libaegisub/lua/utils.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>
#include <lauxlib.h>

namespace agi { namespace lua {
	bool LoadFile(lua_State *L, agi::fs::path const& raw_filename) {
		auto filename = raw_filename;
		try {
			filename = agi::fs::Canonicalize(raw_filename);
		}
		catch (agi::fs::FileSystemUnknownError const& e) {
			LOG_E("auto4/lua") << "Error canonicalizing path: " << e.GetChainedMessage();
		}

		agi::read_file_mapping file(filename);
		auto buff = file.read();
		size_t size = static_cast<size_t>(file.size());

		// Discard the BOM if present
		if (size >= 3 && buff[0] == -17 && buff[1] == -69 && buff[2] == -65) {
			buff += 3;
			size -= 3;
		}

		if (!agi::fs::HasExtension(filename, "moon"))
			return luaL_loadbuffer(L, buff, size, filename.string().c_str()) == 0;

		// We have a MoonScript file, so we need to load it with that
		// It might be nice to have a dedicated lua state for compiling
		// MoonScript to Lua
		if (luaL_dostring(L, "return require('moonscript').loadstring"))
			return false; // Leaves error message on stack

		// Save the text we'll be loading for the line number rewriting in the
		// error handling
		lua_pushlstring(L, buff, size);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, ("raw moonscript: " + filename.string()).c_str());

		push_value(L, filename);
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

	static int module_loader(lua_State *L) {
		int pretop = lua_gettop(L);
		std::string module(check_string(L, -1));
		boost::replace_all(module, ".", LUA_DIRSEP);

		// Get the lua package include path (which the user may have modified)
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path");
		std::string package_paths(check_string(L, -1));
		lua_pop(L, 2);

		boost::char_separator<char> sep(";");
		for (auto filename : boost::tokenizer<boost::char_separator<char>>(package_paths, sep)) {
			boost::replace_all(filename, "?", module);

			// If there's a .moon file at that path, load it instead of the
			// .lua file
			agi::fs::path path = filename;
			if (agi::fs::HasExtension(path, "lua")) {
				agi::fs::path moonpath = path;
				moonpath.replace_extension("moon");
				if (agi::fs::FileExists(moonpath))
					path = moonpath;
			}

			try {
				if (!LoadFile(L, path))
					return error(L, "Error loading Lua module \"%s\":\n%s", path.string().c_str(), check_string(L, 1).c_str());
				break;
			}
			catch (agi::fs::FileNotFound const&) {
				// Not an error so swallow and continue on
			}
			catch (agi::fs::NotAFile const&) {
				// Not an error so swallow and continue on
			}
			catch (agi::Exception const& e) {
				return error(L, "Error loading Lua module \"%s\":\n%s", path.string().c_str(), e.GetChainedMessage().c_str());
			}
		}

		return lua_gettop(L) - pretop;
	}

	void Install(lua_State *L, std::vector<fs::path> const& include_path) {
		// set the module load path to include_path
		lua_getglobal(L, "package");
		push_value(L, "path");
#ifdef _WIN32
		// No point in checking any of the default locations on Windows since
		// there won't be anything there
		push_value(L, "");
#else
		push_value(L, "path");
		lua_gettable(L, -3);
#endif

		for (auto const& path : include_path) {
			lua_pushfstring(L, ";%s/?.lua;%s/?/init.lua", path.string().c_str(), path.string().c_str());
			lua_concat(L, 2);
		}

		lua_settable(L, -3);

		// Replace the default lua module loader with our unicode compatible one
		lua_getfield(L, -1, "loaders");
		push_value(L, exception_wrapper<module_loader>);
		lua_rawseti(L, -2, 2);
		lua_pop(L, 2);
	}
} }
