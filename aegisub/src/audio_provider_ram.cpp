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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_provider_ram.cpp
/// @brief Caching audio provider using heap memory for backing
/// @ingroup audio_input
///

#include "config.h"

#include <libaegisub/background_runner.h>

#include "audio_provider_ram.h"

#include "audio_controller.h"
#include "compat.h"
#include "main.h"
#include "utils.h"

#define CacheBits ((22))

#define CacheBlockSize ((1 << CacheBits))

RAMAudioProvider::RAMAudioProvider(AudioProvider *src, agi::BackgroundRunner *br) {
	std::auto_ptr<AudioProvider> source(src);

	samples_native_endian = source->AreSamplesNativeEndian();

	// Allocate cache
	int64_t ssize = source->GetNumSamples() * source->GetBytesPerSample();
	blockcount = (ssize + CacheBlockSize - 1) >> CacheBits;
	blockcache = new char*[blockcount];
	memset(blockcache, 0, blockcount * sizeof(char*));

	// Allocate cache blocks
	try {
		for (int i = 0; i < blockcount; i++) {
			blockcache[i] = new char[std::min<size_t>(CacheBlockSize, ssize - i * CacheBlockSize)];
		}
	}
	catch (std::bad_alloc const&) {
		Clear();
		throw agi::AudioCacheOpenError("Couldn't open audio, not enough ram available.", 0);
	}

	// Copy parameters
	bytes_per_sample = source->GetBytesPerSample();
	num_samples = source->GetNumSamples();
	channels = source->GetChannels();
	sample_rate = source->GetSampleRate();
	filename = source->GetFilename();

	br->Run(std::tr1::bind(&RAMAudioProvider::FillCache, this, src, std::tr1::placeholders::_1));
}

RAMAudioProvider::~RAMAudioProvider() {
	Clear();
}

void RAMAudioProvider::FillCache(AudioProvider *source, agi::ProgressSink *ps) {
	ps->SetMessage(STD_STR(_("Reading into RAM")));

	int64_t readsize = CacheBlockSize / source->GetBytesPerSample();
	for (int i = 0; i < blockcount; i++) {
		source->GetAudio((char*)blockcache[i], i * readsize, std::min(readsize, num_samples - i * readsize));

		ps->SetProgress(i, (blockcount - 1));
		if (ps->IsCancelled()) {
			Clear();
			return;
		}
	}
}

void RAMAudioProvider::Clear() {
	// Free ram cache
	if (blockcache) {
		for (int i = 0; i < blockcount; i++) {
			delete [] blockcache[i];
		}
		delete [] blockcache;
	}
}

void RAMAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
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
			int readsize = std::min<int>(bytesremaining, CacheBlockSize - start_offset);

			memcpy(charbuf,(char *)(blockcache[i++]+start_offset),readsize);

			charbuf+=readsize;

			start_offset=0;
			bytesremaining-=readsize;
		}
	}
}
