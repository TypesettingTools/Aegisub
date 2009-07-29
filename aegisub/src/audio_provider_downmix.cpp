// Copyright (c) 2007-2008, Niels Martin Hansen
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

/// @file audio_provider_downmix.cpp
/// @brief Intermediate audio provider downmixing the signal to mono
/// @ingroup audio_input
///


//////////////////
// Headers
#include "config.h"

#include "audio_provider_downmix.h"


//////////////////
// Constructor
DownmixingAudioProvider::DownmixingAudioProvider(AudioProvider *source) {
	filename = source->GetFilename();
	channels = 1; // target
	src_channels = source->GetChannels();
	num_samples = source->GetNumSamples();
	bytes_per_sample = source->GetBytesPerSample();
	sample_rate = source->GetSampleRate();

	// We now own this
	provider = source;

	if (!(bytes_per_sample == 1 || bytes_per_sample == 2))
		throw _T("Downmixing Audio Provider: Can only downmix 8 and 16 bit audio");
	if (!source->AreSamplesNativeEndian())
		throw _T("Downmixing Audio Provider: Source must have machine endian samples");
}

/////////////////
// Destructor
DownmixingAudioProvider::~DownmixingAudioProvider()	{
		delete provider;
}

////////////////
// Actual work happens here
void DownmixingAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) {
	if (count == 0) return;

	// We can do this ourselves
	if (start >= num_samples) {
		if (bytes_per_sample == 1)
			// 8 bit formats are usually unsigned with bias 127
			memset(buf, 127, count);
		else
			// While everything else is signed
			memset(buf, 0, count*bytes_per_sample);

		return;
	}

	// So alloc some temporary memory for this
	// Depending on use, this might be made faster by using
	// a pre-allocced block of memory...?
	char *tmp = new char[count*bytes_per_sample*src_channels];

	provider->GetAudio(tmp, start, count);

	// Now downmix
	// Just average the samples over the channels (really bad if they're out of phase!)
	// XXX: Assuming here that sample data are in machine endian, an upstream provider should ensure that
	if (bytes_per_sample == 1) {
		uint8_t *src = (uint8_t *)tmp;
		uint8_t *dst = (uint8_t *)buf;

		while (count > 0) {
			int sum = 0;
			for (int c = 0; c < src_channels; c++)
				sum += *(src++);
			*(dst++) = (uint8_t)(sum / src_channels);
			count--;
		}
	}
	else if (bytes_per_sample == 2) {
		int16_t *src = (int16_t *)tmp;
		int16_t *dst = (int16_t *)buf;

		while (count > 0) {
			int sum = 0;
			for (int c = 0; c < src_channels; c++)
				sum += *(src++);
			*(dst++) = (int16_t)(sum / src_channels);
			count--;
		}
	}

	// Done downmixing, free the work buffer
	delete[] tmp;
}

