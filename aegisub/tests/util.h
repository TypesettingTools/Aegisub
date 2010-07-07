// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

/// @file util.cpp
/// @brief Common utilities used in tests.
/// @ingroup util

#include <string>
#include <vector>

#include <stdarg.h>

namespace util {

void copy(const std::string from, const std::string to);
void remove(const std::string& file);

template<typename T>
static std::vector<T> make_vector(int len, ...) {
	std::vector<T> vec(len);

	va_list argp;
	va_start(argp, len);
	for (int i = 0; i < len; i++) {
		vec[i] = va_arg(argp, T);
	}
	va_end(argp);
	return vec;
}

} // namespace util


