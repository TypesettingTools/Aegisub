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

/// @file libaegisub_mru.cpp
/// @brief agi::mru (Most Recently Used)
/// @ingroup mru

#include <libaegisub/mru.h>
#include "main.h"
#include "util.h"

class lagi_mru : public libagi {
protected:
	std::string default_mru;
	std::string conf_ok;

	void SetUp() {
		default_mru = "{\"Valid\" : []}";
		conf_ok = "./data/mru_ok.json";
	}
};


TEST_F(lagi_mru, MRUConstructFromFile) {
	ASSERT_NO_THROW(agi::MRUManager mru(conf_ok, default_mru));
	agi::MRUManager mru(conf_ok, default_mru);
	agi::MRUManager::MRUListMap::const_iterator entry = mru.Get("Valid")->begin();
	EXPECT_STREQ("Entry One", (*entry++).c_str());
	EXPECT_STREQ("Entry Two", (*entry++).c_str());
	EXPECT_TRUE(mru.Get("Valid")->end() == entry);
}

TEST_F(lagi_mru, MRUConstructFromString) {
	util::remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
}

TEST_F(lagi_mru, MRUConstructInvalid) {
	util::copy("data/mru_invalid.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_TRUE(mru.Get("Invalid")->empty());
}

TEST_F(lagi_mru, MRUEntryAdd) {
	util::copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Valid", "/path/to/file"));
	EXPECT_STREQ("/path/to/file", mru.Get("Valid")->front().c_str());
}

TEST_F(lagi_mru, MRUEntryRemove) {
	util::copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Valid", "/path/to/file"));
	EXPECT_NO_THROW(mru.Remove("Valid", "/path/to/file"));
	EXPECT_STRNE("/path/to/file", mru.Get("Valid")->front().c_str());
}

TEST_F(lagi_mru, MRUKeyInvalid) {
	util::copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_THROW(mru.Add("Invalid", "/path/to/file"), agi::MRUErrorInvalidKey);
	EXPECT_THROW(mru.Get("Invalid"), agi::MRUErrorInvalidKey);
}

TEST_F(lagi_mru, MRUKeyValid) {
	util::copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Valid", "/path/to/file"));
}

TEST_F(lagi_mru, MRUAddSeveral) {
	util::remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);

	EXPECT_NO_THROW(mru.Add("Valid", "/file/1"));
	EXPECT_NO_THROW(mru.Add("Valid", "/file/2"));
	EXPECT_NO_THROW(mru.Add("Valid", "/file/3"));
	EXPECT_NO_THROW(mru.Add("Valid", "/file/1"));
	EXPECT_NO_THROW(mru.Add("Valid", "/file/1"));
	EXPECT_NO_THROW(mru.Add("Valid", "/file/3"));

	EXPECT_STREQ("/file/3", mru.GetEntry("Valid", 0).c_str());
	EXPECT_STREQ("/file/1", mru.GetEntry("Valid", 1).c_str());
	EXPECT_STREQ("/file/2", mru.GetEntry("Valid", 2).c_str());
	EXPECT_THROW(mru.GetEntry("Valid", 3), agi::MRUErrorIndexOutOfRange);
}

// Check to make sure an entry is really removed.  This was fixed in
// r4347, the entry was being removed from a copy of the map internally.
TEST_F(lagi_mru, MRUEntryRemove_r4347) {

	agi::MRUManager mru(conf_ok, default_mru);
	EXPECT_NO_THROW(mru.Add("Valid", "/path/to/file"));
	EXPECT_NO_THROW(mru.Remove("Valid", "/path/to/file"));

	const agi::MRUManager::MRUListMap *map_list = mru.Get("Valid");
	agi::MRUManager::MRUListMap::const_iterator i_lst = map_list->begin();

	if ((i_lst != map_list->end()) && (*i_lst == "/path/to/file"))
		FAIL() << "r4347 regression, Entry exists after remove";
}
