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

#include "charset_conv.h"

#include <stdint.h>
#include <errno.h>
#include <wx/hashmap.h>
#include <wx/intl.h>

WX_DECLARE_STRING_HASH_MAP(wxString, PrettyNamesHash);

#if wxUSE_THREADS

/// DOCME
static wxMutex encodingListMutex;
#endif


/// DOCME
static const iconv_t iconv_invalid = (iconv_t)-1;

/// DOCME
static const size_t  iconv_failed  = (size_t)-1;

/// DOCME
#define ICONV_CONST_CAST(a) const_cast<ICONV_CONST char *>(a)

#ifndef ICONV_POSIX
static int addEncoding(unsigned int namescount, const char * const * names, void* data);
#endif

/// DOCME
static wxArrayString   *supportedEncodings = NULL;

/// DOCME
static wxArrayString   *prettyEncodingList = NULL;

/// DOCME
static PrettyNamesHash *prettyEncodingHash = NULL;


/// @brief DOCME
/// @param mbEncName   
/// @param enableSubst 
///
AegisubCSConv::AegisubCSConv(const wxChar *mbEncName, bool enableSubst)
:	mbCharsetName(GetRealEncodingName(mbEncName)), mbNulLen(0), enableSubst(enableSubst)
{
	wcCharsetName = wxString::FromAscii(WCHAR_T_ENCODING);

	m2w = iconv_open(wcCharsetName.ToAscii(), mbCharsetName.ToAscii());
	w2m = iconv_open(mbCharsetName.ToAscii(), wcCharsetName.ToAscii());

	if (m2w == iconv_invalid || w2m == iconv_invalid) {
		if (m2w != iconv_invalid) iconv_close(m2w);
		if (w2m != iconv_invalid) iconv_close(w2m);

		throw wxString::Format(_T("Character set %s is not supported."), mbEncName);
	}

	if (enableSubst) {
		invalidRepSize = FromWChar(invalidRep, sizeof(invalidRep), L"?") - GetMBNulLen();

#ifndef ICONV_POSIX
		fallbacks.data = this;
		fallbacks.mb_to_uc_fallback = NULL;
		fallbacks.mb_to_wc_fallback = NULL;
		fallbacks.uc_to_mb_fallback = ucToMbFallback;
		fallbacks.wc_to_mb_fallback = NULL;
#endif
	}
}

/// @brief DOCME
///
AegisubCSConv::~AegisubCSConv() {
	if (m2w != iconv_invalid) iconv_close(m2w);
	if (w2m != iconv_invalid) iconv_close(w2m);
}

/// @brief DOCME
/// @return 
///
wxMBConv * AegisubCSConv::Clone() const {
	AegisubCSConv *c = new AegisubCSConv(mbCharsetName);
	c->mbNulLen = mbNulLen;
	return c;
}


/// @brief Calculate the size of NUL in the target encoding via iconv
/// @return 
///
size_t AegisubCSConv::GetMBNulLen() const {
	if (mbNulLen == 0) {
		const wchar_t nulStr[] = L"";
		char outBuff[8];
		size_t inLen  = sizeof(wchar_t);
		size_t outLen = sizeof(outBuff);
		char * inPtr  = (char *)nulStr;
		char * outPtr = outBuff;

		size_t res = iconv(w2m, &inPtr, &inLen, &outPtr, &outLen);

		if (res != 0)
			const_cast<AegisubCSConv *>(this)->mbNulLen = (size_t)-1;
		else
			const_cast<AegisubCSConv *>(this)->mbNulLen = sizeof(outBuff) - outLen;
	}
	return mbNulLen;
}


/// @brief Calculate the length (in bytes) of a MB string, not including the terminator
/// @param str 
/// @return 
///
size_t AegisubCSConv::MBBuffLen(const char * str) const {
	size_t nulLen = GetMBNulLen();
	const char *ptr;
	switch (nulLen) {
		case 1:
			return strlen(str);
		case 2:
			for (ptr = str; *reinterpret_cast<const uint16_t *>(ptr) != 0; ptr += 2) ;
			return ptr - str;
		case 4:
			for (ptr = str; *reinterpret_cast<const uint32_t *>(ptr) != 0; ptr += 4) ;
			return ptr - str;
		default:
			return (size_t)-1;
	}
}


/// @brief DOCME
/// @param dst     
/// @param dstSize 
/// @param src     
/// @param srcLen  
/// @return 
///
size_t AegisubCSConv::ToWChar(wchar_t *dst, size_t dstSize, const char *src, size_t srcLen) const {
	return doConversion(
		m2w,
		reinterpret_cast<char *>(dst),
		dstSize * sizeof(wchar_t),
		const_cast<char *>(src),
		srcLen == wxNO_LEN ? MBBuffLen(src) + GetMBNulLen() : srcLen
	) / sizeof(wchar_t);
}


