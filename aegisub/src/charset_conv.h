// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file charset_conv.h
/// @see charset_conv.cpp
/// @ingroup utility
///

#include <wx/string.h>
#include <wx/strconv.h>
#include <wx/thread.h>

#include <libaegisub/charset_conv.h>

/// @class AegisubCSConv
/// @brief wxMBConv implementation for converting to and from unicode
class AegisubCSConv : public wxMBConv {
public:

	// wxMBConv implementation; see strconv.h for usage details
	size_t ToWChar(wchar_t *dst, size_t dstLen, const char *src, size_t srcLen = wxNO_LEN) const;
	size_t FromWChar(char *dst, size_t dstLen, const wchar_t *src, size_t srcLen = wxNO_LEN) const;
	wxMBConv *Clone() const { return nullptr; };

protected:
	AegisubCSConv();
private:
	AegisubCSConv(const AegisubCSConv&);
	AegisubCSConv& operator=(const AegisubCSConv&);
	wxString localCharset;

	mutable wxMutex iconvMutex;

	// ToWChar and FromWChar are const in wxMBConv, but iconv can't be used
	// immutably
	mutable agi::charset::IconvWrapper conv;
};

extern AegisubCSConv& csConvLocal;
