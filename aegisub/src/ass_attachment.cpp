// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file ass_attachment.cpp
/// @brief Manage files embedded in subtitles
/// @ingroup subs_storage
///


////////////
// Includes
#include "config.h"

#include <wx/wfstream.h>
#include <wx/filename.h>
#include "ass_attachment.h"


///////////////
// Constructor
AssAttachment::AssAttachment(wxString _name) {
	// Parse name
	filename = _name;
	wxFileName fname(GetFileName());
	wxString ext = fname.GetExt().Lower();
	wxString name;
	if (ext == _T("ttf")) {
		name = fname.GetName() + _T("_0.") + ext;
	}
	else name = _name;

	// Set data
	filename = name;
	data = boost::shared_ptr<AttachData> (new AttachData);
}


//////////////
// Destructor
AssAttachment::~AssAttachment() {
}


/////////
// Clone
AssEntry *AssAttachment::Clone() {
	// New object
	AssAttachment *clone = new AssAttachment(filename);

	// Copy fields
	clone->data = data;

	// Return
	return clone;
}


////////////
// Get data
const DataVec &AssAttachment::GetData() {
	return data->GetData();
}


/////////////////
// Add more data
void AssAttachment::AddData(wxString _data) {
	data->AddData(_data);
}


//////////////////////
// Finish adding data
void AssAttachment::Finish() {
	data->Finish();
}


/////////////////////////////////////
// Get encoded data to write on file
const wxString AssAttachment::GetEntryData() {
	// Get data
	const DataVec &dat = data->GetData();
	int pos = 0;
	int size = dat.size();
	int written = 0;
	unsigned char src[3];
	unsigned char dst[4];

	// Write header
	wxString entryData;
	if (group == _T("[Fonts]")) entryData = _T("fontname: ");
	else entryData = _T("filename: ");
	entryData += filename + _T("\r\n");

	// Read three bytes
	while (pos < size) {
		// Number to read
		int read = size - pos;
		if (read > 3) read = 3;

		// Read source
		src[0] = dat[pos];
		if (read >= 2) src[1] = dat[pos+1];
		else src[1] = 0;
		if (read == 3) src[2] = dat[pos+2];
		else src[2] = 0;
		pos += read;

		// Codify
		dst[0] = src[0] >> 2;
		dst[1] = ((src[0] & 0x3) << 4) | ((src[1] & 0xF0) >> 4);
		dst[2] = ((src[1] & 0xF) << 2) | ((src[2] & 0xC0) >> 6);
		dst[3] = src[2] & 0x3F;

		// Number to write
		int toWrite = read+1;

		// Convert to text
		for (int i=0;i<toWrite;i++) {
			entryData += wxChar(dst[i]+33);
			written++;

			// Line break
			if (written == 80 && pos < size) {
				written = 0;
				entryData += _T("\r\n");
			}
		}
	}

	// Return
	return entryData;
}


/////////////////////
// Extract as a file
void AssAttachment::Extract(wxString filename) {
	// Open file
	wxFileOutputStream fp(filename);
	if (!fp.Ok()) return;
	fp.Write(&data->GetData()[0],data->GetData().size());
}


/////////////////////////////
// Read a file as attachment
void AssAttachment::Import(wxString filename) {
	// Data
	DataVec &datavec = data->GetData();

	// Open file and get size
	wxFileInputStream fp(filename);
	if (!fp.Ok()) throw _T("Failed opening file");
	int size = fp.SeekI(0,wxFromEnd);
	fp.SeekI(0,wxFromStart);

	// Set size and read
	datavec.resize(size);
	fp.Read(&datavec[0],size);
}


////////////////
// Get filename
wxString AssAttachment::GetFileName(bool raw) {
	// Raw
	if (raw || filename.Right(4).Lower() != _T(".ttf")) return filename;

	// Remove stuff after last underscore if it's a font
	int lastUnder = -1;
	for (size_t i=0;i<filename.Length();i++) {
		if (filename[i] == _T('_')) lastUnder = i;
	}

	// Underline found
	wxString final = filename;
	if (lastUnder != -1) {
		final = filename.Left(lastUnder) + _T(".ttf");
	}
	return final;
}



/////////////////// Attachment //////////////////
///////////////
// Constructor
AttachData::AttachData() {
}


//////////////
// Destructor
AttachData::~AttachData() {
}


////////////
// Get data
DataVec &AttachData::GetData() {
	return data;
}


////////////
// Add data
void AttachData::AddData(wxString data) {
	buffer += data;
}


//////////
// Finish
void AttachData::Finish() {
	// Source and dest buffers
	unsigned char src[4];
	unsigned char dst[3];
	int bufPos = 0;
	bool ok = true;

	// Read buffer
	while (ok) {
		// Find characters left
		int read = buffer.Length() - bufPos;
		if (read > 4) read = 4;
		int nbytes;

		// At least four, proceed normally
		if (read >= 2) {
			// Move 4 bytes from buffer to src
			for (int i=0;i<read;i++) {
				src[i] = (unsigned char) buffer[bufPos] - 33;
				bufPos++;
			}
			for (int i=read;i<4;i++) src[i] = 0;
			ok = true;
			nbytes = read-1;
		}

		// Zero, end
		else {
			ok = false;
			break;
		}

		// Convert the 4 bytes from source to 3 in dst
		dst[0] = (src[0] << 2) | (src[1] >> 4);
		dst[1] = ((src[1] & 0xF) << 4) | (src[2] >> 2);
		dst[2] = ((src[2] & 0x3) << 6) | (src[3]);

		// Push into vector
		size_t size = data.size(); 
		data.resize(size+nbytes);
		for (int i=0;i<nbytes;i++) data[size+i] = dst[i];
	}

	// Clear buffer
	buffer.Clear();
	buffer.Shrink();
}