/// @brief DOCME
/// @param dst     
/// @param dstSize 
/// @param src     
/// @param srcLen  
/// @return 
///
size_t AegisubCSConv::FromWChar(char *dst, size_t dstSize, const wchar_t *src, size_t srcLen) const {
	return doConversion(
		w2m,
		dst,
		dstSize,
		reinterpret_cast<char *>(const_cast<wchar_t *>(src)),
		(srcLen == wxNO_LEN ? wcslen(src) + 1 : srcLen) * sizeof(wchar_t)
	);
}


/// @brief DOCME
/// @param cd      
/// @param dst     
/// @param dstSize 
/// @param src     
/// @param srcSize 
/// @return 
///
size_t AegisubCSConv::doConversion(iconv_t cd, char *dst, size_t dstSize, char *src, size_t srcSize) const {
	if (dstSize > 0) {
		return iconvWrapper(cd, &src, &srcSize, &dst, &dstSize);
	}

	// No destination given, so calculate the needed buffer size instead
	char buff[32];
	size_t buffSize = 32;
	size_t charsWritten = 0;
	size_t res;

	do {
		dst = buff;
		dstSize = buffSize;
		res = iconvWrapper(cd, &src, &srcSize, &dst, &dstSize);

		charsWritten += dst - buff;
	} while (res == iconv_failed && errno == E2BIG);

	if (res == iconv_failed) return wxCONV_FAILED;
	return charsWritten;
}


/// @brief DOCME
/// @param cd           
/// @param inbuf        
/// @param inbytesleft  
/// @param outbuf       
/// @param outbytesleft 
/// @return 
///
size_t AegisubCSConv::iconvWrapper(iconv_t cd, char **inbuf, size_t *inbytesleft,
							 char **outbuf, size_t *outbytesleft) const {

#if wxUSE_THREADS
	wxMutexLocker lock(const_cast<AegisubCSConv *>(this)->iconvMutex);
#endif

	char *outbuforig = *outbuf;
	size_t res = iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);

	if (res != iconv_failed)
		return *outbuf - outbuforig;
	if (!enableSubst)
		return iconv_failed;

#ifdef ICONV_POSIX
	if (errno == EILSEQ) {
		throw	_T("One or more characters do not fit in the selected ")
				_T("encoding and the version of iconv Aegisub was built with")
				_T(" does not have useful fallbacks. For best results, ")
				_T("please rebuild Aegisub using a recent version of GNU iconv.");
	}
	return wxCONV_FAILED;
#else
	// Save original errno so we can return it rather than the result from iconvctl
	int err = errno;

	// Some characters in the input string do not exist in the output encoding
	if (res == iconv_failed && err == EILSEQ) {
		// first try transliteration only
		int transliterate = 1;
		iconvctl(cd, ICONV_SET_TRANSLITERATE, &transliterate);
		res = iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
		err = errno;
		transliterate = 0;
		iconvctl(cd, ICONV_SET_TRANSLITERATE, &transliterate);
	}
	if (res == iconv_failed && err == EILSEQ) {
		// Conversion still failed with transliteration enabled, so try our substitution
		iconvctl(cd, ICONV_SET_FALLBACKS, const_cast<iconv_fallbacks *>(&fallbacks));
		res = iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
		err = errno;
		iconvctl(cd, ICONV_SET_FALLBACKS, NULL);
	}
	if (res == iconv_failed && err == EILSEQ) {
		// Conversion still failed, so just drop any invalid characters
		int discard = 1;
		iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &discard);
		res = iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
		err = errno;
		discard = 0;
		iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &discard);
	}

	errno = err;
	if (res == iconv_failed) return wxCONV_FAILED;
	return *outbuf - outbuforig;
#endif
}


/// @brief DOCME
/// @param code          
/// @param buf           
/// @param buflen        
/// @param callback_arg) 
/// @param callback_arg  
/// @param convPtr       
/// @return 
///
void AegisubCSConv::ucToMbFallback(
	unsigned int code,
	void (*callback) (const char *buf, size_t buflen, void* callback_arg),
	void *callback_arg,
	void *convPtr)
{
	// At some point in the future, this should probably switch to a real mapping
	// For now, there's just three cases: BOM to nothing, \ to itself (lol Shift-JIS) and everything else to ?
	if (code == 0xFEFF) return;
	if (code == 0x5C) callback("\\", 1, callback_arg);
	else {
		AegisubCSConv *self = static_cast<AegisubCSConv *>(convPtr);
		callback(self->invalidRep, self->invalidRepSize, callback_arg);
	}
}

