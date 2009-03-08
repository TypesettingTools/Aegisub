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
#include "utils.h"
using namespace Athenasub;


///////////////
// Constructor
TextFileWriter::TextFileWriter(wxOutputStream &stream,String enc)
: file(stream)
{
	// Setup
	IsFirst = true;
	buffer.resize(16384);
	bufferPos = 0;
	SetEncoding(enc);
}


//////////////
// Destructor
TextFileWriter::~TextFileWriter() {
	Flush();
	file.Close();
}


/////////////////
// Write to file
void TextFileWriter::WriteLineToFile(String line,bool addLineBreak) {
	// Add line break
	String temp = line;//line.GetWxString();
	if (addLineBreak && Is16) temp += _T("\r\n");

	// Add BOM if it's the first line and the target format is Unicode
	if (IsFirst && IsUnicode) {
		wchar_t bom = 0xFEFF;
		temp = String(bom) + temp;
	}
	IsFirst = false;

	// 16-bit
	if (Is16) {
		wxString temp2 = temp.GetWxString();
		wxWCharBuffer buf = temp2.wc_str(*conv);
		if (!buf.data()) return;
		size_t len = wcslen(buf.data());
		file.Write((const char*)buf.data(),(std::streamsize)len*sizeof(wchar_t));
	}

	// 8-bit
	else {
		if (encoding == _T("UTF-8")) {
			// Calculate metrics
			const char* src = temp.c_str();
			size_t len = temp.Length()+1;
			if (addLineBreak) len += 2;

			// Resize buffer if it won't fit
			if (buffer.size() < bufferPos+len) {
				Flush();

				// Resize if it still doesn't fit
				if (buffer.size() < len) buffer.resize(len);
			}

			// Write UTF-8
			size_t toWrite = strlen(src);
			memcpy(&buffer[bufferPos],src,toWrite);
			bufferPos += toWrite;
			if (addLineBreak) {
				buffer[bufferPos++] = '\r';
				buffer[bufferPos++] = '\n';
			}
		}
		else {
			wxString temp2 = temp.GetWxString();
			wxCharBuffer buf = temp2.mb_str(*conv);
			if (!buf.data()) return;
			size_t len = strlen(buf.data());
			file.Write(buf.data(),(std::streamsize)len);
		}
	}
}


////////////////
// Set encoding
void TextFileWriter::SetEncoding(String enc) {
	// Prepare
	Is16 = false;

	// Set encoding
	encoding = enc.GetWxString();
	if (encoding == _T("Local")) conv = shared_ptr<wxMBConv> (wxConvCurrent,NullDeleter());
	else {
		if (encoding.IsEmpty()) encoding = _T("UTF-8");
		if (encoding == _T("US-ASCII")) encoding = _T("ISO-8859-1");
		conv = shared_ptr<wxMBConv> (new wxCSConv(encoding));
		IsUnicode = encoding.Left(3) == _T("UTF");
		if (encoding.Left(6) == _T("UTF-16")) {
			Is16 = true;
		}
	}
}


/////////
// Flush
void TextFileWriter::Flush() {
	if (bufferPos) {
		file.Write(&buffer[0],(std::streamsize)bufferPos);
		bufferPos = 0;
	}
}
