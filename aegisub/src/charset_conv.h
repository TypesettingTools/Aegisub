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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://www.aegisub.net/
// Contact: mailto:zeratul@cellosoft.com
//

#ifndef AEGISUB_CHARSET_CONV_H
#define AEGISUB_CHARSET_CONV_H

#include <iconv.h>
#include <wchar.h>
#include <wx/arrstr.h>
#include <wx/thread.h>
#include <wx/string.h>
#include <wx/strconv.h>

#include "aegisub_endian.h"

#if !defined(_LIBICONV_VERSION) || _LIBICONV_VERSION < 0x010A || defined(LIBICONV_PLUG)
#define ICONV_POSIX
#endif

class AegisubCSConv : public wxMBConv {
public:
	// By default, any conversion that would be lossy will fail
	// When enableSubst is true, conversions to multibyte with a sufficiently large buffer
	// are guaranteed to succeed, with characters dropped or changed as needed to fit the
	// string into the target encoding.
	AegisubCSConv(const wxChar *mbEncName, bool enableSubst = false);
	virtual ~AegisubCSConv();

	// wxMBConv implementation; see strconv.h for usage details
	virtual size_t ToWChar(wchar_t *dst, size_t dstLen, const char *src, size_t srcLen = wxNO_LEN) const;
	virtual size_t FromWChar(char *dst, size_t dstLen, const wchar_t *src, size_t srcLen = wxNO_LEN) const;
	virtual size_t GetMBNulLen() const;
	virtual wxMBConv *Clone() const;

	// Get the length (in bytes) of a null-terminated string whose encoding is mbEncName
	size_t MBBuffLen(const char *str) const;

	// Get a list of support encodings with somewhat user-friendly names
	static wxArrayString GetEncodingsList();
	// Get a list of all encodings supported by iconv
	static wxArrayString GetAllSupportedEncodings();
	// Map a user-friendly encoding name to iconv's name
	static wxString GetRealEncodingName(wxString name);

protected:
	iconv_t m2w, w2m;

private:
	wxString wcCharsetName;
	wxString mbCharsetName;
	size_t   mbNulLen;
	bool     enableSubst;

	size_t doConversion(iconv_t cd, char *dst, size_t dstSize, char *src, size_t srcSize) const;
	size_t iconvWrapper(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) const;

	static void ucToMbFallback(
		unsigned int code,
		void (*callback) (const char *buf, size_t buflen, void* callback_arg),
		void *callback_arg,
		void *convPtr);
	char invalidRep[8];
	size_t invalidRepSize;

#ifndef ICONV_POSIX
	iconv_fallbacks fallbacks;
#endif

#if wxUSE_THREADS
	// While iconv itself is thread-safe, using the same iconv_t on multiple threads is not
	wxMutex iconvMutex;
#endif
};

// Predefined conversion for the current locale. Should be a drop-in replacement for wxConvLocal
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

#endif
