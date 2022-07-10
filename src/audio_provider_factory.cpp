// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "audio_provider_factory.h"

#include "factory_manager.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/audio/provider.h>
#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>

#include <boost/range/iterator_range.hpp>

using namespace agi;

std::unique_ptr<AudioProvider> CreateAvisynthAudioProvider(fs::path const& filename,
                                                           BackgroundRunner*);
std::unique_ptr<AudioProvider> CreateFFmpegSourceAudioProvider(fs::path const& filename,
                                                               BackgroundRunner*);

namespace {
struct factory {
	const char* name;
	std::unique_ptr<AudioProvider> (*create)(fs::path const&, BackgroundRunner*);
	bool hidden;
};

const factory providers[] = {
	{ "Dummy", CreateDummyAudioProvider, true },
	{ "PCM", CreatePCMAudioProvider, true },
#ifdef WITH_FFMS2
	{ "FFmpegSource", CreateFFmpegSourceAudioProvider, false },
#endif
#ifdef WITH_AVISYNTH
	{ "Avisynth", CreateAvisynthAudioProvider, false },
#endif
};
} // namespace

std::vector<std::string> GetAudioProviderNames() {
	return ::GetClasses(boost::make_iterator_range(std::begin(providers), std::end(providers)));
}

std::unique_ptr<agi::AudioProvider>
GetAudioProvider(fs::path const& filename, Path const& path_helper, BackgroundRunner* br) {
	auto preferred = OPT_GET("Audio/Provider")->GetString();
	auto sorted = GetSorted(boost::make_iterator_range(std::begin(providers), std::end(providers)),
	                        preferred);

	std::unique_ptr<AudioProvider> provider;
	bool found_file = false;
	bool found_audio = false;
	std::string msg_all;     // error messages from all attempted providers
	std::string msg_partial; // error messages from providers that could partially load the file
	                         // (knows container, missing codec)

	for(auto const& factory : sorted) {
		try {
			provider = factory->create(filename, br);
			if(!provider) continue;
			LOG_I("audio_provider") << "Using audio provider: " << factory->name;
			break;
		} catch(fs::FileNotFound const& err) {
			LOG_D("audio_provider") << err.GetMessage();
			msg_all += std::string(factory->name) + ": " + err.GetMessage() + " not found.\n";
		} catch(AudioDataNotFound const& err) {
			LOG_D("audio_provider") << err.GetMessage();
			found_file = true;
			msg_all += std::string(factory->name) + ": " + err.GetMessage() + "\n";
		} catch(AudioProviderError const& err) {
			LOG_D("audio_provider") << err.GetMessage();
			found_audio = true;
			found_file = true;
			std::string thismsg = std::string(factory->name) + ": " + err.GetMessage() + "\n";
			msg_all += thismsg;
			msg_partial += thismsg;
		}
	}

	if(!provider) {
		if(found_audio) throw AudioProviderError(msg_partial);
		if(found_file) throw AudioDataNotFound(msg_all);
		throw fs::FileNotFound(filename);
	}

	bool needs_cache = provider->NeedsCache();

	// Give it a converter if needed
	if(provider->GetBytesPerSample() != 2 || provider->GetSampleRate() < 32000 ||
	   provider->GetChannels() != 1)
		provider = CreateConvertAudioProvider(std::move(provider));

	// Change provider to RAM/HD cache if needed
	int cache = OPT_GET("Audio/Cache/Type")->GetInt();
	if(!cache || !needs_cache) return CreateLockAudioProvider(std::move(provider));

	// Convert to RAM
	if(cache == 1) return CreateRAMAudioProvider(std::move(provider));

	// Convert to HD
	if(cache == 2) {
		auto path = OPT_GET("Audio/Cache/HD/Location")->GetString();
		if(path == "default") path = "?temp";
		auto cache_dir = path_helper.MakeAbsolute(path_helper.Decode(path), "?temp");
		return CreateHDAudioProvider(std::move(provider), cache_dir);
	}

	throw InternalError("Invalid audio caching method");
}
