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

#include "../config.h"

#include "libaegisub/ass/dialogue_parser.h"

#include "libaegisub/scoped_ptr.h"
#include "libaegisub/spellchecker.h"

#include "iconv.h"

namespace {

typedef std::vector<agi::ass::DialogueToken> TokenVec;
namespace dt = agi::ass::DialogueTokenType;
namespace ss = agi::ass::SyntaxStyle;

class SyntaxHighlighter {
	TokenVec ranges;
	std::string const& text;
	agi::SpellChecker *spellchecker;
	agi::scoped_holder<iconv_t, int(&)(iconv_t)> utf8_to_utf32;

	void SetStyling(int len, int type) {
		if (ranges.size() && ranges.back().type == type)
			ranges.back().length += len;
		else
			ranges.push_back(agi::ass::DialogueToken(type, len));
	}

	void CheckWord(int start, int end) {
		int len = end - start;
		if (!len) return;

		if (!spellchecker->CheckWord(text.substr(start, len)))
			SetStyling(len, ss::SPELLING);
		else
			SetStyling(len, ss::NORMAL);
	}

	int NextChar(int pos, int len, int& char_len) {
		int chr = 0;
		char *inptr = const_cast<char *>(&text[pos]);
		size_t inlen = len;
		char *outptr = (char *)&chr;
		size_t outlen = sizeof chr;

		if (iconv(utf8_to_utf32, &inptr, &inlen, &outptr, &outlen) != 1)
			return 0;

		char_len = len - inlen;
		return chr;
	}

	void StyleSpellCheck(int pos, int len) {
		const int delims[] = {
			0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0028,
			0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x003a,
			0x003b, 0x003d, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e,
			0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x007e, 0x00a1, 0x00a2,
			0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00aa, 0x00ab,
			0x00b0, 0x00b6, 0x00b7, 0x00ba, 0x00bb, 0x00bf, 0x02dc, 0x0e3f,
			0x2010, 0x2013, 0x2014, 0x2015, 0x2018, 0x2019, 0x201c, 0x201d,
			0x2020, 0x2021, 0x2022, 0x2025, 0x2026, 0x2026, 0x2030, 0x2031,
			0x2032, 0x203b, 0x203b, 0x203d, 0x2042, 0x2044, 0x20a6, 0x20a9,
			0x20aa, 0x20ac, 0x20ad, 0x2116, 0x2234, 0x2235, 0x2420, 0x2422,
			0x2423, 0x2506, 0x25ca, 0x2605, 0x261e, 0x2e2e, 0x3000, 0x3001,
			0x3002, 0x3008, 0x3009, 0x300a, 0x300b, 0x300c, 0x300d, 0x300e,
			0x300f, 0x3010, 0x3011, 0x3014, 0x3015, 0x3016, 0x3017, 0x3018,
			0x3019, 0x301a, 0x301b, 0x301c, 0x3030, 0x303d, 0x30fb, 0xff0a,
			0xff5b, 0xff5d, 0xff5e
		};

		int chrlen = 0;
		int start = pos;
		for (; len > 0; pos += chrlen, len -= chrlen) {
			int chr = NextChar(pos, len, chrlen);
			if (!chr) return;

			if (std::binary_search(std::begin(delims), std::end(delims), chr)) {
				CheckWord(start, pos);
				SetStyling(1, ss::NORMAL);
				start = pos + 1;
			}
		}

		CheckWord(start, pos);
	}

public:
	SyntaxHighlighter(std::string const& text, agi::SpellChecker *spellchecker)
	: text(text)
	, spellchecker(spellchecker)
	, utf8_to_utf32(iconv_open("utf-32", "utf-8"), iconv_close)
	{ }

	TokenVec Highlight(TokenVec const& tokens, bool template_line) {
		if (tokens.empty()) return ranges;

		bool in_drawing = false;
		size_t pos = 0;

		// VSFilter treats unclosed override blocks as plain text, so pretend
		// all tokens after the last override block are TEXT
		size_t last_ovr_end = 0;
		for (size_t i = tokens.size(); i > 0; --i) {
			if (tokens[i - 1].type == dt::OVR_END) {
				last_ovr_end = i - 1;
				break;
			}
		}

		for (size_t i = 0; i < tokens.size(); ++i) {
			size_t len = tokens[i].length;
			switch (i > last_ovr_end ? dt::TEXT : tokens[i].type) {
				case dt::LINE_BREAK: SetStyling(len, ss::LINE_BREAK); break;
				case dt::ERROR:      SetStyling(len, ss::ERROR); break;
				case dt::ARG:        SetStyling(len, ss::PARAMETER); break;
				case dt::COMMENT:    SetStyling(len, ss::COMMENT); break;
				case dt::WHITESPACE: SetStyling(len, ss::NORMAL); break;
				case dt::OPEN_PAREN: case dt::CLOSE_PAREN: case dt::ARG_SEP: case dt::TAG_START:
					SetStyling(len, ss::PUNCTUATION);
					break;
				case dt::OVR_BEGIN: case dt::OVR_END:
					SetStyling(len, ss::OVERRIDE);
					break;

				case dt::TEXT:
					if (in_drawing)
						SetStyling(len, ss::DRAWING);
					else if (spellchecker)
						StyleSpellCheck(pos, len);
					else
						SetStyling(len, ss::NORMAL);
					break;

				case dt::TAG_NAME:
					SetStyling(len, ss::TAG);

					if (len != 1 || i + 1 >= tokens.size() || text[pos] != 'p')
						break;

					in_drawing = false;

					if (tokens[i + 1].type != dt::ARG)
						break;

					for (size_t j = pos + len; j < pos + len + tokens[i + 1].length; ++j) {
						char c = text[j];
						// I have no idea why one would use leading zeros for
						// the scale, but vsfilter allows it
						if (c >= '1' && c <= '9')
							in_drawing = true;
						else if (c != '0')
							break;
					}
					break;
			}

			pos += len;
			// karaoke templater
		}

		return ranges;
	}
};
}

namespace agi {
namespace ass {

std::vector<DialogueToken> SyntaxHighlight(std::string const& text, std::vector<DialogueToken> const& tokens, bool template_line, SpellChecker *spellchecker) {
	return SyntaxHighlighter(text, spellchecker).Highlight(tokens, template_line);
}

}
}
