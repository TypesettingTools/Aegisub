// Aegisub Project http://www.aegisub.org/

#include "provider_cache.h"

#include <libaegisub/log.h>

#include <algorithm>
#include <iterator>

namespace agi {
CacheAudioProvider::CacheAudioProvider(std::unique_ptr<AudioProvider> src)
: AudioProviderWrapper(std::move(src))
, ready((num_samples + chunk_samples - 1) / chunk_samples)
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
	for (int64_t chunk = 0; chunk < std::ssize(ready) && !cancelled; ++chunk) {
		if (!DecodeChunk(chunk)) {
			if (!cancelled)
				RetrySequentially();
			break;
		}
	}
	filling = false;
}

bool CacheAudioProvider::DecodeChunk(int64_t chunk, void *discard) try {
	const auto start = chunk * chunk_samples;
	const auto count = std::min(chunk_samples, num_samples - start);
	source->GetAudio(discard ? discard : WriteCache(start, count), start, count);
	if (!discard) {
		ready[chunk].store(true, std::memory_order_release);
		decoded_samples += count;
	}
	return true;
}
catch (Exception const& e) {
	LOG_E("audio_provider") << e.GetMessage();
	return false;
}
catch (...) {
	LOG_E("audio_provider") << "Unknown audio cache fill error";
	return false;
}

void CacheAudioProvider::RetrySequentially() try {
	// Reopen the source. Decode through ready chunks to advance the decoder
	// without rewriting samples already published to cache readers.
	source->ResetAudioDecoder();
	auto discard = std::make_unique_for_overwrite<char[]>(chunk_samples * sample_bytes);
	for (int64_t chunk = 0; !cancelled && decoded_samples < num_samples; ++chunk) {
		if (!DecodeChunk(chunk, ready[chunk].load(std::memory_order_relaxed) ? discard.get() : nullptr))
			break;
	}
}
catch (Exception const& e) {
	LOG_E("audio_provider") << e.GetMessage();
}
catch (...) {
	LOG_E("audio_provider") << "Unknown error restarting the audio cache";
}

bool CacheAudioProvider::IsAudioReady(int64_t start, int64_t end) const {
	start = std::max<int64_t>(start, 0);
	end = std::min(end, num_samples);
	if (start >= end || GetDecodedSamples() == num_samples)
		return true;

	for (auto chunk = start / chunk_samples; chunk <= (end - 1) / chunk_samples; ++chunk) {
		if (!ready[chunk].load(std::memory_order_acquire))
			return false;
	}
	return true;
}

void CacheAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	auto out = static_cast<char *>(buf);
	while (count > 0) {
		const auto chunk = start / chunk_samples;
		const auto size = std::min(count, chunk_samples - start % chunk_samples);
		if (ready[chunk].load(std::memory_order_acquire))
			ReadCache(out, start, size);
		else
			ZeroFill(out, size);
		start += size;
		count -= size;
		out += size * sample_bytes;
	}
}
}
