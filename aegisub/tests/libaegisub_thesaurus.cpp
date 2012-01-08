// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/thesaurus.h>

#include "main.h"
#include "util.h"

#include <fstream>

class lagi_thes : public libagi {
protected:
	std::string idx_path;
	std::string dat_path;

	void SetUp() {
		using std::endl;

		idx_path = "data/thes.idx";
		dat_path = "data/thes.dat";

		std::ofstream idx(idx_path.c_str());
		std::ofstream dat(dat_path.c_str());

		idx << "UTF-8" << endl;
		dat << "UTF-8" << endl;
		idx << 7 << endl; // entry count

		idx << "Word 1|" << dat.tellp() << endl;
		dat << "Word 1|1" << endl;
		dat << "(noun)|Word 1|Word 1A|Word 1B|Word 1C" << endl;

		idx << "Word 2|" << dat.tellp() << endl;
		dat << "Word 2|2" << endl;
		dat << "(adj)|Word 2|Word 2 adj" << endl;
		dat << "(noun)|Word 2|Word 2 noun" << endl;

		dat << "Unindexed Word|1" << endl;
		dat << "(adv)|Unindexed Word|Indexed Word" << endl;

		idx << "Word 3|" << dat.tellp() << endl;
		dat << "Word 3|1" << endl;
		dat << "(verb)|Not Word 3|Four" << endl;

		idx << "Too few fields" << endl;
		idx << "Too many fields|100|100" << endl;
		idx << "Not a number|foo" << endl;
		idx << "Out of range|" << dat.tellp() << endl;
		idx << "Further out of range|" << 1 + dat.tellp() << endl;
	}
};

TEST_F(lagi_thes, parse) {
	ASSERT_NO_THROW(agi::Thesaurus(dat_path, idx_path));
}

TEST_F(lagi_thes, word_1) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Word 1", &entries));
	ASSERT_EQ(1, entries.size());
	ASSERT_EQ(3, entries[0].second.size());
	EXPECT_STREQ("(noun) Word 1", entries[0].first.c_str());
	EXPECT_STREQ("Word 1A", entries[0].second[0].c_str());
	EXPECT_STREQ("Word 1B", entries[0].second[1].c_str());
	EXPECT_STREQ("Word 1C", entries[0].second[2].c_str());
}

TEST_F(lagi_thes, word_2) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Word 2", &entries));
	ASSERT_EQ(2, entries.size());
	ASSERT_EQ(1, entries[0].second.size());
	ASSERT_EQ(1, entries[1].second.size());
	EXPECT_STREQ("(adj) Word 2", entries[0].first.c_str());
	EXPECT_STREQ("(noun) Word 2", entries[1].first.c_str());
	EXPECT_STREQ("Word 2 adj", entries[0].second[0].c_str());
	EXPECT_STREQ("Word 2 noun", entries[1].second[0].c_str());
}

TEST_F(lagi_thes, word_3) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Word 3", &entries));
	ASSERT_EQ(1, entries.size());
	ASSERT_EQ(1, entries[0].second.size());
	EXPECT_STREQ("(verb) Not Word 3", entries[0].first.c_str());
	EXPECT_STREQ("Four", entries[0].second[0].c_str());
}

TEST_F(lagi_thes, bad_word) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Nonexistent word", &entries));
	EXPECT_EQ(0, entries.size());
}

TEST_F(lagi_thes, lookup_clears) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Word 1", &entries));
	ASSERT_NO_THROW(thes.Lookup("Word 2", &entries));
	ASSERT_NO_THROW(thes.Lookup("Word 3", &entries));
	EXPECT_EQ(1, entries.size());
}

TEST_F(lagi_thes, malformed_index_lines) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Too few fields", &entries));
	EXPECT_EQ(0, entries.size());
	ASSERT_NO_THROW(thes.Lookup("Too many fields", &entries));
	EXPECT_EQ(0, entries.size());
	ASSERT_NO_THROW(thes.Lookup("Not a number", &entries));
	EXPECT_EQ(0, entries.size());
	ASSERT_NO_THROW(thes.Lookup("Out of range", &entries));
	EXPECT_EQ(0, entries.size());
	ASSERT_NO_THROW(thes.Lookup("Further out of range", &entries));
	EXPECT_EQ(0, entries.size());
}

TEST_F(lagi_thes, unindexed_word) {
	agi::Thesaurus thes(dat_path, idx_path);

	std::vector<agi::Thesaurus::Entry> entries;
	ASSERT_NO_THROW(thes.Lookup("Unindexed Word", &entries));
	EXPECT_EQ(0, entries.size());
}
