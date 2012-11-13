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

/// @file audio_provider.cpp
/// @brief Baseclass for audio providers
/// @ingroup audio_input
///


#include "config.h"

#ifndef AGI_PRE
#include <wx/thread.h>
#endif

#include "audio_controller.h"
#ifdef WITH_AVISYNTH
#include "audio_provider_avs.h"
#endif
#include "audio_provider_convert.h"
#ifdef WITH_FFMS2
#include "audio_provider_ffmpegsource.h"
#endif
#include "audio_provider_hd.h"
#include "audio_provider_lock.h"
#include "audio_provider_pcm.h"
#include "audio_provider_ram.h"
#include "compat.h"
#include "dialog_progress.h"
#include "frame_main.h"
#include "main.h"

#include <libaegisub/log.h>

void AudioProvider::GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) const {
	GetAudio(buf,start,count);

	if (volume == 1.0) return;

	if (bytes_per_sample == 2) {
		// Read raw samples
		short *buffer = (short*) buf;
		int value;

		// Modify
		for (size_t i = 0; i < (size_t)count; ++i) {
			value = (int)(buffer[i]*volume+0.5);
			if (value < -0x8000) value = -0x8000;
			if (value > 0x7FFF) value = 0x7FFF;
			buffer[i] = value;
		}
	}
}

void AudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
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

	if (start + count > num_samples) {
		int64_t zero_count = std::min(count, start + count - num_samples);
		count -= zero_count;
		char *zero_buf = static_cast<char *>(buf) + count * bytes_per_sample * channels;

		if (bytes_per_sample == 1)
			// 8 bit formats are usually unsigned with bias 127
			memset(zero_buf, 127, zero_count * channels);
		else
			// While everything else is signed
			memset(zero_buf, 0, zero_count * bytes_per_sample * channels);
	}

	if (count > 0) {
		try {
			FillBuffer(buf, start, count);
		}
		catch (AudioDecodeError const& e) {
			LOG_E("audio_provider") << e.GetChainedMessage();
			memset(buf, 0, count*bytes_per_sample);
			return;
		}
		catch (...) {
			// FIXME: Poor error handling though better than none, to patch issue #800.
			// Just return blank audio if real provider fails.
			LOG_E("audio_provider") << "Unknown audio decoding error";
			memset(buf, 0, count*bytes_per_sample);
			return;
		}
	}
}

AudioProvider *AudioProviderFactory::GetProvider(wxString const& filename, int cache) {
	AudioProvider *provider = 0;
	bool found_file = false;
	bool found_audio = false;
	std::string msg;

	if (!OPT_GET("Provider/Audio/PCM/Disable")->GetBool()) {
		// Try a PCM provider first
		try {
			provider = CreatePCMAudioProvider(filename);
			LOG_D("audio_provider") << "Using PCM provider";
		}
		catch (agi::FileNotFoundError const& err) {
			msg = "PCM audio provider: " + err.GetMessage() + " not found.\n";
		}
		catch (agi::AudioOpenError const& err) {
			found_file = true;
			msg += err.GetChainedMessage() + "\n";
		}
	}

	if (!provider) {
		std::vector<std::string> list = GetClasses(OPT_GET("Audio/Provider")->GetString());
		if (list.empty()) throw agi::NoAudioProvidersError("No audio providers are available.", 0);

		for (size_t i = 0; i < list.size() ; ++i) {
			try {
				provider = Create(list[i], filename);
				if (provider) {
					LOG_D("audio_provider") << "Using audio provider: " << list[i];
					break;
				}
			}
			catch (agi::FileNotFoundError const& err) {
				msg += list[i] + ": " + err.GetMessage() + " not found.\n";
			}
			catch (agi::AudioDataNotFoundError const& err) {
				found_file = true;
				msg += list[i] + ": " + err.GetChainedMessage() + "\n";
			}
			catch (agi::AudioOpenError const& err) {
				found_audio = true;
				found_file = true;
				msg += list[i] + ": " + err.GetChainedMessage() + "\n";
			}
		}
	}

	if (!provider) {
		if (found_audio)
			throw agi::AudioProviderOpenError(msg, 0);
		if (found_file)
			throw agi::AudioDataNotFoundError(msg, 0);
		throw agi::FileNotFoundError(STD_STR(filename));
	}

	bool needsCache = provider->NeedsCache();

	// Give it a converter if needed
	if (provider->GetBytesPerSample() != 2 || provider->GetSampleRate() < 32000 || provider->GetChannels() != 1)
		provider = CreateConvertAudioProvider(provider);

	// Change provider to RAM/HD cache if needed
	if (cache == -1) cache = OPT_GET("Audio/Cache/Type")->GetInt();
	if (!cache || !needsCache) {
		return new LockAudioProvider(provider);
	}

	DialogProgress progress(wxGetApp().frame, _("Load audio"));

	// Convert to RAM
	if (cache == 1) return new RAMAudioProvider(provider, &progress);

	// Convert to HD
	if (cache == 2) return new HDAudioProvider(provider, &progress);

	throw agi::AudioCacheOpenError("Unknown caching method", 0);
}

/// @brief Register all providers
///
void AudioProviderFactory::RegisterProviders() {
#ifdef WITH_AVISYNTH
	Register<AvisynthAudioProvider>("Avisynth");
#endif
#ifdef WITH_FFMS2
	Register<FFmpegSourceAudioProvider>("FFmpegSource");
#endif
}

template<> AudioProviderFactory::map *FactoryBase<AudioProvider *(*)(wxString)>::classes = nullptr;
