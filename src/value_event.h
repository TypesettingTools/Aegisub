// Copyright (c) 2015, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <wx/event.h>

/// A wxEvent which holds a single templated value
template <typename T> class ValueEvent : public wxEvent {
	const T value;

  public:
	ValueEvent(wxEventType type, int id, T value) : wxEvent(id, type), value(std::move(value)) {}

	wxEvent* Clone() const override;
	T const& Get() const { return value; }
};

// Defined out-of-line so that `extern template` can suppress the emission of
// the vtable in every object file that includes the declaration
template <typename T> wxEvent* ValueEvent<T>::Clone() const {
	return new ValueEvent<T>(*this);
}

#define AGI_DECLARE_EVENT(evt_type, value_type) \
	wxDECLARE_EVENT(evt_type, ValueEvent<value_type>); \
	extern template class ValueEvent<value_type>;
#define AGI_DEFINE_EVENT(evt_type, value_type) \
	wxDEFINE_EVENT(evt_type, ValueEvent<value_type>); \
	template class ValueEvent<value_type>;
