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

#include <main.h>

#include <libaegisub/access.h>
#include <libaegisub/fs.h>

using namespace agi;
using namespace agi::acs;
using namespace agi::fs;

// Yes, this is a horrifying use of macros, since these are all void static
// methods I couldn't think of a better way to test these without massive code
// duplication.
#define DO_TEST(func, name, fail, fail_ex, pass) \
	TEST(lagi_acs, name) { \
		EXPECT_THROW(func(fail), fail_ex); \
		EXPECT_NO_THROW(func(pass)); \
	}

#define EX_FileNotFound(func, pass) \
	DO_TEST(func, func##ExFileNotFound, "data/nonexistent", FileNotFound, pass)

#define EX_Fatal(func, fail, pass) \
	DO_TEST(func, func##ExFatal, fail, Fatal, pass)

#define EX_NotAFile(func, fail, pass) \
	DO_TEST(func, func##ExNotAFile, fail, NotAFile, pass)

#define EX_NotADirectory(func, fail, pass) \
	DO_TEST(func, func##ExNotADirectory, fail, NotADirectory, pass)

#define EX_Read(func, fail, pass) \
	DO_TEST(func, func##ExReadDenied, fail, ReadDenied, pass)

#define EX_Write(func, fail, pass) \
	DO_TEST(func, func##ExWriteDenied, fail, WriteDenied, pass)

EX_FileNotFound(CheckFileRead, "data/file")
EX_Read(CheckFileRead, "data/file_access_denied", "data/file")
EX_NotAFile(CheckFileRead, "data/dir", "data/file")
TEST(lagi_acs, CheckFileRead) {
	EXPECT_NO_THROW(CheckFileRead("data/file"));
}

EX_FileNotFound(CheckFileWrite, "data/file")
EX_Read(CheckFileWrite, "data/file_access_denied", "data/file")
EX_NotAFile(CheckFileWrite, "data/dir", "data/file")
EX_Write(CheckFileWrite, "data/file_read_only", "data/file")
TEST(lagi_acs, CheckFileWrite) {
	EXPECT_NO_THROW(CheckFileRead("data/file"));
}

EX_FileNotFound(CheckDirRead, "data/dir")
EX_Read(CheckDirRead, "data/dir_access_denied", "data/dir")
EX_NotADirectory(CheckDirRead, "data/file", "data/dir")
TEST(lagi_acs, CheckDirRead) {
	EXPECT_NO_THROW(CheckDirRead("data/dir"));
}

EX_FileNotFound(CheckDirWrite, "data/dir")
EX_Read(CheckDirWrite, "data/dir_access_denied", "data/dir")
EX_NotADirectory(CheckDirWrite, "data/file", "data/dir")
EX_Write(CheckDirWrite, "data/dir_read_only", "data/dir")
TEST(lagi_acs, CheckDirWrite) {
	EXPECT_NO_THROW(CheckDirWrite("data/dir"));
}
