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

#include "libaegisub/fs.h"
#include "libaegisub/type_name.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <lua.hpp>

using namespace agi::fs;
namespace bfs = boost::filesystem;

namespace agi {
AGI_DEFINE_TYPE_NAME(DirectoryIterator);
}

namespace {
template<typename Func>
auto wrap(char **err, Func f) -> decltype(f()) {
	try {
		return f();
	}
	catch (bfs::filesystem_error const& e) {
		*err = strdup(e.what());
		return 0;
	}
	catch (agi::Exception const& e) {
		*err = strdup(e.GetMessage().c_str());
		return 0;
	}
}

template<typename Ret>
bool setter(const char *path, char **err, Ret (*f)(bfs::path const&)) {
	return wrap(err, [=]{
		f(path);
		return true;
	});
}

bool lfs_chdir(const char *dir, char **err) {
	return setter(dir, err, &bfs::current_path);
}

char *currentdir(char **err) {
	return wrap(err, []{
		return strdup(bfs::current_path().string().c_str());
	});
}

bool mkdir(const char *dir, char **err) {
	return setter(dir, err, &CreateDirectory);
}

bool lfs_rmdir(const char *dir, char **err) {
	return setter(dir, err, &Remove);
}

bool touch(const char *path, char **err) {
	return setter(path, err, &Touch);
}

char *dir_next(DirectoryIterator &it, char **err) {
	if (it == end(it)) return nullptr;
	return wrap(err, [&]{
		auto str = strdup((*it).c_str());
		++it;
		return str;
	});
}

void dir_close(DirectoryIterator &it) {
	it = DirectoryIterator();
}

void dir_free(DirectoryIterator *it) {
	delete it;
}

DirectoryIterator *dir_new(const char *path, char **err) {
	return wrap(err, [=]{
		return new DirectoryIterator(path, "");
	});
}

const char *get_mode(const char *path, char **err) {
	return wrap(err, [=]() -> const char * {
		switch (bfs::status(path).type()) {
			case bfs::file_not_found: return nullptr;         break;
			case bfs::regular_file:   return "file";          break;
			case bfs::directory_file: return "directory";     break;
			case bfs::symlink_file:   return "link";          break;
			case bfs::block_file:     return "block device";  break;
			case bfs::character_file: return "char device";   break;
			case bfs::fifo_file:      return "fifo";          break;
			case bfs::socket_file:    return "socket";        break;
			case bfs::reparse_file:   return "reparse point"; break;
			default:                  return "other";         break;
		}
	});
}

time_t get_mtime(const char *path, char **err) {
	return wrap(err, [=] { return ModifiedTime(path); });
}

uintmax_t get_size(const char *path, char **err) {
	return wrap(err, [=] { return Size(path); });
}

template<typename T>
void push_ffi_function(lua_State *L, const char *name, T *func) {
	lua_pushvalue(L, -2); // push cast function
	lua_pushstring(L, agi::type_name<T*>::name().c_str());
	// This cast isn't legal, but LuaJIT internally requires that it work
	lua_pushlightuserdata(L, (void *)func);
	lua_call(L, 2, 1);
	lua_setfield(L, -2, name);
}
}

extern "C" int luaopen_lfs_impl(lua_State *L) {
	lua_getglobal(L, "require");
	lua_pushstring(L, "ffi");
	lua_call(L, 1, 1);

	lua_getfield(L, -1, "cdef");
	lua_pushstring(L, "typedef struct DirectoryIterator DirectoryIterator;");
	lua_call(L, 1, 0);

	lua_getfield(L, -1, "cast");
	lua_remove(L, -2); // ffi table

	lua_createtable(L, 0, 12);
	push_ffi_function(L, "chdir", lfs_chdir);
	push_ffi_function(L, "currentdir", currentdir);
	push_ffi_function(L, "mkdir", mkdir);
	push_ffi_function(L, "rmdir", lfs_rmdir);
	push_ffi_function(L, "touch", touch);
	push_ffi_function(L, "get_mtime", get_mtime);
	push_ffi_function(L, "get_mode", get_mode);
	push_ffi_function(L, "get_size", get_size);

	push_ffi_function(L, "dir_new", dir_new);
	push_ffi_function(L, "dir_free", dir_free);
	push_ffi_function(L, "dir_next", dir_next);
	push_ffi_function(L, "dir_close", dir_close);

	lua_remove(L, -2); // ffi.cast function
	return 1;
}
