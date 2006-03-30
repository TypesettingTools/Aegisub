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
#include <stdio.h>
#include "prs_file.h"
#include "prs_entry.h"
#include "prs_image.h"
#include "prs_display.h"


///////////////
// Constructor
PRSFile::PRSFile () {
}


//////////////
// Destructor
PRSFile::~PRSFile() {
	Reset();
}


//////////////
// Reset file
void PRSFile::Reset() {
	// Clear list of entries
	std::list<PRSEntry*>::iterator cur;
	for (cur=entryList.begin();cur!=entryList.end();cur++) {
		delete *cur;
	}
	entryList.clear();
}


////////
// Save
void PRSFile::Save(std::string path) {
	// I'm using C's stdio instead of C++'s fstream because I feel that fstream is
	// pretty lame for binary data, so I need to make sure that no exceptions are thrown from here.

	// TODO: Make this endianness-independent

	// Open file
	FILE *fp = fopen(path.c_str(),"wb");
	if (!fp) throw "Failed to open file";

	try {
		// Write the "PRS" (zero-terminated) string ID (4 bytes)
		fwrite("PRS",1,4,fp);

		// Write version number (4 bytes)
		__int32 temp = 1;
		fwrite(&temp,4,1,fp);

		// Write stream name (for future scalability, there is only one for now)
		// This is writen as 4 bytes for length, and then length bytes for actual name.
		// Since we set length to zero, the actual name string is never writen.
		temp = 0;
		fwrite(&temp,4,1,fp);

		// Write data blocks
		std::list<PRSEntry*>::iterator cur;
		for (cur=entryList.begin();cur!=entryList.end();cur++) {
			// Data blocks take care of writing themselves
			// All of them start with a 4-byte string identifier, and a 4-byte length identifier
			// A decoder can (and should!) ignore any block that it doesn't recognize
			(*cur)->WriteData(fp);
		}
	}
	catch (...) {}

	// Close file
	fclose(fp);
}


////////
// Load
void PRSFile::Load(std::string path, bool reset) {
}


/////////////
// Add entry
void PRSFile::AddEntry(PRSEntry *entry) {
	entryList.push_back(entry);
}
