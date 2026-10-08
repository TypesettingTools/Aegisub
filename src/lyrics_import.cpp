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

#include "lyrics_import.h"

#include <wx/xml/xml.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {
void Trim(std::string& text) {
	auto first = text.find_first_not_of(" \t\r\n");
	if (first == text.npos) { text.clear(); return; }
	text = text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

// Bound times to the range representable by ASS, before any arithmetic.
constexpr int64_t MaxTime = 10 * 60 * 60 * 1000 - 10;

bool ParseInteger(std::string_view value, int64_t& out) {
	if (value.empty() || value.front() < '0' || value.front() > '9') return false;
	auto result = std::from_chars(value.data(), value.data() + value.size(), out);
	return result.ec == std::errc{} && result.ptr == value.data() + value.size();
}

// Fixed-point decimal conversion is independent of the process locale and
// cannot admit NaN, infinity, exponent notation or overflowing timestamps.
int64_t DecimalTime(std::string_view value, int64_t unit) {
	auto dot = value.find_first_of(".,");
	int64_t whole = 0;
	if (!ParseInteger(value.substr(0, dot), whole) || whole > MaxTime / unit) return -1;
	int64_t fraction = 0, denominator = 1;
	if (dot != value.npos) {
		auto digits = value.substr(dot + 1);
		if (digits.empty()) return -1;
		for (size_t i = 0; i < digits.size(); ++i) {
			if (digits[i] < '0' || digits[i] > '9') return -1;
			if (i < 9) { fraction = fraction * 10 + digits[i] - '0'; denominator *= 10; }
		}
	}
	auto ms = whole * unit + (fraction * unit + denominator / 2) / denominator;
	return ms <= MaxTime ? ms : -1;
}

int64_t ParseLrcTimestamp(std::string_view value) {
	auto colon = value.find(':');
	if (colon == value.npos) return -1;
	int64_t minutes = 0;
	if (!ParseInteger(value.substr(0, colon), minutes) || minutes > MaxTime / 60000) return -1;
	auto seconds = value.substr(colon + 1);
	auto dot = seconds.find('.');
	if (seconds.find(',') != seconds.npos || (dot != seconds.npos && seconds.size() - dot - 1 > 3)) return -1;
	auto ms = DecimalTime(seconds, 1000);
	if (ms < 0 || ms >= 60000 || minutes * 60000 > MaxTime - ms) return -1;
	return minutes * 60000 + ms;
}

bool TryParseTag(std::string const& body, int64_t& out) {
	out = ParseLrcTimestamp(body);
	return out >= 0;
}

// Rounded absolute boundaries keep cumulative centisecond timing aligned;
// rounding each duration separately would drift across many short words.
int64_t Centiseconds(int64_t ms) { return (ms + 5) / 10; }

struct Segment {
	int64_t begin_ms = 0;
	int64_t end_ms = -1;
	std::string text;
	bool bg = false;
};

std::string Karaoke(std::vector<Segment> const& segments, int64_t line_start, int64_t line_end) {
	std::string text;
	int64_t cursor = Centiseconds(line_start);
	auto limit = Centiseconds(line_end);
	for (size_t i = 0; i < segments.size(); ++i) {
		auto const& segment = segments[i];
		auto begin = std::clamp(Centiseconds(segment.begin_ms), Centiseconds(line_start), limit);
		int64_t end = segment.end_ms;
		if (end < 0) end = i + 1 < segments.size() ? segments[i + 1].begin_ms : line_end;
		end = std::clamp(Centiseconds(end), begin, limit);
		if (begin > cursor) {
			text += "{\\k" + std::to_string(begin - cursor) + "}";
			cursor = begin;
		}
		// A single ASS karaoke chain cannot represent overlaps in one voice.
		// Clip overlapping portions to its cursor; x-bg voices are split earlier.
		end = std::max(end, cursor);
		text += "{\\kf" + std::to_string(end - cursor) + "}" + segment.text;
		cursor = end;
	}
	return text;
}
struct LrcLine {
	int64_t start_ms = 0;                    // line start time
	int64_t end_marker_ms = -1;              // explicit line end from a trailing word timestamp
	std::string text;                        // plain text (no syllable tags)
	std::vector<std::pair<int64_t, std::string>> syllables; // <time, text> when enhanced
	bool has_syllables = false;
};

// UTF-8 conversion matches compat.cpp's from_wx; this parser can be linked
// into the test runner without pulling in the application's options/UI.
std::string Utf8(wxString const& value) { return std::string(value.utf8_str()); }

int64_t ParseTTMLTime(std::string_view value) {
	while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.remove_prefix(1);
	while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.remove_suffix(1);
	if (value.empty()) return -1;
	for (auto const& metric : {std::pair<std::string_view, int64_t>{"ms", 1}, {"s", 1000}, {"m", 60000}, {"h", 3600000}})
		if (value.ends_with(metric.first)) return DecimalTime(value.substr(0, value.size() - metric.first.size()), metric.second);
	auto last = value.rfind(':');
	if (last == value.npos) return DecimalTime(value, 1000);
	auto seconds = DecimalTime(value.substr(last + 1), 1000);
	if (seconds < 0 || seconds >= 60000) return -1;
	auto prefix = value.substr(0, last);
	auto colon = prefix.find(':');
	int64_t hours = 0, minutes = 0;
	if (colon == prefix.npos) {
		if (!ParseInteger(prefix, minutes)) return -1;
	}
	else {
		if (!ParseInteger(prefix.substr(0, colon), hours) || !ParseInteger(prefix.substr(colon + 1), minutes) || minutes >= 60) return -1;
	}
	if (hours > MaxTime / 3600000 || minutes > MaxTime / 60000) return -1;
	auto ms = hours * 3600000 + minutes * 60000 + seconds;
	return ms <= MaxTime ? ms : -1;
}
// Local name of an XML node with any namespace prefix stripped.
std::string LocalName(wxXmlNode const* node) {
	auto name = Utf8(node->GetName());
	auto colon = name.find(':');
	return colon == name.npos ? name : name.substr(colon + 1);
}

bool IsElement(wxXmlNode const* node, std::string_view local) {
	return LocalName(node) == local;
}

struct TtmlParagraph {
	int64_t begin_ms = 0;
	int64_t end_ms = 0;
	std::string karaoke_text; // ASS text with \k segments when timed spans exist
	bool has_karaoke = false;
	std::vector<Segment> segments;
};

// Determine (begin, end) for a paragraph from begin/end/dur attributes.
bool ParseParagraphTimes(wxXmlNode const* p, int64_t& begin_ms, int64_t& end_ms) {
	std::string begin_str, end_str, dur_str;
	for (auto attr = p->GetAttributes(); attr; attr = attr->GetNext()) {
		std::string name = Utf8(attr->GetName());
		auto pos = name.find(':');
		if (pos != std::string::npos) name = name.substr(pos + 1);
		if (name == "begin") begin_str = Utf8(attr->GetValue());
		else if (name == "end") end_str = Utf8(attr->GetValue());
		else if (name == "dur") dur_str = Utf8(attr->GetValue());
	}

	if (begin_str.empty()) return false;
	begin_ms = ParseTTMLTime(begin_str);
	if (begin_ms < 0) throw std::invalid_argument("Invalid or unsupported TTML begin time.");

	if (!end_str.empty()) {
		end_ms = ParseTTMLTime(end_str);
		if (end_ms < 0) throw std::invalid_argument("Invalid or unsupported TTML end time.");
	}
	else if (!dur_str.empty()) {
		int64_t dur = ParseTTMLTime(dur_str);
		if (dur < 0 || dur > MaxTime - begin_ms) throw std::invalid_argument("Invalid or unsupported TTML duration.");
		end_ms = begin_ms + dur;
	}
	else {
		end_ms = begin_ms;
	}

	if (end_ms < begin_ms) end_ms = begin_ms;
	return true;
}

// Walk a paragraph's children, accumulating plain text and <span>-scoped
// karaoke segments. Runs of text between spans attach to the most recent
// span (or the paragraph preamble when no span has been seen yet).
// Collect all visible text under a node in document order, turning <br/>
// into \N and flattening nested elements (spans inside spans).
std::string VisibleText(wxXmlNode const* node) {
	std::string out;
	for (char c : Utf8(node->GetContent())) {
		if (c == '\r' || c == '\n' || c == '\t') c = ' ';
		if (c != ' ' || out.empty() || out.back() != ' ') out += c;
	}
	return out;
}

std::string CollectVisibleText(wxXmlNode const* node) {
	std::string out;
	for (auto child = node->GetChildren(); child; child = child->GetNext()) {
		switch (child->GetType()) {
			case wxXML_TEXT_NODE:
			case wxXML_CDATA_SECTION_NODE:
				out += VisibleText(child);
				break;
			case wxXML_ELEMENT_NODE:
				if (IsElement(child, "br"))
					out += "\\N";
				else
					out += CollectVisibleText(child);
				break;
			default:
				break;
		}
	}
	return out;
}

void BuildParagraphText(wxXmlNode *p,
	std::string& plain, bool& has_karaoke,
	std::vector<Segment>& segments, bool bg_voice = false) {
	for (auto node = p->GetChildren(); node; node = node->GetNext()) {
		switch (node->GetType()) {
			case wxXML_TEXT_NODE:
			case wxXML_CDATA_SECTION_NODE: {
				std::string text = VisibleText(node);
				if (has_karaoke && !segments.empty())
					segments.back().text += text;
				else
					plain += text;
				break;
			}
			case wxXML_ELEMENT_NODE: {
				if (IsElement(node, "br")) {
					if (has_karaoke && !segments.empty())
						segments.back().text += "\\N";
					else
						plain += "\\N";
					break;
				}
				if (IsElement(node, "span")) {
					std::string begin_attr, end_attr;
					bool role_bg = false;
					for (auto attr = node->GetAttributes(); attr; attr = attr->GetNext()) {
						std::string name = Utf8(attr->GetName());
						auto pos = name.find(':');
						if (pos != std::string::npos) name = name.substr(pos + 1);
						if (name == "begin") begin_attr = Utf8(attr->GetValue());
						else if (name == "end") end_attr = Utf8(attr->GetValue());
						else if (name == "role") {
							// Apple Music marks background vocals with
							// ttm:role="x-bg"; they sing alongside the lead.
							std::string role = Utf8(attr->GetValue());
							if (role.rfind("x-bg", 0) == 0) role_bg = true;
						}
					}
					bool voice_bg = bg_voice || role_bg;

					// Collect this span's own visible text. Nested spans are
					// flattened in document order — an untimed wrapper span
					// (e.g. ttm:role="x-bg" background vocals wrapping timed
					// word spans in Apple Music TTML) must not swallow its
					// children, and a timed span containing nested spans
					// (rare) uses its own begin for the whole run.
					std::string span_text = CollectVisibleText(node);
					int64_t span_begin = ParseTTMLTime(begin_attr);
					int64_t span_end = end_attr.empty() ? -1 : ParseTTMLTime(end_attr);
					if ((!begin_attr.empty() && span_begin < 0) || (!end_attr.empty() && span_end < 0))
						throw std::invalid_argument("Invalid or unsupported TTML span time.");
					if (span_begin >= 0 && !span_text.empty()) {
						has_karaoke = true;
						segments.push_back({span_begin, span_end, span_text, voice_bg});
					}
					else if (!span_text.empty()) {
						BuildParagraphText(node, plain, has_karaoke, segments, voice_bg);
					}
					break;
				}
				// Other elements: recurse so metadata wrappers never eat text.
				BuildParagraphText(node, plain, has_karaoke, segments, bg_voice);
				break;
			}
			default:
				break;
		}
	}
}
}

