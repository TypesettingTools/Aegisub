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

#include "libaegisub/ass/dialogue_parser.h"

#include "libaegisub/exception.h"
#include "libaegisub/spellchecker.h"
#include "libaegisub/unicode.h"

namespace {

using TokenVec = std::vector<agi::ass::DialogueToken>;
using namespace agi::ass;
namespace dt = DialogueTokenType;
namespace ss = SyntaxStyle;

class SyntaxHighlighter {
	TokenVec ranges;
	std::string_view text;
	agi::SpellChecker *spellchecker;

	void SetStyling(size_t len, int type) {
		if (ranges.size() && ranges.back().type == type)
			ranges.back().length += len;
		else
			ranges.push_back(DialogueToken{type, len});
	}

public:
	SyntaxHighlighter(std::string_view text, agi::SpellChecker *spellchecker)
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
	std::string_view text;
	std::vector<DialogueToken> &tokens;
	size_t pos = 0;

	void SwitchTo(size_t &i, int type, size_t len) {
		auto old = tokens[i];
		tokens[i].type = type;
		tokens[i].length = len;

		if (old.length != (size_t)len) {
			tokens.insert(tokens.begin() + i + 1, DialogueToken{old.type, old.length - len});
			++i;
		}
	}

	void SplitText(size_t &i) {
		UErrorCode err = U_ZERO_ERROR;
		thread_local std::unique_ptr<icu::BreakIterator> bi(icu::BreakIterator::createWordInstance(icu::Locale::getDefault(), err));
		agi::UTextPtr ut(utext_openUTF8(nullptr, text.data() + pos, tokens[i].length, &err));
		bi->setText(ut.get(), err);
		if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));
		size_t pos = 0;
		while (bi->next() != UBRK_DONE) {
			auto len = bi->current() - pos;

			std::vector<int32_t> rules(8);
			int n = bi->getRuleStatusVec(rules.data(), rules.size(), err);
			if (err == U_BUFFER_OVERFLOW_ERROR) {
				err = U_ZERO_ERROR;
				bi->getRuleStatusVec(rules.data(), rules.size(), err);
			}

			if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));

			auto token_type = dt::TEXT;

			for (size_t i = 0; i < n; i++) {
				if (rules[i] >= UBRK_WORD_LETTER && rules[i] < UBRK_WORD_IDEO_LIMIT) {
					token_type = dt::WORD;
					break;
				}
			}
			SwitchTo(i, token_type, len);
			pos = bi->current();
		}
	}

public:
	WordSplitter(std::string_view text, std::vector<DialogueToken> &tokens)
	: text(text)
	, tokens(tokens)
	{ }

	void SplitWords() {
		if (tokens.empty()) return;

		for (size_t i = 0; i < tokens.size(); ++i) {
			size_t len = tokens[i].length;
			if (tokens[i].type == dt::TEXT)
				SplitText(i);
			pos += len;
		}
	}
};
}

namespace agi::ass {

std::vector<DialogueToken> SyntaxHighlight(std::string_view text,
	                                       std::vector<DialogueToken> const& tokens,
	                                       SpellChecker *spellchecker) {
	return SyntaxHighlighter(text, spellchecker).Highlight(tokens);
}

void MarkDrawings(std::string_view str, std::vector<DialogueToken> &tokens) {
	if (tokens.empty()) return;

	size_t last_ovr_end = 0;
	for (size_t i = tokens.size(); i > 0; --i) {
		if (tokens[i - 1].type == dt::OVR_END) {
			last_ovr_end = i;
			break;
		}
	}

	size_t pos = 0;
	bool in_drawing = false;

	for (size_t i = 0; i < last_ovr_end; ++i) {
		size_t len = tokens[i].length;
		switch (tokens[i].type) {
			case dt::TEXT:
				if (in_drawing)
					tokens[i].type = dt::DRAWING;
				break;
			case dt::TAG_NAME:
				if (len != 1 || i + 1 >= tokens.size() || str[pos] != 'p')
					break;

				in_drawing = false;

				if (i + 1 == last_ovr_end || tokens[i + 1].type != dt::ARG)
					break;

				for (size_t j = pos + len; j < pos + len + tokens[i + 1].length; ++j) {
					char c = str[j];
					// I have no idea why one would use leading zeros for
					// the scale, but vsfilter allows it
					if (c >= '1' && c <= '9')
						in_drawing = true;
					else if (c != '0')
						break;
				}
				break;
			default: break;
		}

		pos += len;
	}

	// VSFilter treats unclosed override blocks as plain text, so merge all
	// the tokens after the last override block into a single TEXT (or DRAWING)
	// token
	for (size_t i = last_ovr_end; i < tokens.size(); ++i) {
		switch (tokens[i].type) {
			case dt::KARAOKE_TEMPLATE: break;
			case dt::KARAOKE_VARIABLE: break;
			case dt::LINE_BREAK: break;
			default:
				tokens[i].type = in_drawing ? dt::DRAWING : dt::TEXT;
				if (i > 0 && tokens[i - 1].type == tokens[i].type) {
					tokens[i - 1].length += tokens[i].length;
					tokens.erase(tokens.begin() + i);
					--i;
				}
		}
	}
}

void SplitWords(std::string_view str, std::vector<DialogueToken> &tokens) {
	MarkDrawings(str, tokens);
	WordSplitter(str, tokens).SplitWords();
}
}
