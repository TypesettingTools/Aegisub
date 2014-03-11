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

#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/split.hpp>

namespace agi {
	typedef boost::iterator_range<std::string::const_iterator> StringRange;

	template<typename Str, typename Char>
	boost::split_iterator<typename Str::const_iterator> Split(Str const& str, Char delim) {
		return boost::make_split_iterator(str, boost::token_finder([=](Char c) { return c == delim; }));
	}

	inline std::string str(StringRange const& r) {
		return std::string(r.begin(), r.end());
	}
}

namespace boost {
	namespace algorithm {
		template<typename Iterator>
		split_iterator<Iterator> begin(split_iterator<Iterator> it) {
			return it;
		}

		template<typename Iterator>
		split_iterator<Iterator> end(split_iterator<Iterator>) {
			return split_iterator<Iterator>();
		}
	}
}
