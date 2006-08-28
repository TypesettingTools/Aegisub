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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <fstream>
#include "text_file_writer.h"
#include "options.h"


///////////////
// Constructor
TextFileWriter::TextFileWriter(wxString _filename,wxString enc) {
	// Setup
	open = false;
	customConv = false;
	IsFirst = true;
	filename = _filename;

	// Set encoding
	encoding = enc;
	if (encoding == _T("Local") || (encoding.IsEmpty() && Options.AsText(_T("Save Charset")).Lower() == _T("local"))) conv = &wxConvLocal;
	else {
		if (encoding.IsEmpty()) encoding = Options.AsText(_T("Save Charset"));
		if (encoding == _T("US-ASCII")) encoding = _T("ISO-8859-1");
		conv = new wxCSConv(encoding);
		customConv = true;
		IsUnicode = encoding.Left(3) == _T("UTF");
	}

	// Open file
	Open();
}


//////////////
// Destructor
TextFileWriter::~TextFileWriter() {
	Close();
}


/////////////
// Open file
void TextFileWriter::Open() {
	// Open file
	if (open) return;
#ifdef WIN32
	file = _tfopen(filename.c_str(), _T("wb"));
	if (!file) {
		throw _T("Failed opening file for writing.");
	}
#else
	file.open(filename.mb_str(wxConvLocal),std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		throw _T("Failed opening file for writing.");
	}
#endif
	open = true;

	// Set encoding
	SetEncoding();
}


//////////////
// Close file
void TextFileWriter::Close() {
	if (!open) return;
#ifdef WIN32
	fclose(file);
#else
	file.close();
#endif
	open = false;
	if (customConv) delete conv;
}


/////////////////
// Write to file
void TextFileWriter::WriteLineToFile(wxString line,bool addLineBreak) {
	// Make sure it's loaded
	if (!open) Open();

	// Add line break
	wxString temp = line;
	if (addLineBreak) temp += _T("\r\n");

	// Add BOM if it's the first line and the target format is Unicode
	if (IsFirst && IsUnicode) {
		wchar_t bom = 0xFEFF;
		temp = wxString(bom) + temp;
	}
	IsFirst = false;

	// 16-bit
	if (Is16) {
		wxWCharBuffer buf = temp.wc_str(*conv);
		if (!buf.data())
			return;
		size_t len = wcslen(buf.data());
#ifdef WIN32
		fwrite(buf.data(), sizeof(wchar_t), len, file);
#else
		file.write((const char*)buf.data(),len*sizeof(wchar_t));
#endif
	}

	// 8-bit
	else {
		wxCharBuffer buf = temp.mb_str(*conv);
		if (!buf.data())
			return;
		size_t len = strlen(buf.data());
#ifdef WIN32
		fwrite(buf.data(), 1, len, file);
#else
		file.write(buf.data(),len);
#endif
	}
}


////////////////
// Set encoding
void TextFileWriter::SetEncoding() {
	// Prepare
	Is16 = false;

	// UTF-16
	if (encoding.Left(6) == _T("UTF-16")) {
		Is16 = true;
	}
}
