// Copyright (c) 2009, Thomas Goyne
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file charset_conv.cpp
/// @brief Iconv-based implementation of character set conversions
/// @ingroup utility
///

#include "config.h"

#include "charset_conv.h"

#ifndef AGI_PRE
#include <errno.h>
#include <stdint.h>

#include <wx/intl.h>
#endif

class AegisubCSConvImpl : public AegisubCSConv {
public:
	AegisubCSConvImpl() { }
};

AegisubCSConv::AegisubCSConv()
: conv("wchar_t", "")
{
}

size_t AegisubCSConv::ToWChar(wchar_t *dst, size_t dstSize, const char *src, size_t srcLen) const {
	throw agi::charset::UnsupportedConversion("Cannot convert to local with csConvLocal");
}

/// @brief Convert a string from wide characters to multibyte
/// @param dst     Destination buffer
/// @param dstSize Length of destination buffer in bytes
/// @param src     Source wide character string
/// @param srcLen  Length in wchar_ts of source, or -1 to autodetect
/// @return The number of bytes needed to store the string in the target charset
size_t AegisubCSConv::FromWChar(char *dst, size_t dstSize, const wchar_t *src, size_t srcLen) const {
	try {
		if (srcLen != (size_t)-1) {
			if (src[srcLen - 1] == 0) srcLen -= 1;
			srcLen *= sizeof(wchar_t);
		}
		if (dstSize == 0) {
			return conv.RequiredBufferSize(reinterpret_cast<const char*>(src), srcLen);
		}
		return conv.Convert(reinterpret_cast<const char*>(src), srcLen, dst, dstSize);
	}
	catch (agi::charset::ConvError const&) {
		return (size_t)-1;
	}
}
static AegisubCSConvImpl localConv;
AegisubCSConv& csConvLocal = localConv;
