// Aegisub Project http://www.aegisub.org/

#include "provider_cache.h"

#include <algorithm>

namespace agi {
CacheAudioProvider::CacheAudioProvider(std::unique_ptr<AudioProvider> src)
: AudioProviderWrapper(std::move(src))
{
	decoded_samples = 0;
}

void CacheAudioProvider::StartDecoder() {
	decoder = std::thread(&CacheAudioProvider::Decode, this);
}

void CacheAudioProvider::StopDecoder() {
	cancelled = true;
	decoder.join();
}

void CacheAudioProvider::Decode() {
	for (int64_t start = 0; start < num_samples && !cancelled; start += chunk_samples) {
		const auto count = std::min(chunk_samples, num_samples - start);
		source->GetAudio(WriteCache(start, count), start, count);
		decoded_samples += count;
	}
}
}
