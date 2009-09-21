// Copyright (c) 2008, Rodrigo Braz Monteiro
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

/// @file audio_provider_convert.cpp
/// @brief Intermediate sample format-converting audio provider
/// @ingroup audio_input
///


///////////
// Headers
#include "config.h"

#include "aegisub_endian.h"
#include "audio_provider_convert.h"
#include "audio_provider_downmix.h"


/// @brief Constructor 
/// @param src 
///
ConvertAudioProvider::ConvertAudioProvider(AudioProvider *src) {
	source = src;
	channels = source->GetChannels();
	num_samples = source->GetNumSamples();
	sample_rate = source->GetSampleRate();
	bytes_per_sample = 2;

	sampleMult = 1;
	if (sample_rate < 16000) sampleMult = 4;
	else if (sample_rate < 32000) sampleMult = 2;
	sample_rate *= sampleMult;
	num_samples *= sampleMult;
}



/// @brief Destructor 
///
ConvertAudioProvider::~ConvertAudioProvider() {
	delete source;
}



/// @brief Convert to 16-bit 
/// @param src   
/// @param dst   
/// @param count 
///
void ConvertAudioProvider::Make16Bit(const char *src, short *dst, int64_t count) {
	for (int64_t i=0;i<count;i++) {
		dst[i] = (short(src[i])-128)*255;
	}
}


//////////////////////
// Change sample rate
// This requres 16-bit input
// The SampleConverter is a class overloading operator() with a function from short to short
template<class SampleConverter>

/// @brief DOCME
/// @param src       
/// @param dst       
/// @param count     
/// @param converter 
///
void ConvertAudioProvider::ChangeSampleRate(const short *src, short *dst, int64_t count, const SampleConverter &converter) {
	// Upsample by 2
	if (sampleMult == 2) {
		int64_t size = count/2;
		short cur;
		short next = 0;
		for (int64_t i=0;i<size;i++) {
			cur = next;
			next = converter(*src++);
			*(dst++) = cur;
			*(dst++) = (cur+next)/2;
		}
		if (count%2) *(dst++) = next;
	}

	// Upsample by 4
	else if (sampleMult == 4) {
		int64_t size = count/4;
		short cur;
		short next = 0;
		for (int64_t i=0;i<size;i++) {
			cur = next;
			next = converter(*src++);
			*(dst++) = cur;
			*(dst++) = (cur*3+next)/4;
			*(dst++) = (cur+next)/2;
			*(dst++) = (cur+next*3)/4;
		}
		for (int i=0;i<count%4;i++) *(dst++) = next;
	}

	// Nothing much to do, just ensure correct endedness
	else if (sampleMult == 1) {
		while (count-- > 0) {
			*dst++ = converter(*src++);
		}
	}
}



/// DOCME
struct NullSampleConverter {

	/// @brief DOCME
	/// @param val 
	/// @return 
	///
	inline short operator()(const short val) const {
		return val;
	}
};


/// DOCME
struct EndianSwapSampleConverter {

	/// @brief DOCME
	/// @param val 
	/// @return 
	///
	inline short operator()(const short val) const {
		return (short)Endian::Reverse((uint16_t)val);
	};
};



/// @brief Get audio 
/// @param destination 
/// @param start       
/// @param count       
///
void ConvertAudioProvider::GetAudio(void *destination, int64_t start, int64_t count) {
	// Bits per sample
	int srcBps = source->GetBytesPerSample();

	// Nothing to do
	if (sampleMult == 1 && srcBps == 2) {
		source->GetAudio(destination,start,count);
	}

	// Convert
	else {
		// Allocate buffers with sufficient size for the entire operation
		size_t fullSize = count;
		int64_t srcCount = count / sampleMult;
		short *buffer1 = NULL;
		short *buffer2 = NULL;
		short *last = NULL;

		// Read audio
		buffer1 = new short[fullSize * channels];
		source->GetAudio(buffer1,start/sampleMult,srcCount);

		// Convert from 8-bit to 16-bit
		if (srcBps == 1) {
			if (sampleMult == 1) {
				Make16Bit((const char*)buffer1,(short*)destination,srcCount * channels);
			}
			else {
				buffer2 = new short[fullSize * channels];
				Make16Bit((const char*)buffer1,buffer2,srcCount * channels);
				last = buffer2;
			}
		}

		// Already 16-bit
		else if (srcBps == 2) last = buffer1;

		// Convert sample rate
		if (sampleMult != 1 && source->AreSamplesNativeEndian()) {
			ChangeSampleRate(last,(short*)destination,count * channels, NullSampleConverter());
		}
		else if (!source->AreSamplesNativeEndian()) {
			ChangeSampleRate(last,(short*)destination,count * channels, EndianSwapSampleConverter());
		}

		delete [] buffer1;
		delete [] buffer2;
	}
}


/// @brief See if we need to downmix the number of channels
/// @param source_provider 
///
AudioProvider *CreateConvertAudioProvider(AudioProvider *source_provider) {
	AudioProvider *provider = source_provider;

	// Aegisub requires 16 bit samples,
	// some audio players break with low samplerates,
	// everything breaks with wrong-ended samples.
	if (provider->GetBytesPerSample() != 2 ||
		provider->GetSampleRate() < 32000 ||
		!provider->AreSamplesNativeEndian())
	{
		// @todo add support for more bitdepths (i.e. 24- and 32-bit audio)
		if (provider->GetBytesPerSample() > 2)
			throw _T("Audio format converter: audio with bitdepths greater than 16 bits/sample is currently unsupported");

		provider = new ConvertAudioProvider(provider);
	}

	// We also require mono audio for historical reasons
	if (provider->GetChannels() != 1)
	{
		provider = new DownmixingAudioProvider(provider);
	}

	return provider;
}


