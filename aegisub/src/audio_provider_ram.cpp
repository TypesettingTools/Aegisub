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

/// @file audio_provider_ram.cpp
/// @brief Caching audio provider using heap memory for backing
/// @ingroup audio_input
///

#include "config.h"

#include "audio_provider_ram.h"

#include "audio_controller.h"
#include "compat.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/background_runner.h>

#define CacheBits 22
#define CacheBlockSize (1 << CacheBits)

RAMAudioProvider::RAMAudioProvider(std::unique_ptr<AudioProvider> src, agi::BackgroundRunner *br) {
	try {
		blockcache.resize((src->GetNumSamples() * src->GetBytesPerSample() + CacheBlockSize - 1) >> CacheBits);
	}
	catch (std::bad_alloc const&) {
		throw agi::AudioCacheOpenError("Couldn't open audio, not enough ram available.", 0);
	}

	// Copy parameters
	bytes_per_sample = src->GetBytesPerSample();
	num_samples = src->GetNumSamples();
	channels = src->GetChannels();
	sample_rate = src->GetSampleRate();
	filename = src->GetFilename();
	float_samples = src->AreSamplesFloat();

	br->Run(std::bind(&RAMAudioProvider::FillCache, this, src.get(), std::placeholders::_1));
}

void RAMAudioProvider::FillCache(AudioProvider *source, agi::ProgressSink *ps) {
	ps->SetMessage(from_wx(_("Reading into RAM")));

	int64_t readsize = CacheBlockSize / source->GetBytesPerSample();
	for (size_t i = 0; i < blockcache.size(); i++) {
		ps->SetProgress(i + 1, blockcache.size());
		source->GetAudio(&blockcache[i][0], i * readsize, std::min<int64_t>(readsize, num_samples - i * readsize));

		if (ps->IsCancelled()) {
			return;
		}
	}
}

void RAMAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	char *charbuf = static_cast<char *>(buf);
	int i = (start * bytes_per_sample) >> CacheBits;
	int start_offset = (start * bytes_per_sample) & (CacheBlockSize-1);
	int64_t bytesremaining = count * bytes_per_sample;

	while (bytesremaining) {
		int readsize = std::min<int>(bytesremaining, CacheBlockSize - start_offset);

		memcpy(charbuf, &blockcache[i++][start_offset], readsize);

		charbuf += readsize;

		start_offset = 0;
		bytesremaining -= readsize;
	}
}
