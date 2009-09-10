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

#include <fstream>
#include "text_file_writer.h"
#include "options.h"
#include "charset_conv.h"


/// @brief DOCME
/// @param filename 
/// @param encoding 
///
TextFileWriter::TextFileWriter(wxString filename, wxString encoding)
: conv() {
#ifdef WIN32
	file.open(filename.wc_str(),std::ios::out | std::ios::binary | std::ios::trunc);
#else
	file.open(wxFNCONV(filename),std::ios::out | std::ios::binary | std::ios::trunc);
#endif
	if (!file.is_open()) {
		throw _T("Failed opening file for writing.");
	}

	if (encoding.IsEmpty()) encoding = Options.AsText(_T("Save Charset"));
	conv.reset(new AegisubCSConv(encoding, true));

	// Write the BOM
	try {
		WriteLineToFile(_T("\uFEFF"), false);
	}
	catch (wxString ignore) {
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
///
void TextFileWriter::WriteLineToFile(wxString line, bool addLineBreak) {
	wxString temp = line;
	if (addLineBreak) temp += _T("\r\n");

	wxCharBuffer buf = temp.mb_str(*conv);
	if (buf.data())
		file.write(buf.data(), conv->MBBuffLen(buf.data()));
}


