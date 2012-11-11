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
using namespace agi::ass;
namespace dt = DialogueTokenType;
namespace ss = SyntaxStyle;

class SyntaxHighlighter {
	TokenVec ranges;
	std::string const& text;
	agi::SpellChecker *spellchecker;

	void SetStyling(int len, int type) {
		if (ranges.size() && ranges.back().type == type)
			ranges.back().length += len;
		else
			ranges.push_back(DialogueToken(type, len));
	}

public:
	SyntaxHighlighter(std::string const& text, agi::SpellChecker *spellchecker)
	: text(text)
	, spellchecker(spellchecker)
	{ }

	TokenVec Highlight(TokenVec const& tokens) {
		if (tokens.empty()) return ranges;

		size_t pos = 0;

		for (auto tok : tokens) {
			switch (tok.type) {
				case dt::KARAOKE_TEMPLATE: SetStyling(tok.length, ss::KARAOKE_TEMPLATE); break;
				case dt::KARAOKE_VARIABLE: SetStyling(tok.length, ss::KARAOKE_VARIABLE); break;
				case dt::LINE_BREAK: SetStyling(tok.length, ss::LINE_BREAK); break;
				case dt::ERROR:      SetStyling(tok.length, ss::ERROR);      break;
				case dt::ARG:        SetStyling(tok.length, ss::PARAMETER);  break;
				case dt::COMMENT:    SetStyling(tok.length, ss::COMMENT);    break;
				case dt::DRAWING:    SetStyling(tok.length, ss::DRAWING);    break;
				case dt::TEXT:       SetStyling(tok.length, ss::NORMAL);     break;
				case dt::TAG_NAME:   SetStyling(tok.length, ss::TAG);        break;
				case dt::OPEN_PAREN: case dt::CLOSE_PAREN: case dt::ARG_SEP: case dt::TAG_START:
					SetStyling(tok.length, ss::PUNCTUATION);
					break;
				case dt::OVR_BEGIN: case dt::OVR_END:
					SetStyling(tok.length, ss::OVERRIDE);
					break;
				case dt::WHITESPACE:
					if (ranges.size() && ranges.back().type == ss::PARAMETER)
						SetStyling(tok.length, ss::PARAMETER);
					else
						SetStyling(tok.length, ss::NORMAL);
					break;
				case dt::WORD:
					if (spellchecker && !spellchecker->CheckWord(text.substr(pos, tok.length)))
						SetStyling(tok.length, ss::SPELLING);
					else
						SetStyling(tok.length, ss::NORMAL);
					break;
			}

			pos += tok.length;
		}

		return ranges;
	}
};

class WordSplitter {
	std::string const& text;
	std::vector<DialogueToken> &tokens;
	agi::scoped_holder<iconv_t, int(&)(iconv_t)> utf8_to_utf32;
	size_t last_ovr_end;
	size_t pos;
	bool in_drawing;

	bool IsWordSep(int chr) {
		static const int delims[] = {
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

		return std::binary_search(std::begin(delims), std::end(delims), chr);
	}

	int NextChar(int pos, int len, int& char_len) {
		int chr = 0;
		char *inptr = const_cast<char *>(&text[pos]);
		size_t inlen = len;
		char *outptr = (char *)&chr;
		size_t outlen = sizeof chr;

		iconv(utf8_to_utf32, &inptr, &inlen, &outptr, &outlen);
		if (outlen != 0)
			return 0;

		char_len = len - inlen;
		return chr;
	}

	void SwitchTo(size_t &i, int type, int len) {
		if (tokens[i].type == type) return;

		if (tokens[i].length == (size_t)len)
			tokens[i].type = type;
		else {
			tokens.insert(tokens.begin() + i + 1, DialogueToken(type, len));
			tokens[i].length -= len;
			++i;
			++last_ovr_end;
		}
	}

	void SplitText(size_t &i) {
		if (in_drawing) {
			tokens[i].type = dt::DRAWING;
			return;
		}

		int chrlen = 0;
		int len = tokens[i].length;
		int tpos = pos;
		for (; len > 0; tpos += chrlen, len -= chrlen) {
			int chr = NextChar(tpos, len, chrlen);
			if (!chr) return;

			if (IsWordSep(chr))
				SwitchTo(i, dt::TEXT, len);
			else
				SwitchTo(i, dt::WORD, len);
		}
	}

public:
	WordSplitter(std::string const& text, std::vector<DialogueToken> &tokens)
	: text(text)
	, tokens(tokens)
	, utf8_to_utf32(iconv_open("utf-32le", "utf-8"), iconv_close)
	, last_ovr_end(0)
	, pos(0)
	, in_drawing(false)
	{ }

	void SplitWords() {
		if (tokens.empty()) return;

		// VSFilter treats unclosed override blocks as plain text, so pretend
		// all tokens after the last override block are TEXT
		for (size_t i = tokens.size(); i > 0; --i) {
			if (tokens[i - 1].type == dt::OVR_END) {
				last_ovr_end = i;
				break;
			}
		}

		for (size_t i = 0; i < tokens.size(); ++i) {
			size_t len = tokens[i].length;
			switch (tokens[i].type) {
				case dt::KARAOKE_TEMPLATE: break;
				case dt::KARAOKE_VARIABLE: break;
				case dt::LINE_BREAK: break;
				case dt::TEXT: SplitText(i); break;
				case dt::TAG_NAME:
					if (i + 1 > last_ovr_end) {
						SplitText(i);
						break;
					}

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
				default:
					if (i + 1 > last_ovr_end)
						SplitText(i);
					break;
			}

			pos += len;
		}
	}
};
}

namespace agi {
namespace ass {

std::vector<DialogueToken> SyntaxHighlight(std::string const& text, std::vector<DialogueToken> const& tokens, SpellChecker *spellchecker) {
	return SyntaxHighlighter(text, spellchecker).Highlight(tokens);
}

void SplitWords(std::string const& str, std::vector<DialogueToken> &tokens) {
	WordSplitter(str, tokens).SplitWords();
}

}
}
