// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "libaegisub/lua/ffi.h"
#include "libaegisub/make_unique.h"

#include <boost/regex/icu.hpp>

using boost::u32regex;
namespace {
// A cmatch with a match range attached to it so that we can return a pointer to
// an int pair without an extra heap allocation each time (LuaJIT can't compile
// ffi calls which return aggregates by value)
struct agi_re_match {
	boost::cmatch m;
	int range[2];
};

struct agi_re_flag {
	const char* name;
	int value;
};
} // namespace

namespace agi {
AGI_DEFINE_TYPE_NAME(u32regex);
AGI_DEFINE_TYPE_NAME(agi_re_match);
AGI_DEFINE_TYPE_NAME(agi_re_flag);
} // namespace agi

namespace {
using match = agi_re_match;
bool search(u32regex& re, const char* str, size_t len, int start, boost::cmatch& result) {
	return u32regex_search(str + start, str + len, result, re,
	                       start > 0 ? boost::match_prev_avail | boost::match_not_bob
	                                 : boost::match_default);
}

match* regex_match(u32regex& re, const char* str, size_t len, int start) {
	auto result = agi::make_unique<match>();
	if(!search(re, str, len, start, result->m)) return nullptr;
	return result.release();
}

int* regex_get_match(match& match, size_t idx) {
	if(idx > match.m.size() || !match.m[idx].matched) return nullptr;
	match.range[0] = std::distance(match.m.prefix().first, match.m[idx].first + 1);
	match.range[1] = std::distance(match.m.prefix().first, match.m[idx].second);
	return match.range;
}

int* regex_search(u32regex& re, const char* str, size_t len, size_t start) {
	boost::cmatch result;
	if(!search(re, str, len, start, result)) return nullptr;

	auto ret = static_cast<int*>(malloc(sizeof(int) * 2));
	ret[0] = start + result.position() + 1;
	ret[1] = start + result.position() + result.length();
	return ret;
}

char* regex_replace(u32regex& re, const char* replacement, const char* str, size_t len,
                    int max_count) {
	// Can't just use regex_replace here since it can only do one or infinite replacements
	auto match = boost::u32regex_iterator<const char*>(str, str + len, re);
	auto end_it = boost::u32regex_iterator<const char*>();

	auto suffix = str;

	std::string ret;
	auto out = back_inserter(ret);
	while(match != end_it && max_count > 0) {
		copy(suffix, match->prefix().second, out);
		match->format(out, replacement);
		suffix = match->suffix().first;
		++match;
		--max_count;
	}

	ret += suffix;
	return agi::lua::strndup(ret);
}

u32regex* regex_compile(const char* pattern, int flags, char** err) {
	auto re = agi::make_unique<u32regex>();
	try {
		*re = boost::make_u32regex(pattern, boost::u32regex::perl | flags);
		return re.release();
	} catch(std::exception const& e) {
		*err = strdup(e.what());
		return nullptr;
	}
}

void regex_free(u32regex* re) {
	delete re;
}
void match_free(match* m) {
	delete m;
}

const agi_re_flag* get_regex_flags() {
	static const agi_re_flag flags[] = { { "ICASE", boost::u32regex::icase },
		                                 { "NOSUB", boost::u32regex::nosubs },
		                                 { "COLLATE", boost::u32regex::collate },
		                                 { "NEWLINE_ALT", boost::u32regex::newline_alt },
		                                 { "NO_MOD_M", boost::u32regex::no_mod_m },
		                                 { "NO_MOD_S", boost::u32regex::no_mod_s },
		                                 { "MOD_S", boost::u32regex::mod_s },
		                                 { "MOD_X", boost::u32regex::mod_x },
		                                 { "NO_EMPTY_SUBEXPRESSIONS",
		                                   boost::u32regex::no_empty_expressions },
		                                 { nullptr, 0 } };
	return flags;
}
} // namespace

extern "C" int luaopen_re_impl(lua_State* L) {
	agi::lua::register_lib_table(
	    L, { "agi_re_match", "u32regex" }, "search", regex_search, "match", regex_match,
	    "get_match", regex_get_match, "replace", regex_replace, "compile", regex_compile,
	    "get_flags", get_regex_flags, "match_free", match_free, "regex_free", regex_free);
	return 1;
}