std::vector<lyrics::Line> lyrics::ParseLrc(std::vector<std::string> const& input) {
	int64_t offset_ms = 0;
	std::vector<LrcLine> lines;
	std::vector<std::string> untimed_lines;

	for (std::string line : input) {
		Trim(line);
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
			Trim(body);

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
				if (!val.empty() && val.front() == '+') {
					val.remove_prefix(1);
					if (val.empty() || val.front() < '0' || val.front() > '9')
						throw std::invalid_argument("Invalid LRC offset.");
				}
				auto parsed = std::from_chars(val.data(), val.data() + val.size(), off);
				if (val.empty() || parsed.ec != std::errc{} || parsed.ptr != val.data() + val.size()
					|| off < -MaxTime || off > MaxTime)
					throw std::invalid_argument("Invalid LRC offset.");
				offset_ms = off; // applied to all rows after reading metadata
			}
			// other metadata tags (ti/ar/al/by/re/ve/...) are ignored

			pos = close + 1;
		}

		std::string text = line.substr(pos);
		if (timestamps.empty()) continue;
		Trim(text);

		// Enhanced LRC: split <mm:ss.xx> syllable markers inside the text.
		size_t scan = 0;
		int64_t last_syllable_ms = timestamps.front();
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
				syllables.emplace_back(last_syllable_ms, pending_text);
				pending_text.clear();
			}
			last_syllable_ms = ms;
			has_syllables = true;
			scan = gt + 1;
		}
		Trim(pending_text);
		if (has_syllables && !pending_text.empty())
			syllables.emplace_back(last_syllable_ms, pending_text);
		else if (has_syllables)
			// A word timestamp with no text after it (the trailing
			// "<mm:ss.xx>" Apple Music exports) marks where the line ends,
			// so the final word must not stretch to the next line's start.
			end_marker_ms = last_syllable_ms;

		for (auto ts : timestamps) {
			LrcLine out;
			out.start_ms = ts;
			out.end_marker_ms = end_marker_ms;
			out.has_syllables = has_syllables;
			if (has_syllables) {
				out.syllables = syllables;
				// Repeated line timestamps repeat the same relative word timing.
				auto shift = ts - timestamps.front();
				for (auto& syllable : out.syllables)
					syllable.first = std::clamp(syllable.first + shift, int64_t{0}, MaxTime);
				if (out.end_marker_ms >= 0)
					out.end_marker_ms = std::clamp(out.end_marker_ms + shift, int64_t{0}, MaxTime);
			}
			else {
				out.text = text;
				out.text.erase(std::remove(out.text.begin(), out.text.end(), '\r'), out.text.end());
			}

			// Empty timestamp-only entries remain clearing boundaries, but
			// never become empty dialogue rows.
			lines.push_back(std::move(out));
		}
	}

	if (lines.empty() && !untimed_lines.empty()) {
		// Untimed LRC: import like the plain-text reader does, one untimed
		// row per lyric line, for the user to time.
		std::vector<Line> rows;
		for (auto& text : untimed_lines) rows.push_back({0, 0, std::move(text)});
		return rows;
	}

	if (lines.empty())
		throw std::invalid_argument("No timed lyrics lines found in LRC file.");

	for (auto& line : lines) {
		line.start_ms = std::clamp(line.start_ms - offset_ms, int64_t{0}, MaxTime);
		if (line.end_marker_ms >= 0)
			line.end_marker_ms = std::clamp(line.end_marker_ms - offset_ms, int64_t{0}, MaxTime);
		for (auto& syllable : line.syllables)
			syllable.first = std::clamp(syllable.first - offset_ms, int64_t{0}, MaxTime);
	}
	// Sort by start time; LRC files are usually ordered but multi-timestamp
	// expansion can interleave.
	std::stable_sort(lines.begin(), lines.end(),
		[](LrcLine const& a, LrcLine const& b) { return a.start_ms < b.start_ms; });

	std::vector<Line> rows;
	for (size_t i = 0; i < lines.size(); ++i) {
		auto const& cur = lines[i];
		if (cur.text.empty() && cur.syllables.empty()) continue;
		auto end = cur.end_marker_ms >= 0 ? cur.end_marker_ms : std::min(cur.start_ms + 5000, MaxTime);
		if (cur.end_marker_ms < 0 && i + 1 < lines.size()) end = lines[i + 1].start_ms;
		end = std::max(end, cur.start_ms);
		if (i + 1 < lines.size()) end = std::min(end, lines[i + 1].start_ms);
		std::string text = cur.text;
		if (cur.has_syllables && !cur.syllables.empty()) {
			std::vector<Segment> segments;
			for (auto const& syllable : cur.syllables)
				segments.push_back({syllable.first, -1, syllable.second});
			text = Karaoke(segments, cur.start_ms, end);
		}
		rows.push_back({cur.start_ms, end, std::move(text)});
	}
	if (rows.empty()) throw std::invalid_argument("No lyrics lines found in LRC file.");
	return rows;
}

