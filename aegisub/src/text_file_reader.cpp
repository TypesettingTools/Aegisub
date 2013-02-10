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

#include "text_file_reader.h"

#include "charset_detect.h"

#include <libaegisub/io.h>

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

TextFileReader::TextFileReader(agi::fs::path const& filename, std::string encoding, bool trim)
: trim(trim)
{
	if (encoding.empty())
		encoding = CharSetDetect::GetEncoding(filename);
	file = agi::io::Open(filename, true);
	iter = agi::line_iterator<std::string>(file, encoding);
}

TextFileReader::~TextFileReader() {
}

std::string TextFileReader::ReadLineFromFile() {
	std::string str = *iter;
	++iter;
	if (trim)
		boost::trim(str);
	if (boost::starts_with(str, "\xEF\xBB\xBF"))
		str.erase(0, 3);
	return str;
}
