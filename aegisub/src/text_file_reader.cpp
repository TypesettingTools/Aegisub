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

/// @file text_file_reader.cpp
/// @brief Read plain text files line by line
/// @ingroup utility
///

#include "config.h"

#ifndef AGI_PRE
#include <assert.h>
#include <errno.h>

#include <algorithm>
#include <string>

#include <wx/string.h>
#endif

#include <libaegisub/io.h>
#include <libaegisub/log.h>

#include "charset_detect.h"
#include "compat.h"
#include "text_file_reader.h"

TextFileReader::TextFileReader(wxString const& filename, wxString encoding, bool trim)
: trim(trim)
, isBinary(false)
{
	if (encoding.empty()) encoding = CharSetDetect::GetEncoding(filename);
	if (encoding == L"binary") {
		isBinary = true;
		return;
	}
	file.reset(agi::io::Open(STD_STR(filename)));
	iter = agi::line_iterator<wxString>(*file, STD_STR(encoding));
}

TextFileReader::~TextFileReader() {
}

wxString TextFileReader::ReadLineFromFile() {
	wxString str = *iter;
	++iter;
	if (trim) str.Trim(true).Trim(false);
	if (str.StartsWith(L"\uFEFF")) str = str.Mid(1);
	return str;
}

namespace agi {
#ifdef _WIN32
	template<> void line_iterator<wxString>::init() {
		conv.reset(new agi::charset::IconvWrapper(encoding.c_str(), "utf-16le"));
	}
	template<> bool line_iterator<wxString>::convert(std::string &str) {
		value = wxString(str.c_str(), wxMBConvUTF16LE(), str.size());
		return true;
	}
#else
	template<> bool line_iterator<wxString>::convert(std::string &str) {
		value = str;
		return true;
	}
#endif
}
