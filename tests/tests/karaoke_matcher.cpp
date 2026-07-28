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

#include <libaegisub/karaoke_matcher.h>

#include <main.h>
#include <util.h>

class lagi_auto_match_karaoke : public libagi { };

namespace {
void print(std::ostream& os, int depth, agi::ass::KaraokeSyllable const& syl);
void print(std::ostream& os, int depth, agi::KaraokeMatcher::MatchGroup const& grp);

template<typename T>
void print(std::ostream& os, int depth, std::span<T> span, const char *name) {
	std::string indent(depth, '\t');
	os << "span<" << name << ">{\n";
	for (auto&& element : span) {
		print(os, depth + 1, element);
		os << ",\n";
	}
	os << indent << "}";
}

void print(std::ostream& os, int depth, agi::ass::KaraokeSyllable const& syl) {
	std::string indent(depth, '\t');
	os << indent << "KaraokeSyllable{\n"
	   << indent << "\t.start_time = " << syl.start_time << ",\n"
	   << indent << "\t.duration   = " << syl.duration << ",\n"
	   << indent << "\t.text       = " << syl.text << ",\n"
	   << indent << "\t.tag_type   = " << syl.tag_type << ",\n"
	   << indent << "}";
}

void print(std::ostream& os, int depth, agi::KaraokeMatcher::MatchGroup const& grp) {
	std::string indent(depth, '\t');
	os << indent << "MatchGroup{\n"
	   << indent << "\t.src = ";
	print(os, depth + 1, grp.src, "KaraokeSyllable");
	os << ",\n"
	   << indent << "\t.dst = " << grp.dst << ",\n"
	   << indent << "}";
}
}

namespace std {
template<typename T>
bool operator==(span<T> const& a, span<T> const& b) {
	if (a.size() != b.size())
		return false;
	for (size_t i = 0; i < a.size(); ++i) {
		if (a[i] != b[i])
			return false;
	}
	return true;
}

::std::ostream& operator<<(::std::ostream& os, span<const agi::ass::KaraokeSyllable> const& s) {
	print(os, 0, s, "KaraokeSyllable");
	return os;
}

::std::ostream& operator<<(::std::ostream& os, span<const agi::KaraokeMatcher::MatchGroup> const& s) {
	print(os, 0, s, "MatchGroup");
	return os;
}
} // namespace std

namespace agi {
::std::ostream& operator<<(::std::ostream& os, KaraokeMatchResult const& r) {
	return os << "KaraokeMatchResult{" << r.source_length << ", " << r.destination_length << "}";
}

bool operator==(KaraokeMatcher::MatchGroup const& a, KaraokeMatcher::MatchGroup const& b) {
	return a.src == b.src && a.dst == b.dst;
}
} // namespace agi


using agi::AutoMatchKaraoke;
using agi::KaraokeMatchResult;
using agi::ass::KaraokeSyllable;

TEST(lagi_auto_match_karaoke, empty_src_gives_zero_src_length) {
	EXPECT_EQ(0, AutoMatchKaraoke({}, "").source_length);
	EXPECT_EQ(0, AutoMatchKaraoke({}, "a").source_length);
}

TEST(lagi_auto_match_karaoke, empty_dest_gives_zero_dest_length) {
	EXPECT_EQ(0, AutoMatchKaraoke({}, "").destination_length);
}

TEST(lagi_auto_match_karaoke, empty_dest_with_source_selects_all_source) {
	EXPECT_EQ(2, AutoMatchKaraoke({"a", "b"}, "").source_length);
}

TEST(lagi_auto_match_karaoke, empty_but_present_src_syllable_matches_no_dest) {
	EXPECT_EQ((KaraokeMatchResult{1, 0}),
	          AutoMatchKaraoke({"", "b"}, "cc"));
}

TEST(lagi_auto_match_karaoke, dest_with_non_match_selects_first_character) {
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"a", "b"}, "cc"));
}

TEST(lagi_auto_match_karaoke, dest_with_identical_match_selects_match) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"abc", "de"}, "abcde"));
}

TEST(lagi_auto_match_karaoke, match_is_case_insensitive) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"abc", "de"}, "ABCDE"));
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"ABC", "DE"}, "abcde"));
}

TEST(lagi_auto_match_karaoke, leading_whitespace_in_source_is_ignored) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({" abc", "de"}, "abcde"));
}

TEST(lagi_auto_match_karaoke, trailing_whitespace_in_source_is_ignored) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"abc ", "de"}, "abcde"));
}

TEST(lagi_auto_match_karaoke, whitespace_in_dest_is_consumed) {
	EXPECT_EQ((KaraokeMatchResult{1, 4}),
	          AutoMatchKaraoke({"abc ", "de"}, " abcde"));
	EXPECT_EQ((KaraokeMatchResult{1, 4}),
	          AutoMatchKaraoke({"abc ", "de"}, "abc de"));
	EXPECT_EQ((KaraokeMatchResult{1, 5}),
	          AutoMatchKaraoke({"abc ", "de"}, "ab c de"));
}

TEST(lagi_auto_match_karaoke, dest_match_is_in_characters) {
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"∫∫", "de"}, "∫∫a"));
}

