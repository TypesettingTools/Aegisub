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

#pragma once

#include <boost/intrusive/list.hpp>

namespace agi {

template<typename T>
class owning_intrusive_list final : private boost::intrusive::make_list<T, boost::intrusive::constant_time_size<false>>::type {
	typedef typename boost::intrusive::make_list<T, boost::intrusive::constant_time_size<false>>::type base;
public:
	using base::back;
	using base::begin;
	using base::cbegin;
	using base::cend;
	using base::crbegin;
	using base::crend;
	using base::empty;
	using base::end;
	using base::front;
	using base::insert;
	using base::iterator_to;
	using base::merge;
	using base::push_back;
	using base::push_front;
	using base::rbegin;
	using base::rend;
	using base::reverse;
	using base::s_iterator_to;
	using base::shift_backwards;
	using base::shift_forward;
	using base::size;
	using base::sort;
	using base::splice;
	using base::swap;

	using typename base::const_node_ptr;
	using typename base::const_pointer;
	using typename base::const_reverse_iterator;
	using typename base::node;
	using typename base::node_algorithms;
	using typename base::node_ptr;
	using typename base::node_traits;
	using typename base::pointer;
	using typename base::reference;
	using typename base::reverse_iterator;
	using typename base::size_type;
	using typename base::value_type;
	using typename base::iterator;
	using typename base::const_iterator;
	using typename base::const_reference;
	using typename base::difference_type;

	iterator erase(const_iterator b, const_iterator e) { return this->erase_and_dispose(b, e, [](T *e) { delete e; }); }
	iterator erase(const_iterator b, const_iterator e, difference_type n) { return this->erase_and_dispose(b, e, n, [](T *e) { delete e; }); }
	iterator erase(const_iterator i) { return this->erase_and_dispose(i, [](T *e) { delete e; }); }
	void clear() { this->clear_and_dispose([](T *e) { delete e; }); }
	void pop_back() { this->pop_back_and_dispose([](T *e) { delete e; }); }
	void pop_front() { this->pop_front_and_dispose([](T *e) { delete e; }); }
	void remove(const_reference value) { return this->remove_and_dispose(value, [](T *e) { delete e; }); }
	void unique() { this->unique_and_dispose([](T *e) { delete e; }); }

	template<class Pred> void remove_if(Pred&& pred) {
		this->remove_if_and_dispose(std::forward<Pred>(pred), [](T *e) { delete e; });
	}

	template<class BinaryPredicate> void unique(BinaryPredicate&& pred) {
		this->unique_and_dispose(std::forward<BinaryPredicate>(pred), [](T *e) { delete e; });
	}

	~owning_intrusive_list() {
		clear();
	}
};

}

