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
#endif

#ifdef WITH_AVISYNTH
#include "../audio/avs_audio.h"
#endif
#include "../audio/convert.h"
#ifdef WITH_FFMPEGSOURCE
#include "../audio/ffms_audio.h"
#endif
#include "../cache/audio_hd.h"
#include "../cache/audio_ram.h"
#include "../audio/pcm.h"


//#include "compat.h"
//#include "main.h"

namespace media {

/// @brief Constructor 
///
AudioProvider::AudioProvider() : raw(NULL) {
}

/// @brief Destructor 
///
AudioProvider::~AudioProvider() {
	delete[] raw;
}

/// @brief Get audio with volume 
/// @param buf    
/// @param start  
/// @param count  
/// @param volume 
/// @return 
///
void AudioProvider::GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) const {
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
AudioProvider *AudioProviderFactory::GetProvider(std::string filename, int cache) {
	AudioProvider *provider = NULL;
	bool found = false;
	std::string msg;

//XXX	if (!OPT_GET("Provider/Audio/PCM/Disable")->GetBool()) {
	if (1) {
		// Try a PCM provider first
		try {
			provider = CreatePCMAudioProvider(filename);
		}
		catch (agi::FileNotFoundError const& err) {
			msg = "PCM audio provider: " + err.GetMessage() + " not found.\n";
		}
		catch (AudioOpenError const& err) {
			found = true;
			msg += err.GetMessage();
		}
	}
	if (!provider) {
//XXX		std::vector<std::string> list = GetClasses(OPT_GET("Audio/Provider")->GetString());
		std::vector<std::string> list = GetClasses("ffmpegsource");

		if (list.empty()) throw AudioOpenError("No audio providers are available.");

		for (unsigned int i=0;i<list.size();i++) {
			try {
				provider = Create(list[i], filename);
				if (provider) break;
			}
			catch (agi::FileNotFoundError const& err) {
				msg += list[i] + ": " + err.GetMessage() + " not found.\n";
			}
			catch (AudioOpenError const& err) {
				found = true;
				msg += list[i] + ": " + err.GetMessage();
			}
		}
	}
	if (!provider) {
		if (found) {
			throw AudioOpenError(msg);
		}
		else {
			throw agi::FileNotFoundError(filename);
		}
	}
	bool needsCache = provider->NeedsCache();

	// Give it a converter if needed
	if (provider->GetBytesPerSample() != 2 || provider->GetSampleRate() < 32000 || provider->GetChannels() != 1)
		provider = CreateConvertAudioProvider(provider);

	// Change provider to RAM/HD cache if needed
//XXX	if (cache == -1) cache = OPT_GET("Audio/Cache/Type")->GetInt();
	if (cache == -1) cache = 1;
	if (!cache || !needsCache) {
		return provider;
	}

	// Convert to RAM
	if (cache == 1) return new RAMAudioProvider(provider);

	// Convert to HD
	if (cache == 2) return new HDAudioProvider(provider);

	throw AudioOpenError("Unknown caching method");
}

/// @brief Register all providers 
///
void AudioProviderFactory::RegisterProviders() {
#ifdef WITH_AVISYNTH
	Register<AvisynthAudioProvider>("Avisynth");
#endif
#ifdef WITH_FFMPEGSOURCE
	Register<ffms::FFmpegSourceAudioProvider>("FFmpegSource");
#endif
}

template<> AudioProviderFactory::map *FactoryBase<AudioProvider *(*)(std::string)>::classes = NULL;

} // namespace media
