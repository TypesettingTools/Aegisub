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

/// @file io.cpp
/// @brief Windows IO methods.
/// @ingroup libaegisub

#include "libaegisub/io.h"

#include <libaegisub/access.h>
#include "libaegisub/fs.h"
#include "libaegisub/log.h"
#include "libaegisub/make_unique.h"
#include "libaegisub/util.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

namespace agi {
	namespace io {

std::unique_ptr<std::istream> Open(fs::path const& file, bool binary) {
	LOG_D("agi/io/open/file") << file;
	acs::CheckFileRead(file);

	auto stream = agi::make_unique<boost::filesystem::ifstream>(file, (binary ? std::ios::binary : std::ios::in));

	if (stream->fail())
		throw IOFatal("Unknown fatal error as occurred");

	return std::unique_ptr<std::istream>(stream.release());
}

Save::Save(fs::path const& file, bool binary)
: file_name(file)
, tmp_name(unique_path(file.parent_path()/(file.stem().string() + "_tmp_%%%%" + file.extension().string())))
{
	LOG_D("agi/io/save/file") << file;
	acs::CheckDirWrite(file.parent_path());

	try {
		acs::CheckFileWrite(file);
	}
	catch (fs::FileNotFound const&) {
		// Not an error
	}

	fp = agi::make_unique<boost::filesystem::ofstream>(tmp_name, binary ? std::ios::binary : std::ios::out);
	if (!fp->good())
		throw fs::WriteDenied(tmp_name);
}

Save::~Save() {
	fp.reset(); // Need to close before rename on Windows to unlock the file
	for (int i = 0; i < 10; ++i) {
		try {
			fs::Rename(tmp_name, file_name);
			return;
		}
		catch (agi::fs::FileSystemError const&) {
			// Retry up to ten times in case it's just locked by a poorly-written antivirus scanner
			if (i == 9)
				throw;
			util::sleep_for(100);
		}
	}
}

	} // namespace io
} // namespace agi
