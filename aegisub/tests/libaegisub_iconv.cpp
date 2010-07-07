// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file libaegisub_iconv.cpp
/// @brief agi::charset
/// @ingroup iconv

#include <stdint.h>
#include <libaegisub/charset_conv.h>


#include "main.h"
#include "util.h"

using namespace agi::charset;

TEST(lagi_iconv, BasicSetup) {
	EXPECT_NO_THROW(IconvWrapper("UTF-8", "UTF-16LE"));
}

TEST(lagi_iconv, InvalidConversions) {
	EXPECT_THROW(IconvWrapper("nonexistent charset", "UTF-16LE"), UnsupportedConversion);
	EXPECT_THROW(IconvWrapper("UTF-16LE", "nonexistent charset"), UnsupportedConversion);
	EXPECT_THROW(IconvWrapper("nonexistent charset", "nonexistent charset"), UnsupportedConversion);
}

TEST(lagi_iconv, StrLen1) {
	IconvWrapper conv("UTF-8", "UTF-8", false);
	for (int i = 0; i < 10; i++) {
		std::string str(i, ' ');
		ASSERT_EQ(i, conv.SrcStrLen(str.c_str()));
		ASSERT_EQ(i, conv.DstStrLen(str.c_str()));
	}
}
TEST(lagi_iconv, StrLen2) {
	IconvWrapper conv("UTF-16LE", "UTF-16LE", false);
	for (int i = 0; i < 10; i++) {
		std::basic_string<int16_t> str(i, ' ');
		ASSERT_EQ(2*i, conv.SrcStrLen((const char *)str.c_str()));
		ASSERT_EQ(2*i, conv.DstStrLen((const char *)str.c_str()));
	}
}
TEST(lagi_iconv, StrLen4) {
	IconvWrapper conv("UTF-32LE", "UTF-32LE", false);
	for (int i = 0; i < 10; i++) {
		std::basic_string<int32_t> str(i, ' ');
		ASSERT_EQ(4*i, conv.SrcStrLen((const char *)str.c_str()));
		ASSERT_EQ(4*i, conv.DstStrLen((const char *)str.c_str()));
	}
}

TEST(lagi_iconv, Fallbacks) {
	IconvWrapper nofallback("UTF-8", "Shift-JIS", false);
	IconvWrapper fallback("UTF-8", "Shift-JIS", true);
	IconvWrapper noneneeded("UTF-8", "UTF-16LE", false);

	// Shift-JIS does not have a backslash
	EXPECT_THROW(nofallback.Convert("\\"), BadOutput);
	ASSERT_NO_THROW(fallback.Convert("\\"));
	EXPECT_EQ("\\", fallback.Convert("\\"));
	EXPECT_NO_THROW(noneneeded.Convert("\\"));

	// BOM into non-unicode
	char bom[] = "\xEF\xBB\xBF";
	EXPECT_THROW(nofallback.Convert(bom), BadOutput);
	ASSERT_NO_THROW(fallback.Convert(bom));
	EXPECT_EQ("", fallback.Convert(bom));
	EXPECT_NO_THROW(noneneeded.Convert(bom));

	// A snowman (U+2603)
	char snowman[] = "\xE2\x98\x83";
	EXPECT_THROW(nofallback.Convert(snowman), BadOutput);
	EXPECT_NO_THROW(noneneeded.Convert(snowman));
	ASSERT_NO_THROW(fallback.Convert(snowman));
	EXPECT_EQ("?", fallback.Convert(snowman));
}

TEST(lagi_iconv, BadInput) {
	IconvWrapper utf16("UTF-16LE", "UTF-8");
	EXPECT_THROW(utf16.Convert(" "), BadInput);
	IconvWrapper utf8("UTF-8", "UTF-16LE");
	EXPECT_THROW(utf8.Convert("\xE2\xFF"), BadInput);
}

TEST(lagi_iconv, Conversions) {
	IconvWrapper utf16le("UTF-16LE", "UTF-8", false);
	IconvWrapper utf16be("UTF-16BE", "UTF-8", false);
	IconvWrapper utf8("UTF-8", "UTF-16LE", false);

	char space_utf8_[] = " ";
	char space_utf16be_[] = {0, 32, 0, 0};
	char space_utf16le_[] = {32, 0, 0, 0};
	std::string space_utf8(space_utf8_);
	std::string space_utf16be(space_utf16be_, 2);
	std::string space_utf16le(space_utf16le_, 2);

	EXPECT_EQ(space_utf8, utf16le.Convert(space_utf16le));
	EXPECT_EQ(space_utf8, utf16be.Convert(space_utf16be));
	EXPECT_EQ(space_utf16le, utf8.Convert(space_utf8));
}

// Basic overflow tests
TEST(lagi_iconv, Buffer) {
	IconvWrapper conv("UTF-8", "UTF-16LE", false);
	char buff[32];
	memset(buff, 0xFF, sizeof(buff));

	EXPECT_THROW(conv.Convert("", 1, buff, 0), BufferTooSmall);
	EXPECT_EQ('\xFF', buff[0]);
	EXPECT_THROW(conv.Convert("", 1, buff, 1), BufferTooSmall);
	EXPECT_EQ('\xFF', buff[0]);
	EXPECT_NO_THROW(conv.Convert("", 1, buff, 2));
	EXPECT_EQ('\0', buff[0]);
	EXPECT_EQ('\0', buff[1]);
	EXPECT_EQ('\xFF', buff[2]);
}

TEST(lagi_iconv, LocalSupport) {
	ASSERT_NO_THROW(IconvWrapper("UTF-8", ""));
	IconvWrapper conv("UTF-8", "");
	ASSERT_NO_THROW(conv.Convert(" "));
	EXPECT_EQ(" ", conv.Convert(" "));
}
TEST(lagi_iconv, wchar_tSupport) {
	EXPECT_NO_THROW(IconvWrapper("UTF-8", "wchar_t"));
}

TEST(lagi_iconv, Roundtrip) {
	std::vector<std::string> names = GetEncodingsList<std::vector<std::string> >();
	for (std::vector<std::string>::iterator cur = names.begin(); cur != names.end(); ++cur) {
		EXPECT_NO_THROW(IconvWrapper("utf-8", cur->c_str()));
		EXPECT_NO_THROW(IconvWrapper(cur->c_str(), "utf-8"));
		EXPECT_EQ(
			"Jackdaws love my big sphinx of quartz",
			IconvWrapper(cur->c_str(), "utf-8").Convert(
				IconvWrapper("utf-8", cur->c_str()).Convert(
					"Jackdaws love my big sphinx of quartz")));
	}
}
