// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace agi {
	namespace of_type_detail {
		using namespace boost::adaptors;

		/// Tag type returned from of_type<T>() to select the operator| overload
		template<class T> struct of_type_tag {};

		/// Take the address of a value and dynamic_cast it to Type*
		template<class Type>
		struct cast_to {
			typedef Type *result_type;

			template<class InType> Type *operator()(InType &ptr) const {
				return dynamic_cast<Type *>(&ptr);
			}

			template<class InType> Type *operator()(InType *ptr) const {
				return dynamic_cast<Type *>(ptr);
			}
		};

		template<class Type>
		inline bool not_null(Type *ptr) {
			return !!ptr;
		}

		// Defined here for ADL reasons, since we don't want the tag type in
		// the top-level agi namespace (and it lets us get away with the using
		// namespace above)
		template<class Rng, class Type>
		inline auto operator|(Rng& r, of_type_tag<Type>)
			-> decltype(r | transformed(cast_to<Type>()) | filtered(not_null<Type>))
		{
			return r | transformed(cast_to<Type>()) | filtered(not_null<Type>);
		}

		// const overload of the above
		template<class Rng, class Type>
		inline auto operator|(Rng const& r, of_type_tag<Type>)
			-> decltype(r | transformed(cast_to<const Type>()) | filtered(not_null<const Type>))
		{
			return r | transformed(cast_to<const Type>()) | filtered(not_null<const Type>);
		}
	}

	template<class T>
	inline of_type_detail::of_type_tag<T> of_type() {
		return of_type_detail::of_type_tag<T>();
	}
}
