// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <gtest/gtest.h>
#include "../../src/lyrics_import.h"
#include <wx/mstream.h>
#include <stdexcept>
#include <string>

namespace {
std::vector<lyrics::Line> Ttml(std::string const& paragraphs) {
	std::string xml = "<tt xmlns=\"http://www.w3.org/ns/ttml\" xmlns:ttm=\"http://www.w3.org/ns/ttml#metadata\"><body><div>" + paragraphs + "</div></body></tt>";
	wxMemoryInputStream input(xml.data(), xml.size());
	return lyrics::ParseTTML(input);
}
}

TEST(lyrics_import, lrc_fractional_and_integer_timestamps) {
	auto rows = lyrics::ParseLrc({"[00:10]a", "[00:11.5]b", "[00:12.50]c", "[00:13.500]d"});
	ASSERT_EQ(4u, rows.size());
	EXPECT_EQ(10000, rows[0].start_ms);
	EXPECT_EQ(11500, rows[1].start_ms);
	EXPECT_EQ(12500, rows[2].start_ms);
	EXPECT_EQ(13500, rows[3].start_ms);
}

TEST(lyrics_import, lrc_positive_and_negative_offset) {
	for (auto offset : {"[offset:1000]", "[offset:+1000]"}) {
		auto rows = lyrics::ParseLrc({offset, "[00:10]Hello"});
		ASSERT_EQ(1u, rows.size());
		EXPECT_EQ(9000, rows[0].start_ms);
	}
	auto rows = lyrics::ParseLrc({"[offset:-1000]", "[00:10]Hello"});
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ(11000, rows[0].start_ms);
}

TEST(lyrics_import, lrc_offset_at_end_is_global) {
	auto rows = lyrics::ParseLrc({"[00:10]<00:11>Hello<00:12>", "[offset:+1000]"});
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ(9000, rows[0].start_ms);
	EXPECT_EQ(11000, rows[0].end_ms);
	EXPECT_EQ("{\\k100}{\\kf100}Hello", rows[0].text);
}

TEST(lyrics_import, lrc_leading_gap_and_explicit_end) {
	auto rows = lyrics::ParseLrc({"[00:10.00]<00:11.00>Hello <00:12.00>world<00:13.00>"});
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ(10000, rows[0].start_ms);
	EXPECT_EQ(13000, rows[0].end_ms);
	EXPECT_EQ("{\\k100}{\\kf100}Hello {\\kf100}world", rows[0].text);
}

TEST(lyrics_import, lrc_close_rows_do_not_overlap) {
	auto rows = lyrics::ParseLrc({"[00:10.00]First", "[00:10.10]Second", "[00:10.10]Third"});
	ASSERT_EQ(3u, rows.size());
	EXPECT_EQ(10100, rows[0].end_ms);
	EXPECT_LE(rows[0].end_ms, rows[1].start_ms);
	EXPECT_LE(rows[1].end_ms, rows[2].start_ms);
	EXPECT_EQ("Second", rows[1].text);
}

TEST(lyrics_import, lrc_empty_timestamp_clears_previous_row) {
	auto rows = lyrics::ParseLrc({"[00:10.00]First phrase", "[00:12.00]", "[01:00.00]Next phrase"});
	ASSERT_EQ(2u, rows.size());
	EXPECT_EQ(12000, rows[0].end_ms);
	EXPECT_EQ(60000, rows[1].start_ms);
}

TEST(lyrics_import, lrc_clamps_karaoke_to_next_row) {
	auto rows = lyrics::ParseLrc({"[00:10]<00:10>A<00:14>B<00:15>", "[00:12]Next"});
	ASSERT_EQ(2u, rows.size());
	EXPECT_EQ(12000, rows[0].end_ms);
	EXPECT_EQ("{\\kf200}A{\\kf0}B", rows[0].text);
}

