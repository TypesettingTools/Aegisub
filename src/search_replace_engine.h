// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <functional>
#include <boost/regex/icu.hpp>
#include <string>

namespace agi { struct Context; }
class AssDialogue;

struct MatchState {
	boost::u32regex *re;
	size_t start, end;

	operator bool() const { return end != (size_t)-1; }
};

struct SearchReplaceSettings {
	enum class Field {
		TEXT = 0,
		STYLE,
		ACTOR,
		EFFECT
	};

	enum class Limit {
		ALL = 0,
		SELECTED
	};

	std::string find;
	std::string replace_with;

	Field field;
	Limit limit_to;

	bool match_case;
	bool use_regex;
	bool ignore_comments;
	bool skip_tags;
	bool exact_match;
};

class SearchReplaceEngine {
	agi::Context *context;
	bool initialized;
	SearchReplaceSettings settings;

	bool FindReplace(bool replace);
	void Replace(AssDialogue *line, MatchState &ms);

public:
	bool FindNext() { return FindReplace(false); }
	bool ReplaceNext() { return FindReplace(true); }
	bool ReplaceAll();

	void Configure(SearchReplaceSettings const& new_settings);

	static std::function<MatchState (const AssDialogue*, size_t)> GetMatcher(SearchReplaceSettings const& settings);

	SearchReplaceEngine(agi::Context *c);
};
