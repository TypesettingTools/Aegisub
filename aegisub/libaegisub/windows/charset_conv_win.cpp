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
// $Id$

/// @file charset_conv_win.h
/// @brief Windows-specific charset conversion stuff
/// @ingroup libaegisub windows

#include <libaegisub/charset_conv_win.h>

namespace agi {
	namespace charset {

std::wstring ConvertW(std::string const& source) {
	static IconvWrapper w32Conv("utf-8", "utf-16le", false);

	std::wstring dest;
	size_t len = w32Conv.RequiredBufferSize(source);
	dest.resize(len / sizeof(wchar_t));
	w32Conv.Convert(source.data(), source.size(), reinterpret_cast<char *>(&dest[0]), len);
	return dest;
}

std::string ConvertW(std::wstring const& source) {
	static IconvWrapper w32Conv("utf-16le", "utf-8", false);

	std::string dest;
	size_t srcLen = source.size() * sizeof(wchar_t);
	const char* src = reinterpret_cast<const char *>(source.c_str());
	size_t len = w32Conv.RequiredBufferSize(src, srcLen);
	dest.resize(len);
	w32Conv.Convert(src, srcLen, &dest[0], len);
	return dest;
}

	}
}
