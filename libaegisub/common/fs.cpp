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
//
// Aegisub Project http://www.aegisub.org/

#include "libaegisub/fs.h"

#include "libaegisub/access.h"
#include "libaegisub/log.h"

#include <boost/algorithm/string/predicate.hpp>
#include <system_error>

namespace sfs = std::filesystem;

namespace agi::fs {
namespace {
void check_error(std::error_code ec, const char *exp, sfs::path const& src_path, sfs::path const& dst_path) {
	if (ec == std::error_code{}) return;
	using enum std::errc;
	switch (ec.value()) {
		case int(no_such_file_or_directory): throw FileNotFound(src_path);
		case int(is_a_directory): throw NotAFile(src_path);
		case int(not_a_directory): throw NotADirectory(src_path);
		case int(no_space_on_device): throw DriveFull(dst_path);
		case int(permission_denied):
			if (!src_path.empty())
				acs::CheckFileRead(src_path);
			if (!dst_path.empty())
				acs::CheckFileWrite(dst_path);
			throw AccessDenied(src_path);
		default:
			LOG_D("filesystem") << "Unknown error when calling '" << exp << "': " << ec << ": " << ec.message();
			throw FileSystemUnknownError(ec.message());
	}
}

// std::filesystem functions throw a single exception type for all
// errors, which isn't really what we want, so do some crazy wrapper
// shit to map error codes to more useful exceptions.
#define CHECKED_CALL(exp, src_path, dst_path) \
	std::error_code ec; \
	exp; \
	check_error(ec, #exp, src_path, dst_path);

#define CHECKED_CALL_RETURN(exp, src_path) \
	CHECKED_CALL(auto ret = exp, src_path, std::filesystem::path()); \
	return ret

#define WRAP_SFS(sfs_name, agi_name) \
	auto agi_name(path const& p) -> decltype(sfs::sfs_name(p)) { \
		CHECKED_CALL_RETURN(sfs::sfs_name(p, ec), p); \
	}

#define WRAP_SFS_IGNORE_ERROR(sfs_name, agi_name) \
	auto agi_name(path const& p) -> decltype(sfs::sfs_name(p)) { \
		std::error_code ec; \
		return sfs::sfs_name(p, ec); \
	}

// sasuga windows.h
#undef CreateDirectory

	WRAP_SFS(file_size, SizeImpl)
	WRAP_SFS(space, Space)
} // anonymous namespace

	WRAP_SFS_IGNORE_ERROR(exists, Exists)
	WRAP_SFS_IGNORE_ERROR(is_regular_file, FileExists)
	WRAP_SFS_IGNORE_ERROR(is_directory, DirectoryExists)
	WRAP_SFS(last_write_time, ModifiedTime)
	WRAP_SFS(create_directories, CreateDirectory)
	WRAP_SFS(remove, Remove)
	WRAP_SFS(canonical, Canonicalize)

	uintmax_t Size(path const& p) {
		if (DirectoryExists(p))
			throw NotAFile(p);
		return SizeImpl(p);
	}

	uintmax_t FreeSpace(path const& p) {
		return Space(p).available;
	}

	void Rename(const path& from, const path& to) {
		CHECKED_CALL(sfs::rename(from, to, ec), from, to);
	}

	bool HasExtension(path const& p, std::string const& ext) {
		auto filename = p.filename().string();
		if (filename.size() < ext.size() + 1) return false;
		if (filename[filename.size() - ext.size() - 1] != '.') return false;
		return boost::iends_with(filename, ext);
	}
}
