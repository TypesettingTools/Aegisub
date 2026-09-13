// Copyright (c) 2026, 伤感咩吖
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/


/// @file subtitle_format_lrc.cpp
/// @brief Reading LRC lyrics, including the enhanced (syllable-level, "A2")
/// variant with inline <mm:ss.xx> word timestamps. Word timestamps map to
/// ASS \kf sweeping karaoke durations so the import is karaoke-ready, and a
/// trailing word timestamp (as exported by Apple Music) marks the line end.
/// @ingroup subtitle_io

#include "subtitle_format_lrc.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "options.h"
#include "text_file_reader.h"

#include <libaegisub/ass/time.h>
#include <libaegisub/log.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <boost/algorithm/string/trim.hpp>

namespace {
struct LrcLine {
	int64_t start_ms = 0;                    // line start time
	int64_t end_marker_ms = -1;              // explicit line end from a trailing word timestamp
	std::string text;                        // plain text (no syllable tags)
	std::vector<std::pair<int64_t, std::string>> syllables; // <time, text> when enhanced
	bool has_syllables = false;
};

// Parse "mm:ss", "mm:ss.xx" or "mm:ss.xxx" (leading [ already consumed).
// Returns milliseconds, or -1 if the tag is not a timestamp.
int64_t ParseLrcTimestamp(std::string const& body) {
	size_t colon = body.find(':');
	if (colon == std::string::npos) return -1;

	int64_t minutes = 0;
	if (std::from_chars(body.data(), body.data() + colon, minutes).ec != std::errc{})
		return -1;

	size_t dot = body.find('.', colon);
	std::string_view sec_str(body.data() + colon + 1, dot == std::string::npos ? std::string::npos : dot - colon - 1);
	int64_t seconds = 0;
	if (std::from_chars(sec_str.data(), sec_str.data() + sec_str.size(), seconds).ec != std::errc{})
		return -1;

	int64_t frac_ms = 0;
	if (dot != std::string::npos) {
		std::string_view frac_str(body.data() + dot + 1, body.size() - dot - 1);
		if (frac_str.empty() || frac_str.size() > 3) return -1;
		int64_t frac = 0;
		if (std::from_chars(frac_str.data(), frac_str.data() + frac_str.size(), frac).ec != std::errc{})
			return -1;
		// 1 digit = tenths, 2 = centiseconds, 3 = milliseconds
		frac_ms = frac_str.size() == 1 ? frac * 100 : frac_str.size() == 2 ? frac * 10 : frac;
	}

	return (minutes * 60 + seconds) * 1000 + frac_ms;
}

// Parse one bracketed tag body; returns true when it is a timestamp whose
// value is written to ms_out, false for metadata/other tags.
bool TryParseTag(std::string const& body, int64_t& ms_out) {
	int64_t ms = ParseLrcTimestamp(body);
	if (ms < 0) return false;
	ms_out = ms;
	return true;
}
}

LrcSubtitleFormat::LrcSubtitleFormat()
: SubtitleFormat("LRC Lyrics")
{
}

std::vector<std::string> LrcSubtitleFormat::GetReadWildcards() const {
	return {"lrc"};
}

void LrcSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename, agi::vfr::Framerate const&, const char *encoding) const {
	TextFileReader file(filename, encoding, false);
	target->LoadDefault(false, OPT_GET("Subtitle Format/LRC/Default Style Catalog")->GetString());

	int64_t offset_ms = 0;
	std::vector<LrcLine> lines;
	std::vector<std::string> untimed_lines;

	while (file.HasMoreLines()) {
		std::string line = file.ReadLineFromFile();
		boost::trim(line);
		if (line.empty()) continue;
		if (line[0] != '[') {
			// Some sources export plain lyrics text under an .lrc extension
			// with no timestamps at all; keep those lines as a fallback.
			untimed_lines.push_back(std::move(line));
			continue;
		}

		// Peel leading [tags] off the line.
		std::vector<int64_t> timestamps;
		std::vector<std::pair<int64_t, std::string>> syllables;
		bool has_syllables = false;
		int64_t end_marker_ms = -1;

		size_t pos = 0;
		while (pos < line.size() && line[pos] == '[') {
			size_t close = line.find(']', pos);
			if (close == std::string::npos) break;
			std::string body = line.substr(pos + 1, close - pos - 1);
			boost::trim(body);

			int64_t ms = 0;
			if (TryParseTag(body, ms)) {
				timestamps.push_back(ms);
			}
			else if (body.starts_with("offset:")) {
				int64_t off = 0;
				auto val = std::string_view(body).substr(7);
				// string_view has no erase(); strip spaces by moving the ends.
				while (!val.empty() && (val.front() == ' ' || val.front() == '\t'))
					val.remove_prefix(1);
				while (!val.empty() && (val.back() == ' ' || val.back() == '\t'))
					val.remove_suffix(1);
				if (!val.empty())
					if (std::from_chars(val.data(), val.data() + val.size(), off).ec == std::errc{})
						offset_ms = off; // positive shifts lyrics earlier per spec
			}
			// other metadata tags (ti/ar/al/by/re/ve/...) are ignored

			pos = close + 1;
		}

		std::string text = line.substr(pos);
		if (timestamps.empty()) continue;
		boost::trim(text);

		// Enhanced LRC: split <mm:ss.xx> syllable markers inside the text.
		LrcLine entry;
		size_t scan = 0;
		int64_t last_syllable_ms = timestamps.front() - offset_ms;
		std::string pending_text;
		while (true) {
			size_t lt = text.find('<', scan);
			size_t gt = text.find('>', lt == std::string::npos ? std::string::npos : lt + 1);
			if (lt == std::string::npos || gt == std::string::npos) {
				pending_text += text.substr(scan);
				break;
			}
			std::string tag_body = text.substr(lt + 1, gt - lt - 1);
			int64_t ms = 0;
			// Enhanced-LRC syllable tags use the same timestamp syntax.
			if (!TryParseTag(tag_body, ms)) {
				// Not a syllable timestamp; keep it verbatim (could be an
				// ASS-injected tag someone left in) and move past this '<'.
				pending_text += text.substr(scan, gt - scan + 1);
				scan = gt + 1;
				continue;
			}

			pending_text += text.substr(scan, lt - scan);
			if (!pending_text.empty()) {
				syllables.emplace_back(std::max<int64_t>(last_syllable_ms, 0), pending_text);
				pending_text.clear();
			}
			last_syllable_ms = ms - offset_ms;
			has_syllables = true;
			scan = gt + 1;
		}
		boost::trim(pending_text);
		if (has_syllables && !pending_text.empty())
			syllables.emplace_back(std::max<int64_t>(last_syllable_ms, 0), pending_text);
		else if (has_syllables)
			// A word timestamp with no text after it (the trailing
			// "<mm:ss.xx>" Apple Music exports) marks where the line ends,
			// so the final word must not stretch to the next line's start.
			end_marker_ms = last_syllable_ms;

		for (auto ts : timestamps) {
			LrcLine out;
			out.start_ms = ts - offset_ms;
			out.end_marker_ms = end_marker_ms;
			out.has_syllables = has_syllables;
			if (has_syllables) {
				out.syllables = syllables;
			}
			else {
				out.text = text;
				out.text.erase(std::remove(out.text.begin(), out.text.end(), '\r'), out.text.end());
			}
			if (out.start_ms < 0) out.start_ms = 0;
			// Timestamp-only lines (e.g. "[00:23.05]" with no text after it,
			// common in Apple Music exports as section spacers) and lines
			// whose word markers carry no text would become empty dialogue
			// rows; skip them.
			bool empty_line = (!out.has_syllables || out.syllables.empty())
				&& out.text.find_first_not_of(" \t") == std::string::npos;
			if (!empty_line)
				lines.push_back(std::move(out));
		}
	}

	if (lines.empty() && !untimed_lines.empty()) {
		// Untimed LRC: import like the plain-text reader does, one untimed
		// row per lyric line, for the user to time.
		for (std::string& text : untimed_lines) {
			auto diag = new AssDialogue;
			diag->Text = std::move(text);
			target->Events.push_back(*diag);
		}
		return;
	}

	if (lines.empty())
		throw SubtitleFormatParseError("No timed lyrics lines found in LRC file.");

	// Sort by start time; LRC files are usually ordered but multi-timestamp
	// expansion can interleave.
	std::stable_sort(lines.begin(), lines.end(),
		[](LrcLine const& a, LrcLine const& b) { return a.start_ms < b.start_ms; });

	// Build dialogues; each line ends where the next begins (clamped to at
	// least 500 ms so zero-length gaps never produce empty renders), and the
	// final line gets a nominal 5 s display time. An explicit trailing word
	// timestamp overrides the synthesized end so held final notes keep their
	// real duration, but echo/harmony layers in Apple Music exports can start
	// before the previous line's marked end — rows must never overlap.
	auto karaoke_cs = [](int64_t ms) { return static_cast<int>((ms + 5) / 10); };

	for (size_t i = 0; i < lines.size(); ++i) {
		auto const& cur = lines[i];
		int64_t end_ms = i + 1 < lines.size() ? lines[i + 1].start_ms : cur.start_ms + 5000;
		if (cur.end_marker_ms >= 0)
			end_ms = cur.end_marker_ms;
		if (i + 1 < lines.size() && end_ms > lines[i + 1].start_ms)
			end_ms = lines[i + 1].start_ms;
		if (end_ms < cur.start_ms + 500) end_ms = cur.start_ms + 500;

		// The events list uses an auto-unlink intrusive hook and owns its
		// nodes via delete-on-dispose, so entries must be raw new'd like
		// every other reader does; a smart pointer would unlink and free
		// each row the moment it goes out of scope.
		auto diag = new AssDialogue;
		diag->Start = agi::Time(cur.start_ms);
		diag->End = agi::Time(end_ms);

		if (cur.has_syllables && !cur.syllables.empty()) {
			// \kf takes centiseconds of *duration per segment* and sweeps the
			// highlight across each word for that duration (the Apple Music
			// word-fill look); the final segment runs to the line end.
			std::string text;
			for (size_t s = 0; s < cur.syllables.size(); ++s) {
				auto const& syl = cur.syllables[s];
				int64_t seg_end = s + 1 < cur.syllables.size() ? cur.syllables[s + 1].first : end_ms;
				int64_t dur = std::max<int64_t>(seg_end - syl.first, 0);
				text += "{\\kf" + std::to_string(karaoke_cs(dur)) + "}" + syl.second;
			}
			diag->Text = text;
		}
		else {
			diag->Text = cur.text;
		}

		target->Events.push_back(*diag);
	}
}