TEST(lagi_auto_match_karaoke, decomposed_characters_are_handled_atomically) {
	// YODO
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"∫∫", "de"}, "∫∫a"));
}

TEST(lagi_auto_match_karaoke, single_hiragana_is_matched) {
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"ro" "de"}, "ろ"));
}

TEST(lagi_auto_match_karaoke, single_katakana_is_matched) {
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"ro" "de"}, "ロ"));
}

TEST(lagi_auto_match_karaoke, multiple_characters_matched) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"romaji" "de"}, "ろまじ"));
}
TEST(lagi_auto_match_karaoke, multiple_character_kana) {
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"kya", "e"}, "きゃe"));
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"kya"}, "きゃ"));
}

TEST(lagi_auto_match_karaoke, whitespace_between_characters_in_source_ignored) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"ro ma ji" "de"}, "ろまじ"));
}

TEST(lagi_auto_match_karaoke, whitespace_inside_characters_in_source_breaks_match) {
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"r om aj i" "de"}, "ろまじ"));
}

TEST(lagi_auto_match_karaoke, single_dest_character_consumes_all_source) {
	EXPECT_EQ((KaraokeMatchResult{3, 1}),
	          AutoMatchKaraoke({"a", "b", "c"}, "ろ"));
}

TEST(lagi_auto_match_karaoke, fullwidth_letters_are_matched_to_ascii) {
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"ab", "cd"}, "ａｂc"));
}

TEST(lagi_auto_match_karaoke, simple_lookahead) {
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"ab", "ro"}, "eeろ"));
}

TEST(lagi_auto_match_karaoke, lookahead_ignores_empty_syllables) {
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"ab", "", "ro"}, "eeろ"));
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"ab", "", "", "", "", "", "ro"}, "eeろ"));
}

TEST(lagi_auto_match_karaoke, lookahead_only_looks_at_three_characters_of_dst) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"abc", "", "ro"}, "eeeろ"));
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"abcd", "", "ro"}, "eeeeろ"));
}

TEST(lagi_auto_match_karaoke, lookahead_two_syllables) {
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"a", "b", "ro"}, "eeろ"));
	EXPECT_EQ((KaraokeMatchResult{2, 1}),
	          AutoMatchKaraoke({"a", "b", "c", "ro"}, "eeろ"));
	EXPECT_EQ((KaraokeMatchResult{2, 1}),
	          AutoMatchKaraoke({"a", "b", "c", "d", "ro"}, "eeろ"));
	EXPECT_EQ((KaraokeMatchResult{3, 1}),
	          AutoMatchKaraoke({"a", "b", "c", "d", "f", "ro"}, "eeろ"));
	EXPECT_EQ((KaraokeMatchResult{3, 2}),
	          AutoMatchKaraoke({"a", "b", "c", "d", "f", "ro"}, " eeろ"));
}

TEST(lagi_auto_match_karaoke, lookahead_multicharacter_kana) {
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"aa", "kya"}, "eeきゃ"));
	EXPECT_EQ((KaraokeMatchResult{1, 2}),
	          AutoMatchKaraoke({"aa", "kyan"}, "eeきゃ"));
}

TEST(lagi_auto_match_karaoke, ha_is_wa) {
	EXPECT_EQ((KaraokeMatchResult{2, 1}),
	          AutoMatchKaraoke({"Bo", "ku", "wa"}, "僕は"));
}

TEST(lagi_auto_match_karaoke, he_is_e) {
	EXPECT_EQ((KaraokeMatchResult{2, 1}),
	          AutoMatchKaraoke({"Bo", "ku", "e"}, "僕へ"));
}

TEST(lagi_auto_match_karaoke, shitta) {
	EXPECT_EQ((KaraokeMatchResult{1, 1}),
	          AutoMatchKaraoke({"shi", "tta", ""}, "知った"));
	EXPECT_EQ((KaraokeMatchResult{2, 2}),
	          AutoMatchKaraoke({"tta", ""}, "った"));
}

TEST(lagi_auto_match_karaoke, lookahead_is_case_insensitive) {
	EXPECT_EQ((KaraokeMatchResult{1, 3}),
	          AutoMatchKaraoke({"Oh... ", "Nan", "ka ", "ta", "ri", "nai"}, "Oh…なんか足りない"));
}

class lagi_karaoke_matcher : public libagi {
protected:
	agi::KaraokeMatcher matcher;
	std::vector<KaraokeSyllable> source;
	std::span<const KaraokeSyllable> source_span;
	std::string_view dest;

	void init(const char *dst, const char *tag, int start, std::initializer_list<std::pair<int, const char *>> syls) {
		source.clear();
		for (auto syl : syls) {
			source.push_back(KaraokeSyllable{
				.start_time = start,
				.duration = syl.first,
				.text = syl.second,
				.tag_type = tag,
				.ovr_tags = {},
			});
			start += syl.first;
		}
		source_span = source;
		dest = dst;
		matcher.SetInputData(std::vector(source), dst);
	}
};

