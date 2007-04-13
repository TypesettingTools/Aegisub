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
#include <algorithm>
#include <string>
#include "text_file_reader.h"

#ifdef __WINDOWS__
#define AUTODETECT_CHARSET
#include "charset_detect.h"
#endif


///////////////
// Constructor
TextFileReader::TextFileReader(wxString _filename,wxString enc,bool _trim) {
	// Setup
	open = false;
	customConv = false;
	trim = _trim;
	filename = _filename;

	// Open file
	Open();

	// Set encoding
	encoding = enc;
	if (encoding.IsEmpty()) encoding = GetEncoding(filename);
	SetEncodingConfiguration();
}


//////////////
// Destructor
TextFileReader::~TextFileReader() {
	Close();

	// Clean up conversion
	if (customConv) delete conv;
}


///////////////////////////
// Determine file encoding
wxString TextFileReader::GetEncoding(const wxString _filename) {
	// Prepare
	using namespace std;
	unsigned char b[4];
	for (int i=0;i<4;i++) b[i] = 0;

	// Read four bytes from file
#ifdef TEXT_READER_USE_STDIO
	// TODO: maybe make this use posix-style fopen() api's instead as well?
	HANDLE ifile = CreateFile(
		_filename.c_str(),			// filename
		FILE_READ_DATA,				// access mode
		FILE_SHARE_READ,			// share mode
		0,							// security descriptor
		OPEN_EXISTING,				// creation disposition
		FILE_FLAG_SEQUENTIAL_SCAN,	// flags
		0);							// template file
	if (ifile == INVALID_HANDLE_VALUE) {
		return _T("unknown");
	}
	DWORD numread;
	if (!ReadFile(ifile, (char*)b, 4, &numread, 0)) {
		// Unable to open
		return _T("unknown");
	}
	if (numread < 4) {
		// File too short to decide, assume local
		return _T("Local");
	}
	CloseHandle(ifile);
#else
	ifstream ifile;
	ifile.open(wxFNCONV(_filename));
	if (!ifile.is_open()) {
		return _T("unknown");
	}
	ifile.read((char*)b,4);
	ifile.close();
#endif

	// Try to get the byte order mark from them
	if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) return _T("UTF-8");
	else if (b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) return _T("UTF-32LE");
	else if (b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) return _T("UTF-32BE");
	else if (b[0] == 0xFF && b[1] == 0xFE) return _T("UTF-16LE");
	else if (b[0] == 0xFE && b[1] == 0xFF) return _T("UTF-16BE");
	else if (b[0] == 0x2B && b[1] == 0x2F && b[2] == 0x76) return _T("UTF-7");

	// Try to guess UTF-16
	else if (b[0] == 0x00 && b[2] == 0x00) return _T("UTF-16BE");
	else if (b[1] == 0x00 && b[3] == 0x00) return _T("UTF-16LE");

	// If any of the first four bytes are under 0x20 (the first printable character),
	// except for 9-13 range, assume binary
	for (int i=0;i<4;i++) {
		if (b[i] < 9 || (b[i] > 13 && b[i] < 32)) return _T("binary");
	}

	#ifdef AUTODETECT_CHARSET
	// Use universalchardet library to detect charset
	CharSetDetect det;
	return det.GetEncoding(_filename);
	#else
	// Fall back to local
	return _T("Local");
	#endif
}


//////////////////////////////
// Set encoding configuration
void TextFileReader::SetEncodingConfiguration() {
	// Set encoding configuration
	swap = false;
	Is16 = false;
	customConv = false;
	conv = NULL;
	if (encoding == _T("UTF-8")) {
		conv = new wxMBConvUTF8;
		customConv = true;
	}
	else if (encoding == _T("UTF-16LE")) {
		Is16 = true;
	}
	else if (encoding == _T("UTF-16BE")) {
		Is16 = true;
		swap = true;
	}
	else if (encoding == _T("UTF-7")) {
		conv = new wxCSConv(encoding);
		customConv = true;
	}
	else if (encoding == _T("Local")) {
		conv = wxConvCurrent;
	}
	else {
		conv = new wxCSConv(encoding);
		customConv = true;
	}
}


