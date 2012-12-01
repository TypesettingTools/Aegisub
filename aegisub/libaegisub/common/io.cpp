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

#include <sys/stat.h>
#include <errno.h>

#include <iostream>
#include <fstream>

#include <libaegisub/access.h>
#include <libaegisub/charset_conv_win.h>
#include "libaegisub/io.h"
#include "libaegisub/log.h"
#include "libaegisub/util.h"

#ifdef _WIN32
#define snprintf sprintf_s
#endif

namespace {
	std::string make_temp_name(std::string const& filename) {
		char tmp[1024];
		snprintf(tmp, sizeof tmp, "_tmp_%lld", (long long)time(0));

		std::string::size_type pos = filename.rfind('.');
		if (pos == std::string::npos)
			return filename + tmp;

		return filename.substr(0, pos) + tmp + filename.substr(pos);
	}
}

namespace agi {
	namespace io {

using agi::charset::ConvertW;

#ifndef _WIN32
#define ConvertW
#endif

std::ifstream* Open(const std::string &file, bool binary) {
	LOG_D("agi/io/open/file") << file;
	acs::CheckFileRead(file);

	std::ifstream *stream = new std::ifstream(ConvertW(file).c_str(), (binary ? std::ios::binary : std::ios::in));

	if (stream->fail()) {
		delete stream;
		throw IOFatal("Unknown fatal error as occurred");
	}

	return stream;
}

Save::Save(const std::string& file, bool binary)
: file_name(file)
, tmp_name(make_temp_name(file))
{
	LOG_D("agi/io/save/file") << file;
	const std::string pwd = util::DirName(file);

	acs::CheckDirWrite(pwd);

	try {
		acs::CheckFileWrite(file);
	} catch (FileNotFoundError const&) {
		// If the file doesn't exist we create a 0 byte file, this so so
		// util::Rename will find it, and to let users know something went
		// wrong by leaving a 0 byte file.
		std::ofstream(ConvertW(file).c_str());
	}

	fp = new std::ofstream(ConvertW(tmp_name).c_str(), binary ? std::ios::binary : std::ios::out);
	if (!fp->good()) {
		delete fp;
		throw agi::FileNotAccessibleError("Could not create temporary file at: " + tmp_name);
	}
}

Save::~Save() {
	delete fp;
	util::Rename(tmp_name, file_name);
}

std::ofstream& Save::Get() {
	return *fp;
}

	} // namespace io
} // namespace agi
