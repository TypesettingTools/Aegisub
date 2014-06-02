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

#include <boost/locale/gnu_gettext.hpp>
#include <boost/locale/localization_backend.hpp>
#include <unicode/locid.h>

// Boost.locale doesn't support partial builds of ICU, so provide stub versions
// of some of the things we don't use
namespace boost { namespace locale {
namespace impl_icu {
struct cdata {
	icu::Locale locale;
	std::string encoding;
	bool utf8;
};

std::locale create_formatting(std::locale const& in, cdata const& cd, character_facet_type type) {
	return in;
}

std::locale create_parsing(std::locale const& in, cdata const& cd, character_facet_type type) {
	return in;
}

std::locale create_calendar(std::locale const& in, cdata const& cd) {
	return in;
}

}
namespace gnu_gettext {
template<>
message_format<char> *create_messages_facet(messages_info const &info) {
	return nullptr;
}

template<>
message_format<wchar_t> *create_messages_facet(messages_info const &info) {
	return nullptr;
}

#ifdef BOOST_HAS_CHAR16_T
template<>
message_format<char16_t> *create_messages_facet(messages_info const &info) {
	return nullptr;
}
#endif

#ifdef BOOST_HAS_CHAR32_T
template<>
message_format<char32_t> *create_messages_facet(messages_info const &info) {
	return nullptr;
}
#endif
}
} }
