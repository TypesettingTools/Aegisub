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

/// @file io.h
/// @brief Public interface for IO methods.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#endif

#include <libaegisub/exception.h>

namespace agi {
	namespace io {

DEFINE_BASE_EXCEPTION_NOINNER(IOError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(IOFatal, IOError, "io/fatal")

/*
DEFINE_SIMPLE_EXCEPTION_NOINNER(IOAccess, IOError, "io/access")
DEFINE_SIMPLE_EXCEPTION_NOINNER(IONotFound, IOError, "io/notfound")
DEFINE_SIMPLE_EXCEPTION_NOINNER(IONotAFile, IOError, "io/file")
DEFINE_SIMPLE_EXCEPTION_NOINNER(IONotADirectory, IOError, "io/directory")
DEFINE_SIMPLE_EXCEPTION_NOINNER(IOAccessRead, IOError, "io/read")
DEFINE_SIMPLE_EXCEPTION_NOINNER(IOAccessWrite, IOError, "io/write")
*/

std::ifstream* Open(const std::string &file);

class Save {
	std::ofstream *fp;
	const std::string file_name;

public:
    Save(const std::string& file);
    ~Save();
    std::ofstream& Get();
};


	} // namespace io
} // namespace agi


