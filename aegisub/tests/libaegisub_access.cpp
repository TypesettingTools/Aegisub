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

using namespace agi::acs;

class lagi_acs : public libagi {

protected:
    // place holder for future code placement
};


// Yes, this is a horrifying use of macros, since these are all void static
// methods I couldn't think of a better way to test these without massive code
// duplication.
#define EX_AcsNotFound(func, pass) \
	TEST_F(lagi_acs, func##ExAcsNotFound) { \
		EXPECT_THROW(func("data/nonexistent"), AcsNotFound); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_AcsAccess(func, fail, pass) \
	TEST_F(lagi_acs, func##ExAcsAccess) { \
		EXPECT_THROW(func(fail), AcsAccess); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_AcsNotAFile(func, fail, pass) \
	TEST_F(lagi_acs, func##ExAcsNotAFile) { \
		EXPECT_THROW(func(fail), AcsNotAFile); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_AcsNotADirectory(func, fail, pass) \
	TEST_F(lagi_acs, func##ExAcsNotADirectory) { \
		EXPECT_THROW(func(fail), AcsNotADirectory); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_AcsRead(func, fail, pass) \
	TEST_F(lagi_acs, func##ExAcsRead) { \
		EXPECT_THROW(func(fail), AcsRead); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_AcsWrite(func, fail, pass) \
	TEST_F(lagi_acs, func##ExAcsWrite) { \
		EXPECT_THROW(func(fail), AcsWrite); \
		EXPECT_NO_THROW(func(pass)); \
	}


/*
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsFatal, AcsError, "io/fatal")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsAccessRead, AcsError, "io/read")
DEFINE_SIMPLE_EXCEPTION_NOINNER(AcsAccessWrite, AcsError, "io/write")
*/


EX_AcsNotFound(CheckFileRead, "data/file")
EX_AcsAccess(CheckFileRead, "data/file_access_denied", "data/file")
EX_AcsNotAFile(CheckFileRead, "data/dir", "data/file")
TEST_F(lagi_acs, CheckFileRead) {
	EXPECT_NO_THROW(CheckFileRead("data/file"));
}

EX_AcsNotFound(CheckFileWrite, "data/file")
EX_AcsAccess(CheckFileWrite, "data/file_access_denied", "data/file")
EX_AcsNotAFile(CheckFileWrite, "data/dir", "data/file")
EX_AcsWrite(CheckFileWrite, "data/file_read_only", "data/file")
TEST_F(lagi_acs, CheckFileWrite) {
	EXPECT_NO_THROW(CheckFileRead("data/file"));
}

EX_AcsNotFound(CheckDirRead, "data/dir")
EX_AcsAccess(CheckDirRead, "data/dir_access_denied", "data/dir")
EX_AcsNotADirectory(CheckDirRead, "data/file", "data/dir")
TEST_F(lagi_acs, CheckDirRead) {
	EXPECT_NO_THROW(CheckDirRead("data/dir"));
}

EX_AcsNotFound(CheckDirWrite, "data/dir")
EX_AcsAccess(CheckDirWrite, "data/dir_access_denied", "data/dir")
EX_AcsNotADirectory(CheckDirWrite, "data/file", "data/dir")
EX_AcsWrite(CheckDirWrite, "data/dir_read_only", "data/dir")
TEST_F(lagi_acs, CheckDirWrite) {
	EXPECT_NO_THROW(CheckDirWrite("data/dir"));
}
