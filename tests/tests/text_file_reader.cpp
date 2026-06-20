// Copyright (c) 2026, Aegisub contributors
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
// Aegisub Project https://aegisub.org/

#include <text_file_reader.h>
#include <main.h>
#include <util.h>

// BOM (U+FEFF) at the start of a file: existing behaviour, must remain working.
TEST(lagi_text_file_reader, strips_bom_on_first_line) {
	auto dir = util::test_data_dir() / "text_file_reader";
	TextFileReader reader(dir / "U+FEFF_BOM_zero_width_no_break_space.srt", "utf-8", true);
	ASSERT_TRUE(reader.HasMoreLines());
	EXPECT_EQ("1", reader.ReadLineFromFile());
}

// A single invisible character (U+200F, RIGHT-TO-LEFT MARK) at the very start
// of the file must be stripped before the subtitle index is parsed.
TEST(lagi_text_file_reader, strips_leading_invisible_char) {
	auto dir = util::test_data_dir() / "text_file_reader";
	TextFileReader reader(dir / "U+200F_right_to_left_mark.srt", "utf-8", true);
	ASSERT_TRUE(reader.HasMoreLines());
	EXPECT_EQ("1", reader.ReadLineFromFile());
}

// Multiple invisible characters stacked at the start of the file must all
// be stripped before the subtitle index is parsed.
TEST(lagi_text_file_reader, strips_stacked_leading_invisible_chars) {
	auto dir = util::test_data_dir() / "text_file_reader";
	TextFileReader reader(dir / "U+FEFF_U+200E_U+202A_U+200B_U+2060_U+200C_stacked.srt", "utf-8", true);
	ASSERT_TRUE(reader.HasMoreLines());
	EXPECT_EQ("1", reader.ReadLineFromFile());
}

// An invisible character (U+200F) inside subtitle content must NOT be stripped.
// RTL marks are meaningful in Arabic and other RTL subtitle content.
TEST(lagi_text_file_reader, preserves_invisible_chars_in_content) {
	auto dir = util::test_data_dir() / "text_file_reader";
	TextFileReader reader(dir / "U+200F_rtl_mark_preserved.srt", "utf-8", true);
	ASSERT_TRUE(reader.HasMoreLines());
	reader.ReadLineFromFile(); // subtitle index "1"
	ASSERT_TRUE(reader.HasMoreLines());
	reader.ReadLineFromFile(); // timecode line
	ASSERT_TRUE(reader.HasMoreLines());
	std::string content = reader.ReadLineFromFile();
	EXPECT_TRUE(content.starts_with("\xE2\x80\x8F")); // U+200F must be intact
}
