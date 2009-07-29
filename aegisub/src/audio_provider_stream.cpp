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

/// @file audio_provider_stream.cpp
/// @brief Unused aggregate audio provider, intended to be used for audio scrubbing feature
/// @ingroup audio_input
///


///////////
// Headers
#include "config.h"

#include "audio_provider_stream.h"
#include "utils.h"



/// DOCME
#define BUFSIZE 65536



/// @brief Constructor 
///
StreamAudioProvider::StreamAudioProvider() {
	bufLen = 8192;
	startPos = 0;
	endPos = BUFSIZE;
	buffered = 0;
	hasBuf = false;
	num_samples = ~0ULL;
}



/// @brief Destructor 
///
StreamAudioProvider::~StreamAudioProvider() {
	for (std::list<BufferChunk*>::iterator cur=buffer.begin();cur!=buffer.end();cur++) {
		delete *cur;
	}
	buffer.clear();
}



/// @brief Get audio 
/// @param buf   
/// @param start 
/// @param count 
///
void StreamAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) {
	// Write
	int64_t left = count;
	int written = 0;
	int toWrite;
	short *dst = (short*) buf;
	while (hasBuf && left > 0) {
		// Discard done
		if (startPos == BUFSIZE) {
			delete buffer.front();
			buffer.pop_front();
			startPos = 0;
		}

		// Is last?
		bool isLast = buffer.size() == 1;
		int size = BUFSIZE;
		if (isLast) size = endPos;

		// Write
		toWrite = MIN(size-startPos,int(left));
		memcpy(dst+written,&(buffer.front()->buf[startPos]),toWrite*2);
		startPos += toWrite;
		written += toWrite;
		left -= toWrite;
		buffered -= toWrite;

		// Last
		if (isLast) break;
	}

	// Still left, fill with zero
	if (left > 0) {
		hasBuf = false;
		for (int64_t i=written;i<count;i++) {
			dst[i] = 0;
		}
	}
}



/// @brief Append audio to stream 
/// @param voidptr 
/// @param count   
///
void StreamAudioProvider::Append(void *voidptr, int64_t count) {
	// Read
	int64_t left = count;
	int read = 0;
	int toRead;
	short *src = (short*) voidptr;
	while (left > 0) {
		// Check space
		if (endPos == BUFSIZE) {
			buffer.push_back(new BufferChunk);
			endPos = 0;
		}

		// Read
		toRead = MIN(int(BUFSIZE-endPos),int(left));
		memcpy(&(buffer.back()->buf[endPos]),src+read,toRead*2);
		endPos += toRead;
		read += toRead;
		buffered += toRead;
		left -= toRead;
	}

	// Set buffered status
	if (buffered > bufLen) hasBuf = true;
}



/// @brief Set parameters 
/// @param chan 
/// @param rate 
/// @param bps  
///
void StreamAudioProvider::SetParams(int chan,int rate,int bps) {
	channels = chan;
	sample_rate = rate;
	bytes_per_sample = bps;
}



/// @brief Buffer chunk constructor 
///
StreamAudioProvider::BufferChunk::BufferChunk() {
	buf.resize(BUFSIZE);
	isFree = true;
}


