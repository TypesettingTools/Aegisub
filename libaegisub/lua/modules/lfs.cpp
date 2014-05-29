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

#include "libaegisub/lua/utils.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/size.hpp>

namespace {
using namespace agi::lua;
using namespace agi::fs;
namespace bfs = boost::filesystem;

template<void (*func)(lua_State *L)>
int wrap(lua_State *L) {
	try {
		func(L);
		return 1;
	}
	catch (bfs::filesystem_error const& e) {
		lua_pushnil(L);
		push_value(L, e.what());
		return 2;
	}
	catch (agi::Exception const& e) {
		lua_pushnil(L);
		push_value(L, e.GetMessage());
		return 2;
	}
	catch (error_tag) {
		return lua_error(L);
	}
}

void chdir(lua_State *L) {
	bfs::current_path(check_string(L, 1));
	push_value(L, true);
}

void currentdir(lua_State *L) {
	push_value(L, bfs::current_path());
}

void mkdir(lua_State *L) {
	CreateDirectory(check_string(L, 1));
	push_value(L, true);
}

void rmdir(lua_State *L) {
	Remove(check_string(L, 1));
	push_value(L, true);
}

void touch(lua_State *L) {
	Touch(check_string(L, 1));
	push_value(L, true);
}

int dir_next(lua_State *L) {
	auto& it = get<agi::fs::DirectoryIterator>(L, 1, "aegisub.lfs.dir");
	if (it == end(it)) return 0;
	push_value(L, *it);
	++it;
	return 1;
}

int dir_close(lua_State *L) {
	auto& it = get<agi::fs::DirectoryIterator>(L, 1, "aegisub.lfs.dir");
	// Convert to end iterator rather than destroying to avoid crashes if this
	// is called multiple times
	it = DirectoryIterator();
	return 0;
}

int dir(lua_State *L) {
	const path p = check_string(L, 1);
	push_value(L, dir_next);
	make<agi::fs::DirectoryIterator>(L, "aegisub.lfs.dir", check_string(L, 1), "");
	return 2;
}

void attributes(lua_State *L) {
	static std::pair<const char *, void (*)(lua_State *, path const&)> fields[] = {
		{"mode", [](lua_State *L, path const& p) {
			switch (status(p).type()) {
			case bfs::file_not_found: lua_pushnil(L);                 break;
			case bfs::regular_file:   push_value(L, "file");          break;
			case bfs::directory_file: push_value(L, "directory");     break;
			case bfs::symlink_file:   push_value(L, "link");          break;
			case bfs::block_file:     push_value(L, "block device");  break;
			case bfs::character_file: push_value(L, "char device");   break;
			case bfs::fifo_file:      push_value(L, "fifo");          break;
			case bfs::socket_file:    push_value(L, "socket");        break;
			case bfs::reparse_file:   push_value(L, "reparse point"); break;
			default:                  push_value(L, "other");         break;
			}
		}},
		{"modification", [](lua_State *L, path const& p) { push_value(L, ModifiedTime(p)); }},
		{"size",         [](lua_State *L, path const& p) { push_value(L, Size(p)); }}
	};

	const path p = check_string(L, 1);

	const auto field = get_string(L, 2);
	if (!field.empty()) {
		for (const auto getter : fields) {
			if (field == getter.first) {
				getter.second(L, p);
				return;
			}
		}
		error(L, "Invalid attribute name: %s", field.c_str());
	}

	lua_createtable(L, 0, boost::size(fields));
	for (const auto getter : fields) {
		getter.second(L, p);
		lua_setfield(L, -2, getter.first);
	}
}
}

extern "C" int luaopen_lfs(lua_State *L) {
	if (luaL_newmetatable(L, "aegisub.lfs.dir")) {
		set_field<dir_close>(L, "__gc");

		lua_createtable(L, 0, 2);
		set_field<dir_next>(L, "next");
		set_field<dir_close>(L, "close");
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}

	const struct luaL_Reg lib[] = {
		{"attributes", wrap<attributes>},
		{"chdir", wrap<chdir>},
		{"currentdir", wrap<currentdir>},
		{"dir", exception_wrapper<dir>},
		{"mkdir", wrap<mkdir>},
		{"rmdir", wrap<rmdir>},
		{"touch", wrap<touch>},
		{nullptr, nullptr},
	};
	lua_createtable(L, 0, boost::size(lib) - 1);
	luaL_register(L, nullptr, lib);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "lfs");
	return 1;
}