TEST(lyrics_import, lrc_plain_multiple_timestamps_and_untimed_text) {
	auto rows = lyrics::ParseLrc({"[00:20][00:10]Hello"});
	ASSERT_EQ(2u, rows.size());
	EXPECT_EQ(10000, rows[0].start_ms);
	EXPECT_EQ(20000, rows[1].start_ms);
	EXPECT_EQ("Hello", rows[1].text);
	rows = lyrics::ParseLrc({"[ar:Artist]", "First", "Second"});
	ASSERT_EQ(2u, rows.size());
	EXPECT_EQ("First", rows[0].text);
	EXPECT_EQ("Second", rows[1].text);
	EXPECT_EQ(0, rows[0].end_ms);
}

TEST(lyrics_import, lrc_rejects_invalid_and_overflowing_times) {
	for (auto line : {"[00:10x.50]bad", "[999999999999999999999:00]bad", "[00:10.1234]bad"})
		EXPECT_THROW(lyrics::ParseLrc({line}), std::invalid_argument);
	EXPECT_THROW(lyrics::ParseLrc({"[offset:1000bad]", "[00:10]x"}), std::invalid_argument);
}

TEST(lyrics_import, ttml_preserves_document_order_when_untimed_or_equal) {
	for (auto paragraphs : {"<p>First</p><p>Second</p>", "<p begin=\"1s\" end=\"2s\">First</p><p begin=\"1s\" end=\"2s\">Second</p>"}) {
		auto rows = Ttml(paragraphs);
		ASSERT_EQ(2u, rows.size());
		EXPECT_EQ("First", rows[0].text);
		EXPECT_EQ("Second", rows[1].text);
	}
}

