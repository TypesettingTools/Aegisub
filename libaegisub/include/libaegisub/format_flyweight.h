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

#include <libaegisub/fs_fwd.h>

#include <boost/flyweight.hpp>

namespace agi {
template <> struct writer<char, boost::flyweight<std::string>> {
	static void write(std::basic_ostream<char>& out, int max_len,
	                  boost::flyweight<std::string> const& value) {
		writer<char, std::string>::write(out, max_len, value.get());
	}
};

template <> struct writer<wchar_t, boost::flyweight<std::string>> {
	static void write(std::basic_ostream<wchar_t>& out, int max_len,
	                  boost::flyweight<std::string> const& value) {
		writer<wchar_t, std::string>::write(out, max_len, value.get());
	}
};
} // namespace agi
