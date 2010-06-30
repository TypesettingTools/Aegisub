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

/// @file text_file_writer.cpp
/// @brief Write plain text files line by line
/// @ingroup utility
///

#include "config.h"

#ifndef AGI_PRE
#include <fstream>
#endif

#include "charset_conv.h"
#include "compat.h"
#include "main.h"
#include "options.h"
#include "text_file_writer.h"


/// @brief DOCME
/// @param filename 
/// @param encoding 
///
TextFileWriter::TextFileWriter(wxString const& filename, wxString encoding)
: conv() {
#ifdef WIN32
	file.open(filename.wc_str(),std::ios::out | std::ios::binary | std::ios::trunc);
#else
	file.open(wxFNCONV(filename),std::ios::out | std::ios::binary | std::ios::trunc);
#endif
	if (!file.is_open()) {
		throw L"Failed opening file for writing.";
	}

	if (encoding.empty()) encoding = lagi_wxString(OPT_GET("App/Save Charset")->GetString());
	if (encoding.Lower() != wxSTRING_ENCODING)
		conv.reset(new agi::charset::IconvWrapper(wxSTRING_ENCODING, encoding.c_str(), true));

	// Write the BOM
	try {
		WriteLineToFile(L"\uFEFF", false);
	}
	catch (agi::charset::ConversionFailure&) {
		// If the BOM could not be converted to the target encoding it isn't needed
	}
}


/// @brief DOCME
///
TextFileWriter::~TextFileWriter() {
	// Explicit empty destructor required with an auto_ptr to an incomplete class
}


/// @brief DOCME
/// @param line         
/// @param addLineBreak 
void TextFileWriter::WriteLineToFile(wxString line, bool addLineBreak) {
	if (addLineBreak) line += L"\n";

	// On non-windows this cast does nothing
	const char *data = reinterpret_cast<const char *>(line.wx_str());
#if wxUSE_UNICODE_UTF8
	size_t len = line.utf8_length();
#else
	size_t len = line.length() * sizeof(wxStringCharType);
#endif

	if (conv.get()) {
		std::string buf = conv->Convert(std::string(data, len));
		file.write(buf.data(), buf.size());
	}
	else {
		file.write(data, len);
	}
}