TEST(lyrics_import, ttml_untimed_nested_spans_are_not_duplicated) {
	auto rows = Ttml("<p><span><span>Hello</span></span> world</p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("Hello world", rows[0].text);
}

TEST(lyrics_import, ttml_leading_and_inter_word_gaps) {
	auto rows = Ttml("<p begin=\"0s\" end=\"5s\"><span begin=\"1s\" end=\"2s\">A </span><span begin=\"3s\" end=\"4s\">B</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ(0, rows[0].start_ms);
	EXPECT_EQ(5000, rows[0].end_ms);
	EXPECT_EQ("{\\k100}{\\kf100}A {\\k100}{\\kf100}B", rows[0].text);
}

TEST(lyrics_import, ttml_preserves_space_between_spans) {
	auto rows = Ttml("<p begin=\"0s\" end=\"2s\"><span begin=\"0s\" end=\"1s\">Hello</span> <span begin=\"1s\" end=\"2s\">world</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("{\\kf100}Hello {\\kf100}world", rows[0].text);
}

TEST(lyrics_import, ttml_prefixed_elements_utf8_and_line_breaks) {
	std::string xml = "<tt:tt xmlns:tt=\"http://www.w3.org/ns/ttml\"><tt:body><tt:div><tt:p>你好<tt:br/>世界</tt:p></tt:div></tt:body></tt:tt>";
	wxMemoryInputStream input(xml.data(), xml.size());
	auto rows = lyrics::ParseTTML(input);
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("你好\\N世界", rows[0].text);
}

TEST(lyrics_import, ttml_nested_wrapper_preserves_suffix_order) {
	auto rows = Ttml("<p begin=\"0s\" end=\"2s\"><span><span begin=\"0s\" end=\"1s\">A</span></span>!<span begin=\"1s\" end=\"2s\">B</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("{\\kf100}A!{\\kf100}B", rows[0].text);
}

TEST(lyrics_import, ttml_word_times_derive_untimed_paragraph_bounds) {
	auto rows = Ttml("<p><span begin=\"1s\" end=\"2s\">A</span><span begin=\"3s\" end=\"4s\">B</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ(1000, rows[0].start_ms);
	EXPECT_EQ(4000, rows[0].end_ms);
	EXPECT_EQ("{\\kf100}A{\\k100}{\\kf100}B", rows[0].text);
}

TEST(lyrics_import, ttml_background_overlap_splits_rows) {
	auto rows = Ttml("<p begin=\"0s\" end=\"5s\"><span begin=\"1s\" end=\"4s\">Lead</span><span ttm:role=\"x-bg\"><span begin=\"2s\" end=\"3s\">BG</span></span></p>");
	ASSERT_EQ(2u, rows.size());
	EXPECT_EQ("{\\k100}{\\kf300}Lead", rows[0].text);
	EXPECT_EQ(2000, rows[1].start_ms);
	EXPECT_EQ("{\\kf100}BG", rows[1].text);
}

TEST(lyrics_import, ttml_background_sequential_stays_inline_with_gap) {
	auto rows = Ttml("<p begin=\"0s\" end=\"5s\"><span begin=\"0s\" end=\"1s\">Lead</span><span ttm:role=\"x-bg\"><span begin=\"3s\" end=\"4s\">BG</span></span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("{\\kf100}Lead{\\k200}{\\kf100}BG", rows[0].text);
}

TEST(lyrics_import, ttml_word_without_end_and_paragraph_duration) {
	auto rows = Ttml("<p begin=\"1s\" dur=\"3s\"><span begin=\"1s\">A</span><span begin=\"3s\">B</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ(4000, rows[0].end_ms);
	EXPECT_EQ("{\\kf200}A{\\kf100}B", rows[0].text);
}

TEST(lyrics_import, ttml_clips_word_to_paragraph_end) {
	auto rows = Ttml("<p begin=\"0s\" end=\"2s\"><span begin=\"1s\" end=\"4s\">A</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("{\\k100}{\\kf100}A", rows[0].text);
}

TEST(lyrics_import, ttml_rounds_absolute_boundaries_without_drift) {
	auto rows = Ttml("<p begin=\"0s\" end=\"24ms\"><span begin=\"0ms\" end=\"6ms\">A</span><span begin=\"6ms\" end=\"12ms\">B</span><span begin=\"12ms\" end=\"18ms\">C</span><span begin=\"18ms\" end=\"24ms\">D</span></p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("{\\kf1}A{\\kf0}B{\\kf1}C{\\kf0}D", rows[0].text);
}

TEST(lyrics_import, ttml_clock_and_metric_times) {
	for (auto begin : {"00:00:01.5", "00:01,5", "1.5", "1500ms", "1.5s", "0.025m"}) {
		auto rows = Ttml(std::string("<p begin=\"") + begin + "\" end=\"3s\">Text</p>");
		ASSERT_EQ(1u, rows.size());
		EXPECT_EQ(1500, rows[0].start_ms) << begin;
	}
}

TEST(lyrics_import, ttml_rejects_nonfinite_overflow_and_unsupported_times) {
	for (auto begin : {"nanms", "1e309s", "999999999999999999999s", "00:00:01:12", "12f", "12t", "-1s"})
		EXPECT_THROW(Ttml(std::string("<p begin=\"") + begin + "\" end=\"3s\">Text</p>"), std::invalid_argument) << begin;
}

TEST(lyrics_import, lrc_enhanced_multiple_timestamps_repeat_relative_word_times) {
	auto rows = lyrics::ParseLrc({"[00:10][00:20]<00:11>A<00:12>"});
	ASSERT_EQ(2u, rows.size());
	EXPECT_EQ(12000, rows[0].end_ms);
	EXPECT_EQ(22000, rows[1].end_ms);
	EXPECT_EQ("{\\k100}{\\kf100}A", rows[0].text);
	EXPECT_EQ(rows[0].text, rows[1].text);
}

TEST(lyrics_import, ttml_xml_indentation_does_not_create_raw_ass_newlines) {
	auto rows = Ttml("<p>\n  <span>Hello</span>\n  <span>world</span>\n</p>");
	ASSERT_EQ(1u, rows.size());
	EXPECT_EQ("Hello world", rows[0].text);
}
