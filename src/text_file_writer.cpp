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

#include "text_file_writer.h"

#include "options.h"

#include <libaegisub/io.h>
#include <libaegisub/charset_conv.h>

#include <boost/algorithm/string/case_conv.hpp>

TextFileWriter::TextFileWriter(agi::fs::path const& filename, std::string encoding)
: file(new agi::io::Save(filename, true))
{
	if (encoding.empty())
		encoding = OPT_GET("App/Save Charset")->GetString();
	if (encoding != "utf-8" && encoding != "UTF-8") {
		conv = std::make_unique<agi::charset::IconvWrapper>("utf-8", encoding.data(), true);
		newline = conv->Convert(newline);
	}

	try {
		// Write the BOM
		WriteLineToFile("\xEF\xBB\xBF", false);
	}
	catch (agi::charset::ConversionFailure&) {
		// If the BOM could not be converted to the target encoding it isn't needed
	}
}

// Explicit empty destructor required with a unique_ptr to an incomplete class
TextFileWriter::~TextFileWriter() = default;

void TextFileWriter::WriteLineToFile(std::string_view line, bool addLineBreak) {
	if (conv) {
		auto converted = conv->Convert(line);
		file->Get().write(converted.data(), converted.size());
	}
	else
		file->Get().write(line.data(), line.size());

	if (addLineBreak)
		file->Get().write(newline.data(), newline.size());
}
