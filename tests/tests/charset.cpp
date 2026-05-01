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

// FIXME: this shouldn't be needed!
// #include "../../acconf.h"
// needed to make CI pass for now
#define WITH_UCHARDET

#include <libaegisub/charset.h>

#include <optional>
#include <string_view>

#include <main.h>
#include <util.h>

using agi::charset::DetectReason;

namespace {
void test_charset_detection(const agi::fs::path &file, std::optional<std::string_view> expected_encoding, DetectReason expected_reason) {
	SCOPED_TRACE("Testing file: " + file.string());
#ifndef WITH_UCHARDET
	if (expected_reason == DetectReason::Uchardet || expected_reason == DetectReason::ValidUtf8) {
		expected_reason = DetectReason::Utf8Fallback;
		expected_encoding = "utf-8";
	}
#endif
	DetectReason reason;
	auto detected_encoding = agi::charset::Detect(file, &reason);
	EXPECT_EQ(expected_reason, reason);
	if (expected_encoding)
		EXPECT_EQ(expected_encoding, detected_encoding);
}
}

TEST(lagi_charset, detect) {
	auto dir = util::test_data_dir() / "charset";
	test_charset_detection(dir / "utf-8.txt", "utf-8", DetectReason::ValidUtf8);
	test_charset_detection(dir / "ascii.txt", "utf-8", DetectReason::ValidUtf8);
	test_charset_detection(dir / "windows-1250.txt", "WINDOWS-1250", DetectReason::Uchardet);
	test_charset_detection(dir / "utf-8-bom.txt", "utf-8", DetectReason::Signature);
	test_charset_detection(dir / "binary.png", "binary", DetectReason::BinaryHeuristic);
	test_charset_detection(dir / "utf-8-truncated.txt", std::nullopt, DetectReason::Uchardet);
}

