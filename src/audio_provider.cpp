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

#include "audio_provider_avs.h"
#include "audio_provider_convert.h"
#include "audio_provider_dummy.h"
#include "audio_provider_ffmpegsource.h"
#include "audio_provider_hd.h"
#include "audio_provider_lock.h"
#include "audio_provider_ram.h"

#include "audio_controller.h"
#include "dialog_progress.h"
#include "frame_main.h"
#include "main.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/util.h>

// Defined in audio_provider_pcm.cpp
std::unique_ptr<AudioProvider> CreatePCMAudioProvider(agi::fs::path const& filename);

void AudioProvider::GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) const {
	GetAudio(buf, start, count);

	if (volume == 1.0) return;
	if (bytes_per_sample != 2)
		throw agi::InternalError("GetAudioWithVolume called on unconverted audio stream", nullptr);

	short *buffer = static_cast<int16_t *>(buf);
	for (size_t i = 0; i < (size_t)count; ++i)
		buffer[i] = mid<int>(-0x8000, buffer[i] * volume + 0.5, 0x7FFF);
}

void AudioProvider::ZeroFill(void *buf, int64_t count) const {
	if (bytes_per_sample == 1)
		// 8 bit formats are usually unsigned with bias 127
		memset(buf, 127, count * channels);
	else
		// While everything else is signed
		memset(buf, 0, count * bytes_per_sample * channels);
}

void AudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
	if (start < 0) {
		ZeroFill(buf, std::min(-start, count));
		buf = static_cast<char *>(buf) + -start * bytes_per_sample * channels;
		count += start;
		start = 0;
	}

	if (start + count > num_samples) {
		int64_t zero_count = std::min(count, start + count - num_samples);
		count -= zero_count;
		ZeroFill(static_cast<char *>(buf) + count * bytes_per_sample * channels, zero_count);
	}

	if (count <= 0) return;

	try {
		FillBuffer(buf, start, count);
	}
	catch (AudioDecodeError const& e) {
		LOG_E("audio_provider") << e.GetChainedMessage();
		ZeroFill(buf, count);
		return;
	}
	catch (...) {
		// FIXME: Poor error handling though better than none, to patch issue #800.
		// Just return blank audio if real provider fails.
		LOG_E("audio_provider") << "Unknown audio decoding error";
		ZeroFill(buf, count);
		return;
	}
}

namespace {
struct provider_creator {
	bool found_file = false;
	bool found_audio = false;
	std::string msg;

	template<typename Factory>
	std::unique_ptr<AudioProvider> try_create(std::string const& name, Factory&& create) {
		try {
			std::unique_ptr<AudioProvider> provider = create();
			if (provider)
				LOG_I("audio_provider") << "Using audio provider: " << name;
			return provider;
		}
		catch (agi::fs::FileNotFound const& err) {
			LOG_D("audio_provider") << err.GetChainedMessage();
			msg += name + ": " + err.GetMessage() + " not found.\n";
		}
		catch (agi::AudioDataNotFoundError const& err) {
			LOG_D("audio_provider") << err.GetChainedMessage();
			found_file = true;
			msg += name + ": " + err.GetChainedMessage() + "\n";
		}
		catch (agi::AudioOpenError const& err) {
			LOG_D("audio_provider") << err.GetChainedMessage();
			found_audio = true;
			found_file = true;
			msg += name + ": " + err.GetChainedMessage() + "\n";
		}

		return nullptr;
	}
};
}

std::unique_ptr<AudioProvider> AudioProviderFactory::GetProvider(agi::fs::path const& filename) {
	provider_creator creator;
	std::unique_ptr<AudioProvider> provider;

	provider = creator.try_create("Dummy audio provider", [&]() {
		return agi::util::make_unique<DummyAudioProvider>(filename);
	});

	// Try a PCM provider first
	if (!provider && !OPT_GET("Provider/Audio/PCM/Disable")->GetBool())
		provider = creator.try_create("PCM audio provider", [&]() { return CreatePCMAudioProvider(filename); });

	if (!provider) {
		std::vector<std::string> list = GetClasses(OPT_GET("Audio/Provider")->GetString());
		if (list.empty()) throw agi::NoAudioProvidersError("No audio providers are available.", nullptr);

		for (auto const& name : list) {
			provider = creator.try_create(name, [&]() { return Create(name, filename); });
			if (provider) break;
		}
	}

	if (!provider) {
		if (creator.found_audio)
			throw agi::AudioProviderOpenError(creator.msg, nullptr);
		if (creator.found_file)
			throw agi::AudioDataNotFoundError(creator.msg, nullptr);
		throw agi::fs::FileNotFound(filename);
	}

	bool needsCache = provider->NeedsCache();

	// Give it a converter if needed
	if (provider->GetBytesPerSample() != 2 || provider->GetSampleRate() < 32000 || provider->GetChannels() != 1)
		provider = CreateConvertAudioProvider(std::move(provider));

	// Change provider to RAM/HD cache if needed
	int cache = OPT_GET("Audio/Cache/Type")->GetInt();
	if (!cache || !needsCache)
		return agi::util::make_unique<LockAudioProvider>(std::move(provider));

	DialogProgress progress(wxGetApp().frame, _("Load audio"));

	// Convert to RAM
	if (cache == 1) return agi::util::make_unique<RAMAudioProvider>(std::move(provider), &progress);

	// Convert to HD
	if (cache == 2) return agi::util::make_unique<HDAudioProvider>(std::move(provider), &progress);

	throw agi::AudioCacheOpenError("Unknown caching method", nullptr);
}

void AudioProviderFactory::RegisterProviders() {
#ifdef WITH_AVISYNTH
	Register<AvisynthAudioProvider>("Avisynth");
#endif
#ifdef WITH_FFMS2
	Register<FFmpegSourceAudioProvider>("FFmpegSource");
#endif
}
