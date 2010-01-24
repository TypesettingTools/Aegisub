// Copyright (c) 2010, Thomas Goyne
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

/// @file charset_conv.h
/// @see charset_conv.cpp
/// @ingroup utility
///

#ifndef AGI_PRE
#include <iconv.h>
#include <wchar.h>

#include <wx/arrstr.h>
#include <wx/string.h>
#include <wx/strconv.h>
#include <wx/thread.h>
#endif

#include "aegisub_endian.h"

#if !defined(_LIBICONV_VERSION) || _LIBICONV_VERSION < 0x010A || defined(LIBICONV_PLUG)
#define ICONV_POSIX
#endif

/// @class iconv_wrapper
/// @brief RAII wrapper for iconv
class iconv_wrapper {
private:
	iconv_t conv;
public:
	iconv_wrapper(const char *to, const char *from)
	: conv(iconv_open(to, from))
	{ }
	iconv_wrapper(wxString const& to, wxString const& from)
	: conv(iconv_open(to.ToAscii(), from.ToAscii()))
	{ }
	iconv_wrapper(const char *to, wxString const& from)
	: conv(iconv_open(to, from.ToAscii()))
	{ }
	iconv_wrapper(wxString const& to, const char *from)
	: conv(iconv_open(to.ToAscii(), from))
	{ }
	~iconv_wrapper() {
		if (conv != (iconv_t)-1) iconv_close(conv);
	}
	operator iconv_t() {
		return conv;
	}
	operator const iconv_t() const {
		return conv;
	}
};

/// @class AegisubCSConv
/// @brief wxMBConv implementation for converting to and from unicode
class AegisubCSConv : public wxMBConv {
public:
	/// @param mbEncName   Multibyte encoding to convert to/from
	/// @param enableSubst Whether to substitute characters when needed.
	/// By default, any conversion that would be lossy will fail
	/// When enableSubst is true, conversions to multibyte with a sufficiently
	/// large buffer are guaranteed to succeed, with characters dropped or
	/// changed as needed to fit the string into the target encoding.
	AegisubCSConv(const wxChar *mbEncName, bool enableSubst = false);

	// wxMBConv implementation; see strconv.h for usage details
	size_t ToWChar(wchar_t *dst, size_t dstLen, const char *src, size_t srcLen = wxNO_LEN) const;
	size_t FromWChar(char *dst, size_t dstLen, const wchar_t *src, size_t srcLen = wxNO_LEN) const;
	size_t GetMBNulLen() const;
	wxMBConv *Clone() const;

	/// @brief Multibyte-aware strlen
	/// @return Length in bytes of str (excluding terminator)
	size_t MBBuffLen(const char *str) const;

	/// @brief Get a list of support encodings with user-friendly names
	static wxArrayString GetEncodingsList();
	/// @brief Get a list of all encodings supported by iconv
	/// Requires GNU iconv for useful results
	static wxArrayString GetAllSupportedEncodings();
	/// @brief Map a user-friendly encoding name to the real encoding name
	static wxString GetRealEncodingName(wxString name);

private:
	// The smattering of mutable variables here are due to that ToWChar and
	// FromWChar are const in wxMBConv, but we require minor mutation for
	// things like locks (as iconv is not thread-safe)
	wxString wcCharsetName;
	wxString mbCharsetName;
	mutable size_t mbNulLen;
	bool enableSubst;

	size_t doConversion(iconv_t cd, char *dst, size_t dstSize, char *src, size_t srcSize) const;
	size_t iconvWrapper(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) const;

	static void ucToMbFallback(
		unsigned int code,
		void (*callback) (const char *buf, size_t buflen, void* callback_arg),
		void *callback_arg,
		void *convPtr);

	/// Replacement character for characters which do not fit in the target
	/// encoding and iconv does not have an appropriate substitute for
	char invalidRep[8];
	size_t invalidRepSize;

#ifndef ICONV_POSIX
	mutable iconv_fallbacks fallbacks;
#endif

#if wxUSE_THREADS
	mutable wxMutex iconvMutex;
#endif

protected:
	iconv_wrapper m2w, w2m;
};

// Predefined conversion for the current locale, intended to be a drop-in
// replacement for wxConvLocal
extern AegisubCSConv& csConvLocal;

#ifdef HAVE_BIG_ENDIAN
#	if SIZEOF_WCHAR_T == 4
#		define WCHAR_T_ENCODING "UTF-32BE"
#	elif SIZEOF_WCHAR_T == 2
#		define WCHAR_T_ENCODING "UTF-16BE"
#	endif
#elif defined(HAVE_LITTLE_ENDIAN)
#	if SIZEOF_WCHAR_T == 4
#		define WCHAR_T_ENCODING "UTF-32LE"
#	elif SIZEOF_WCHAR_T == 2
#		define WCHAR_T_ENCODING "UTF-16LE"
#	endif
#else
#	if SIZEOF_WCHAR_T == 4
#		define WCHAR_T_ENCODING ((Endian::MachineToBig((uint32_t)1) == 1) ? "UTF-32BE" : "UTF-32LE")
#	elif SIZEOF_WCHAR_T == 2
#		define WCHAR_T_ENCODING ((Endian::MachineToBig((uint32_t)1) == 1) ? "UTF-16BE" : "UTF-16LE")
#	endif
#endif
