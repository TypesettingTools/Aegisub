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

/// @file text_file_reader.cpp
/// @brief Read plain text files line by line
/// @ingroup utility
///

#include "config.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <string>

#include <wx/string.h>

#include <libaegisub/io.h>
#include <libaegisub/log.h>

#include "charset_detect.h"
#include "compat.h"
#include "text_file_reader.h"

TextFileReader::TextFileReader(wxString const& filename, wxString encoding, bool trim)
: trim(trim)
{
	if (encoding.empty()) encoding = CharSetDetect::GetEncoding(filename);
	file.reset(agi::io::Open(from_wx(filename), true));
	iter = agi::line_iterator<wxString>(*file, from_wx(encoding));
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
