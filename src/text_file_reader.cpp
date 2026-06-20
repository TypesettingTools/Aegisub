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
// Aegisub Project http://www.aegisub.org/

#include "text_file_reader.h"

#include <libaegisub/file_mapping.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

TextFileReader::TextFileReader(agi::fs::path const& filename, const char *encoding, bool trim)
: file(std::make_unique<agi::read_file_mapping>(filename))
, stream(std::make_unique<boost::interprocess::ibufferstream>(file->read(), file->size()))
, trim(trim)
, iter(agi::line_iterator<std::string>(*stream, encoding))
, first_line(true)
{
}

TextFileReader::~TextFileReader() {
}

std::string TextFileReader::ReadLineFromFile() {
	std::string str = *iter;
	++iter;
	if (trim)
		boost::trim(str);
	if (first_line) {
		first_line = false;
		static constexpr std::string_view invisible_chars[] = {
			"\xC2\xAD",       // U+00AD  SOFT HYPHEN
			"\xCD\x8F",       // U+034F  COMBINING GRAPHEME JOINER
			"\xE1\xA0\x8E",   // U+180E  MONGOLIAN VOWEL SEPARATOR
			"\xE2\x80\x8B",   // U+200B  ZERO WIDTH SPACE
			"\xE2\x80\x8C",   // U+200C  ZERO WIDTH NON-JOINER
			"\xE2\x80\x8D",   // U+200D  ZERO WIDTH JOINER
			"\xE2\x80\x8E",   // U+200E  LEFT-TO-RIGHT MARK
			"\xE2\x80\x8F",   // U+200F  RIGHT-TO-LEFT MARK
			"\xE2\x80\xA8",   // U+2028  LINE SEPARATOR
			"\xE2\x80\xA9",   // U+2029  PARAGRAPH SEPARATOR
			"\xE2\x80\xAA",   // U+202A  LEFT-TO-RIGHT EMBEDDING
			"\xE2\x80\xAB",   // U+202B  RIGHT-TO-LEFT EMBEDDING
			"\xE2\x80\xAC",   // U+202C  POP DIRECTIONAL FORMATTING
			"\xE2\x80\xAD",   // U+202D  LEFT-TO-RIGHT OVERRIDE
			"\xE2\x80\xAE",   // U+202E  RIGHT-TO-LEFT OVERRIDE
			"\xE2\x81\xA0",   // U+2060  WORD JOINER
			"\xE2\x81\xA6",   // U+2066  LEFT-TO-RIGHT ISOLATE
			"\xE2\x81\xA7",   // U+2067  RIGHT-TO-LEFT ISOLATE
			"\xE2\x81\xA8",   // U+2068  FIRST STRONG ISOLATE
			"\xE2\x81\xA9",   // U+2069  POP DIRECTIONAL ISOLATE
			"\xEF\xBB\xBF"    // U+FEFF  ZERO WIDTH NO-BREAK SPACE (BOM)
		};
		bool found;
		do {
			found = false;
			for (auto const& seq : invisible_chars) {
				if (str.starts_with(seq)) {
					str.erase(0, seq.size());
					found = true;
					break;
				}
			}
		} while (found);
	}
	return str;
}
