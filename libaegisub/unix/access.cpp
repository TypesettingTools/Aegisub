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

#include "libaegisub/access.h"

#include "libaegisub/fs.h"

#include <sys/stat.h>
#include <cerrno>
#include <unistd.h>

namespace agi::acs {

void Check(agi::fs::path const& file, acs::Type type) {
	auto cu = std::filesystem::current_path();
	std::error_code ec;
	auto s = std::filesystem::status(file, ec);
	if (ec != std::error_code{}) {
		using enum std::errc;
		switch (ec.value()) {
			case int(no_such_file_or_directory): throw fs::FileNotFound(file);
			case int(permission_denied): throw fs::ReadDenied(file);
			case int(io_error):
				throw fs::FileSystemUnknownError("Fatal I/O error in 'stat' on path: " + file.string());
			default:
				throw fs::FileSystemUnknownError("Fatal I/O error in 'stat' on path: " + file.string() + ": " + ec.message());
		}
	}

	using std::filesystem::file_type;
	switch (type) {
		case FileRead:
		case FileWrite:
			if (s.type() != file_type::regular)
				throw fs::NotAFile(file);
		break;

		case DirRead:
		case DirWrite:
			if (s.type() != file_type::directory)
				throw fs::NotADirectory(file);
		break;
	}

	int file_status = access(file.c_str(), R_OK);
	if (file_status != 0)
		throw fs::ReadDenied(file);

	if (type == DirWrite || type == FileWrite) {
		file_status = access(file.c_str(), W_OK);
		if (file_status != 0)
			throw fs::WriteDenied(file);
	}
}

} // namespace agi::acs
