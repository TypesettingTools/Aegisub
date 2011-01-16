// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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
// $Id: scoped_ptr.h 153 2010-12-08 14:45:38Z verm $

/// @file scoped_ptr.h
/// @brief
/// @ingroup libaegisub

#pragma once

namespace agi {

/// @class scoped_ptr
/// @brief auto_ptr without the transfer of ownership semantics
template<class T>
class scoped_ptr {
	T* ptr;
	scoped_ptr(scoped_ptr const&);
	scoped_ptr& operator=(scoped_ptr const&);
public:
	typedef T element_type;

	T& operator*() const {return *ptr; }
	T* operator->() const { return ptr; }
	T* get() const { return ptr; }

	void reset(T *p = NULL) {
		delete ptr;
		ptr = p;
	}

	void swap(scoped_ptr &b) { using std::swap; swap(ptr, b.ptr); }

	explicit scoped_ptr(T *ptr = NULL) : ptr(ptr){ }
	~scoped_ptr() { delete ptr; }
};

template<class T>
inline void swap(scoped_ptr<T> &a, scoped_ptr<T> &b) {
	a.swap(b);
}
}
