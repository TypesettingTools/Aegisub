// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

/// @file access.cpp
/// @brief Unix access methods.
/// @ingroup libaegisub unix

#include "libaegisub/access.h"

#include "libaegisub/fs.h"

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <boost/filesystem/path.hpp>

namespace agi {
	namespace acs {

void Check(agi::fs::path const& file, acs::Type type) {
	struct stat file_stat;

	int file_status = stat(file.c_str(), &file_stat);

	if (file_status != 0) {
		switch (errno) {
			case ENOENT:
				throw fs::FileNotFound(file);
			case EACCES:
				throw fs::ReadDenied(file);
			case EIO:
				throw fs::FileSystemUnknownError("Fatal I/O error in 'stat' on path: " + file.string());
		}
	}

	switch (type) {
		case FileRead:
		case FileWrite:
			if ((file_stat.st_mode & S_IFREG) == 0)
				throw fs::NotAFile(file);
		break;

		case DirRead:
		case DirWrite:
			if ((file_stat.st_mode & S_IFDIR) == 0)
				throw fs::NotADirectory(file);
		break;
	}

	file_status = access(file.c_str(), R_OK);
	if (file_status != 0)
		throw fs::ReadDenied(file);

	if (type == DirWrite || type == FileWrite) {
		file_status = access(file.c_str(), W_OK);
		if (file_status != 0)
			throw fs::WriteDenied(file);
	}
}

	} // namespace acs
} // namespace agi
