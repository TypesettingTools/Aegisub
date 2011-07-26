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

/// @file access.h
/// @brief Public interface for access methods.
/// @ingroup libaegisub

#include <libaegisub/exception.h>

namespace agi {
	namespace acs {

DEFINE_BASE_EXCEPTION_NOINNER(AcsError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsFatal, AcsError, "acs/fatal")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsNotFound, AcsError, "acs/notfound")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsNotAFile, AcsError, "acs/file")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsNotADirectory, AcsError, "acs/directory")

DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsAccess, AcsError, "acs/access")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsRead, AcsAccess, "acs/access/read")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsWrite, AcsAccess, "acs/access/write")


enum Type {
	FileRead,
	DirRead,
	FileWrite,
	DirWrite
};


void Check(const std::string &file, acs::Type);

void CheckFileRead(const std::string &file);
void CheckDirRead(const std::string &dir);

void CheckFileWrite(const std::string &file);
void CheckDirWrite(const std::string &dir);


	} // namespace axs
} // namespace agi