//////////////////////////
// Reads a line from file
wxString TextFileReader::ReadLineFromFile() {
	Open();
	wxString wxbuffer = _T("");

	// Read UTF-16 line from file
	if (Is16) {
		char charbuffer[3];
		charbuffer[2] = 0;
		char aux;
		wchar_t ch = 0;
		int n = 0;
#ifdef TEXT_READER_USE_STDIO
		while (ch != L'\n' && !feof(file)) {
			// Read two chars from file
			fread(charbuffer, 2, 1, file);
#else
		while (ch != L'\n' && !file.eof()) {
			// Read two chars from file
			file.read(charbuffer,2);
#endif

			// Swap bytes for big endian
			if (swap) {
				aux = charbuffer[0];
				charbuffer[0] = charbuffer[1];
				charbuffer[1] = aux;
			}

			// Convert two chars into a widechar and append to string
			ch = *((wchar_t*)charbuffer);
			wxbuffer += ch;
			n++;
		}
	}

	// Read ASCII/UTF-8 line from file
	else {
#ifdef TEXT_READER_USE_STDIO
		char buffer[512];
		while (1) {
			buffer[511] = '\1';
			if (fgets(buffer, 512, file)) {
				// read succeeded
				// FIXME, this might break on incomplete multibyte characters
				wxString linepart(buffer, *conv);
				wxbuffer += linepart;
				if (buffer[511] == '\1' || buffer[510] == '\n') {
					// our sentinel \1 wasn't overwritten, meaning an EOL was found
					break;
				}
				// otherwise the sentinel \1 was overwritten (presumably with \0), so just loop on
			}
			else {
				// hit EOF
				break;
			}
		}
#else
		std::string buffer;
		getline(file,buffer);
		wxString lineresult(buffer.c_str(),*conv);
		wxbuffer = lineresult;
#endif
	}

	// Remove line breaks
	wxbuffer.Replace(_T("\r"),_T(""));
	wxbuffer.Replace(_T("\n"),_T(""));

	// Final string
	wxString final = wxString(wxbuffer);

	// Remove BOM
	if (final.length() > 0 && final[0] == 0xFEFF) {
		final = final.Mid(1);
	}

	// Trim
	if (trim) {
		final.Trim(true);
		final.Trim(false);
	}
	return final;
}


/////////////
// Open file
void TextFileReader::Open() {
	if (open) return;
#ifdef TEXT_READER_USE_STDIO
	// binary mode, because ascii mode is never to be trusted
	file = _tfopen(filename.c_str(), _T("rb"));
	if (file == 0) {
		throw _T("Failed opening file for reading.");
	}
#else
	file.open(wxFNCONV(filename),std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		throw _T("Failed opening file for reading.");
	}
#endif
	open = true;
}


//////////////
// Close file
void TextFileReader::Close() {
	if (!open) return;
#ifdef TEXT_READER_USE_STDIO
	fclose(file);
#else
	file.close();
#endif
	open = false;
}


//////////////////////////////////
// Checks if there's more to read
bool TextFileReader::HasMoreLines() {
#ifdef TEXT_READER_USE_STDIO
	if (encoding == _T("binary")) return false;
	return !feof(file);
#else
	return (!file.eof());
#endif
}


////////////////////////////////
// Ensure that charset is valid
void TextFileReader::EnsureValid(wxString enc) {
	if (enc == _T("unknown") || enc == _T("UTF-32BE") || enc == _T("UTF-32LE")) {
		wxString error = _T("Character set ");
		error += enc;
		error += _T(" is not supported.");
		throw error;
	}
}


///////////////////////////
// Get encoding being used
wxString TextFileReader::GetCurrentEncoding() {
	return encoding;
}
