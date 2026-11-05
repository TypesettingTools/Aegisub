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
#include "libaegisub/format.h"
#include "libaegisub/io.h"
#include "libaegisub/log.h"
#include "libaegisub/lua/utils.h"
#include "libaegisub/split.h"

#include <algorithm>
#include <bit>
#include <optional>

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/crc.hpp>
#include <lauxlib.h>

// windows.h is a special cookie
#undef CreateDirectory

namespace agi::lua {
	namespace {

	bool load_support_file(lua_State *L, fs::path const& filename) {
		try {
			return LoadFile(L, filename);
		}
		catch (agi::Exception const& e) {
			lua_pushstring(L, e.GetMessage().c_str());
			return false;
		}
	}

	struct LineTableEntry {
		long key;
		long value;
	};

	std::optional<agi::fs::path> lua_chunk_cachepath(lua_State *L, agi::fs::path const& filename) {
		lua_getfield(L, LUA_REGISTRYINDEX, "cachepath");
		auto cachepath = check_string(L, -1);
		lua_pop(L, 1);

		if (cachepath.empty())
			return std::nullopt;

		auto filename_str = filename.string();

		boost::crc_32_type hash;
		hash.process_bytes(filename_str.data(), filename_str.size());

		auto cachekey = agi::format("%s-%d.luachunk", filename.stem().string(), hash.checksum());

		return agi::fs::path(cachepath) / cachekey;
	}

	// The resulting cache key is written into the beginning of a compiled Lua chunk file
	std::string lua_chunk_cachekey(agi::fs::path const& filename) {
		auto mtime = std::chrono::duration_cast<std::chrono::milliseconds>(agi::fs::ModifiedTime(filename).time_since_epoch()).count();
		auto size = agi::fs::Size(filename);
		auto inode = agi::fs::InodeNumber(filename);

		auto filenameenc = boost::algorithm::hex(filename.string());

		// The leading 01 is a version indicator that can be bumped if the cache file format changes
		return agi::format("01:%s:%d:%d:%d:%d", filenameenc, mtime, size, inode[0], inode[1]);
	}

	// Cache files have the following format:
	// - The source file's cache key, followed by '\n'
	// - The moonscript module's cache key, followed by '\n'
	// - A ptrdiff_t indicating the size of the line table, or a negative value if none is present
	// - The corresponding number of line table entries
	// - The raw Lua chunk

	bool try_load_cachefile(lua_State *L, agi::fs::path const& filename, agi::fs::path const& cachepath, std::string_view cachekey) {
		if (!agi::fs::FileExists(cachepath))
			return false;

		auto cachefile = std::make_unique<agi::read_file_mapping>(cachepath);
		auto buf = cachefile->read();

		auto bad_cache_file = [&]() {
			cachefile.reset();		// Needed on Windows to not block the deletion
			agi::fs::Remove(cachepath);
			return false;
		};

		// Check the file's cache key

		auto newline = std::ranges::find(buf, '\n');

		if (newline == buf.end())
			return bad_cache_file();

		auto bufkey = buf.first(std::distance(buf.begin(), newline));
		buf = buf.subspan(std::distance(buf.begin(), newline) + 1);

		if (std::string_view{bufkey.data(), bufkey.size()} != cachekey)
			return bad_cache_file();

		// Check the moonscript cache key

		newline = std::ranges::find(buf, '\n');

		if (newline == buf.end()) {
			return bad_cache_file();
		}

		auto file_moonkey = buf.first(std::distance(buf.begin(), newline));
		buf = buf.subspan(std::distance(buf.begin(), newline) + 1);

		if (buf.size() < sizeof(ptrdiff_t))
			return bad_cache_file();

		ptrdiff_t line_table_len{};
		memcpy(&line_table_len, buf.data(), sizeof(line_table_len));
		buf = buf.subspan(sizeof(line_table_len));

		if (line_table_len >= 0) {
			if (line_table_len > std::ssize(buf) / static_cast<ptrdiff_t>(sizeof(LineTableEntry)))
				return bad_cache_file();

			auto line_table_size = line_table_len * sizeof(LineTableEntry);

			std::vector<LineTableEntry> line_table(line_table_len);
			memcpy(line_table.data(), buf.data(), line_table_size);
			buf = buf.subspan(line_table_size);

			lua_getfield(L, LUA_GLOBALSINDEX, "require");
			lua_pushstring(L, "moonscript.line_tables");
			lua_pcall(L, 1, 1, 0);

			if (!lua_istable(L, -1)) {
				LOG_E("auto4/lua") << "Failed to set MoonScript line tables for " << filename << ": Could not access line_tables";
				lua_pop(L, 1);
				return false;
			}

			lua_createtable(L, line_table_len, 0);

			for (auto const& e : line_table) {
				push_value(L, e.value);
				lua_rawseti(L, -2, e.key);
			}

			lua_setfield(L, -2, filename.string().c_str());
			lua_pop(L, 1);
		}

		lua_getfield(L, LUA_REGISTRYINDEX, "moonscript_cachekey");
		auto lua_moonkey = check_string(L, -1);
		lua_pop(L, 1);

		if (std::string_view{file_moonkey.data(), file_moonkey.size()} != lua_moonkey) {
			agi::fs::Remove(cachepath);
			return bad_cache_file();
		}

		if (luaL_loadbuffer(L, buf.data(), buf.size(), filename.string().c_str()) != 0) {
			// Corrupted cache file; delete it and try loading the source file.
			LOG_E("auto4/lua") << "Failed to load cached Lua chunk at " << filename;

			lua_pop(L, 1);	// pop the error message

			return bad_cache_file();
		}

		return true;
	}

