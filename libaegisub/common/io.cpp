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

#include "libaegisub/io.h"

#include <libaegisub/access.h>
#include "libaegisub/fs.h"
#include "libaegisub/log.h"
#include "libaegisub/util.h"

#include <fstream>

namespace agi::io {
using namespace agi::fs;

std::unique_ptr<std::istream> Open(path const& file, bool binary) {
	LOG_D("agi/io/open/file") << file;

	auto stream = std::make_unique<std::ifstream>(file, (binary ? std::ios::binary : std::ios::in));
	if (stream->fail()) {
		acs::CheckFileRead(file);
		throw IOFatal("Unknown fatal error occurred opening " + file.string());
	}

	return std::unique_ptr<std::istream>(stream.release());
}

Save::Save(path const& file, bool binary)
: file_name(file)
, tmp_name(file.parent_path()/(file.stem().string() + ".tmp" + file.extension().string()))
{
	LOG_D("agi/io/save/file") << file;

	fp = std::make_unique<std::ofstream>(tmp_name, binary ? std::ios::binary : std::ios::out);
	if (!fp->good()) {
		acs::CheckDirWrite(file.parent_path());
		acs::CheckFileWrite(file);
		throw fs::WriteDenied(tmp_name);
	}
}

Save::~Save() noexcept(false) {
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

} // namespace agi::io
