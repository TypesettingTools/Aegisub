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

/// @file io.cpp
/// @brief Unix IO methods.
/// @ingroup libaegisub unix

#ifndef LAGI_PRE
#include <sys/stat.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#endif

#include "libaegisub/io.h"
#include "libaegisub/log.h"
#include "libaegisub/util.h"


namespace agi {
	namespace io {

std::ifstream* Open(const std::string &file) {
    LOG_D("agi/io/open/file") << file;
	acs::CheckFileRead(file);

	std::ifstream *stream = new std::ifstream(file.c_str());

	if (stream->fail())
		throw IOFatal("Unknown fatal error as occurred");

	return stream;
}


Save::Save(const std::string& file, bool binary): file_name(file) {
    LOG_D("agi/io/save/file") << file;
	const std::string pwd = util::DirName(file);

	acs::CheckDirWrite(pwd.c_str());

	try {
		acs::CheckFileWrite(file);
	} catch (acs::AcsNotFound& e) {
		// If the file doesn't exist we create a 0 byte file, this so so
		// util::Rename will find it, and to let users know something went
		// wrong by leaving a 0 byte file.
		std::ofstream fp_touch(file.c_str());
		fp_touch.close();
	}

	/// @todo This is a temp hack, proper implementation needs to come after
	///       Windows support is added.  The code in the destructor needs fixing
	///       as well.
	const std::string tmp = file + "_tmp";

	// This will open to file.XXXX. (tempfile)
	fp = new std::ofstream(tmp.c_str());
}

Save::~Save() {

	const std::string tmp(file_name + "_tmp");
	util::Rename(tmp, file_name);
	delete fp;
	fp = 0;	// to avoid any silly errors.
}

std::ofstream& Save::Get() {
	return *fp;
}


	} // namespace io
} // namespace agi
