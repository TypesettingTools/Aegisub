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

/// @file text_file_reader.h
/// @see text_file_reader.cpp
/// @ingroup utility
///

#pragma once

#include <fstream>
#include <memory>
#include <string>

#include <libaegisub/fs_fwd.h>
#include <libaegisub/line_iterator.h>

/// @class TextFileReader
/// @brief A line-based text file reader
class TextFileReader {
	std::unique_ptr<std::ifstream> file;
	bool trim;
	agi::line_iterator<std::string> iter;

	TextFileReader(const TextFileReader&);
	TextFileReader& operator=(const TextFileReader&);

public:
	/// @brief Constructor
	/// @param filename File to open
	/// @param enc      Encoding to use, or empty to autodetect
	/// @param trim     Whether to trim whitespace from lines read
	TextFileReader(agi::fs::path const& filename, std::string encoding="", bool trim=true);
	/// @brief Destructor
	~TextFileReader();

	/// @brief Read a line from the file
	/// @return The line, possibly trimmed
	std::string ReadLineFromFile();
	/// @brief Check if there are any more lines to read
	bool HasMoreLines() const { return iter != agi::line_iterator<std::string>(); }
};
