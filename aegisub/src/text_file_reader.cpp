// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file text_file_reader.cpp
/// @brief Read plain text files line by line
/// @ingroup utility
///

#include "config.h"

#include <fstream>
#include <algorithm>
#include <string>
#include <assert.h>
#include <errno.h>
#include "text_file_reader.h"
#include "charset_conv.h"


#ifdef WITH_UNIVCHARDET
#include "charset_detect.h"
#endif


/// @brief DOCME
/// @param filename 
/// @param enc      
/// @param trim     
/// @return 
///
TextFileReader::TextFileReader(wxString filename, wxString enc, bool trim)
: encoding(enc), conv((iconv_t)-1), trim(trim), readComplete(false), currout(0), outptr(0), currentLine(0) {
#ifdef __WINDOWS__
	file.open(filename.wc_str(),std::ios::in | std::ios::binary);
#else
	file.open(wxFNCONV(filename),std::ios::in | std::ios::binary);
#endif
	if (!file.is_open()) {
		throw _T("Failed opening file for reading.");
	}

	if (encoding.IsEmpty()) encoding = GetEncoding(filename);
	if (encoding == _T("binary")) return;
	encoding = AegisubCSConv::GetRealEncodingName(encoding);
	conv = iconv_open(WCHAR_T_ENCODING, encoding.ToAscii());
}


/// @brief DOCME
///
TextFileReader::~TextFileReader() {
	if (conv != (iconv_t)-1) iconv_close(conv);
}


/// @brief DOCME
/// @param filename 
/// @return 
///
wxString TextFileReader::GetEncoding(const wxString filename) {
	// Prepare
	unsigned char b[4];
	memset(b, 0, sizeof(b));

	// Read four bytes from file
	std::ifstream ifile;
#ifdef __WINDOWS__
	ifile.open(filename.wc_str());
#else
	ifile.open(wxFNCONV(filename));
#endif
	if (!ifile.is_open()) {
		return _T("unknown");
	}
	ifile.read(reinterpret_cast<char *>(b),4);
	ifile.close();

	// Try to get the byte order mark from them
	if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) return _T("UTF-8");
	else if (b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) return _T("UTF-32LE");
	else if (b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) return _T("UTF-32BE");
	else if (b[0] == 0xFF && b[1] == 0xFE) return _T("UTF-16LE");
	else if (b[0] == 0xFE && b[1] == 0xFF) return _T("UTF-16BE");
	else if (b[0] == 0x2B && b[1] == 0x2F && b[2] == 0x76) return _T("UTF-7");

	// Try to guess UTF-16
	else if (b[0] == 0 && b[1] >= 32 && b[2] == 0 && b[3] >= 32) return _T("UTF-16BE");
	else if (b[0] >= 32 && b[1] == 0 && b[2] >= 32 && b[3] == 0) return _T("UTF-16LE");

	// If any of the first four bytes are under 0x20 (the first printable character),
	// except for 9-13 range, assume binary
	for (int i=0;i<4;i++) {
		if (b[i] < 9 || (b[i] > 13 && b[i] < 32)) return _T("binary");
	}

#ifdef WITH_UNIVCHARDET
	// Use universalchardet library to detect charset
	CharSetDetect det;
	return det.GetEncoding(filename);
#else
	// Fall back to local
	return _T("Local");
#endif
}


/// @brief DOCME
/// @return 
///
wchar_t TextFileReader::GetWChar() {
	// If there's already some converted characters waiting, return the next one
	if (++currout < outptr) {
		return *currout;
	}

	if (file.eof()) return 0;

	// Otherwise convert another block
	char    inbuf[64];
	char    *inptr = inbuf;
	size_t  inbytesleft = sizeof(inbuf) - 4;
	int     bytesAdded = 0;
	memset(inbuf, 0, inbytesleft);

	outptr       = outbuf;
	outbytesleft = sizeof(outbuf);
	currout      = outbuf;

	file.read(inbuf, inbytesleft);
	inbytesleft = file.gcount();
	if (inbytesleft == 0)
		return 0;

	do {
		size_t ret = iconv(conv, &inptr, &inbytesleft, reinterpret_cast<char **>(&outptr), &outbytesleft);
		if (ret != (size_t)-1) break;

		int err = errno;
		// If 64 chars do not fit into 256 wchar_ts the environment is so bizzare that doing
		// anything is probably futile
		assert(err != E2BIG);

		// (Hopefully) the edge of the buffer happened to split a multibyte character, so keep
		// adding one byte to the input buffer until either it succeeds or we add enough bytes to
		// complete any character
		if (++bytesAdded > 3)
			throw wxString::Format(_T("Invalid input character found near line %u"), currentLine);

		file.read(inptr + inbytesleft, 1);
		inbytesleft++;
	} while (!file.eof() && file.gcount());

	if (outptr > outbuf)
		return *currout;

	throw wxString::Format(_T("Invalid input character found near line %u"), currentLine);
}


/// @brief DOCME
/// @return 
///
wxString TextFileReader::ReadLineFromFile() {
	wxString buffer;
	size_t bufAlloc = 1024;
	buffer.Alloc(bufAlloc);

	currentLine++;
	// Read a line
	wchar_t ch;
	size_t len = 0;
	for (ch = GetWChar(); ch != L'\n' && ch != 0; ch = GetWChar()) {
		if (ch == L'\r') continue;
		// Skip the BOM -- we don't need it as the encoding is already known
		// and it sometimes causes conversion problems
		if (ch == 0xFEFF && len == 0) continue;

		if (len >= bufAlloc - 1) {
			bufAlloc *= 2;
			buffer.Alloc(bufAlloc);
		}
		buffer += ch;
		len++;
	}
	if (ch == 0)
		readComplete = true;

	// Trim
	if (trim) {
		buffer.Trim(true);
		buffer.Trim(false);
	}
	return buffer;
}


/// @brief DOCME
/// @return 
///
bool TextFileReader::HasMoreLines() {
	return !readComplete;
}


/// @brief DOCME
/// @param encoding
/// @return 
///
void TextFileReader::EnsureValid(wxString enc) {
	if (enc == _T("binary")) return;

	enc = AegisubCSConv::GetRealEncodingName(enc);
	iconv_t cd = iconv_open(WCHAR_T_ENCODING, enc.ToAscii());
	bool canOpen = cd != (iconv_t)-1;
	iconv_close(cd);
	if (!canOpen) {
		throw wxString::Format(_T("Character set %s is not supported."), enc.c_str());
	}
}


/// @brief DOCME
///
wxString TextFileReader::GetCurrentEncoding() {
	return encoding;
}