	using luawriter_func = std::function<void(const void *, size_t)>;

	int luawriter(lua_State *, const void *p, size_t sz, void *ud) {
		auto func = *reinterpret_cast<luawriter_func*>(ud);

		try {
			func(p, sz);
			return 0;
		} catch (std::exception &) {
			return 1;
		}
	}

	}

	// Pushes either a Lua chunk or an error message onto the stack
	bool LoadFile(lua_State *L, agi::fs::path const& raw_filename) {
		auto filename = raw_filename;
		try {
			filename = agi::fs::Canonicalize(raw_filename);
		}
		catch (agi::fs::FileSystemUnknownError const& e) {
			LOG_E("auto4/lua") << "Error canonicalizing path: " << e.GetMessage();
		}

		auto discard_bom = [](std::span<const char> buff) {
			if (buff.size() >= 3 && buff[0] == '\xef' && buff[1] == '\xbb' && buff[2] == '\xbf') {
				return buff.subspan(3);
			}
			return buff;
		};

		bool is_moonscript = agi::fs::HasExtension(filename, "moon");

		if (is_moonscript) {
			// Even if there is a cached chunk, we need to save the original code
			// for the line number rewriting in the error handler

			agi::read_file_mapping file(filename);
			auto buff = discard_bom(file.read());

			lua_pushlstring(L, buff.data(), buff.size());
			lua_pushvalue(L, -1);
			lua_setfield(L, LUA_REGISTRYINDEX, ("lua file source: " + filename.string()).c_str());

			// Leave the buffer on the Lua stack for later
		}

		// If we're loading a MoonScript file, the code is now on the Lua stack.
		// If we're loading a Lua file, it's not.

		// Now, see if there is a cached chunk for this file

		std::optional<agi::fs::path> cachepath;
		std::string cachekey;

		try {
			cachepath = lua_chunk_cachepath(L, filename);
			cachekey = lua_chunk_cachekey(filename);

			if (cachepath.has_value() && try_load_cachefile(L, filename, *cachepath, cachekey)) {
				if (is_moonscript)
					lua_remove(L, -2);		// Remove the source code buffer

				return true;
			}
		} catch (agi::fs::FileSystemError const&) {
			// Ignore and treat as a missing cache file
		}

		std::vector<LineTableEntry> line_table;

		if (!is_moonscript) {
			agi::read_file_mapping file(filename);
			auto buff = discard_bom(file.read());

			if (luaL_loadbuffer(L, buff.data(), buff.size(), filename.string().c_str()) != 0)
				return false;	// Leave error message on stack
		} else {
			// The code is already on the stack

			lua_getfield(L, LUA_REGISTRYINDEX, "moonscript");

			lua_insert(L, -2);	// Swap the source code back to the top

			push_value(L, filename);
			if (lua_pcall(L, 2, 2, 0))
				return false; // Leaves error message on stack

			// loadstring returns nil, error on error or a function on success
			if (lua_isnil(L, -2)) {
				lua_remove(L, -2);	// Remove the nil and leave the error message
				return false;
			}

			// loadstring succeeded and pushed the function followed by nil. Remove the nil and leave the function.
			lua_pop(L, 1);

			// We loaded the module, but still need to save the line tables.

			lua_getfield(L, LUA_GLOBALSINDEX, "require");
			lua_pushstring(L, "moonscript.line_tables");
			lua_pcall(L, 1, 1, 0);

			if (!lua_istable(L, -1)) {
				LOG_E("auto4/lua") << "Failed to save MoonScript line tables for " << filename << ": Could not access line_tables";
				lua_pop(L, 1);
				return true;
			}

			lua_pushstring(L, filename.string().c_str());
			lua_rawget(L, -2);
			lua_remove(L, -2);

			if (!lua_istable(L, -1)) {
				LOG_E("auto4/lua") << "Failed to save MoonScript line tables for " << filename << ": Could not retrieve file's line_tables";
				lua_pop(L, 1);
				return true;
			}

			lua_for_each(L, [&]() {
				int key = check_int(L, -2);
				int value = check_int(L, -1);

				line_table.push_back({key, value});
			});
		}

		// We successfully loaded our function, so now we can write a cache file.

		if (!cachepath.has_value() || cachekey.empty()) // If caching is disabled or if only the cache path/key computation failed for some reason
			return true;

		lua_getfield(L, LUA_REGISTRYINDEX, "moonscript_cachekey");
		auto moonkey = check_string(L, -1);
		lua_pop(L, 1);

		try {
			agi::fs::CreateDirectory(cachepath->parent_path());
			agi::io::Save save(*cachepath, true, true);
			std::ostream &out = save.Get();

			out << cachekey << "\n";
			out << moonkey << "\n";

			auto line_table_size = std::bit_cast<std::array<char, sizeof(ptrdiff_t)>>(is_moonscript ? std::ssize(line_table) : static_cast<ptrdiff_t>(-1));
			out.write(line_table_size.data(), line_table_size.size());

			if (is_moonscript) {
				std::span<const std::byte> raw_line_table = std::as_bytes(std::span<LineTableEntry>{line_table});
				out.write(reinterpret_cast<const char *>(raw_line_table.data()), raw_line_table.size());
			}

			luawriter_func writer = [&](const void *p, size_t sz) {
				out.write(static_cast<const char *>(p), sz);
			};

			lua_dump(L, luawriter, &writer);
		} catch (agi::fs::FileBusy const&) {
			// Ignore
		} catch (agi::fs::FileSystemError const&) {
			LOG_E("auto4/lua") << "Failed to save cached Lua chunk at " << filename;
		}

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

		for (auto tok : agi::Split(package_paths, ';')) {
			std::string filename;
			boost::replace_all_copy(std::back_inserter(filename), tok, "?", module);

			// If there's a .moon file at that path, load it instead of the
			// .lua file
			agi::fs::path path = filename;
			if (agi::fs::HasExtension(path, "lua")) {
				agi::fs::path moonpath = path;
				moonpath.replace_extension("moon");
				if (agi::fs::FileExists(moonpath))
					path = moonpath;
			}

			if (!agi::fs::FileExists(path))
				continue;

			try {
				if (!LoadFile(L, path))
					return error(L, "Error loading Lua module \"%s\":\n%s", path.string().c_str(), check_string(L, -1).c_str());
				break;
			}
			catch (agi::fs::FileNotFound const&) {
				// Not an error so swallow and continue on
			}
			catch (agi::fs::NotAFile const&) {
				// Not an error so swallow and continue on
			}
			catch (agi::Exception const& e) {
				return error(L, "Error loading Lua module \"%s\":\n%s", path.string().c_str(), e.GetMessage().c_str());
			}
		}

		return lua_gettop(L) - pretop;
	}

