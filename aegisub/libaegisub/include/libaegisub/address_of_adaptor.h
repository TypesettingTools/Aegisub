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

#include <boost/range/adaptor/transformed.hpp>

namespace agi {
namespace address_of_detail {
	using namespace boost::adaptors;

	// Tag type to select the operator| overload
	struct address_of_tag_type { };

	template<typename Iterator>
	struct take_address_of {
		using result_type = typename std::iterator_traits<Iterator>::pointer;
		using input_type = typename std::iterator_traits<Iterator>::reference;

		result_type operator()(input_type v) const { return &v; }
	};

	template<typename Rng>
	auto operator|(Rng&& r, address_of_tag_type)
		-> boost::transformed_range<take_address_of<typename Rng::iterator>, Rng>
	{
		return r | transformed(take_address_of<typename Rng::iterator>());
	}

	template<typename Rng>
	auto operator|(Rng& r, address_of_tag_type)
		-> boost::transformed_range<take_address_of<typename Rng::iterator>, Rng>
	{
		return r | transformed(take_address_of<typename Rng::iterator>());
	}
}

namespace {
	const auto address_of = address_of_detail::address_of_tag_type{};
}
}
