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

/// @file charset_conv.h
/// @brief Wrapper for libiconv to present a more C++-friendly API
/// @ingroup libaegisub

#pragma once

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <libaegisub/exception.h>

namespace agi {
	namespace charset {

DEFINE_BASE_EXCEPTION_NOINNER(ConvError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(UnsupportedConversion, ConvError, "iconv/unsupported")
DEFINE_SIMPLE_EXCEPTION_NOINNER(ConversionFailure, ConvError, "iconv/failed")
DEFINE_SIMPLE_EXCEPTION_NOINNER(BufferTooSmall, ConversionFailure, "iconv/failed/E2BIG")
DEFINE_SIMPLE_EXCEPTION_NOINNER(BadInput, ConversionFailure, "iconv/failed/EILSEQ")
DEFINE_SIMPLE_EXCEPTION_NOINNER(BadOutput, ConversionFailure, "iconv/failed/EINVAL")

typedef void* iconv_t;

/// Helper class that abstracts away the differences between libiconv and
/// POSIX iconv implementations
struct Converter {
	virtual ~Converter() { }
	virtual size_t Convert(const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft) = 0;
};

/// @brief A C++ wrapper for iconv
class IconvWrapper {
	size_t toNulLen;
	size_t fromNulLen;
	std::unique_ptr<Converter> conv;

public:
	/// @brief Create a converter
	/// @param sourceEncoding Source encoding name, may be a pretty name
	/// @param destEncoding   Destination encoding name, may be a pretty name
	/// @param enableSubst    If true, when possible characters will be
	///                       mutilated or dropped rather than a letting a
	///                       conversion fail
	IconvWrapper(const char* sourceEncoding, const char* destEncoding, bool enableSubst = true);
	~IconvWrapper();

	/// @brief Convert a string from the source to destination charset
	/// @param source String to convert
	/// @return Converted string. Note that std::string always uses a single byte
	///         terminator, so c_str() may not return a valid string if the dest
	///         charset has wider terminators
	std::string Convert(std::string const& source);
	/// @brief Convert a string from the source to destination charset
	/// @param source String to convert
	/// @param[out] dest String to place the result in
	void Convert(std::string const& source, std::string &dest);
	size_t Convert(const char* source, size_t sourceSize, char* dest, size_t destSize);
	/// Bare wrapper around iconv; see iconv documention for details
	size_t Convert(const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);

	/// @brief Get the required buffer size required to fit the source string in the target charset
	/// @param source A string in the source charset
	/// @param sourceSize Length of the source in bytes
	/// @return Bytes required, including NUL terminator if applicable
	size_t RequiredBufferSize(const char* source, size_t sourceSize);
	/// @brief Get the required buffer size required to fit the source string in the target charset
	/// @param str A string in the source charset
	/// @return Bytes required, not including space needed for NUL terminator
	size_t RequiredBufferSize(std::string const& str);

	/// Encoding-aware strlen for strings encoding in the source charset
	size_t SrcStrLen(const char* str);
	/// Encoding-aware strlen for strings encoding in the destination charset
	size_t DstStrLen(const char* str);
};

/// Is the conversion from src to dst supported by the linked iconv library?
/// @param src Source encoding name
/// @param dst Destination encoding name
/// @return false if either charset is not supported or the conversion cannot be done directly, true otherwise
bool IsConversionSupported(const char *src, const char *dst);

/// Get a list of supported encodings with user-friendly names
template<class T>
T const& GetEncodingsList() {
	static T name_list;
	if (name_list.empty()) {
#		define ADD(pretty, real) if (IsConversionSupported(real, "utf-8")) name_list.push_back(pretty)
#		include <libaegisub/charsets.def>
#		undef ADD
	}
	return name_list;
}

	}
}