std::vector<KaraokeSyllable> make_syllables(const char *tag, int start, std::initializer_list<std::pair<int, const char *>> syls) {
	std::vector<KaraokeSyllable> ret;
	for (auto syl : syls) {
		ret.push_back(KaraokeSyllable{
			.start_time = start,
			.duration = syl.first,
			.text = syl.second,
			.tag_type = tag,
			.ovr_tags = {}
		});
		start += syl.first;
	}
	return ret;
}

#define EXPECT_STATE(src_start, src_len, dst_start, dst_len) do { \
	EXPECT_EQ(matcher.CurrentSourceSelection(), source_span.subspan(src_start, src_len)); \
	EXPECT_EQ(matcher.CurrentDestinationSelection(), dest.substr(dst_start, dst_len)); \
	EXPECT_EQ(matcher.UnmatchedSource(), source_span.subspan(src_start + src_len)); \
	EXPECT_EQ(matcher.UnmatchedDestination(), dest.substr(dst_start + dst_len)); \
} while (0)

TEST_F(lagi_karaoke_matcher, empty_source_and_dest) {
	init("", "\\k", 0, {});
	EXPECT_EQ(matcher.GetOutputLine(), "");
	EXPECT_TRUE(matcher.MatchedGroups().empty());
	EXPECT_STATE(0, 0, 0, 0);
	EXPECT_FALSE(matcher.IncreaseSourceMatch());
	EXPECT_FALSE(matcher.DecreaseSourceMatch());
	EXPECT_FALSE(matcher.IncreaseDestinationMatch());
	EXPECT_FALSE(matcher.DecreaseDestinationMatch());
	EXPECT_FALSE(matcher.AcceptMatch());
	EXPECT_FALSE(matcher.UndoMatch());
}

TEST_F(lagi_karaoke_matcher, source_match_walking) {
	init("abcd", "\\k", 100, {{100, "a"}, {200, "b"}, {300, "c"}, {400, "d"}});
	EXPECT_EQ(matcher.GetOutputLine(), "");
	EXPECT_TRUE(matcher.MatchedGroups().empty());
	EXPECT_STATE(0, 1, 0, 1);

	EXPECT_TRUE(matcher.DecreaseSourceMatch());
	EXPECT_STATE(0, 0, 0, 1);

	EXPECT_FALSE(matcher.DecreaseSourceMatch());
	EXPECT_STATE(0, 0, 0, 1);

	EXPECT_TRUE(matcher.IncreaseSourceMatch());
	EXPECT_STATE(0, 1, 0, 1);
	EXPECT_TRUE(matcher.IncreaseSourceMatch());
	EXPECT_STATE(0, 2, 0, 1);
	EXPECT_TRUE(matcher.IncreaseSourceMatch());
	EXPECT_STATE(0, 3, 0, 1);
	EXPECT_TRUE(matcher.IncreaseSourceMatch());
	EXPECT_STATE(0, 4, 0, 1);
	EXPECT_FALSE(matcher.IncreaseSourceMatch());
	EXPECT_STATE(0, 4, 0, 1);
}

TEST_F(lagi_karaoke_matcher, dest_match_walking) {
	init("abcd", "\\k", 100, {{100, "a"}, {200, "b"}, {300, "c"}, {400, "d"}});
	EXPECT_EQ(matcher.GetOutputLine(), "");
	EXPECT_TRUE(matcher.MatchedGroups().empty());

	EXPECT_STATE(0, 1, 0, 1);

	EXPECT_TRUE(matcher.DecreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 0);

	EXPECT_FALSE(matcher.DecreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 0);

	EXPECT_TRUE(matcher.IncreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 1);
	EXPECT_TRUE(matcher.IncreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 2);
	EXPECT_TRUE(matcher.IncreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 3);
	EXPECT_TRUE(matcher.IncreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 4);
	EXPECT_FALSE(matcher.IncreaseDestinationMatch());
	EXPECT_STATE(0, 1, 0, 4);
}

TEST_F(lagi_karaoke_matcher, simple_grouping) {
	init("abcd", "\\k", 100, {{100, "1"}, {200, "2"}, {300, "3"}, {400, "4"}});
	EXPECT_EQ(matcher.GetOutputLine(), "");
	EXPECT_TRUE(matcher.MatchedGroups().empty());

	EXPECT_TRUE(matcher.AcceptMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a");

	EXPECT_TRUE(matcher.AcceptMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a{\\k20}b");

	EXPECT_TRUE(matcher.AcceptMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a{\\k20}b{\\k30}c");

	EXPECT_TRUE(matcher.AcceptMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a{\\k20}b{\\k30}c{\\k40}d");

	EXPECT_FALSE(matcher.AcceptMatch());

	EXPECT_TRUE(matcher.UndoMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a{\\k20}b{\\k30}c");

	EXPECT_TRUE(matcher.UndoMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a{\\k20}b");

	EXPECT_TRUE(matcher.UndoMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "{\\k10}a");

	EXPECT_TRUE(matcher.UndoMatch());
	EXPECT_EQ(matcher.GetOutputLine(), "");

	EXPECT_FALSE(matcher.UndoMatch());
}
