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
#include "libaegisub/lua/ffi.h"

#include <chrono>

using namespace agi::fs;
using namespace agi::lua;
namespace sfs = std::filesystem;

namespace agi {
AGI_DEFINE_TYPE_NAME(DirectoryIterator);
}

namespace {
template<typename Func>
auto wrap(char **err, Func f) -> decltype(f()) {
	try {
		return f();
	}
	catch (std::exception const& e) {
		*err = strdup(e.what());
	}
	catch (agi::Exception const& e) {
		*err = strndup(e.GetMessage());
	}
	catch (...) {
		*err = strdup("Unknown error");
	}
	return 0;
}

template<typename Ret>
bool setter(const char *path, char **err, Ret (*f)(agi::fs::path const&)) {
	return wrap(err, [=]{
		f(path);
		return true;
	});
}

bool lfs_chdir(const char *dir, char **err) {
	return setter(dir, err, CurrentPath);
}

char *currentdir(char **err) {
	return wrap(err, []{
		return strndup(CurrentPath().string());
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
		auto str = strndup(*it);
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
		using enum sfs::file_type;
		switch (sfs::status(path).type()) {
			case not_found: return nullptr;
			case regular:   return "file";
			case directory: return "directory";
			case symlink:   return "link";
			case block:     return "block device";
			case character: return "char device";
			case fifo:      return "fifo";
			case socket:    return "socket";
			default:        return "other";
		}
	});
}

time_t get_mtime(const char *path, char **err) {
	return wrap(err, [=]() -> time_t {
		using namespace std::chrono;
		return duration_cast<seconds>(ModifiedTime(path).time_since_epoch()).count();
	});
}

uintmax_t get_size(const char *path, char **err) {
	return wrap(err, [=] { return Size(path); });
}
}

extern "C" int luaopen_lfs_impl(lua_State *L) {
	agi::lua::register_lib_table(L, {"DirectoryIterator"},
		"chdir", lfs_chdir,
		"currentdir", currentdir,
		"mkdir", mkdir,
		"rmdir", lfs_rmdir,
		"touch", touch,
		"get_mtime", get_mtime,
		"get_mode", get_mode,
		"get_size", get_size,
		"dir_new", dir_new,
		"dir_free", dir_free,
		"dir_next", dir_next,
		"dir_close", dir_close);
	return 1;
}
