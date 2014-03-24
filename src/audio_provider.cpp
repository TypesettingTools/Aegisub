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

#include "include/aegisub/audio_provider.h"

#include "audio_controller.h"
#include "dialog_progress.h"
#include "factory_manager.h"
#include "frame_main.h"
#include "main.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/util.h>

#include <boost/range/iterator_range.hpp>

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

std::unique_ptr<AudioProvider> CreateDummyAudioProvider(agi::fs::path const& filename);
std::unique_ptr<AudioProvider> CreatePCMAudioProvider(agi::fs::path const& filename);
std::unique_ptr<AudioProvider> CreateAvisynthAudioProvider(agi::fs::path const& filename);
std::unique_ptr<AudioProvider> CreateFFmpegSourceAudioProvider(agi::fs::path const& filename);

std::unique_ptr<AudioProvider> CreateConvertAudioProvider(std::unique_ptr<AudioProvider> source_provider);
std::unique_ptr<AudioProvider> CreateLockAudioProvider(std::unique_ptr<AudioProvider> source_provider);
std::unique_ptr<AudioProvider> CreateHDAudioProvider(std::unique_ptr<AudioProvider> source_provider, agi::BackgroundRunner *br);
std::unique_ptr<AudioProvider> CreateRAMAudioProvider(std::unique_ptr<AudioProvider> source_provider, agi::BackgroundRunner *br);

namespace {
	struct factory {
		const char *name;
		std::unique_ptr<AudioProvider> (*create)(agi::fs::path const&);
		bool hidden;
	};

	const factory providers[] = {
		{"Dummy", CreateDummyAudioProvider, true},
		{"PCM", CreatePCMAudioProvider, true},
#ifdef WITH_FFMS2
		{"FFmpegSource", CreateFFmpegSourceAudioProvider, false},
#endif
#ifdef WITH_AVISYNTH
		{"Avisynth", CreateAvisynthAudioProvider, false},
#endif
	};
}

std::vector<std::string> AudioProviderFactory::GetClasses() {
	return ::GetClasses(boost::make_iterator_range(std::begin(providers), std::end(providers)));
}

std::unique_ptr<AudioProvider> AudioProviderFactory::GetProvider(agi::fs::path const& filename) {
	auto preferred = OPT_GET("Audio/Provider")->GetString();
	auto sorted = GetSorted(boost::make_iterator_range(std::begin(providers), std::end(providers)), preferred);

	std::unique_ptr<AudioProvider> provider;
	bool found_file = false;
	bool found_audio = false;
	std::string msg;

	for (auto const& factory : sorted) {
		try {
			provider = factory->create(filename);
			if (!provider) continue;
			LOG_I("audio_provider") << "Using audio provider: " << factory->name;
			break;
		}
		catch (agi::fs::FileNotFound const& err) {
			LOG_D("audio_provider") << err.GetChainedMessage();
			msg += std::string(factory->name) + ": " + err.GetMessage() + " not found.\n";
		}
		catch (agi::AudioDataNotFoundError const& err) {
			LOG_D("audio_provider") << err.GetChainedMessage();
			found_file = true;
			msg += std::string(factory->name) + ": " + err.GetChainedMessage() + "\n";
		}
		catch (agi::AudioOpenError const& err) {
			LOG_D("audio_provider") << err.GetChainedMessage();
			found_audio = true;
			found_file = true;
			msg += std::string(factory->name) + ": " + err.GetChainedMessage() + "\n";
		}
	}

	if (!provider) {
		if (found_audio)
			throw agi::AudioProviderOpenError(msg, nullptr);
		if (found_file)
			throw agi::AudioDataNotFoundError(msg, nullptr);
		throw agi::fs::FileNotFound(filename);
	}

	bool needsCache = provider->NeedsCache();

	// Give it a converter if needed
	if (provider->GetBytesPerSample() != 2 || provider->GetSampleRate() < 32000 || provider->GetChannels() != 1)
		provider = CreateConvertAudioProvider(std::move(provider));

	// Change provider to RAM/HD cache if needed
	int cache = OPT_GET("Audio/Cache/Type")->GetInt();
	if (!cache || !needsCache)
		return CreateLockAudioProvider(std::move(provider));

	DialogProgress progress(wxGetApp().frame, _("Load audio"));

	// Convert to RAM
	if (cache == 1) return CreateRAMAudioProvider(std::move(provider), &progress);

	// Convert to HD
	if (cache == 2) return CreateHDAudioProvider(std::move(provider), &progress);

	throw agi::AudioCacheOpenError("Unknown caching method", nullptr);
}
