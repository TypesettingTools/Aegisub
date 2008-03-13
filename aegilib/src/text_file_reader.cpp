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
#include <algorithm>
#include <string>
#include <wx/wfstream.h>
#include "text_file_reader.h"
using namespace Aegilib;

#ifdef WITH_UNIVCHARDET
#include "charset_detect.h"
#endif


///////////////
// Constructor
TextFileReader::TextFileReader(wxInputStream &stream,Aegilib::String enc,bool _trim)
: file(stream)
{
	// Setup
	trim = _trim;

	// Set encoding
	encoding = enc.c_str();
	if (encoding == _T("binary")) return;
	SetEncodingConfiguration();
}


//////////////
// Destructor
TextFileReader::~TextFileReader() {
}


//////////////////////////////
// Set encoding configuration
void TextFileReader::SetEncodingConfiguration() {
	// Set encoding configuration
	swap = false;
	Is16 = false;
	conv = shared_ptr<wxMBConv>();
	if (encoding == _T("UTF-8")) {
		conv = shared_ptr<wxMBConv> (new wxMBConvUTF8);
	}
	else if (encoding == _T("UTF-16LE")) {
		Is16 = true;
	}
	else if (encoding == _T("UTF-16BE")) {
		Is16 = true;
		swap = true;
	}
	else if (encoding == _T("UTF-7")) {
		conv = shared_ptr<wxMBConv>(new wxCSConv(encoding));
	}
	else if (encoding == _T("Local")) {
		conv = shared_ptr<wxMBConv> (wxConvCurrent,NullDeleter());
	}
	else {
		conv = shared_ptr<wxMBConv> (new wxCSConv(encoding));
	}
}


//////////////////////////
// Reads a line from file
Aegilib::String TextFileReader::ReadLineFromFile() {
	wxString wxbuffer;
	size_t bufAlloc = 1024;
	wxbuffer.Alloc(bufAlloc);
	std::string buffer = "";

	// Read UTF-16 line from file
	if (Is16) {
		char charbuffer[3];
		charbuffer[2] = 0;
		wchar_t ch = 0;
		size_t len = 0;
		while (ch != L'\n' && !file.Eof()) {
			// Read two chars from file
			charbuffer[0] = 0;
			charbuffer[1] = 0;
			file.Read(charbuffer,2);

			// Swap bytes for big endian
			if (swap) {
				register char aux = charbuffer[0];
				charbuffer[0] = charbuffer[1];
				charbuffer[1] = aux;
			}

			// Convert two chars into a widechar and append to string
			ch = *((wchar_t*)charbuffer);
			if (len >= bufAlloc - 1) {
				bufAlloc *= 2;
				wxbuffer.Alloc(bufAlloc);
			}
			wxbuffer += ch;
			len++;
		}
	}

	// Read ASCII/UTF-8 line from file
	else {
		//getline(file,buffer);
		//wxbuffer.Clear();
		//if (buffer.length()) wxbuffer = wxString(buffer.c_str(),*conv);
		char temp = 0;
		std::string buff;
		while (temp != '\n' && !file.Eof()) {
			file.Read(&temp,1);
			if (temp != '\r') {
				buff += temp;
			}
		}
		if (buff.size()) wxbuffer = wxString(buff.c_str(),*conv);
	}

	// Remove line breaks
	//wxbuffer.Replace(_T("\r"),_T("\0"));
	//wxbuffer.Replace(_T("\n"),_T("\0"));
	size_t len=wxbuffer.Length();
	for (size_t i=0;i<len;i++) {
		if (wxbuffer[i] == _T('\r') || wxbuffer[i] == _T('\n')) wxbuffer[i] = _T(' ');
	}

	// Remove BOM
	if (wxbuffer.Length() > 0 && wxbuffer[0] == 0xFEFF) {
		wxbuffer = wxbuffer.Mid(1);
	}

	// Trim
	if (trim) {
		wxbuffer.Trim(true);
		wxbuffer.Trim(false);
	}
	return Aegilib::String(wxbuffer.c_str());
}


//////////////////////////////////
// Checks if there's more to read
bool TextFileReader::HasMoreLines() {
	return (!file.Eof());
}


////////////////////////////////
// Ensure that charset is valid
void TextFileReader::EnsureValid(Aegilib::String enc) {
	if (enc == _T("unknown") || enc == _T("UTF-32BE") || enc == _T("UTF-32LE")) {
		wxString error = _T("Character set ");
		error += enc;
		error += _T(" is not supported.");
		throw error.c_str();
	}
}


///////////////////////////
// Get encoding being used
Aegilib::String TextFileReader::GetCurrentEncoding() {
	return encoding.c_str();
}
