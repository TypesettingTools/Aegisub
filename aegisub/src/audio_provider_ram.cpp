// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
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
#include "config.h"

#include "dialog_progress.h"
#include "audio_provider_ram.h"
#include "utils.h"
#include "frame_main.h"
#include "main.h"


///////////
// Defines
#define CacheBits ((22))
#define CacheBlockSize ((1 << CacheBits))


///////////////
// Constructor
RAMAudioProvider::RAMAudioProvider(AudioProvider *source) {
	// Init
	blockcache = NULL;
	blockcount = 0;

	// Allocate cache
	int64_t ssize = source->GetNumSamples() * source->GetBytesPerSample();
	blockcount = (ssize + CacheBlockSize - 1) >> CacheBits;
	blockcache = new char*[blockcount];
	for (int i = 0; i < blockcount; i++) {
		blockcache[i] = NULL;
	}

	// Allocate cache blocks
	try {
		for (int i = 0; i < blockcount; i++) {
			blockcache[i] = new char[MIN(CacheBlockSize,ssize-i*CacheBlockSize)];
		}
	}
	catch (...) { 
		Clear();
		throw wxString(_T("Couldn't open audio, not enough ram available."));
	}

	// Copy parameters
	bytes_per_sample = source->GetBytesPerSample();
	num_samples = source->GetNumSamples();
	channels = source->GetChannels();
	sample_rate = source->GetSampleRate();
	filename = source->GetFilename();

	// Start progress
	volatile bool canceled = false;
	DialogProgress *progress = new DialogProgress(AegisubApp::Get()->frame,_("Load audio"),&canceled,_("Reading into RAM"),0,source->GetNumSamples());
	progress->Show();
	progress->SetProgress(0,1);

	// Read cache
	int readsize = CacheBlockSize / source->GetBytesPerSample();
	for (int i=0;i<blockcount && !canceled; i++) {
		//tempclip->GetAudio((char*)blockcache[i],i*readsize, i == blockcount-1 ? (num_samples - i*readsize) : readsize,env);
		source->GetAudio((char*)blockcache[i],i*readsize, i == blockcount-1 ? (source->GetNumSamples() - i*readsize) : readsize);
		progress->SetProgress(i,blockcount-1);
	}

	// Clean up progress
	if (!canceled) progress->Destroy();
	else throw wxString(_T("Audio loading cancelled by user"));
}


//////////////
// Destructor
RAMAudioProvider::~RAMAudioProvider() {
	Clear();
}


/////////
// Clear
void RAMAudioProvider::Clear() {
	// Free ram cache
	if (blockcache) {
		for (int i = 0; i < blockcount; i++) {
			delete [] blockcache[i];
		}
		delete [] blockcache;
	}
	blockcache = NULL;
	blockcount = 0;
}


/////////////
// Get audio
void RAMAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) {
	// Requested beyond the length of audio
	if (start+count > num_samples) {
		int64_t oldcount = count;
		count = num_samples-start;
		if (count < 0) count = 0;

		// Fill beyond with zero
		if (bytes_per_sample == 1) {
			char *temp = (char *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
		if (bytes_per_sample == 2) {
			short *temp = (short *) buf;
			for (int i=count;i<oldcount;i++) {
				temp[i] = 0;
			}
		}
	}

	if (count) {
		// Prepare copy
		char *charbuf = (char *)buf;
		int i = (start*bytes_per_sample) >> CacheBits;
		int start_offset = (start*bytes_per_sample) & (CacheBlockSize-1);
		int64_t bytesremaining = count*bytes_per_sample;
		
		// Copy
		while (bytesremaining) {
			int readsize=MIN(bytesremaining,CacheBlockSize); 
			readsize = MIN(readsize,CacheBlockSize - start_offset);

			memcpy(charbuf,(char *)(blockcache[i++]+start_offset),readsize);

			charbuf+=readsize;

			start_offset=0;
			bytesremaining-=readsize;
		}
	}
}
