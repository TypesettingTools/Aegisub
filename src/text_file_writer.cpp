// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file text_file_writer.cpp
/// @brief Write plain text files line by line
/// @ingroup utility
///

#include "text_file_writer.h"

#include "options.h"

#include <libaegisub/io.h>
#include <libaegisub/charset_conv.h>
#include <libaegisub/make_unique.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

TextFileWriter::TextFileWriter(agi::fs::path const& filename, std::string encoding)
: file(new agi::io::Save(filename, true))
, ostr(file->Get())
{
	if (encoding.empty())
		encoding = OPT_GET("App/Save Charset")->GetString();
	if (boost::iequals(encoding, "utf-8"))
		conv = agi::make_unique<agi::charset::IconvWrapper>("utf-8", encoding.c_str(), true);

	try {
		// Write the BOM
		WriteLineToFile("\xEF\xBB\xBF", false);
	}
	catch (agi::charset::ConversionFailure&) {
		// If the BOM could not be converted to the target encoding it isn't needed
	}
}

TextFileWriter::TextFileWriter(std::ostream &ostr) : ostr(ostr) {
	WriteLineToFile("\xEF\xBB\xBF", false);
}

TextFileWriter::~TextFileWriter() {
	// Explicit empty destructor required with a unique_ptr to an incomplete class
}

void TextFileWriter::WriteLineToFile(std::string line, bool addLineBreak) {
	if (addLineBreak)
#ifdef _WIN32
		line += "\r\n";
#else
		line += "\n";
#endif

	if (conv)
		line = conv->Convert(line);
	ostr.write(line.data(), line.size());
}
