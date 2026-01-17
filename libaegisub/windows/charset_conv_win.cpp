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

/// @file charset_conv_win.cpp
/// @brief Windows-specific charset conversion stuff
/// @ingroup libaegisub windows

#include <libaegisub/charset_conv_win.h>

namespace {
std::string from_w(agi::charset::IconvWrapper &w32Conv, std::wstring_view source) {
	return w32Conv.Convert(std::string_view(reinterpret_cast<const char *>(source.data()), source.size() * sizeof(wchar_t)));
}
}

namespace agi::charset {

std::wstring ConvertW(std::string_view source) {
	static IconvWrapper w32Conv("utf-8", "utf-16le", false);

	std::string result = w32Conv.Convert(source);
	return std::wstring(reinterpret_cast<const wchar_t *>(result.data()), result.size() / sizeof(wchar_t));
}

std::string ConvertW(std::wstring_view source) {
	static IconvWrapper w32Conv("utf-16le", "utf-8", false);
	return from_w(w32Conv, source);
}

std::string ConvertLocal(std::wstring_view source) {
	static IconvWrapper w32Conv("utf-16le", "char", false);
	return from_w(w32Conv, source);
}

	}