	bool Install(lua_State *L, std::vector<fs::path> const& include_path, fs::path const& support_path, fs::path const& cachepath) {
		// set the module load path to include_path
		lua_getglobal(L, "package");
		push_value(L, "path");

		push_value(L, "");
		for (auto const& path : include_path) {
			lua_pushfstring(L, "%s/?.lua;%s/?/init.lua;", path.string().c_str(), path.string().c_str());
			lua_concat(L, 2);
		}

#ifndef _WIN32
		// No point in checking any of the default locations on Windows since
		// there won't be anything there
		push_value(L, "path");
		lua_gettable(L, -4);
		lua_concat(L, 2);
#endif

		lua_settable(L, -3);

		// Replace the default lua module loader with our unicode compatible one
		lua_getfield(L, -1, "loaders");
		push_value(L, exception_wrapper<module_loader>);
		lua_rawseti(L, -2, 2);
		lua_pop(L, 2); // loaders, package

		// Set the registry variables needed for the lua chunk caching.
		// These need to be set before moonscript.lua is loaded, since that, too, can be cached.
		auto moonscript_path = support_path / "moonscript.lua";
		push_value(L, lua_chunk_cachekey(moonscript_path));
		lua_setfield(L, LUA_REGISTRYINDEX, "moonscript_cachekey");

		push_value(L, cachepath);
		lua_setfield(L, LUA_REGISTRYINDEX, "cachepath");

#ifdef _WIN32
		// Replace the default lua IO functions with our unicode compatible ones
		if (!load_support_file(L, support_path / "unicode-monkeypatch.lua"))
			return false;
		if (lua_pcall(L, 0, 0, 0)) {
			return false; // leave error message
		}
#endif

		// Load mandatory support by exact path. In particular, do not allow a
		// moonscript.lua beside an approved subtitle-local script to run first.
		if (!load_support_file(L, moonscript_path))
			return false;
		if (lua_pcall(L, 0, 1, 0)) {
			return false; // leave error message
		}
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_pushliteral(L, "Bundled moonscript.lua did not return a module table");
			return false;
		}

		lua_getfield(L, -1, "loadstring");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			lua_pushliteral(L, "Bundled moonscript.lua has no loadstring function");
			return false;
		}
		lua_setfield(L, LUA_REGISTRYINDEX, "moonscript");

		// Direct loading bypasses require(), so record the trusted module as
		// loaded before user code gets control.
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "loaded");
		lua_pushvalue(L, -3);
		lua_setfield(L, -2, "moonscript");
		lua_pop(L, 3); // loaded, package, module

		return true;
	}
}
