// Copyright (c) 2005-2006, Rodrigo Braz Monteiro
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

/// @file audio_provider.cpp
/// @brief Baseclass for audio providers
/// @ingroup audio_input
///


#include "config.h"

#ifndef AGI_PRE
#include <wx/thread.h>
#endif

#ifdef WITH_AVISYNTH
#include "audio_provider_avs.h"
#endif
#include "audio_provider_convert.h"
#ifdef WITH_FFMPEGSOURCE
#include "audio_provider_ffmpegsource.h"
#endif
#include "audio_provider_hd.h"
#include "audio_provider_pcm.h"
#include "audio_provider_ram.h"
#include "compat.h"
#include "main.h"

/// @brief Constructor 
///
AudioProvider::AudioProvider() : raw(NULL) {
}

/// @brief Destructor 
///
AudioProvider::~AudioProvider() {
	delete[] raw;
}

/// @brief Get waveform 
/// @param min     
/// @param peak    
/// @param start   
/// @param w       
/// @param h       
/// @param samples 
/// @param scale   
///
void AudioProvider::GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale) {
	// Setup
	int channels = GetChannels();
	int n = w * samples;
	for (int i=0;i<w;i++) {
		peak[i] = 0;
		min[i] = h;
	}

	// Prepare waveform
	int cur;
	int curvalue;

	// Prepare buffers
	int needLen = n*channels*bytes_per_sample;
	if (raw) {
		if (raw_len < needLen) {
			delete[] raw;
			raw = NULL;
		}
	}
	if (!raw) {
		raw_len = needLen;
		raw = new char[raw_len];
	}

	if (bytes_per_sample == 1) {
		// Read raw samples
		unsigned char *raw_char = (unsigned char*) raw;
		GetAudio(raw,start,n);
		int amplitude = int(h*scale);

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = h - (int(raw_char[i*channels])*amplitude)/0xFF;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}

	if (bytes_per_sample == 2) {
		// Read raw samples
		short *raw_short = (short*) raw;
		GetAudio(raw,start,n);
		int half_h = h/2;
		int half_amplitude = int(half_h * scale);

		// Calculate waveform
		for (int i=0;i<n;i++) {
			cur = i/samples;
			curvalue = half_h - (int(raw_short[i*channels])*half_amplitude)/0x8000;
			if (curvalue > h) curvalue = h;
			if (curvalue < 0) curvalue = 0;
			if (curvalue < min[cur]) min[cur] = curvalue;
			if (curvalue > peak[cur]) peak[cur] = curvalue;
		}
	}
}

/// @brief Get audio with volume 
/// @param buf    
/// @param start  
/// @param count  
/// @param volume 
/// @return 
///
void AudioProvider::GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) {
	try {
		GetAudio(buf,start,count);
	}
	catch (...) {
		// FIXME: Poor error handling though better than none, to patch issue #800.
		// Just return blank audio if real provider fails.
		memset(buf, 0, count*bytes_per_sample);
		return;
	}

	if (volume == 1.0) return;

	if (bytes_per_sample == 2) {
		// Read raw samples
		short *buffer = (short*) buf;
		int value;

		// Modify
		for (int64_t i=0;i<count;i++) {
			value = (int)(buffer[i]*volume+0.5);
			if (value < -0x8000) value = -0x8000;
			if (value > 0x7FFF) value = 0x7FFF;
			buffer[i] = value;
		}
	}
}

/// @brief Get provider 
/// @param filename 
/// @param cache    
/// @return 
///
AudioProvider *AudioProviderFactory::GetProvider(wxString filename, int cache) {
	AudioProvider *provider = NULL;

	if (!OPT_GET("Provider/Audio/PCM/Disable")->GetBool()) {
		// Try a PCM provider first
		provider = CreatePCMAudioProvider(filename);
		if (provider) {
			if (provider->GetBytesPerSample() == 2 && provider->GetSampleRate() >= 32000 && provider->GetChannels() == 1)
				return provider;
			else {
				return CreateConvertAudioProvider(provider);
			}
		}
	}

	// List of providers
	std::vector<std::string> list = GetClasses(OPT_GET("Audio/Provider")->GetString());

	if (list.empty()) throw _T("No audio providers are available.");

	// Get provider
	wxString error;
	for (unsigned int i=0;i<list.size();i++) {
		try {
			provider = Create(list[i], filename);
			if (provider) break;
		}
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}

	// Failed
	if (!provider) throw error;

	// Give it a conversor if needed
	if (provider->GetBytesPerSample() != 2 || provider->GetSampleRate() < 32000 || provider->GetChannels() != 1)
		provider = CreateConvertAudioProvider(provider);

	// Change provider to RAM/HD cache if needed
	if (cache == -1) cache = OPT_GET("Audio/Cache/Type")->GetInt();
	if (cache) {
		AudioProvider *final = NULL;

		// Convert to RAM
		if (cache == 1) final = new RAMAudioProvider(provider);
		
		// Convert to HD
		if (cache == 2) final = new HDAudioProvider(provider);
		
		// Reassign
		if (final) {
			delete provider;
			return final;
		}
	}

	return provider;
}



/// @brief Register all providers 
///
void AudioProviderFactory::RegisterProviders() {
#ifdef WITH_AVISYNTH
	Register<AvisynthAudioProvider>("Avisynth");
#endif
#ifdef WITH_FFMPEGSOURCE
	Register<FFmpegSourceAudioProvider>("FFmpegSource");
#endif
}

template<> AudioProviderFactory::map *FactoryBase<AudioProvider *(*)(wxString)>::classes = NULL;
