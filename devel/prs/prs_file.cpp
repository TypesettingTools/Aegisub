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
#include <fstream>
#include "prs_file.h"
#include "prs_entry.h"
#include "prs_image.h"
#include "prs_display.h"


///////////////
// Constructor
PRSFile::PRSFile () {
	// Cache data
	cacheMemSize = 0;
	maxCache = 8 << 20;
}


//////////////
// Destructor
PRSFile::~PRSFile() {
	Reset();
	ClearCache();
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


///////////////////
// Clear the cache
void PRSFile::ClearCache() {
	// Clear list of cached frames
	frameCache.clear();

	// Zero size
	cacheMemSize = 0;
}


////////
// Save
void PRSFile::Save(std::string path) {
	// I'm using C's stdio instead of C++'s fstream because I feel that fstream is
	// pretty lame for binary data, so I need to make sure that no exceptions are thrown from here.

	// TODO: Make this endianness-independent

	// Open file
	FILE *fp = fopen(path.c_str(),"wb");
	if (!fp) throw std::exception("Failed to open file");

	try {
		// Write the "PRS" (zero-terminated) string ID (4 bytes)
		fwrite("PRS-BIN",1,8,fp);

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
	if (!fp) throw std::exception("Failed to open file");

	try {
		// Read first eight bytes
		char buf[16];
		fread(buf,1,8,fp);
		if (memcmp(buf,"PRS-BIN",8) != 0) {
			// Is actually ASCII, read as that
			if (memcmp(buf,"PRS-ASC",7) == 0) {
				LoadText(path,false);
				return;
			}

			// Invalid
			throw std::exception("Invalid file type.");
		}

		// Read version number
		unsigned __int32 temp = 0;
		fread(&temp,4,1,fp);
		if (temp != 1) throw std::exception("Invalid version.");

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
		// Get display block and frame
		PRSDisplay *display = blocks[i];
		PRSVideoFrame *overFrame = CachedGetFrameByID(display->id);

		// Draw image on frame
		if (overFrame) overFrame->Overlay(frame,display->x,display->y,display->alpha,display->blend);

		// DON'T delete the frame!
		// The cache takes care of doing so.
	}
}


///////////////////////////////////////////////////////////////////
// Gets a frame from cache, or load it there if it's not available
PRSVideoFrame* PRSFile::CachedGetFrameByID(int id) {
	// Check if the image is already decoded on cache, fetch it if it is
	PRSVideoFrame *frame = NULL;
	std::list<PRSCachedFrame>::iterator cur;
	for (cur=frameCache.begin();cur!=frameCache.end();cur++) {
		if ((*cur).id == id) {
			return (*cur).frame;
		}
	}

	// It isn't; decode and add it to cache
	// Get image
	PRSImage *image = GetImageByID(id);
	if (!image) return NULL;

	// Get frame
	frame = image->GetDecodedFrame();

	// Add to cache
	if (frame) {
		// Add and raise size
		PRSCachedFrame cached;
		cached.frame = frame;
		cached.id = id;
		frameCache.push_front(cached);
		cached.frame = NULL;
		cacheMemSize += frame->GetSize();

		// Update cache
		EnforceCacheLimit();
	}

	// Return it
	return frame;
}


/////////////////////////////////////////////////////////////
// Enforce the cache limit, that is, delete anything over it
// This function will always leave at least one image on cache
void PRSFile::EnforceCacheLimit() {
	while (cacheMemSize > maxCache && frameCache.size() > 1) {
		cacheMemSize -= frameCache.back().frame->GetSize();
		frameCache.pop_back();
	}
}


////////////////////////////
// Set maximum cache memory
void PRSFile::SetCacheLimit(int bytes) {
	maxCache = bytes;
	EnforceCacheLimit();
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


//////////////////////
// Save as plain-text
void PRSFile::SaveText(std::string path) {
	// Open file
	std::ofstream file;
	file.open(path.c_str(),std::fstream::out);

	// Write version string
	file << "PRS-ASCII v1" << std::endl;

	// Write events
	std::list<PRSEntry*>::iterator cur;
	for (cur=entryList.begin();cur!=entryList.end();cur++) {
		PRSImage *img;
		PRSDisplay *display;

		// Is image?
		img = PRSEntry::GetImage(*cur);
		if (img) {
			// Get image filename
			char idStr[64];
			itoa(img->id,idStr,10);
			std::string imgfile = path;
			imgfile += ".id.";
			imgfile += idStr;
			imgfile += ".png";

			// Copy to disk
			FILE *fp = fopen(imgfile.c_str(),"wb");
			if (fp) {
				fwrite(img->data,1,img->dataLen,fp);
				fclose(fp);
			}

			// Write text
			file << "IMG: " << img->id << "," << img->imageType << "," << imgfile.c_str() << std::endl;
			continue;
		}

		// Is display?
		display = PRSEntry::GetDisplay(*cur);
		if (display) {
			// Write text
			file << "DSP: " << display->startFrame << "," << display->endFrame << "," << display->start << "," << display->end << ",";
			file << display->id << "," << display->x << "," << display->y << "," << (int)(display->alpha) << "," << (int)(display->blend);
			file << std::endl;
			continue;
		}
	}

	// Close file
	file.close();
}


///////////////////
// Load plain-text
void PRSFile::LoadText(std::string path, bool reset) {
}
