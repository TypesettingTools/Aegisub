// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/exception.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>

#include <main.h>

#include <boost/filesystem.hpp>

using agi::Path;

#ifdef _WIN32
#define DS "\\"
#else
#define DS "/"
#endif

TEST(lagi_path, invalid_token_name_throws_internal_error) {
	Path p;

	// These are InternalError because the tokens are currently always hardcoded
	EXPECT_THROW(p.SetToken("no ?", "path"), agi::InternalError);
	EXPECT_THROW(p.SetToken("?bad", "path"), agi::InternalError);
}

TEST(lagi_path, relative_path_clears_token) {
	Path p;

	EXPECT_NO_THROW(p.SetToken("?video", "relative/path"));
	EXPECT_STREQ("?video", p.Decode("?video").string().c_str());

	EXPECT_NO_THROW(p.SetToken("?video", boost::filesystem::current_path()));
	EXPECT_STRNE("?video", p.Decode("?video").string().c_str());

	EXPECT_NO_THROW(p.SetToken("?video", "relative/path"));
	EXPECT_STREQ("?video", p.Decode("?video").string().c_str());
}

TEST(lagi_path, empty_path_clears_token) {
	Path p;

	EXPECT_NO_THROW(p.SetToken("?video", boost::filesystem::current_path()));
	EXPECT_STRNE("?video", p.Decode("?video").string().c_str());

	EXPECT_NO_THROW(p.SetToken("?video", ""));
	EXPECT_STREQ("?video", p.Decode("?video").string().c_str());
}

TEST(lagi_path, decode_sets_uses_right_slashes) {
	Path p;

	agi::fs::path expected = boost::filesystem::current_path()/"foo/bar.txt";
	expected.make_preferred();

	EXPECT_NO_THROW(p.SetToken("?video", boost::filesystem::current_path()));

	agi::fs::path decoded;
	ASSERT_NO_THROW(decoded = p.Decode("?video/foo/bar.txt"));
	EXPECT_STREQ(expected.string().c_str(), decoded.string().c_str());
}

TEST(lagi_path, trailing_slash_on_token_is_optional) {
	Path p;

	agi::fs::path expected = boost::filesystem::current_path()/"foo.txt";
	expected.make_preferred();

	EXPECT_NO_THROW(p.SetToken("?audio", boost::filesystem::current_path()));

	agi::fs::path decoded;
	ASSERT_NO_THROW(decoded = p.Decode("?audiofoo.txt"));
	EXPECT_STREQ(expected.string().c_str(), decoded.string().c_str());

	ASSERT_NO_THROW(decoded = p.Decode("?audio/foo.txt"));
	EXPECT_STREQ(expected.string().c_str(), decoded.string().c_str());
}

TEST(lagi_path, setting_token_to_file_sets_to_parent_directory_instead) {
	Path p;

	agi::fs::path file = boost::filesystem::system_complete("data/file");
	ASSERT_NO_THROW(p.SetToken("?script", file));
	EXPECT_STREQ(file.parent_path().string().c_str(), p.Decode("?script").string().c_str());

	file = boost::filesystem::system_complete("data/dir");
	ASSERT_NO_THROW(p.SetToken("?script", file));
	EXPECT_STREQ(file.string().c_str(), p.Decode("?script").string().c_str());
}

TEST(lagi_path, valid_token_names) {
	Path p;

	EXPECT_NO_THROW(p.SetToken("?user", ""));
	EXPECT_NO_THROW(p.SetToken("?local", ""));
	EXPECT_NO_THROW(p.SetToken("?data", ""));
	EXPECT_NO_THROW(p.SetToken("?temp", ""));
	EXPECT_NO_THROW(p.SetToken("?dictionary", ""));
	EXPECT_NO_THROW(p.SetToken("?audio", ""));
	EXPECT_NO_THROW(p.SetToken("?script", ""));
	EXPECT_NO_THROW(p.SetToken("?video", ""));
}

#define TEST_PLATFORM_PATH_TOKEN(tok) \
	do { \
		agi::fs::path d; \
		ASSERT_NO_THROW(d = p.Decode(tok)); \
		ASSERT_FALSE(d.empty()); \
		ASSERT_STRNE(tok, d.string().c_str()); \
		EXPECT_TRUE(agi::fs::DirectoryExists(d)); \
	} while (false)

TEST(lagi_path, platform_paths_have_values_and_exist) {
	Path p;
#ifndef __APPLE__
	// Isn't set on OS X due to the app not being in a bundle
	TEST_PLATFORM_PATH_TOKEN("?data");
#endif
	TEST_PLATFORM_PATH_TOKEN("?user");
	TEST_PLATFORM_PATH_TOKEN("?local");
	TEST_PLATFORM_PATH_TOKEN("?temp");
}

TEST(lagi_path, making_empty_absolute_gives_empty) {
	Path p;
	ASSERT_NO_THROW(p.MakeAbsolute("", "?data"));
	EXPECT_TRUE(p.MakeAbsolute("", "?data").empty());
}

TEST(lagi_path, making_empty_relative_gives_empty) {
	Path p;
	ASSERT_NO_THROW(p.MakeRelative("", "?data"));
	EXPECT_TRUE(p.MakeRelative("", "?data").empty());
}

#ifdef _WIN32
TEST(lagi_path, make_absolute_on_network_path) {
	Path p;
	ASSERT_NO_THROW(p.MakeAbsolute("//foo/bar", "?data"));
	EXPECT_STREQ("\\\\foo\\bar", p.MakeAbsolute("//foo/bar", "?data").string().c_str());
}

TEST(lagi_path, make_relative_on_network_path) {
	Path p;
	ASSERT_NO_THROW(p.MakeRelative("\\\\foo\\bar", "?data"));
	EXPECT_STREQ("\\\\foo\\bar", p.MakeRelative("\\\\foo\\bar", "?data").string().c_str());
}
#endif

#define EXPECT_UNCHANGED(url, func) EXPECT_STREQ(url, p.func(url, "?data").string().c_str())

TEST(lagi_path, make_absolute_on_dummy_url) {
	Path p;
	EXPECT_UNCHANGED("dummy-audio:silence?sr=44100&bd=16&ch=1&ln=396900000", MakeAbsolute);
	EXPECT_UNCHANGED("?dummy:23.976000:40000:1280:720:47:163:254:", MakeAbsolute);
}

TEST(lagi_path, make_relative_on_dummy_url) {
	Path p;
	EXPECT_UNCHANGED("dummy-audio:silence?sr=44100&bd=16&ch=1&ln=396900000", MakeRelative);
	EXPECT_UNCHANGED("?dummy:23.976000:40000:1280:720:47:163:254:", MakeRelative);
}
