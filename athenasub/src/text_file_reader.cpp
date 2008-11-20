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
using namespace Athenasub;

#ifdef WITH_UNIVCHARDET
#include "charset_detect.h"
#endif


///////////////
// Constructor
TextFileReader::TextFileReader(wxInputStream &stream,String enc,bool _trim)
: file(stream)
{
	// Setup
	trim = _trim;

	// Set encoding
	encoding = enc.GetWxString();
	if (encoding == _T("binary")) return;
	SetEncodingConfiguration();
}


//////////////
// Destructor
TextFileReader::~TextFileReader()
{
}


//////////////////////////////
// Set encoding configuration
void TextFileReader::SetEncodingConfiguration()
{
	// Set encoding configuration
	swap = false;
	Is16 = false;
	isUtf8 = false;
	//conv = shared_ptr<wxMBConv>();
	if (encoding == _T("UTF-8")) {
		conv = shared_ptr<wxMBConv> (new wxMBConvUTF8);
		isUtf8 = true;
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

	// Allocate buffer
	if (!Is16) buffer1.Alloc(4096);
	else buffer2.Alloc(4096);
}


////////////////////
// Helper functions
String GetString(char *read,shared_ptr<wxMBConv> conv,bool isUtf8)
{ 
	if (isUtf8) {
		return String(read);
	} else {
		return String(wxString(read,*conv));
	}
}
String GetString(wchar_t *read,shared_ptr<wxMBConv> conv,bool isUtf8)
{
	(void)conv;
	(void)isUtf8;
	return String(read);
}
inline void Swap(wchar_t &a) {
	char *c = (char*) &a;
	char aux = c[0];
	c[0] = c[1];
	c[1] = aux;
}
inline void Swap(char &a) { (void) a; }


////////////////
// Parse a line
template <typename T>
void ParseLine(FastBuffer<T> &_buffer,wxInputStream &file,String &stringBuffer,shared_ptr<wxMBConv> conv,bool swap,bool isUtf8)
{
	// Look for a new line
	int newLinePos = -1;
	T newLineChar = 0;
	size_t size = _buffer.GetSize();

	// Find first line break
	if (size) _buffer.FindLineBreak(0,size,newLinePos,newLineChar);

	// If no line breaks were found, load more data into file
	while (newLinePos == -1) {
		// Read 2048 bytes
		const size_t readBytes = 1024;
		const size_t read = readBytes/sizeof(T);
		size_t oldSize = _buffer.GetSize();
		T *ptr = _buffer.GetWritePtr(read);
		file.Read(ptr,readBytes);
		size_t lastRead = file.LastRead()/sizeof(T);
		_buffer.AssumeSize(_buffer.GetSize()+lastRead-read);

		// Swap
		if (swap) {
			T* ptr2 = ptr;
			for (size_t i=0;i<lastRead;i++) {
				Swap(*ptr2++);
			}
		}

		// Find line break
		_buffer.FindLineBreak(oldSize,lastRead+oldSize,newLinePos,newLineChar);

		// End of file, force a line break
		if (file.Eof() && newLinePos == -1) newLinePos = (int) _buffer.GetSize();
	}

	// Found newline
	if (newLinePos != -1) {
		T *read = _buffer.GetMutableReadPtr();
		// Replace newline with null character and convert to proper charset
		if (newLinePos) {
			read[newLinePos] = 0;
			stringBuffer = GetString(read,conv,isUtf8);
		}

		// Remove an extra character if the new is the complement of \n,\r (13^7=10, 10^7=13)
		if (read[newLinePos+1] == (newLineChar ^ 7)) newLinePos++;
		_buffer.ShiftLeft(newLinePos+1);
	}
}


//////////////////////////
// Reads a line from file
Athenasub::String TextFileReader::ReadLineFromFile()
{
	String stringBuffer;
	size_t bufAlloc = 1024;
	stringBuffer.reserve(bufAlloc);
	std::string buffer = "";

	// Read UTF-16 line from file
	if (Is16) ParseLine<wchar_t>(buffer2,file,stringBuffer,conv,swap,false);

	// Read ASCII/UTF-8 line from file
	else ParseLine<char>(buffer1,file,stringBuffer,conv,false,isUtf8);

	// Remove BOM (UTF-8 EF BB BF)
	size_t startPos = 0;
	if (stringBuffer.Length() >= 3) {
		int b1 = (unsigned char) stringBuffer[0];
		int b2 = (unsigned char) stringBuffer[1];
		int b3 = (unsigned char) stringBuffer[2];
		if (b1 == 0xEF && b2 == 0xBB && b3 == 0xBF) startPos = 3;
	}

	// Trim
	String str = String(stringBuffer);
	if (trim) return String(String::StringTrim(str,startPos));
	if (startPos) return String(str.c_str() + startPos);
	return str;
}


//////////////////////////////////
// Checks if there's more to read
bool TextFileReader::HasMoreLines()
{
	return (!file.Eof() || buffer1.GetSize() || buffer2.GetSize());
}


////////////////////////////////
// Ensure that charset is valid
void TextFileReader::EnsureValid(Athenasub::String enc)
{
	if (enc == "unknown" || enc == "UTF-32BE" || enc == "UTF-32LE") {
		String error = "Character set ";
		error += enc;
		error += " is not supported.";
		throw error.c_str();
	}
}


///////////////////////////
// Get encoding being used
String TextFileReader::GetCurrentEncoding()
{
	return String(encoding.c_str());
}


///////////////////
// Rewind the file
void TextFileReader::Rewind()
{
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}