#ifndef ICONV_POSIX

/// @brief DOCME
/// @param namescount 
/// @param names      
/// @param data       
/// @return 
///
int addEncoding(unsigned int namescount, const char * const * names, void* data) {
	for (unsigned int i = 0; i < namescount; i++) {
		supportedEncodings->Add(wxString::FromAscii(names[i]));
	}
	return 0;
}
#endif


/// @brief DOCME
/// @return 
///
wxArrayString AegisubCSConv::GetAllSupportedEncodings() {
#if wxUSE_THREADS
	wxMutexLocker lock(encodingListMutex);
#endif
	if (supportedEncodings == NULL) {
		supportedEncodings = new wxArrayString();
#ifndef ICONV_POSIX
		iconvlist(addEncoding, NULL);
		supportedEncodings->Sort();
#endif
	}
	return *supportedEncodings;
}


/// @brief Map pretty names to the real encoding names
/// @param name 
/// @return 
///
wxString AegisubCSConv::GetRealEncodingName(wxString name) {
	if (name.Lower() == _T("local")) return wxLocale::GetSystemEncodingName();
	if (prettyEncodingList == NULL) return name;

	PrettyNamesHash::iterator realName = prettyEncodingHash->find(name);
	if (realName != prettyEncodingHash->end()) {
		return realName->second;
	}
	return name;
}


