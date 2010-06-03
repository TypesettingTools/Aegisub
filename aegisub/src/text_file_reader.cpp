// Copyright (c) 2010, Rodrigo Braz Monteiro, Thomas Goyne
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

#ifndef AGI_PRE
#include <assert.h>
#include <errno.h>

#include <algorithm>
#include <fstream>
#include <string>
#endif

#include <libaegisub/log.h>

#include "charset_conv.h"
#include "charset_detect.h"
#include "text_file_reader.h"

TextFileReader::TextFileReader(wxString const& filename, wxString encoding, bool trim)
: isBinary(false)
, conv()
, trim(trim)
, readComplete(false)
, currout(0)
, outptr(0)
, currentLine(0)
{
#ifdef __WINDOWS__
	file.open(filename.wc_str(),std::ios::in | std::ios::binary);
#else
	file.open(wxFNCONV(filename),std::ios::in | std::ios::binary);
#endif
	if (!file.is_open()) throw L"Failed opening file for reading.";

	if (encoding.IsEmpty()) encoding = CharSetDetect::GetEncoding(filename);
	if (encoding == L"binary") {
		isBinary = true;
		return;
	}
	conv.reset(new agi::charset::IconvWrapper(encoding.c_str(), "wchar_t"));
}

TextFileReader::~TextFileReader() {
}

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
		// Without this const_cast the wrong overload is chosen
		size_t ret = conv->Convert(const_cast<const char**>(&inptr), &inbytesleft, reinterpret_cast<char **>(&outptr), &outbytesleft);
		if (ret != (size_t)-1) break;

		int err = errno;
		// If 64 chars do not fit into 256 wchar_ts the environment is so bizzare that doing
		// anything is probably futile
		assert(err != E2BIG);

		// (Hopefully) the edge of the buffer happened to split a multibyte character, so keep
		// adding one byte to the input buffer until either it succeeds or we add enough bytes to
		// complete any character
		if (++bytesAdded > 3)
			throw wxString::Format(L"Invalid input character found near line %u", currentLine);

		file.read(inptr + inbytesleft, 1);
		inbytesleft++;
	} while (!file.eof() && file.gcount());

	if (outptr > outbuf)
		return *currout;

	throw wxString::Format(L"Invalid input character found near line %u", currentLine);
}

wxString TextFileReader::ReadLineFromFile() {
	wxString buffer;
	buffer.Alloc(1024);

	currentLine++;
	// Read a line
	wchar_t ch;
	bool first = true;
	// This doesn't work for \r deliminated files, but it's very unlikely
	// that we'll run into one of those
	for (ch = GetWChar(); ch != L'\n' && ch != 0; ch = GetWChar()) {
		if (ch == L'\r') continue;
		// Skip the BOM -- we don't need it as the encoding is already known
		// and it sometimes causes conversion problems
		if (ch == 0xFEFF && first) continue;

		buffer += ch;
		first = false;
	}
	if (ch == 0)
		readComplete = true;

	if (trim) {
		buffer.Trim(true);
		buffer.Trim(false);
	}
	return buffer;
}

bool TextFileReader::HasMoreLines() {
	return !readComplete;
}
