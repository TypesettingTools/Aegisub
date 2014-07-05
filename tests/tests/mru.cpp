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

#include <libaegisub/mru.h>

#include <libaegisub/fs.h>
#include <libaegisub/option.h>
#include <libaegisub/option_value.h>

#include <main.h>

static const char default_mru[] = "{\"Video\" : []}";
static const char conf_ok[] = "data/mru_ok.json";

TEST(lagi_mru, load_from_file) {
	ASSERT_NO_THROW(agi::MRUManager mru(conf_ok, default_mru));
	agi::MRUManager mru(conf_ok, default_mru);
	ASSERT_NO_THROW(mru.Get("Video"));
	ASSERT_EQ(2u, mru.Get("Video")->size());
	auto entry = mru.Get("Video")->begin();
	EXPECT_STREQ("Entry One", (*entry++).string().c_str());
	EXPECT_STREQ("Entry Two", (*entry++).string().c_str());
	EXPECT_TRUE(mru.Get("Video")->end() == entry);
}

TEST(lagi_mru, load_from_default_string) {
	agi::fs::Remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
}

TEST(lagi_mru, load_from_invalid_file) {
	agi::fs::Copy("data/mru_invalid.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_TRUE(mru.Get("Video")->empty());
}

TEST(lagi_mru, add_entry) {
	agi::fs::Copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Video", "/path/to/file"));
	EXPECT_STREQ("/path/to/file", mru.Get("Video")->front().string().c_str());
}

TEST(lagi_mru, remove_entry) {
	agi::fs::Copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Video", "/path/to/file"));
	EXPECT_NO_THROW(mru.Remove("Video", "/path/to/file"));
	EXPECT_STRNE("/path/to/file", mru.Get("Video")->front().string().c_str());
}

TEST(lagi_mru, invalid_mru_key_throws) {
	agi::fs::Copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_THROW(mru.Add("Invalid", "/path/to/file"), agi::MRUError);
	EXPECT_THROW(mru.Get("Invalid"), agi::MRUError);
}

TEST(lagi_mru, valid_mru_key_doesnt_throw) {
	agi::fs::Copy("data/mru_ok.json", "data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Video", "/path/to/file"));
}

TEST(lagi_mru, adding_existing_moves_to_front) {
	agi::fs::Remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);

	EXPECT_NO_THROW(mru.Add("Video", "/file/1"));
	EXPECT_NO_THROW(mru.Add("Video", "/file/2"));
	EXPECT_NO_THROW(mru.Add("Video", "/file/3"));
	EXPECT_NO_THROW(mru.Add("Video", "/file/1"));
	EXPECT_NO_THROW(mru.Add("Video", "/file/1"));
	EXPECT_NO_THROW(mru.Add("Video", "/file/3"));

	EXPECT_STREQ("/file/3", mru.GetEntry("Video", 0).string().c_str());
	EXPECT_STREQ("/file/1", mru.GetEntry("Video", 1).string().c_str());
	EXPECT_STREQ("/file/2", mru.GetEntry("Video", 2).string().c_str());
	EXPECT_THROW(mru.GetEntry("Video", 3), agi::MRUError);
}

TEST(lagi_mru, all_valid_keys_work) {
	agi::fs::Remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);
	EXPECT_NO_THROW(mru.Add("Audio", "/file"));
	EXPECT_NO_THROW(mru.Add("Find", "/file"));
	EXPECT_NO_THROW(mru.Add("Keyframes", "/file"));
	EXPECT_NO_THROW(mru.Add("Replace", "/file"));
	EXPECT_NO_THROW(mru.Add("Subtitle", "/file"));
	EXPECT_NO_THROW(mru.Add("Timecodes", "/file"));
	EXPECT_NO_THROW(mru.Add("Video", "/file"));
}

TEST(lagi_mru, prune_works) {
	agi::fs::Remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", default_mru);

	for (int i = 0; i < 20; ++i) {
		ASSERT_NO_THROW(mru.Add("Audio", std::to_string(i)));
	}

	EXPECT_EQ(16, mru.Get("Audio")->size());
	EXPECT_EQ("19", mru.Get("Audio")->front());
}

TEST(lagi_mru, prune_obeys_option) {
	static const char opt[] = R"raw({"Limits": {"MRU": 1}})raw";
	agi::Options options("", opt, agi::Options::FLUSH_SKIP);

	agi::fs::Remove("data/mru_tmp");
	agi::MRUManager mru("data/mru_tmp", {default_mru, sizeof(default_mru)}, &options);

	ASSERT_NO_THROW(mru.Add("Audio", "1"));
	ASSERT_NO_THROW(mru.Add("Audio", "2"));

	EXPECT_EQ(1, mru.Get("Audio")->size());
}

// Check to make sure an entry is really removed.  This was fixed in
// r4347, the entry was being removed from a copy of the map internally.
TEST(lagi_mru, MRUEntryRemove_r4347) {
	agi::MRUManager mru(conf_ok, default_mru);
	EXPECT_NO_THROW(mru.Add("Video", "/path/to/file"));
	EXPECT_NO_THROW(mru.Remove("Video", "/path/to/file"));

	const agi::MRUManager::MRUListMap *map_list = mru.Get("Video");
	auto i_lst = map_list->begin();

	if ((i_lst != map_list->end()) && (*i_lst == "/path/to/file"))
		FAIL() << "r4347 regression, Entry exists after remove";
}
