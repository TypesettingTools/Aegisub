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
//
// $Id$

/// @file util.cpp
/// @brief Unix utility methods.
/// @ingroup libaegisub unix

#include "config.h"

#include "libaegisub/util.h"

#ifndef LAGI_PRE
#include <stdarg.h>
#include <stdio.h>
#include <sys/statvfs.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <string>
#include <fstream>
#endif

#include <string.h>

namespace agi {
	namespace util {


const std::string DirName(const std::string& path) {
    if (path.find('/') == std::string::npos) {
		return ".";
	}

	return path.substr(0, path.rfind("/")+1);
}

void Rename(const std::string& from, const std::string& to) {
	acs::CheckFileWrite(from);

	try {
		acs::CheckFileWrite(to);
	} catch (acs::AcsNotFound& e) {
		acs::CheckDirWrite(DirName(to));
	}

	rename(from.c_str(), to.c_str());
}

void time_log(timeval &tv) {
	gettimeofday(&tv, (struct timezone *)NULL);
}

uint64_t freespace(std::string &path, PathType type) {
	struct statvfs fs;
	std::string check(path);

	if (type == TypeFile)
		check.assign(DirName(path));

	acs::CheckDirRead(check);

	if ((statvfs(check.c_str(), &fs)) == 0) {
		return fs.f_bsize * fs.f_bavail;
	} else {
		/// @todo We need a collective set of exceptions for ENOTDIR, EIO etc.
		throw("Failed getting free space");
	}
}


	} // namespace io
} // namespace agi