/// @brief DOCME
///
wxArrayString AegisubCSConv::GetEncodingsList() {
#if wxUSE_THREADS
	wxMutexLocker lock(encodingListMutex);
#endif
	if (prettyEncodingList == NULL) {
		struct { const char *pretty, *real; } encodingNames[] = {
			{"Unicode (UTF-8)",                   "utf-8"},
			{"Unicode (UTF-16)",                  "utf-16"},
			{"Unicode (UTF-16BE)",                "utf-16be"},
			{"Unicode (UTF-16LE)",                "utf-16le"},
			{"Unicode (UTF-32)",                  "utf-32"},
			{"Unicode (UTF-32BE)",                "utf-32be"},
			{"Unicode (UTF-32LE)",                "utf-32le"},
			{"Unicode (UTF-7)",                   "utf-7"},

			{"Arabic (IBM-864)",                  "ibm864"},
			{"Arabic (IBM-864-I)",                "ibm864i"},
			{"Arabic (ISO-8859-6)",               "iso-8859-6"},
			{"Arabic (ISO-8859-6-E)",             "iso-8859-6-e"},
			{"Arabic (ISO-8859-6-I)",             "iso-8859-6-i"},
			{"Arabic (Langbox ISO-8859-6.16)",    "x-iso-8859-6-16"},
			{"Arabic (Langbox ISO-8859-6.8x)",    "x-iso-8859-6-8-x"},
			{"Arabic (MacArabic)",                "x-mac-arabic"},
			{"Arabic (Windows-1256)",             "windows-1256"},

			{"Armenian (ARMSCII-8)",              "armscii-8"},

			{"Baltic (ISO-8859-13)",              "iso-8859-13"},
			{"Baltic (ISO-8859-4)",               "iso-8859-4"},
			{"Baltic (Windows-1257)",             "windows-1257"},

			{"Celtic (ISO-8859-14)",              "iso-8859-14"},

			{"Central European (IBM-852)",        "ibm852"},
			{"Central European (ISO-8859-2)",     "iso-8859-2"},
			{"Central European (MacCE)",          "x-mac-ce"},
			{"Central European (Windows-1250)",   "windows-1250"},

			{"Chinese Simplified (GB18030)",      "gb18030"},
			{"Chinese Simplified (GB2312)",       "gb2312"},
			{"Chinese Simplified (GBK)",          "x-gbk"},
			{"Chinese Simplified (HZ)",           "hz-gb-2312"},
			{"Chinese Simplified (ISO-2022-CN)",  "iso-2022-cn"},
			{"Chinese Traditional (Big5)",        "big5"},
			{"Chinese Traditional (Big5-HKSCS)",  "big5-hkscs"},
			{"Chinese Traditional (EUC-TW)",      "x-euc-tw"},

			{"Croatian (MacCroatian)",            "x-mac-croatian"},

			{"Cyrillic (IBM-855)",                "ibm855"},
			{"Cyrillic (ISO-8859-5)",             "iso-8859-5"},
			{"Cyrillic (ISO-IR-111)",             "iso-ir-111"},
			{"Cyrillic (KOI8-R)",                 "koi8-r"},
			{"Cyrillic (MacCyrillic)",            "x-mac-cyrillic"},
			{"Cyrillic (Windows-1251)",           "windows-1251"},
			{"Cyrillic/Russian (CP-866)",         "ibm866"},
			{"Cyrillic/Ukrainian (KOI8-U)",       "koi8-u"},
			{"Cyrillic/Ukrainian (MacUkrainian)", "x-mac-ukrainian"},

			{"English (US-ASCII)",                "us-ascii"},

			{"Farsi (MacFarsi)",                  "x-mac-farsi"},

			{"Georgian (GEOSTD8)",                "geostd8"},

			{"Greek (ISO-8859-7)",                "iso-8859-7"},
			{"Greek (MacGreek)",                  "x-mac-greek"},
			{"Greek (Windows-1253)",              "windows-1253"},

			{"Gujarati (MacGujarati)",            "x-mac-gujarati"},
			{"Gurmukhi (MacGurmukhi)",            "x-mac-gurmukhi"},

			{"Hebrew (IBM-862)",                  "ibm862"},
			{"Hebrew (ISO-8859-8-E)",             "iso-8859-8-e"},
			{"Hebrew (ISO-8859-8-I)",             "iso-8859-8-i"},
			{"Hebrew (MacHebrew)",                "x-mac-hebrew"},
			{"Hebrew (Windows-1255)",             "windows-1255"},
			{"Hebrew Visual (ISO-8859-8)",        "iso-8859-8"},

			{"Hindi (MacDevanagari)",             "x-mac-devanagari"},
			{"Hindi (SunDevanagari)",             "x-sun-unicode-india-0"},

			{"Icelandic (MacIcelandic)",          "x-mac-icelandic"},

			{"Japanese (EUC-JP)",                 "euc-jp"},
			{"Japanese (ISO-2022-JP)",            "iso-2022-jp"},
			{"Japanese (Shift_JIS)",              "shift_jis"},

			{"Korean (EUC-KR)",                   "euc-kr"},
			{"Korean (ISO-2022-KR)",              "iso-2022-kr"},
			{"Korean (JOHAB)",                    "x-johab"},
			{"Korean (UHC)",                      "x-windows-949"},

			{"Nordic (ISO-8859-10)",              "iso-8859-10"},

			{"Romanian (ISO-8859-16)",            "iso-8859-16"},
			{"Romanian (MacRomanian)",            "x-mac-romanian"},

			{"South European (ISO-8859-3)",       "iso-8859-3"},

			{"Thai (IBM-874)",                    "ibm874"},
			{"Thai (ISO-8859-11)",                "iso-8859-11"},
			{"Thai (TIS-620)",                    "tis-620"},
			{"Thai (Windows-874)",                "windows-874"},

			{"Turkish (IBM-857)",                 "ibm857"},
			{"Turkish (ISO-8859-9)",              "iso-8859-9"},
			{"Turkish (MacTurkish)",              "x-mac-turkish"},
			{"Turkish (Windows-1254)",            "windows-1254"},

			{"Vietnamese (TCVN)",                 "x-viet-tcvn5712"},
			{"Vietnamese (VISCII)",               "viscii"},
			{"Vietnamese (VPS)",                  "x-viet-vps"},
			{"Vietnamese (Windows-1258)",         "windows-1258"},

			{"Western (IBM-850)",                 "ibm850"},
			{"Western (ISO-8859-1)",              "iso-8859-1"},
			{"Western (ISO-8859-15)",             "iso-8859-15"},
			{"Western (MacRoman)",                "x-mac-roman"},
			{"Western (Windows-1252)",            "windows-1252"},

			{NULL,                                NULL}
		};

		PrettyNamesHash *map = new PrettyNamesHash(100);
		wxArrayString *arr = new wxArrayString();
		arr->Add(_T("Local"));

		for (int i = 0; encodingNames[i].real != NULL; i++) {
			// Verify that iconv actually supports this encoding
			iconv_t cd = iconv_open(encodingNames[i].real, WCHAR_T_ENCODING);
			if (cd == iconv_invalid) continue;
			iconv_close(cd);

			cd = iconv_open(WCHAR_T_ENCODING, encodingNames[i].real);
			if (cd == iconv_invalid) continue;
			iconv_close(cd);

			wxString pretty = wxString::FromAscii(encodingNames[i].pretty);
			arr->Add(pretty);
			(*map)[pretty] = wxString::FromAscii(encodingNames[i].real);
		}

		prettyEncodingList = arr;
		prettyEncodingHash = map;
	}
	return *prettyEncodingList;
}
static AegisubCSConv localConv(_T("Local"), false);
AegisubCSConv& csConvLocal(localConv);