std::vector<lyrics::Line> lyrics::ParseTTML(wxInputStream& input) {
	wxXmlDocument doc;
	if (!doc.Load(input, "UTF-8", wxXMLDOC_KEEP_WHITESPACE_NODES)) throw std::invalid_argument("Failed loading TTML XML file.");
	if (!doc.GetRoot() || !IsElement(doc.GetRoot(), "tt"))
		throw std::invalid_argument("Invalid TTML file: root element is not <tt>.");

	std::vector<TtmlParagraph> paragraphs;

	// Depth-first search for <p> elements anywhere below the root (they live
	// under body/div in valid TTML, but real-world files vary).
	for (auto node = doc.GetRoot(); node; node = node->GetNext()) {
		if (node->GetType() != wxXML_ELEMENT_NODE) continue;

		// Iterative traversal from this top-level node.
		std::vector<wxXmlNode*> stack{node};
		while (!stack.empty()) {
			wxXmlNode *cur = stack.back();
			stack.pop_back();
			if (cur->GetType() != wxXML_ELEMENT_NODE) continue;

			if (IsElement(cur, "p")) {
				TtmlParagraph para;
				if (!ParseParagraphTimes(cur, para.begin_ms, para.end_ms)) {
					// Apple Music exports songs without any timing at all
					// (itunes:timing="None"): every <p> is bare text. Import
					// them like the plain-text reader does — one untimed row
					// per paragraph — instead of failing the whole file.
					para.begin_ms = 0;
					para.end_ms = 0;
				}

				std::string plain;
				BuildParagraphText(cur, plain, para.has_karaoke, para.segments);

				if (para.has_karaoke) {
					std::stable_sort(para.segments.begin(), para.segments.end(),
						[](Segment const& a, Segment const& b) { return a.begin_ms < b.begin_ms; });
					if (para.end_ms <= para.begin_ms) {
						// Untimed <p> whose word spans still carry timing:
						// derive the row's bounds from the spans themselves.
						para.begin_ms = para.segments.front().begin_ms;
						para.end_ms = 0;
						for (auto const& seg : para.segments)
							para.end_ms = std::max(para.end_ms,
								seg.end_ms > seg.begin_ms ? seg.end_ms : seg.begin_ms);
					}

					// Background vocals (x-bg) that enter while the lead voice
					// is still singing cannot share one gapless \kf chain —
					// sorting by begin scrambles the word order and the lead's
					// held notes get truncated at the harmony's first word.
					// Split them into their own dialogue row with their real
					// timings; ASS stacks simultaneous rows by itself.
					std::vector<Segment> lead_segs, bg_segs;
					for (auto const& seg : para.segments)
						(seg.bg ? bg_segs : lead_segs).push_back(seg);

					int64_t lead_last_end = 0;
					for (auto const& seg : lead_segs)
						lead_last_end = std::max(lead_last_end,
							seg.end_ms > seg.begin_ms ? seg.end_ms : seg.begin_ms);

					if (!bg_segs.empty() && !lead_segs.empty()
						&& bg_segs.front().begin_ms < lead_last_end) {
						TtmlParagraph harmony;
						harmony.begin_ms = std::max<int64_t>(para.begin_ms, bg_segs.front().begin_ms);
						harmony.end_ms = std::max(para.end_ms, harmony.begin_ms);
						harmony.karaoke_text = Karaoke(bg_segs, harmony.begin_ms, harmony.end_ms);
						Trim(harmony.karaoke_text);
						para.karaoke_text = plain + Karaoke(lead_segs, para.begin_ms, para.end_ms);
						Trim(para.karaoke_text);

						if (!harmony.karaoke_text.empty() && !para.karaoke_text.empty()) {
							paragraphs.push_back(std::move(para));
							paragraphs.push_back(std::move(harmony));
							continue;
						}
					}
					para.karaoke_text = plain + Karaoke(para.segments, para.begin_ms, para.end_ms);
				}
				else {
					para.karaoke_text = plain;
				}

				Trim(para.karaoke_text);
				if (para.karaoke_text.empty()) continue;
				paragraphs.push_back(std::move(para));
				continue; // do not recurse into a <p>
			}

			std::vector<wxXmlNode*> children;
			for (auto child = cur->GetChildren(); child; child = child->GetNext()) children.push_back(child);
			stack.insert(stack.end(), children.rbegin(), children.rend());
		}
	}

	if (paragraphs.empty())
		throw std::invalid_argument("No <p> paragraphs found in TTML file.");

	std::stable_sort(paragraphs.begin(), paragraphs.end(),
		[](TtmlParagraph const& a, TtmlParagraph const& b) { return a.begin_ms < b.begin_ms; });

	std::vector<Line> rows;
	for (auto& para : paragraphs)
		rows.push_back({para.begin_ms, para.end_ms, std::move(para.karaoke_text)});
	return rows;
}
