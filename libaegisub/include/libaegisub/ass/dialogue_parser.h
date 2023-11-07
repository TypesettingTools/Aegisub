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

#include <string>
#include <vector>

#undef ERROR

namespace agi {
	class SpellChecker;

	namespace ass {
		namespace DialogueTokenType {
			enum {
				TEXT = 1000,
				WORD,
				LINE_BREAK,
				OVR_BEGIN,
				OVR_END,
				TAG_START,
				TAG_NAME,
				OPEN_PAREN,
				CLOSE_PAREN,
				ARG_SEP,
				ARG,
				ERROR,
				COMMENT,
				WHITESPACE,
				DRAWING,
				KARAOKE_TEMPLATE,
				KARAOKE_VARIABLE
			};
		}

		namespace SyntaxStyle {
			enum {
				NORMAL = 0,
				COMMENT,
				DRAWING,
				OVERRIDE,
				PUNCTUATION,
				TAG,
				ERROR,
				PARAMETER,
				LINE_BREAK,
				KARAOKE_TEMPLATE,
				KARAOKE_VARIABLE,

				SPELLING = 32
			};
		}

		struct DialogueToken {
			int type;
			size_t length;
		};

		/// Tokenize the passed string as the body of a dialogue line
		std::vector<DialogueToken> TokenizeDialogueBody(std::string const& str, bool karaoke_templater=false);

		/// Convert the body of drawings to DRAWING tokens
		void MarkDrawings(std::string const& str, std::vector<DialogueToken> &tokens);

		/// Split the words in the TEXT tokens of the lexed line into their
		/// own tokens and convert the body of drawings to DRAWING tokens
		void SplitWords(std::string const& str, std::vector<DialogueToken> &tokens);

		void SplitWords(std::u8string const& str, std::vector<DialogueToken> &tokens);

		std::vector<DialogueToken> SyntaxHighlight(std::string const& text, std::vector<DialogueToken> const& tokens, SpellChecker *spellchecker);
	}
}
