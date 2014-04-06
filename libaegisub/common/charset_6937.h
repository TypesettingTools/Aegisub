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

/// @file charset_6937.h
/// @brief A charset converter for ISO-6937-2
/// @ingroup libaegisub

#include <libaegisub/charset_conv.h>
#include <memory>

namespace agi { namespace charset {

/// @brief A charset converter for ISO-6937-2
///
/// While glibc iconv supports ISO-6937-2, GNU libiconv does not due to that
/// it's not used by anything but old subtitle formats
class Converter6937 : public Converter {
	/// Converter to UCS-4 so that we only have to deal with unicode codepoints
	std::unique_ptr<IconvWrapper> to_ucs4;

	/// Should unsupported characters be replaced with '?'
	const bool subst;

public:
	/// Constructor
	/// @param subst Enable substitution for unsupported characters
	/// @param src Source encoding
	Converter6937(bool subst, const char *src);

	/// Convert a string. Interface is the same as iconv.
	size_t Convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft);
};

} }
