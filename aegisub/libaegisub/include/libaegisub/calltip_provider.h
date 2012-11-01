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

#ifndef LAGI_PRE
#include <map>
#include <string>
#include <vector>
#endif

namespace agi {
	namespace ass { struct DialogueToken; }

	struct Calltip {
		std::string text;       ///< Text of the calltip
		size_t highlight_start; ///< Start index of the current parameter in text
		size_t highlight_end;   ///< End index of the current parameter in text
		size_t tag_position;    ///< Start index of the tag in the input line
	};

	class CalltipProvider {
		struct CalltipProto {
			std::string text;
			std::vector<std::pair<size_t, size_t>> args;
		};
		std::multimap<std::string, CalltipProto> protos;

	public:
		CalltipProvider();

		/// Get the calltip to show for the given cursor position in the text
		Calltip GetCalltip(std::vector<ass::DialogueToken> const& tokens, std::string const& text, size_t pos);
	};
}
