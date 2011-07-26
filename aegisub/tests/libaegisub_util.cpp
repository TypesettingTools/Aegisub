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

/// @file libaegisub_util.cpp
/// @brief agi::util (Utilities)
/// @ingroup util

#include <fstream>
#include <libaegisub/util.h>
#include "main.h"

class lagi_util : public libagi {
protected:
// placeholder
};


namespace agi {

TEST_F(lagi_util, UtilDirnameEmpty) {
	EXPECT_STREQ(".", util::DirName("").c_str());
}

TEST_F(lagi_util, UtilDirnameNoTrailingSlash) {
	EXPECT_STREQ(".", util::DirName("dot").c_str());
}

TEST_F(lagi_util, UtilDirnameRoot) {
	EXPECT_STREQ("/", util::DirName("/").c_str());
}

TEST_F(lagi_util, UtilDirnameHeir) {
	EXPECT_STREQ("/last/part/not_stripped/", util::DirName("/last/part/not_stripped/").c_str());
}

TEST_F(lagi_util, UtilDirnameHeirNoTrailingSlash) {
	EXPECT_STREQ("/last/part/", util::DirName("/last/part/stripped").c_str());
}

TEST_F(lagi_util, UtilRenameOverwrite) {
	util::Rename("./data/rename_me_overwrite", "./data/rename_me_overwrite_renamed");
	util::Rename("./data/rename_me_overwrite_renamed", "./data/rename_me_overwrite");
	std::ofstream fp_touch("./data/rename_me_overwrite_renamed");
}

TEST_F(lagi_util, UtilRenameNew) {
	util::Rename("./data/rename_me", "./data/rename_me_renamed");
	util::Rename("./data/rename_me_renamed", "./data/rename_me");
}

TEST_F(lagi_util, UtilRenameExNotFound) {
	EXPECT_THROW(util::Rename("./data/nonexistent", ""), acs::AcsNotFound);
}

TEST_F(lagi_util, Utilstr_lower) {
	std::string str("-!ABCDEFGHIJKLMNOPQRSTUVWXYZ123");
	util::str_lower(str);
	EXPECT_STREQ("-!abcdefghijklmnopqrstuvwxyz123", str.c_str());
}

TEST_F(lagi_util, UtilstrtoiInvalidRange) {
	std::string str("2147483650");
	EXPECT_EQ(0, util::strtoi(str));

	str.assign("-2147483650");
	EXPECT_EQ(0, util::strtoi(str));
}

TEST_F(lagi_util, UtilstrtoiInvalidString) {
	std::string str("bottles of beer on the wall");
	EXPECT_EQ(0, util::strtoi(str));
}

TEST_F(lagi_util, UtilstrtoiNumberWithString) {
	std::string str("24 bottles of beer on the wall");
	EXPECT_EQ(24, util::strtoi(str));
}

TEST_F(lagi_util, UtilstrtoiValidString) {
	std::string str("24");
	int i;

	EXPECT_NO_THROW(i = util::strtoi(str));
	EXPECT_EQ(24, i);
}

TEST_F(lagi_util, UtilfreespaceFile) {
	std::string path("./data/somefile");
	EXPECT_NO_THROW(util::freespace(path, util::TypeFile));
	EXPECT_ANY_THROW(util::freespace(path));

}

TEST_F(lagi_util, UtilfreespaceDir) {
	std::string path("./data");
	EXPECT_NO_THROW(util::freespace(path));
}

TEST_F(lagi_util, UtilfreespaceNoAccess) {
	std::string path("./data/dir_access_denied");
	EXPECT_THROW(util::freespace(path), acs::AcsAccess);
}

TEST_F(lagi_util, UtilfreespaceInvalid) {
	std::string path("/nonexistent");
	EXPECT_ANY_THROW(util::freespace(path));
}

} // namespace agi
