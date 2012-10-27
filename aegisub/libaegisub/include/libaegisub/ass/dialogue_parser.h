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

#ifndef LAGI_PRE
#include <vector>
#endif

namespace agi {
	namespace ass {
		namespace DialogueTokenType {
			enum {
				TEXT = 1000,
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
				WHITESPACE
			};
		}

		struct DialogueToken {
			int type;
			size_t length;
			DialogueToken(int type, size_t length) : type(type), length(length) { }
		};

		std::vector<DialogueToken> TokenizeDialogueBody(std::string const& str);
	}
}
