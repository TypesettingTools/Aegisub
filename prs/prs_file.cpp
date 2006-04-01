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
#include <vector>
#include <algorithm>
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
	if (!fp) throw new std::exception("Failed to open file");

	try {
		// Write the "PRS" (zero-terminated) string ID (4 bytes)
		fwrite("PRS",1,4,fp);

		// Write version number (4 bytes)
		unsigned __int32 temp = 1;
		fwrite(&temp,4,1,fp);

		// Write stream name (for future scalability, there is only one for now)
		// This is writen as 4 bytes for length, and then length bytes for actual name.
		// Since we set length to zero, the actual name string is never writen.
		temp = 0;
		fwrite(&temp,4,1,fp);

		// Write data blocks
		std::vector<char> vec;
		std::list<PRSEntry*>::iterator cur;
		for (cur=entryList.begin();cur!=entryList.end();cur++) {
			// Data blocks take care of writing themselves
			// All of them start with a 4-byte string identifier, and a 4-byte length identifier
			// A decoder can (and should!) ignore any block that it doesn't recognize
			vec.resize(0);
			(*cur)->WriteData(vec);
			fwrite(&vec[0],1,vec.size(),fp);
		}
	}

	// Rethrow exceptions
	catch (...) {
		fclose(fp);
		throw;
	}

	// Close file
	fclose(fp);
}


////////
// Load
void PRSFile::Load(std::string path, bool reset) {
	// Reset first, if requested
	if (reset) Reset();

	// Open file
	FILE *fp = fopen(path.c_str(),"rb");
	if (!fp) throw new std::exception("Failed to open file");

	try {
		// Read first four bytes
		char buf[5];
		buf[4] = 0;
		fread(buf,1,4,fp);
		if (strcmp(buf,"PRS") != 0) throw new std::exception("Invalid file type.");

		// Read version number
		unsigned __int32 temp = 0;
		fread(&temp,4,1,fp);
		if (temp != 1) throw new std::exception("Invalid version.");

		// Read stream name length
		fread(&temp,4,1,fp);

		// Read stream name
		if (temp > 0) {
			char *streamName = new char[temp+1];
			fread(streamName,1,temp,fp);

			// We don't need it, so delete afterwards
			delete streamName;
		}

		// Temporary vector
		std::vector<char> vec;

		// Read data blocks
		while (!feof(fp)) {
			// Read identifier and size
			fread(buf,1,4,fp);
			fread(&temp,4,1,fp);

			// Image block
			if (strcmp(buf,"IMG") == 0) {
				// Read data
				vec.resize(temp);
				fread(&vec[0],1,temp,fp);

				// Create object
				PRSImage *img = new PRSImage;
				img->ReadData(vec);
				AddEntry(img);
			}

			// Display block
			else if (strcmp(buf,"DSP") == 0) {
				// Read data
				vec.resize(temp);
				fread(&vec[0],1,temp,fp);

				// Create object
				PRSDisplay *disp = new PRSDisplay;
				disp->ReadData(vec);
				AddEntry(disp);
			}

			// Unknown block, ignore it
			else {
				fseek(fp,temp,SEEK_CUR);
			}
		}
	}

	// Rethrow exceptions
	catch (...) {
		fclose(fp);
		throw;
	}

	// Close file
	fclose(fp);
}


/////////////
// Add entry
void PRSFile::AddEntry(PRSEntry *entry) {
	entryList.push_back(entry);
}


//////////////////////////////////////////////////
// Checks if there is any duplicate of this image
PRSImage *PRSFile::FindDuplicateImage(PRSImage *img) {
	// Scan looking for duplicate hashes
	PRSImage *orig;
	std::list<PRSEntry*>::iterator cur;
	for (cur=entryList.begin();cur!=entryList.end();cur++) {
		orig = PRSEntry::GetImage(*cur);
		if (orig) {
			// Compare data lengths
			if (orig->dataLen == img->dataLen) {
				// Identical data lengths, compare hashes
				if (memcmp(orig->md5,img->md5,16) == 0) {
					// Identical hashes, compare image data to be sure
					if (memcmp(orig->data,img->data,orig->dataLen) == 0) {
						// Identical data, return
						return orig;
					}
				}
			}
		}
	}

	// No duplicate found
	return NULL;
}


////////////////
// Draw a frame
void PRSFile::DrawFrame(int n,PRSVideoFrame *frame) {
	// Get list of display blocks
	std::vector<PRSDisplay*> blocks;
	GetDisplayBlocksAtFrame(n,blocks);

	// Draw the blocks
	int nblocks = (int) blocks.size();
	for (int i=0;i<nblocks;i++) {
		// Get display and image pair
		PRSDisplay *display = blocks[i];
		PRSImage *image = GetImageByID(display->id);

		// Decode PNG
		PRSVideoFrame *overFrame = image->GetDecodedFrame();

		// Draw image on frame
		if (overFrame) overFrame->Overlay(frame,display->x,display->y,display->alpha,display->blend);

		// Clean up
		delete overFrame;
	}
}


////////////////////////////////////////////////
// Finds which display blocks are at a position
void PRSFile::GetDisplayBlocksAtFrame(int n,std::vector<PRSDisplay*> &blocks) {
	// Find all blocks that match
	unsigned int fn = n;
	std::list<PRSEntry*>::iterator cur;
	PRSDisplay *display;
	for (cur=entryList.begin();cur!=entryList.end();cur++) {
		display = PRSEntry::GetDisplay(*cur);
		if (display) {
			if (display->startFrame <= fn && display->endFrame >= fn) {
				blocks.push_back(display);
			}
		}
	}

	// Sort them by layer
	// TODO
}


////////////////////////////////////////////
// Checks if there is anything at the frame
bool PRSFile::HasDataAtFrame(int n) {
	// Find all blocks that match
	unsigned int fn = n;
	std::list<PRSEntry*>::iterator cur;
	PRSDisplay *display;
	for (cur=entryList.begin();cur!=entryList.end();cur++) {
		display = PRSEntry::GetDisplay(*cur);
		if (display) {
			if (display->startFrame <= fn && display->endFrame >= fn) {
				return true;
			}
		}
	}
	return false;
}


///////////////////////////////////////////////////////////////
// Gets a PRSImage by its ID, returns NULL if it doesn't exist
PRSImage *PRSFile::GetImageByID(int id) {
	// Search for image
	std::list<PRSEntry*>::iterator cur;
	PRSImage *img;
	for (cur=entryList.begin();cur!=entryList.end();cur++) {
		img = PRSEntry::GetImage(*cur);
		if (img && img->id == id) return img;
	}

	// Not found
	return NULL;
}

