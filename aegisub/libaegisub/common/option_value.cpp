// Copyright (c) 2010, Niels M Hansen <nielsm@aegisub.org>
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
// $Id$

/// @file option_value.cpp
/// @brief Container for holding an actual option value.
/// @ingroup libaegisub

#include "libaegisub/option_value.h"

namespace agi {

void OptionValue::NotifyChanged() {
	for (ChangeListenerSet::const_iterator nfcb = listeners.begin(); nfcb != listeners.end(); ++nfcb) {
		(nfcb->first->*(nfcb->second))(*this);
	}
}

void OptionValue::Subscribe(OptionValueListener *listener, OptionValueListener::ChangeEvent function) {
	assert(listeners.find(listener) == listeners.end());
	listeners[listener] = function;
}

void OptionValue::Unsubscribe(OptionValueListener *listener, OptionValueListener::ChangeEvent function) {
	assert(listeners.find(listener) != listeners.end() && listeners[listener] == function);
	listeners.erase(listener);
}

};
