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
		source->GetAudio(WriteCache(start, count), start, count, std::nothrow);
		decoded_samples += count;
	}
}

void CacheAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	auto out = static_cast<char *>(buf);
	while (count > 0) {
		const auto size = std::min(count, chunk_samples - start % chunk_samples);
		if (start < decoded_samples)
			ReadCache(out, start, size);
		else
			ZeroFill(out, size);
		start += size;
		count -= size;
		out += size * sample_bytes;
	}
}
}
