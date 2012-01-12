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

/// @file libaegisub_access.cpp
/// @brief agi::acs tests.
/// @ingroup acs

#include <libaegisub/access.h>

#include "main.h"

using namespace agi;
using namespace agi::acs;

class lagi_acs : public libagi {

protected:
    // place holder for future code placement
};


// Yes, this is a horrifying use of macros, since these are all void static
// methods I couldn't think of a better way to test these without massive code
// duplication.
#define EX_FileNotFoundError(func, pass) \
	TEST_F(lagi_acs, func##ExFileNotFoundError) { \
		EXPECT_THROW(func("data/nonexistent"), FileNotFoundError); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_Fatal(func, fail, pass) \
	TEST_F(lagi_acs, func##ExFatal) { \
		EXPECT_THROW(func(fail), Fatal); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_NotAFile(func, fail, pass) \
	TEST_F(lagi_acs, func##ExNotAFile) { \
		EXPECT_THROW(func(fail), NotAFile); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_NotADirectory(func, fail, pass) \
	TEST_F(lagi_acs, func##ExNotADirectory) { \
		EXPECT_THROW(func(fail), NotADirectory); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_Read(func, fail, pass) \
	TEST_F(lagi_acs, func##ExRead) { \
		EXPECT_THROW(func(fail), Read); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_Write(func, fail, pass) \
	TEST_F(lagi_acs, func##ExWrite) { \
		EXPECT_THROW(func(fail), Write); \
		EXPECT_NO_THROW(func(pass)); \
	}

EX_FileNotFoundError(CheckFileRead, "data/file")
EX_Read(CheckFileRead, "data/file_access_denied", "data/file")
EX_NotAFile(CheckFileRead, "data/dir", "data/file")
TEST_F(lagi_acs, CheckFileRead) {
	EXPECT_NO_THROW(CheckFileRead("data/file"));
}

EX_FileNotFoundError(CheckFileWrite, "data/file")
EX_Read(CheckFileWrite, "data/file_access_denied", "data/file")
EX_NotAFile(CheckFileWrite, "data/dir", "data/file")
EX_Write(CheckFileWrite, "data/file_read_only", "data/file")
TEST_F(lagi_acs, CheckFileWrite) {
	EXPECT_NO_THROW(CheckFileRead("data/file"));
}

EX_FileNotFoundError(CheckDirRead, "data/dir")
EX_Read(CheckDirRead, "data/dir_access_denied", "data/dir")
EX_NotADirectory(CheckDirRead, "data/file", "data/dir")
TEST_F(lagi_acs, CheckDirRead) {
	EXPECT_NO_THROW(CheckDirRead("data/dir"));
}

EX_FileNotFoundError(CheckDirWrite, "data/dir")
EX_Read(CheckDirWrite, "data/dir_access_denied", "data/dir")
EX_NotADirectory(CheckDirWrite, "data/file", "data/dir")
EX_Write(CheckDirWrite, "data/dir_read_only", "data/dir")
TEST_F(lagi_acs, CheckDirWrite) {
	EXPECT_NO_THROW(CheckDirWrite("data/dir"));
}
